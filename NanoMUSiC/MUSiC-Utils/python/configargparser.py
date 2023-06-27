import copy
import os, os.path
import argparse
import fnmatch
import json
import textwrap
import configparser as configparser
from collections import defaultdict

class ExtendableNamespace( argparse.Namespace ):
    def _extend( self, other ):
        for name in dir( other ):
            if name.startswith( "_" ):
                continue
            value = getattr( other, name )
            setattr( self, name, value )

# Config resolution order:
# 1. explicitly stated on command line
# 2. explicit section in config
# 3. pattern-matching section in config
# 4. DEFAULT section in config
# 5. default value in argparser
class SmartNamespace( object ):
    def __init__( self ):
        self._cli_explicit = ExtendableNamespace()
        self._config_explicit = defaultdict( ExtendableNamespace )
        self._config_patterns = defaultdict( ExtendableNamespace )
        self._config_default = ExtendableNamespace()
        self._cli_default = ExtendableNamespace()
        self._cache = {}

    def __call__( self, match="DEFAULT" ):
        if not match in self._cache:
            ns = ExtendableNamespace()

            ns._extend( self._cli_default )
            ns._extend( self._config_default )

            for section, value in self._config_patterns.items():
                if fnmatch.fnmatch( section, match ):
                    ns._extend( value )

            for section, value in self._config_explicit.items():
                if section == match:
                    ns._extend( value )

            ns._extend( self._cli_explicit )

            self._cache[ match ] = ns

        return self._cache[ match ]

    def __str__( self ):
        return \
            "CLI explicit:    %r \n" % self._cli_explicit + \
            "config explicit: %r \n" % dict( self._config_explicit ) + \
            "config patterns: %r \n" % dict( self._config_patterns ) + \
            "config DEFAULT:  %r \n" % self._config_default + \
            "CLI default:     %r" % self._cli_default

    # called as fallback, if the attribute could not be found on
    # the "conventional" way (_cli_explicit, _config_explicit, etc.
    # are thus not available as config names)
    def __getattr__( self, name ):
        if not name.startswith("_"):
            ns = self() # get default namespace (usually already cached)
            return getattr( ns, name )
        else:
            raise AttributeError( name )

class ArgumentParser( argparse.ArgumentParser ):
    SUPPRESSED = ( "dump", "dumpdefaults", "dumponly", "help" )

    def __init__( self, default_config=None, *args, **kwargs ):
        # We will implement our own help system later
        kwargs[ "add_help" ] = False

        if not "formatter_class" in kwargs:
            kwargs[ "formatter_class" ] = argparse.ArgumentDefaultsHelpFormatter

        super( ArgumentParser, self ).__init__( *args, **kwargs )
        self.default_config = default_config
        self._add_config_argument( self )

    def _add_config_argument( self, parser ):
        parser.add_argument( "-h", "--help", dest="help", action="store_true", help="show this help message and exit" )

        group = parser.add_argument_group( "configuration file options" )
        group.add_argument( "-c", "--load-config", dest="config", type=str, default=self.default_config, help="configuration file to read" )
        group.add_argument( "--dump-config", dest="dump", type=str, default=None, help="dump current configuration to file" )
        group.add_argument( "--dump-defaults", dest="dumpdefaults", action="store_true", help="also include default arguments in dump" )
        group.add_argument( "--dump-only", dest="dumponly", action="store_true", help="only dump the configuration and exit")

    def parse_args( self, args=None ):
        # Create a temporary parser that parses the --config option only
        tmp_parser = argparse.ArgumentParser( add_help=False )
        self._add_config_argument( tmp_parser )
        tmp_namespace, remaining_args = tmp_parser.parse_known_args( args )

        namespace = SmartNamespace()

        namespace._cli_default = self.get_default_namespace()

        if tmp_namespace.config is not None:
            self._load_config( tmp_namespace.config, namespace )

        test_args = namespace()
        required_backup = []
        for action in self._actions:
            if len( action.option_strings ) == 0:
                raise ValueError( "Positional arguments are not supported." )

            if action.required:
                required_backup.append( action )
                if hasattr( test_args, action.dest ):
                    action.required = False

        self.fix_booleans()

        namespace._cli_explicit = self.parse_explicit()

        for action in required_backup:
            action.required = True

        if tmp_namespace.dump is not None:
            self._dump_config( tmp_namespace.dump, namespace, include_defaults=tmp_namespace.dumpdefaults )
            if tmp_namespace.dumponly:
                self.exit( 0, "Configuration dumped." )

        namespace._cache.clear()
        return namespace

    def parse_explicit( self, args=None ):
        defaults_backup = {}

        # replace all defaults with token, make backup of previous default
        for action in self._actions:
            defaults_backup[ action.dest ] = action.default
            action.default = argparse.SUPPRESS

        # Do the actual parsing
        tmp_namespace = super( ArgumentParser, self ).parse_args( args )

        # read all arguments, write to explicit_namespace if the token has been replaced
        explicit_namespace = ExtendableNamespace()
        for name in dir( tmp_namespace ):
            if not name.startswith( "_" ) and not name in self.SUPPRESSED:
                value = getattr( tmp_namespace, name )
                if value != argparse.SUPPRESS:
                    setattr( explicit_namespace, name, value )

        # restore all defaults from backup
        for action in self._actions:
            if not isinstance( action, argparse.Action ):
                continue
            action.default = defaults_backup[ action.dest ]

        if hasattr( tmp_namespace, "help" ) and tmp_namespace.help:
            self.print_help()
            self.exit( 0 )

        return explicit_namespace

    def get_default_namespace( self ):
        ret = ExtendableNamespace()
        for action in self._actions:
            if action.dest in self.SUPPRESSED:
                continue
            elif action.default == argparse.SUPPRESS or action.dest == argparse.SUPPRESS:
                continue
            elif isinstance( action, argparse.Action  ):
                setattr( ret, action.dest, action.default )
            else:
                raise ValueError( "Unhandled action type %r" % action )
        return ret

    def get_action( self, name ):
        for action in self._actions:
            if action.dest == name:
                return action
        return None

    def _load_config( self, filename, namespace ):
        parser = configparser.ConfigParser()
        parser.optionxform = str # make options case-sensitive
        parser.read( filename )
        sections = parser.sections()
        sections.append( "DEFAULT" )
        for section in sections:
            for name, value in parser.items( section, raw=True ):
                action = self.get_action( name )
                if not action:
                    raise ValueError( "Unrecognized configuration option \"%s\"." % name )
                else:
                    if isinstance( action, argparse._StoreAction ):
                        value = json.loads( parser.get( section, name ) )

                        # The case of multiple args has not been implemented yet.
                        # So we hope that json can handle all these cases.
                        if action.nargs is None and action.type is not None and value is not None:
                            value = action.type( value )

                    elif isinstance( action, ( argparse._StoreFalseAction, argparse._StoreTrueAction ) ):
                        value = parser.getboolean( section, name )

                    if section == "DEFAULT":
                        setattr( namespace._config_default, name, value )
                    elif "*" in section:
                        setattr( namespace._config_patterns[ section ], name, value )
                    else:
                        setattr( namespace._config_explicit[ section ], name, value )


    def _dump_config( self, filename, namespace, include_defaults=False ):
        sections = {}
        sections["DEFAULT"] = ExtendableNamespace()
        if include_defaults:
            sections["DEFAULT"]._extend( namespace._cli_default )
        sections["DEFAULT"]._extend( namespace._config_default )
        sections["DEFAULT"]._extend( namespace._cli_explicit )

        sections.update( namespace._config_explicit )
        sections.update( namespace._config_patterns )

        with open( filename, "w" ) as file:
            for name, values in sections.iteritems():
                file.write( "[{}]\n\n".format( name ) )
                for vname in dir( values ):
                    if vname.startswith( "_" ):
                        continue
                    if vname in self.SUPPRESSED or vname == 'config':
                        continue

                    action = self.get_action( vname )
                    if action:
                        if action.help:
                            wrapped_lines = textwrap.wrap( action.help, 70 )
                            for line in wrapped_lines:
                                file.write( "# " + line + "\n" )
                        file.write( "# " + vname + " = " + json.dumps( action.default ) + "\n" )

                    vvalue = getattr( values, vname )
                    if not action or action.default != vvalue:
                        file.write( vname + " = " + json.dumps( vvalue ) + "\n" )

                    file.write( "\n" )

    def fix_booleans( self ):
        # If one provides --option-name False on the command line, Python parses it as
        # namespace.option_name = "False" -- note the quotes. This evaluates to True.
        # This function fixes this.

        def my_bool_parser( value ):
            if value.lower() in ("yes", "true", "on", "1"):
                return True
            elif value.lower() in ("no", "false", "off", "0"):
                return False
            else:
                raise ValueError( "Cannot convert %r to a boolean." % value )

        for action in self._actions:
            if action.type == bool:
                action.type = my_bool_parser


if __name__=="__main__":
    parser = ArgumentParser( description="Test parser." )
    parser.add_argument( "--option", help="Test option.", default="Test default." )
    parser.add_argument( "--filename", help="XXX" )
    parser.add_argument( "--flag", action="store_true" )
    parser.add_argument( "-v", action="count", default=0 )
    parser.add_argument( "-x", action="store_const", const=5 )
    args = parser.parse_args()
    print (args.option)
    #print args("test").option

