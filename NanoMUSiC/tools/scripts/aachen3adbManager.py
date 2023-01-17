#!/usr/bin/env python

## @package aix3aManager
# Get infos about samples / skims from aix3adb and save them
#
#
# @author Tobias Pook

import os,sys
import time
from datetime import datetime
import argparse
import logging
import csv
import json
import itertools

# custom TAPAS libs
import gridlib.se
import gridlib.util
import cesubmit
from pprint import pprint
#import aix3adb
#from  aix3adb import Aix3adbException

import aachen3adb
import loggingFunctions

# setup logging
log = logging.getLogger("aachen3adbManager")

class ManagerConfigurationException(Exception):
    pass

class Aachen3adbBaseManager(object):

    _user = None
    _se = None
    _all_users = None
    skim_key = "sk_"
    sample_key = "sa_"
    update_key = "up_"
    _disjunct_field_tuples = None
    options = {}

    def __init__(self, certificate=None, key=None, **kwargs):
        dbkwargs = {}
        if certificate:
            dbkwargs["certificate"] = certificate
        if key:
            dbkwargs["key"] = key

        self.dbcon = aachen3adb.ObjectDatabaseConnection(**dbkwargs)
        self.dbcon.authorize()
        log.debug("Initalized Aachen3adbBaseManager")

    @property
    def user(self):
        if not self._user:
            import gridlib.util
            self._user = gridlib.util.get_username_cern()
        return self._user

    @property
    def all_users_on_remote(self):
        if self._all_users is None:
            self._all_users = self.se.ls("/store/user/")
        return self._all_users

    def get_user_on_site(self, user):
        for u in self.all_users_on_remote:
            #~ log.debug(u + "  ||  " + user)
            if u == user:
                return u
            if u.startswith(user):
                if "_DISABLED" in u and u.split("_DISABLED")[0] == user:
                    return u
        raise ValueError("User not found in user space in %s" % self.options.site )


    @property
    def se(self):
        if self._se is None:
            self._se = gridlib.se.StorageElement(self.options.site)
        return self._se

    def read_options(self, args):
        options = vars(args)
        for field in options:
            if hasattr(self.options, field):
                msg = "Option %s is already set" % field
                raise ManagerConfigurationException(msg)
        self.options = args

    @property
    def disjunct_field_lists(self):
        if self._disjunct_field_tuples:
            return self._disjunct_field_tuples

        columns = self.dbcon.get_columns()
        disjunct_tuples = {}
        for dbtype, fielddict in columns.items():
            common_fields, mc_fields  = {}, {}
            if not dbtype in ("MCSample", "MCSkim"):
                continue
            data_columns = columns[dbtype.replace("MC","Data")]
            for field, ftype in fielddict.items():
                if not field in data_columns:
                    mc_fields[field] = ftype
                else:
                    common_fields[field] = ftype
            data_fields = {field:ftype for field, ftype in data_columns.items() \
                           if field not in common_fields}
            disjunct_tuples[dbtype.replace("MC","")] = (common_fields,
                                                            mc_fields,
                                                            data_fields)
        self._disjunct_field_tuples = disjunct_tuples
        return self._disjunct_field_tuples

    def list_fields(self):
        for dbtype,field_tuple in self.disjunct_field_lists.items():
            print "\n\n" + dbtype
            for fieldtype, fielddict in zip(("Common","MC only","Data only"),field_tuple):
                print "  " + fieldtype
                for field, ftype in fielddict.items():
                    print "    %s : %s" % (field, ftype)

    def add_cli_options(self, parser):
        # function returns a list of options which should be visible in the
        # usage line
        usage_items = []
        data_choices = [ 'mixed', 'mc', 'data' ]
        help = "Run query for data , mc or mixed default is mixed if query allows both" \
              " default: auto choices: %s" % " ".join( data_choices )
        parser.add_argument("-d",
                            "--dbtype",
                            choices=data_choices,
                            default= data_choices[0],
                            help=help)
        parser.add_argument( "-q",
                            "--quiet",
                            action="store_true",
                            help = "Do not show a summary line")


        general_group = parser.add_argument_group(title="General options")

        mutual_excl_inputs = parser.add_mutually_exclusive_group()
        help = "Use an input csv file with one query per line." \
               "The First line should contain field identifiers" \
               "as used in the aix3adbManager options e.g. sa_name." \
               "The criteria lists in the csv file can be supplemented" \
               " with cli option for specific fields." \
               "In this case the cli option overwrites the information "\
               "in the csv for every row. You may also use the input"\
               " csv files to update fields. Simply add a column with" \
               "an additional up_ befor the header identifier" \
               ", e.g. up_sa_crosssection."
        mutual_excl_inputs.add_argument("-i",
                                        "--input-csv",
                                        metavar="FILE",
                                        help = help)
        usage_items+=["-i --input-csv"]
        help = "Get infos for all samples / skims specified in ram config file"
        mutual_excl_inputs.add_argument("--ram-config",
                                        metavar="FILE",
                                        help=help)
        usage_items+=["--ram-config"]
        general_group.add_argument("--id-json", metavar="FILE",
            help= "Input file with structure {data:[ids], mc:['ids']}")
        usage_items+=["-o --output-csv"]
        general_group.add_argument("-o", "--output-csv", metavar="FILE",
            help= "Output selected sample / skims in output csv file")
        usage_items+=["-o --output-csv"]
        general_group.add_argument("--skimMin", type=int,
            help= "Minimum skim id")
        general_group.add_argument("--skimMax", type=int,
            help= "Maximum skim id")


        help="Dump all fields to output cdv not just fields specified "\
                " in skimCols/sampleCols"
        general_group.add_argument("--full-output",
                                   action="store_true",
                                   help=help)
        usage_items+=["--full-output"]

        general_group.add_argument("--list-fields", action="store_true",
            help = " list of available query fields and exit")
        usage_items+=["--list-fields"]

        help = "Site to use in file related actions " \
               "( This is also the source site for mirroring)."\
               "default: %(default)s"
        general_group.add_argument("--site",
                                   type=str,
                                   default = "T2_DE_RWTH",
                                   help =help )
        usage_items+=["--site"]

        general_group.add_argument("--latest-only", action="store_true",
            help="Consider only the latest skim if several skims are available")


        help="Display columns for both, sample or skim only. Useful e.g. if" \
              "you only want to show fields from one type e.g. datasetpath only"
        general_group.add_argument("--displayed-types",
                                   choices = ["both", "sample", 'skim'],
                                   default='both',
                                   help=help )

        help="Operator to use for query on list type fields default: %(default)s"
        general_group.add_argument("--list-operator",
                                   choices = ["any", "all", 'not in'],
                                   default='any',
                                   help=help )

        help = "Displayed columns for skims in summary output (verbose mode)." \
               "default: %(default)s"
        general_group.add_argument("--skim-cols",
                                   type=str,
                                   nargs='+',
                                   default = ["id",
                                              "version",
                                              "globaltag"],
                                   help=help)
        help="Displayed columns for samples in summary output (verbose mode)."\
             " default: %(default)s"
        general_group.add_argument("--sample-cols",
                                   type=str,
                                   nargs='+',
                                   default = ["id", "name"],
                                   help=help )
        usage_items+=["--skim-cols"]
        usage_items+=["--sample-cols"]

        general_group.add_argument('--latest',
                                   action='store_true',
                                   help='show only the latest sample')

        deprecation_group = parser.add_argument_group(title="User deprecation options")
        deprecation_group = parser.add_argument_group(title="User deprecation options")
        help="Mark all selected skims as deprecated"
        deprecation_group.add_argument("--markDeprecated",
                                       action="store_true",
                                       help=help)
        deprecation_group.add_argument("--markPrivate",
                                       action="store_true",
                                       help="Mark this sample as private" \
                                       " for current user")
        deprecation_group.add_argument("--unmarkPrivate",
            action="store_true",
            help="Remove private marker for current user ")

        deprecation_group.add_argument("--moveOwnership",
            help="Move skim and files to another user as owner")

        usage_items+=["sample_criteria"]
        usage_items+=["skim_criteria"]

        # add field query options, auto generated from db database schema
        for dbtype,key in (("Skim","sk"), ("Sample","sa")):
            for fielddict in self.disjunct_field_lists[dbtype]:
                for field in fielddict:
                    parser.add_argument("--%s_%s" % (key, field),
                                        help=argparse.SUPPRESS)
                    parser.add_argument("--up_%s_%s" % (key, field),
                                        help=argparse.SUPPRESS)

        return usage_items

    def read_csv_query_file(self):
        """ Create a list of criteria dicts for multiple queries and an update
            dict for values to change."""

        header_dict = {}
        header_update_dict = {}
        criteria_dict_list = []
        with open(self.options.input_csv) as input_csv:
            reader = csv.reader( input_csv )
            for i,row in enumerate( reader ):
                if i == 0:
                    header_dict, header_update_dict = self.parse_csv_header(row)
                    if not header_dict:
                        err_msg = "No valid field identifiers found in csv header line"
                        raise ValueError(err_msg)
                    continue
                row_dict = {}
                row_update_dict = {}
                for j, entry in enumerate(row):
                    if j in header_dict:
                        row_dict[ header_dict[j] ] = entry
                    elif j in header_update_dict:
                        row_update_dict[ header_update_dict[j] ] = entry
                criteria_dict_list.append( ( row_dict, row_update_dict ) )
        return criteria_dict_list

    def parse_csv_header( self, header_row ):
        ''' Create a dictionary which maps csv columns to query fields from header line'''
        header_dict = {}
        header_update_dict = {}
        for i,entry in enumerate(header_row):
            update_entry = False
            # Columns with identifiers starting with up_ are identified as
            # update columns
            if entry.startswith("up_"):
                update_entry = True
            if not update_entry and not hasattr(self.options, entry):
                warning_msg = "Field identifier %s in csv header does not exists."
                warning_msg +="Field will be ignored in query construction"
                log.warning( warning_msg )
                continue
            if update_entry:
                header_update_dict[i] = entry
            else:
                header_dict[i] = entry
        return header_dict, header_update_dict

    @property
    def cli_criteria_dicts(self):
        ''' Return create_criteria_dicts putput for CLI options'''
        return self.create_criteria_dicts(vars(self.options))

    @property
    def cli_update_dicts(self):
        ''' Return create_criteria_dicts putput for CLI options'''
        return self.create_criteria_dicts(vars(self.options),update=True)


    def create_criteria_dicts(self, criteria_dict, update=False):
        """ Returns a skim, sample criteria dict as used to run aix3adb queries
            from input dicts with keys corresponding to field options in
            argument parser / csv file
        """
        skim_criteria = {}
        sample_criteria = {}
        skim_key = self.skim_key
        sample_key = self.sample_key
        if update:
            skim_key = self.update_key + self.skim_key
            sample_key = self.update_key + self.sample_key
        for key in criteria_dict:
            if key.startswith( sample_key ) and criteria_dict[key]:
                sample_criteria[ key[len(sample_key):] ] = criteria_dict[key]
            elif key.startswith( skim_key ) and criteria_dict[key]:
                criteria_key = key[len(skim_key):]
                if key in self.dbcon.get_columns()["MCSkim"]:
                    keytype_dict = self.dbcon.get_columns()["MCSkim"]
                else:
                    keytype_dict = self.dbcon.get_columns()["DataSkim"]

                if keytype_dict[criteria_key] == "list":
                    criteria_key += "__" + self.options.list_operator
                skim_criteria[ criteria_key ] = criteria_dict[key]
        return skim_criteria, sample_criteria

    def _row_criteria(self, row_criteria):
        ''' Update row criteria dict with queriy value from CLI '''
        cli_skim_criteria, cli_sample_criteria = self.cli_criteria_dicts()
        skim_criteria, sample_criteria = self.create_criteria_dicts(row_criteria)
        skim_criteria.update(cli_skim_criteria)
        sample_criteria.update(cli_sample_criteria)

    def get_query_type(self, skim_criteria, sample_criteria):
        has_data = False
        has_mc = False
        has_common = False
        for key, crit_dict in (("Skim", skim_criteria),
                               ("Sample", sample_criteria)):
            for crit in crit_dict:
                if crit in self.disjunct_field_lists[key][0]:
                    has_common = True
                if crit in self.disjunct_field_lists[key][1]:
                    has_mc = True
                    continue
                if crit in self.disjunct_field_lists[key][2]:
                    has_data = True
                    continue

        # return false for empty qeries
        if not not has_data and not has_mc and not has_common:
            return False

        # Raise an error if mc and data specific fields are requested
        # at the same time
        if has_data and has_mc:
            msg = "Your requesting a field for"
            msg += "data" if self.options.dbtype == "mc" else "mc"
            msg + " but used option --dbtype" + self.options.dbtype
            raise ValueError(msg)

        if has_data or self.options.dbtype == "data":
            return "data"
        if has_mc or self.options.dbtype == "mc":
            return "mc"
        return "mixed"

    def update_objects(self, dbobject, update_dict=None,save=False):
        # split in sample / skim updates and add cli update options
        skim_update_cli, sample_update_cli = self.cli_update_dicts
        if update_dict:
            skim_update, sample_update = self.create_criteria_dicts(update_dict,
                                                                    update=True)
            skim_update.update(skim_update_cli)
            sample_update.update(sample_update_cli)
        else:
            skim_update, sample_update = skim_update_cli, sample_update_cli



        if not skim_update \
        and not sample_update \
        and not self.options.markPrivate \
        and not self.options.unmarkPrivate \
        and not self.options.markDeprecated \
        and not self.options.moveOwnership:
            return
        if "Sample" in dbobject.entry_type:
            dbobject.update(sample_update)
            for s in dbobject.skims:
                self.update_objects(s, update_dict,save)
            if not sample_update:
                save = False
        else:
            save_skim = False
            if skim_update:
                save_skim = True
            dbobject.update(skim_update)
            if self.options.markPrivate:
                dbobject.add_set_item("private", self.user)
                save_skim = True
            if self.options.unmarkPrivate:
                dbobject.remove_set_item("private", self.user)
                save_skim = True
            if self.options.moveOwnership and self.options.moveOwnership in self.all_users_on_remote:

                def replace_owner(lfn, new_owner):
                    return "/".join(["store", "user", new_owner] + lfn.split("/")[3:])

                if dbobject.files:
                    skim_folder = "/".join(list(dbobject.files)[0].path.split("/")[:-2])
                    user_disabled = None

                    if self.get_user_on_site(dbobject.owner) != dbobject.owner:
                        skim_folder = replace_owner(skim_folder, self.get_user_on_site(dbobject.owner))

                    new_folder = replace_owner(skim_folder, self.options.moveOwnership)

                    try:
                        self.se.mkdir(new_folder[:-1])
                    except:
                        pass
                    self.se.mv(skim_folder, new_folder)

                    for dbfile in dbobject.files:
                        dbfile.path = replace_owner(dbfile.path, self.options.moveOwnership)
                        dbobject.files.add(dbfile)
                dbobject.owner = self.options.moveOwnership
                save_skim = True
            if not save_skim:
                return
        if save:
            dbobject.save()

    def run_query(self, criteria_dict=None):
        """ Run a query to aachen3adb a criteria dict (e.g. creaed from csv)

            @param criteria_dict A dictionary of requirements with keys as used in cli
        """
        # Create criteria dict from flat dict and update with cli inputs
        if criteria_dict:
            skim_criteria, sample_criteria = self.create_criteria_dicts( criteria_dict )
            skim_criteria_cli, sample_criteria_cli = self.cli_criteria_dicts
            skim_criteria.update(skim_criteria_cli)
            sample_criteria.update(sample_criteria_cli)
        else:
            skim_criteria, sample_criteria = self.cli_criteria_dicts

        query_type = self.get_query_type(skim_criteria, sample_criteria)
        queries = []

        # Exit if no criteria is set
        if (not skim_criteria and not sample_criteria) \
            and not (self.options.skimMin or self.options.skimMax)\
            or not query_type:
            return queries

        def add_min_max(filter_list, min=True):
            f = {'field':'id',
                 'operator:':'>' if min else '<',
                 'value': self.options.skimMin if min else self.options.skimMin}
            filter_list.append(f)

        samples = []
        if not query_type == "mc":
            query = self.dbcon.get_criteria_query(sample_criteria,
                                                  skim_criteria,
                                                  data=True)
            if self.options.skimMin:
                add_min_max(query['filters']['DataSkim'], True)
            if self.options.skimMax:
                add_min_max(query['filters']['DataSkim'], False)
            samples += self.dbcon.query_objects(query,
                                            latest=self.options.latest)
        if not query_type == "data":
            query = self.dbcon.get_criteria_query(sample_criteria,
                                                  skim_criteria,
                                                  data=False)
            if self.options.skimMin:
                add_min_max(query['filters']['MCSkim'], True)
            if self.options.skimMax:
                add_min_max(query['filters']['MCSkim'], False)
            samples += self.dbcon.query_objects(query,
                                                latest=self.options.latest)

        return samples

    def update_max_length_dict(self, length_dict, dbobject):
        for field, value in vars(dbobject).items():
            if field == "skims":
                if not field in length_dict:
                    length_dict[field] = {}
                for skim in value:
                    self.update_max_length_dict(length_dict[field], skim)
                continue
            if not field in length_dict:
                length_dict[field] = 0
            l = len(str(value))
            if l > length_dict[field]:
                length_dict[field] = l

    def summary_lines(self,
                      dbobject,
                      length_dict,
                      getHeader=False,
                      prefix='',
                      verbose=True,
                      csvdump=False,
                   ):
        line = prefix
        keys = []
        key_prefix = ""
        cols_type = dbobject.entry_type

        non_output_fields = ["skims", "files"]

        if cols_type.startswith("MC"):
            cols_type = cols_type[2:]
        elif cols_type.startswith("Data"):
            cols_type = cols_type[4:]
        if cols_type == "Skim":
            key_prefix = "sk_"
        if cols_type == "Sample":
            key_prefix = "sa_"
        if self.options.full_output:
            try:
                cols = dbobject.fields
            except:
                cols = dbobject.keys()
        else:
            cols = getattr(self.options, cols_type.lower() + "_cols")
        cols = [c for c in cols if c not in non_output_fields]
        for field in cols:
            if field in ("skims", "files"):
                continue
            line += "{%s:<%d} " % (field, length_dict[field] + 2)
        #~ line_dict = {key : getattr(dbobject, key, "") for key in self.options.skim_cols}

        if hasattr(dbobject, "skims") and dbobject.skims \
            and not self.options.displayed_types == "sample":
            keys = self.options.sample_cols
        else:
            keys = self.options.skim_cols

        if csvdump:
            if getHeader:
                line = [key_prefix + c for c in cols]
            else:
                line= [getattr(dbobject, key, "") for key in cols]
        else:
            if getHeader:
                line_dict = {k:key_prefix + k for k in cols}
            else:
                line_dict = {key : getattr(dbobject, key, "") for key in cols}
            line = line.format(**line_dict)
        lines = [line]

        if hasattr(dbobject, "skims") and dbobject.skims \
            and not self.options.displayed_types == "sample":
            lines = []
            if self.options.displayed_types == "skim":
                if csvdump:
                    skimline = []
                else:
                    skimline = ""
            else:
                skimline = line

            for i,skim in enumerate(dbobject.skims):

                l = self.summary_lines(skim,
                                       length_dict["skims"],
                                       verbose=False,
                                       getHeader=getHeader,
                                       csvdump=csvdump)[0]
                if not csvdump:
                    l = str(l)
                skimline += l
                if not getHeader or i==0:
                    lines.append(skimline)
                    if verbose:
                        print skimline
        elif verbose:
            print line
        return lines


class Aachen3adbDeprecationManager(Aachen3adbBaseManager):

    def find_deprecation_candidates(self):
        skim_criteria, sample_criteria = self.cli_criteria_dicts
        chunksize = 50
        start = 0
        skim_criteria[ "private" ] = []
        skim_criteria[ "nevents__ne" ] = 0

        add_mc = True
        add_data = True
        all_mcs = []
        all_datas = []
        n_mcs = 0
        n_datas = 0
        log.info("Start searching for deprecation candidates")
        while True:
            log.info("Checking next %d entries" % chunksize)
            query_kwargs = {"skim_criteria" : skim_criteria,
                "sample_criteria" : sample_criteria,
                "limit":chunksize,
                "offset":start,
                }
            mcs , datas = [] , []
            if add_mc:
                mcs += self.dbcon.get_objects(**query_kwargs)
            if add_data:
                datas += self.dbcon.get_objects(data=True, **query_kwargs)
            log.info("len mc %d" % len(mcs))
            log.info("len datas %d" % len(datas))
            if not mcs:
                add_mc = False
            if not datas:
                add_data = False
            all_mcs += mcs
            all_datas += datas
            start += chunksize
            if not add_mc and not add_data:
                break
            time.sleep(0.5)

        def merge_samples(sample_list):
            merged_samples = []
            def keyfunc(obj):
                return obj.name
            used_samples = set()
            sample_list = sorted(sample_list, key= lambda x: x.name)
            for k, g in itertools.groupby(sample_list, keyfunc):
                this_sample = None
                i = 0
                for i,s in enumerate(g):
                    if i == 0:
                        this_sample = s
                        continue
                    this_sample.skims += s.skims
                merged_samples.append(this_sample)
            return merged_samples

        return merge_samples(all_mcs), merge_samples(all_datas)

    def prepare_deprecation(self):
        import textwrap
        mc_by_sample, data_by_sample = self.find_deprecation_candidates()

        info = textwrap.dedent( """\
        Dear all,
        This is a aix3adb deprecation information message.\n""" )
        info += "%d data samples and %d mc samples" % ( len( data_by_sample ),
                                                        len( mc_by_sample ) )
        info += " have been detected with non-private duplicated skims.\n"
        info += textwrap.dedent( """\
        The corresponding pxlio files will be removed in 2 weeks if no private mark was set until then.
        All listed skims will be marked as deprecated next week!""" )
        deletion_dict = {}
        for tag, sample_list in (("data",data_by_sample), ("mc",mc_by_sample)):
            if not sample_list:
                continue
            info += "\n\n Deprecated skims checked for %d %s samples\n" % ( len(sample_list), tag )
            if not tag in deletion_dict:
                deletion_dict[tag] = []

            for sample in sample_list:
                if len(sample.skims) < 2:
                    continue


                mayor_version_dict =  { k:0 for k in range(3, 99)}
                skim_list = sorted(sample.skims, key=lambda x: x.id, reverse=True)
                skiminfo = ""
                for skim in skim_list:
                    keep = False
                    if not skim.private \
                    and not skim.isdeprecated:
                        for key, occurences in mayor_version_dict.items():
                            if skim.version.startswith("CMSSW_%d" % key) and not occurences:
                                keep = True
                                if skim.finished is not None:
                                    mayor_version_dict[key] +=1
                                break
                    if keep:
                        continue
                    skiminfo += '{:<5} {:<20} {:<6} {:<6} {:<10} {:<145}\n'.format(skim.id,
                                                                               skim.version,
                                                                               len(skim.files),
                                                                               skim.isdeprecated,
                                                                               skim.private,
                                                                               skim.datasetpath)
                    deletion_dict[tag].append( int(skim.id) )
                if skiminfo:
                    info += sample.name + "\n"
                    info += '{:<5} {:<20} {:<6} {:<6} {:<10} {:<145}\n'.format("ID",
                                                                               "version",
                                                                               "nfiles",
                                                                               "isdep",
                                                                               "private",
                                                                               "datasetpath")
                    info += skiminfo

        with open( "deprecation_info.txt", "w" ) as info_file:
            info_file.write( info  )
        with open( "deletion_candidates.json", "w") as candidate_json:
            json.dump( deletion_dict, candidate_json)

    def prepare_deletion(self):
        """ Prepare a json dict of deletion candidates with file lists"""
        with open( self.options.createDeletionInput, "r") as jfile:
            deletion_candidate_dict = json.load(jfile)
        deletion_dict = {}
        for tag, idlist in deletion_candidate_dict.items():
            if not tag in deletion_dict:
                deletion_dict[tag] = {}
            for id in idlist:
                if tag == "mc":
                    skim = self.dbcon.get("MCSkim", id)
                if tag == "data":
                    skim = self.dbcon.get("DataSkim", id)
                if skim['private']:
                    continue
                deletion_dict[tag][id] = [f["path"] for f in skim["files"]]
        now = datetime.now()
        with open("deletion_list_%s.json" % now.strftime("%Y%m%d"), "w") as jfile:
            json.dump(deletion_dict, jfile)


    def processDeletionLog( self, deletion_log, drop_missing=False ):
        with open(deletion_log, "r") as d_log:
            log = json.load(d_log)
            obj_kwargs = {"init_update" : True, "dbcon": self.dbcon}
            for sk_id, ldict in log.items():
                obj_kwargs["dbid"] = sk_id
                if ldict["type"] == "mc":
                    skim = aachen3adb.MCSkim(**obj_kwargs)
                elif ldict["type"] == "data":
                    skim = aachen3adb.DataSkim(**obj_kwargs)
                if not skim.files:
                    continue
                new_files = []
                for f in skim.files:
                    if f.path in ldict["missing"] and drop_missing:
                        continue
                    if f.path in ldict["deleted"]:
                        continue
                    new_files.append(f)
                n_before = len(skim.files)
                n_after = len(new_files)
                if n_before != n_after:
                    skim.files = new_files
                    skim.save()
                    msg = "Deleted {:<5} of {:<5} files for skim {:<4}"
                    print(msg.format(abs(n_before - n_after),
                                     n_before,
                                     skim.id ) )

    def add_cli_options(self, parser):
        # function returns a list of options which should be visible in the
        # usage line
        usage_items = super(Aachen3adbDeprecationManager,self).add_cli_options(parser)
        title="Deprecation workflow options"
        description="Options used for the deprecation workflow"
        deprecation_group = parser.add_argument_group(title=title,
                                                      description=description)
        usage_items+=["deprecation_options"]
        help = "Produce information for skims which should" \
               "be deprecated and removed "
        deprecation_group.add_argument("--prepareDeprecation",
            action="store_true",
            help=help)

        deprecation_group.add_argument("--processDeletionLog",
            help="Update database based on information ")
        deprecation_group.add_argument("--createDeletionInput",
            help="Produce input file for dCache deletion")
        return usage_items


def parse_command_line(manager):
    parser = argparse.ArgumentParser()
    parser.add_argument( '--debug',
                         metavar='LEVEL',
                         default='INFO',
                         choices=[ 'ERROR', 'WARNING', 'INFO', 'DEBUG' ],
                         help='Set the debug level. default: %(default)s' )


    usage_items = manager.add_cli_options(parser)
    usage_items+= ["debug"]
    formatter = parser._get_formatter()
    text_width = formatter._width - formatter._current_indent
    usage_lines = []
    prefix = "%s " % formatter._prog
    line = prefix
    for i,item in enumerate(usage_items):
        indent = 0
        if len(line) + len(item) + 3 + len("usage: ")> text_width:
            usage_lines.append(line)
            line = " " * len("usage: " + prefix)
        line+= " [%s]" % item
    usage_lines.append( line )
    parser.usage = "\n".join(usage_lines)
    args = parser.parse_args()
    loggingFunctions.setupLogging( log, args.debug, "aachen3adb.log", quiet_crab=False )
    manager.read_options(args)

def main():
    manager = Aachen3adbDeprecationManager()
    parse_command_line(manager)

    # Handle actions related to the deprectaion workflow
    deprecation_workflow(manager)

    # List fields and exit if requested
    if manager.options.list_fields:
        manager.list_fields()
    ### Query all requested objects ###
    if manager.options.input_csv:
        criteria_dict_list = manager.read_csv_query_file()
        samples = []
        for criteria_dict, update_dict in criteria_dict_list:
            res = manager.run_query( criteria_dict )
            for sample in res:
                manager.update_objects(sample, update_dict,save=True)
            samples += res
        samples = sorted(list(set(samples)), key=lambda x : x.id)
    elif manager.options.ram_config:
        from ramlib import ramutils
        settings, samplecfg = ramutils.load_config(manager.options.ram_config)
        res = [s.sample for s in ramutils.get_samples(settings, samplecfg)]
        for sample in res:
            manager.update_objects(sample, update_dict,save=True)
        samples = sorted(res, key=lambda x : x.id)


    else: # default case only CLI options as input
        res = manager.run_query()
        for sample in res:
            manager.update_objects(sample, save=True)
        samples = res

    ### Output Section ###
    ## output to prompt
    # first get max length in query results
    length_dict = {}
    for sample in samples:
        manager.update_max_length_dict(length_dict, sample)
    lines = []
    docsv = bool(manager.options.output_csv)
    for i,sample in enumerate(samples):
        if i == 0:
            lines += manager.summary_lines(sample,
                                          length_dict,
                                          getHeader=True,
                                          verbose=not docsv,
                                          csvdump=docsv)
        lines += manager.summary_lines(sample,
                                      length_dict,
                                      getHeader=False,
                                      verbose=not docsv,
                                      csvdump=docsv)

    if docsv:
        with open(manager.options.output_csv, "w") as ocsv:
            writer = csv.writer(ocsv)
            for line in lines:
                writer.writerow(line)


def deprecation_workflow(manager):
    # check if deprecation should be prepared
    if manager.options.prepareDeprecation:
        manager.prepare_deprecation()
        sys.exit(0)

    if manager.options.createDeletionInput:
        manager.prepare_deletion()
        sys.exit(0)

    if manager.options.markDeprecated:
        if manager.options.id_json:
            with open(manager.options.id_json, "r") as jf:
                j = json.load(jf)
            for skim_id in j["data"]:
                d = aachen3adb.DataSkim(dbid=skim_id, init_update=True)
                d.deprecated = datetime.now()
                d.save()
            for skim_id in j["mc"]:
                d = aachen3adb.MCSkim(dbid=skim_id, init_update=True)
                d.deprecated = datetime.now()
                d.save()
            log.info("Marked all skims in: %s as deprecated" % manager.options.id_json )
            sys.exit(1)
        manager.options["up_sk_deprecated"] = datetime.now()

    if manager.options.processDeletionLog:
        manager.processDeletionLog(manager.options.processDeletionLog)
        sys.exit(1)



if __name__ == '__main__':
    main()
