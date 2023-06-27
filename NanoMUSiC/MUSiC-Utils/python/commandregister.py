import argparse
import inspect

class CommandRegister:
    def __init__( self ):
        self.commands = []

    def command( self, func ):
        self.commands.append( func )
        return func

    def run( self, other ):
        parser = argparse.ArgumentParser( formatter_class=argparse.ArgumentDefaultsHelpFormatter )

        if hasattr( other, '__init__' ):
            self.add_func_args( parser, other.__init__ )

        subparsers = parser.add_subparsers()
        self.dress_subparsers( subparsers )

        args = parser.parse_args()

        func, method_args, instance_args = self.split_args( args )
        instance = other( **instance_args )

        return func( instance, *method_args )

    def dress_subparsers( self, subparsers ):
        for command in sorted( self.commands, key=lambda c: c.__name__ ):
            parser = subparsers.add_parser( command.__name__.replace( "_", "-" ),
                formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                help=command.__doc__ )
            parser.set_defaults( func=command )

            self.add_func_args( parser, command )


    def add_func_args( self, parser, func ):
        argspec = inspect.getargspec( func )

        args, varargs, keywords, defaults = argspec

        if not defaults:
            defaults = tuple()

        # skip "self"
        if len( defaults ) > 0:
            positionals = args[1:-len( defaults )]
        else:
            positionals = args[1:]

        options = dict(zip(args[-len( defaults ):], defaults))

        for positional in positionals:
            parser.add_argument( positional, help=positional )

        for option, default in options.iteritems():
            name = option.replace( "_", "-" )
            flag = "--" + name

            action = None
            nargs = None
            if default is not None:
                type_ = type( default )
            else:
                type_ = str

            if isinstance( default, ( list, tuple, set ) ):
                if len( default ) > 0:
                    type_ = type( default[0] )
                else:
                    type_ = str
                nargs = "*"
            elif isinstance( default, bool ):
                if default:
                    action = "store_false"
                    flag = "--no-" + name
                else:
                    action = "store_true"


            if action is None:
                help = option + " (type: %s)" % type_.__name__
                parser.add_argument( flag, dest=option, default=default, type=type_, help=help, nargs=nargs )
            else:
                parser.add_argument( flag, dest=option, action=action, help=" " )

        return parser


    def split_args( self, args ):
        func = args.func
        del args.func
        args = args.__dict__

        inspected_args = inspect.getargspec( func )[0][1:]
        method_args = [ args[ name ] for name in inspected_args ]

        instance_args = { name: value for name, value in args.iteritems() if not name in inspected_args }

        return func, method_args, instance_args


if __name__=="__main__":
    class EchoExample:
        cmd = CommandRegister()

        # Arguments from __init__ are automatically registered as global args
        def __init__( self, author="ME!" ):
            self.author = author

        @cmd.command
        def echo( self, text, n=5 ):
            """ Echo the given text multiple times. """
            print ("Echo is brought to you by", self.author)
            for i in range( n ):
                print (text)

    EchoExample.cmd.run(EchoExample)


