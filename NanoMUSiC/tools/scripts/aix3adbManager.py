#!/usr/bin/env python

## @package aix3aManager
# Get infos about samples / skims from aix3adb and save them
#
#
# @author Tobias Pook

import os,sys
import subprocess
import time
from datetime import datetime
import multiprocessing
import itertools
import argparse
import getpass
import logging
import csv
import json
# custom TAPAS libs
import gridlib.se
import gridlib.util
import cesubmit

import aix3adb
from  aix3adb import Aix3adbException

# Pirated imports from other tools
from remoteAnalysis import readConfig

# setup logging
log = logging.getLogger("aix3adbManager")

def main():
    user = cesubmit.getCernUserName()
    dblink = aix3adb.createDBlink( user, readOnly= False )

    # get available db Fields
    skimTuple, sampleTuple = aix3adb.getDBFields( dblink )

    # user info only needed if fields are updated but should only be
    # determined once in loop and is therfore defined here
    user = None

    args = parseCommandlineOptions( skimTuple, sampleTuple, int(dblink.getMCMaxSkimID()) )

    # check if file operations work if needed
    if args.deleteDeprecated or args.mirror_files: file_operation_checks( user, args )

    # check if deprecation should be prepared
    if args.prepareDeprecation:
        prepare_deprecation( dblink, args )
        sys.exit(0)
    if args.createDeletionInput:
        prepare_deletion( dblink, args )
        sys.exit(0)

    # process a deletion log output
    if args.processDeletionLog:
        processDeletionLog(dblink, args.processDeletionLog, True)
        sys.exit(0)

    # get a list of (skim, sample ) tuples
    if args.input_csv:
        criteria_dict_list = read_csv_query_file( args )
        query_list = []
        for criteria_dict,update_dict in criteria_dict_list:
            # add/overwrite options from csv with cli arguments
            query_list += runQuery( dblink, args, criteria_dict, update_dict )
        #filter duplicated skims
        seen_ids = set()
        filtered_list = []
        for (skim, sample, skim_update, sample_update ) in query_list:
            if not skim.id in seen_ids:
                filtered_list.append((skim, sample, skim_update, sample_update  ))
        filtered_list = query_list
    elif args.ram_config:
        from ramlib import ramutils
        settings, samplecfg = ramutils.load_config( args.ram_config )
        ram_samples = ramutils.get_samples( settings, samplecfg )
        query_list = [(entry.skim, entry.sample, {}, {}) for entry in ram_samples ]
    else:
        query_list = runQuery( dblink, args )

    if len( query_list ) < 1:
        log.error( "No sample / skim found with requested query" )
        sys.exit(1)
    # find formating info if we want to print a summary
    if not args.quiet: produceMaxLengthDict( query_list , args)
    # init the EventCounter analysis if we want to update files
    if args.updateFiles: initEventCounter()
    #keep only latest skim if requested
    if args.latest_only:
        query_list = filter_latest_skim( query_list )

    # Avoid updating samples several times by caching them in a dict
    # before last update. Only last change e.g. in csv is considered
    updated_samples = {}
    # loop over skims, samples apply changes and insert in db
    for i,( skim, sample, skim_update, sample_update ) in enumerate( query_list ):
        skim, skim_updated = updated_oject_from_dict( skim, skim_update )
        sample, sample_updated = updated_oject_from_dict( sample, sample_update )
        if args.markDeprecated:
            skim_updated = True
            skim.isdeprecated = 1
        if args.updateFiles:
            updateFiles( sample, skim, args )
            setattr(skim, args.site.site, 1)
            skim_updated = True
        if not args.updateSkimmerVersion is None:
            skim.skimmer_version = args.updateSkimmerVersion
            skim_updated = True
        if not args.updateXs is None:
            sample.crosssection = args.updateXs
            sample_updated = True
            if not sample.crosssection_reference: sample.crosssection_reference = user
        if not args.updateXsOrder is None:
            sample.crosssection_order = args.updateXsOrder
            sample_updated = True
        if not args.updateXsRef is None:
            sample.crosssection_reference = args.updateXsRef
            sample_updated = True
        if args.updateAtSite:
            setattr(skim, args.updateAtSite,1)
            skim_updated = True
        if args.updateNotAtSite:
            setattr(skim, args.updateNotAtSite,0)
            skim_updated = True
        if args.markPrivate:
            if not user:
                user = gridlib.util.get_username_cern()
            if not hasattr(skim, "private"):
                skim.private = []
            privateUserList = skim.private.split(",")
            if not user in privateUserList:
                privateUserList.append( user )
                skim_updated = True
            skim.private = ",".join( privateUserList ).strip(",")
            log.info( "added private user %s " % skim.private )
        if args.unmarkPrivate:
            if not user:
                user = gridlib.util.get_username_cern()
            privateUserList = skim.private.split(",")
            try:
                privateUserList.remove( user )
                skim_updated = True
            except:
                pass
            skim.private = ",".join( privateUserList ).strip(",")
        if args.deleteDeprecated and int( skim.isdeprecated ) and len( skim.private ) < 1:
            if not hasattr(skim,'files'):
                skim.files = []
            nfiles_before = len(skim.files)
            # keep files where deletion failed ( returned False )
            skim.files[:] = [fileInfo for fileInfo in skim.files if not deleteSingleFile( fileInfo, skim,args )]
            if len(skim.files) == 0:
                for site_name in args.all_sites:
                    setattr(skim, site_name, 0)
            log.info("deleted %d of %d files" %(nfiles_before - len(skim.files), nfiles_before ) )
            skim_updated = True
        if args.mirror_files and ( not int( skim.isdeprecated ) or len( skim.private ) > 1 ):
            sucess_list = [ mirrorSingleFile( fileInfo, args ) for fileInfo in skim.files ]
            setattr( skim, args.dest_site.site, int( any( sucess_list ) ) )
            skim_updated = True

        if i % 50 == 0 and not args.quiet: print getSummaryLine( args, skim , sample, True)
        if not args.quiet: print getSummaryLine( args, skim , sample )

        # test if we run on mc
        try:
            test =  sample.crosssection
            mc_update = True
        except AttributeError as e:
            mc_update = False
        # insert in db if fields were updated
        if skim_updated:
            if not user:
                user = gridlib.util.get_username_cern()
            skim.updater = user
            try:
                if mc_update:
                    skim = dblink.editMCSkim( skim )
                else:
                    skim = dblink.editDataSkim( skim )
            except Aix3adbException as e:
                log.error( "Unable to insert skim to aix3adb" )
                log.error( "Aix3adbError: %s" % str( Aix3adbException ) )
                sys.exit(1)

        if sample_updated:
            updated_samples[ sample.id ] = sample

        if args.output_csv:
            writeheader = False
            if i == 0:
                writeheader = True
            write_csv_file(skim, sample, args, writeheader)

    for (sa_id, sample) in updated_samples.items():
        # test if we run on mc
        try:
            test = sample.crosssection
            mc_update = True
        except AttributeError as e:
            mc_update = False
        if not user:
            user = gridlib.util.get_username_cern()
        sample.updater = user
        try:
            if mc_update:
                print "updating sample"
                sample = dblink.editMCSample( sample )
            else:
                sample = dblink.editDataSample( sample )
        except Aix3adbException as e:
            log.error( "Unable to insert sample to aix3adb" )
            log.error( "Aix3adbError: %s" % str( Aix3adbException ) )
            sys.exit(1)

    if updateFiles:
        cleanupEventCounter()
def updated_oject_from_dict( dbobj, up_dict ):
    update_flag = False
    if up_dict:
        for key,val in up_dict.items():
            setattr( dbobj, key, val )
        update_flag = True
    return dbobj, update_flag

def file_operation_checks( user, args ):
    ''' Check if file operations can be performed if needed'''
    # check if we can use SRM if needed
    if (args.deleteDeprecated or  args.mirror_files) and not args.site.test_storage():
        log.error( "SRM seems not available for site %s at the moment" % args.site )
        sys.exit(1)

    if args.mirror_files and not args.dest_site.test_storage():
        log.error( "SRM seems not available for site %s at the moment" % args.dest_site )
        sys.exit(1)

def filter_latest_skim(query_list):
    """ filter all sample, skim pairs where a newer skim exists for this sample"""
    filtered_list = []
    sample_names = []
    for (outer_skim, outer_sample, outer_up_skim, outer_up_sample) in query_list:
        selected_skim =  None
        selected_up_skim =  None
        if outer_sample.name in sample_names:
            continue
        for (inner_skim, inner_sample, inner_up_skim, inner_up_sample) in query_list:
            if outer_sample == inner_sample:
                if inner_skim.id >= outer_skim.id:
                    selected_skim = inner_skim
                    selected_up_skim = inner_up_skim
                else:
                    selected_skim = outer_skim
                    selected_up_skim = outer_up_skim
        filtered_list.append( (selected_skim, outer_sample,selected_up_skim, outer_up_sample) )
        sample_names.append( outer_sample.name )
    return filtered_list

def produceMaxLengthDict( query_list , args):
    ''' get a dictionary with the maximum length a field uses in this query '''
    sample_length_dict = {}
    skim_length_dict = {}
    for (skim, sample, skim_update, sample_update ) in query_list:
        for key in sample.__dict__:
            update_val = ""
            if key in sample_update:
                update_val = sample_update[key]
            if key in sample_length_dict: sample_length_dict[key] = max( sample_length_dict[key],
                                                                         len( getattr( sample, key) ),
                                                                         len(key),
                                                                         len( update_val ) )
            else: sample_length_dict[key] =  max ( len(key), len( sample.__dict__[key] ) )
        for key in skim.__dict__:
            update_val = ""
            if key in skim_update:
                update_val = skim_update[key]
            if key in skim_length_dict: skim_length_dict[key] = max( skim_length_dict[key],
                                                                     len( getattr( skim, key) ),
                                                                     len( key ),
                                                                     len( update_val )  )
            else: skim_length_dict[key] =  max( len(key) ,len( skim.__dict__[key] ) )
    args.sample_length_dict = sample_length_dict
    args.skim_length_dict = skim_length_dict
    log.debug("Found maximum lengths for all items in query list")

def getSummaryLine( args, skim, sample, getHeader = False ):
    ''' Print a single summary line ( some room for improvement here ) '''
    line = ''
    for ( db_obj,col_list, length_dict ) in zip( [sample, skim] , [ args.sampleCols, args.skimCols ], [ args.sample_length_dict, args.skim_length_dict ] ):
        lineDict = { }
        for key in col_list:
            line += "{%s:<%d} " % ( key, length_dict[key]+2 )
            if key not in db_obj.__dict__:
                if getHeader: lineDict[ key ] = key
                else: lineDict[ key ] = ""
        # we need to format after each object due to same field names in skim / sample
        for key in db_obj.__dict__:
            if getHeader:
                lineDict[key] = key
            else: lineDict[key] = getattr( db_obj, key)
        line = line.format( **lineDict )
    return line

def prepare_deprecation(dblink, args):
    import textwrap
    mc_by_sample, data_by_sample = find_deprecation_candidates( dblink, args )
    info = textwrap.dedent( """\
    Dear all,
    This is a aix3adb deprecation information message.\n""" )
    info += "%d data samples and %d mc samples" % ( len( data_by_sample ), len( mc_by_sample ) )
    info += " have been detected with non-private duplicated skims.\n"
    info += textwrap.dedent( """\
    The corresponding pxlio files will be removed in 2 weeks if no private mark was set until then.
    All listed skims will be marked as deprecated next week!""" )
    deletion_dict = {}
    for tag, sample_dict in (("data",data_by_sample), ("mc",mc_by_sample)):
        if not sample_dict:
            continue
        info += "\n\n Deprecated skims found for %d %s samples\n" % ( len(sample_dict), tag )
        if not tag in deletion_dict:
            deletion_dict[tag] = []


        for sname, skim_list in sample_dict.items():
            info += sname + "\n"
            info += '{:<5} {:<20} {:<6} {:<6} {:<10} {:<145}\n'.format("ID",
                                                                       "skimmer_version",
                                                                       "nfiles",
                                                                       "isdep",
                                                                       "private",
                                                                       "datasetpath")
            for skim in skim_list:
                info += '{:<5} {:<20} {:<6} {:<6} {:<10} {:<145}\n'.format(skim.id,
                                                                           skim.skimmer_version,
                                                                           len(skim.files),
                                                                           skim.isdeprecated,
                                                                           skim.private,
                                                                           skim.datasetpath)
                deletion_dict[tag].append( int(skim.id) )

    with open( "deprecation_info.txt", "w" ) as info_file:
        info_file.write( info  )
    with open( "deletion_candidates.json", "w") as candidate_json:
        json.dump( deletion_dict, candidate_json)


def find_deprecation_candidates( dblink, args ):
    args_dict = vars( args )
    skim_criteria, sample_criteria = create_criteria_dicts( args_dict )
    skim_criteria[ "private" ] = ""
    chunksize = 100
    start = 110
    add_mc = True
    add_data = True
    all_mcs = []
    all_datas = []
    while True:
        if add_mc:
            mcs = dblink.searchMCSkimsAndSamples( skim_criteria,
                                                  sample_criteria,
                                                  start = start,
                                                  limit = chunksize )
            all_mcs += mcs
        if add_data:
            datas = dblink.searchDataSkimsAndSamples( skim_criteria,
                                                      sample_criteria,
                                                      start = start,
                                                      limit = chunksize )
            all_datas += datas
        print( 'lens', len( mcs ), len( datas ) )
        if len( mcs ) < chunksize:
            add_mc = False
        if len( datas ) < chunksize:
            add_data = False

        start += chunksize
        if not add_mc and not add_data:
            break
        time.sleep(0.5)

    def get_by_sample_dict( output_list ):
        # sort in skim lists by sample without previously cleared or private skims
        by_sample = {}
        for (skim, sample) in output_list:
            if not hasattr( skim, 'files') or (skim.isdeprecated and hasattr( skim, 'files') and not skim.files):
                continue
            if sample.name not in by_sample:
                by_sample[sample.name] = [skim]
            else:
                by_sample[sample.name] += [skim]
        for name, skim_list in by_sample.items():
            by_sample[ name ] = sorted( skim_list, key=lambda x: int( x.id ), reverse=True )
            # remove the first non deprecated sample
            for i in range(len(skim_list)):
                if not int(skim_list[i].isdeprecated):
                    by_sample[ name ].pop(i)
                    break

            if not by_sample[ name ]:
                del by_sample[ name ]
        return by_sample

    mc_by_sample = get_by_sample_dict( all_mcs )
    data_by_sample = get_by_sample_dict( all_datas )
    return mc_by_sample, data_by_sample

def processDeletionLog( dblink, deletion_log, drop_missing=False ):
    with open(deletion_log, "r") as d_log:
        log = json.load(d_log)
        for sk_id, ldict in log.items():
            if ldict["type"] == "mc":
                skim = dblink.getMCSkim( sk_id )
            elif ldict["type"] == "data":
                skim = dblink.getDataSkim( sk_id )
            if not skim.files:
                continue
            new_files = []
            for f in skim.files:
                if f["path"] in ldict["missing"] and drop_missing:
                    continue
                if f["path"] in ldict["deleted"]:
                    continue
                new_files.append(f)
            n_before = len(skim.files)
            n_after = len(new_files)
            if n_before != n_after:
                skim.files = new_files
                if ldict["type"] == "mc":
                    dblink.editMCSkim( skim )
                elif ldict["type"] == "data":
                    skim = dblink.editDataSkim( skim )
                msg = "Deleted {:<5} of {:<5} files for skim {:<4}"
                print(msg.format(abs(n_before - n_after),
                                 n_before,
                                 skim.id ) )

def prepare_deletion( dblink, args ):
    """ Prepare a json dict of deletion candidates with file lists"""
    with open( args.createDeletionInput, "r") as jfile:
        deletion_candidate_dict = json.load(jfile)
    deletion_dict = {}
    for tag, idlist in deletion_candidate_dict.items():
        if not tag in deletion_dict:
            deletion_dict[tag] = {}
        for id in idlist:
            if tag == "mc":
                skim = dblink.getMCSkim(id)
            if tag == "data":
                skim = dblink.getDataSkim(id)
            if skim.private:
                continue
            deletion_dict[tag][id] = [f["path"] for f in skim.files]
    now = datetime.now()
    with open("deletion_list_%s.json" % now.strftime("%Y%m%d"), "w") as jfile:
        json.dump(deletion_dict, jfile)


def runQuery( dblink, args, criteria_dict=None, update_dict = None ):
    """ Run a query to aix3adb using a args dict which may be either created
        from vars(args) or generically with same option names as keys (e.g. from csv)
    """
    # first check if we wan't to use a search query or simply
    # run over a skim id range

    doSearch = True
    args_dict = vars( args )
    skim_updates, sample_updates = {}, {}
    skim_criteria, sample_criteria = create_criteria_dicts( args_dict )
    if not criteria_dict is None:
        skim_criteria_row, sample_criteria_row = create_criteria_dicts( criteria_dict )
        skim_criteria_row.update(skim_criteria)
        skim_criteria = skim_criteria_row
        sample_criteria_row.update(sample_criteria)
        sample_criteria = sample_criteria_row
    if not update_dict is None:
        skim_updates, sample_updates = create_criteria_dicts( update_dict )
    if len( sample_criteria ) < 1 and len( skim_criteria ) < 1:
        doSearch = False
    output_list = []
    if doSearch:
        if not args.hasMC and not args.hasData and args.data == "auto":
            try:
                output_list += dblink.searchMCSkimsAndSamples( skim_criteria, sample_criteria, limit=100000 )
            except Aix3adbException as e:
                log.exception( "Fetching samples / skims for MC not successful" )
            try:
                output_list += dblink.searchDataSkimsAndSamples( skim_criteria, sample_criteria, limit=100000 )
            except Aix3adbException as e:
                log.exception( "Fetching samples / skims for Data not successful" )
        elif (args.hasMC and not args.data == "only") or args.data == "none" :
            try:
                output_list += dblink.searchMCSkimsAndSamples( skim_criteria, sample_criteria, limit=100000 )
            except Aix3adbException as e:
                log.exception( "Fetching samples / skims for MC not successful" )
        elif ( args.hasData and not args.data == "none") or args.data == "only":
            try:
                output_list += dblink.searchDataSkimsAndSamples( skim_criteria, sample_criteria, limit=100000 )
            except Aix3adbException as e:
                log.exception( "Fetching samples / skims for MC not successful" )
        # filter query for skim id range only if we run on only MC or data
        if ( args.hasMC and not args.hasData ) or ( not args.hasMC and args.hasData ):
            output_list = [(skim, sample) for (skim,sample) in output_list if (int(skim.id) > args.skimMin and int(skim.id) < args.skimMax)]
    else: #get skim range
        for i in range(args.skimMin, args.skimMax):
            if args.data == "none":
                try:
                    skim, sample = dblink.getMCSkimAndSampleBySkim( i )
                    output_list.append( (skim, sample ) )
                except Aix3adbException:
                    log.warning( "No skim, sample for skim id %d " % i )
            elif args.data == "only":
                try:
                    skim, sample = dblink.getDataSkimAndSampleBySkim( i )
                    output_list.append( (skim, sample ) )
                except Aix3adbException:
                    log.warning( "No skim, sample for skim id %d " % i )
            else:
                raise ValueError("--data option needs to be set if only skim id range given")
            time.sleep(0.001)

    if len( output_list ) < 1:
        log.warning( "No sample / skim found with requested query" )
        log.warning( "skim criteria:" )
        log.warning( skim_criteria )
        log.warning( "sample criteria:" )
        log.warning( sample_criteria )
    # add update infos to output list infos
    extended_output_list = []
    for ( skim, sample ) in output_list:
        extended_output_list.append( ( skim,
                                       sample,
                                       skim_updates,
                                       sample_updates ) )
    return extended_output_list

def create_criteria_dicts( args_dict ):
    """ Returns a skim, sample criteria dict as used to run aix3adb queries
        from input dicts with keys corresponding to field options in
        argument parser / csv file
    """
    skim_criteria = {}
    sample_criteria = {}
    for key in args_dict:
        if not args_dict[key] is None:
            if key.startswith( "sa_" ):
                sample_criteria[ key[3:] ] = args_dict[key]
            elif key.startswith( "sk_" ) :
                skim_criteria[ key[3:] ] = args_dict[key]
    return skim_criteria, sample_criteria

def write_csv_file(skim, sample, args, writeheader= False):
    if writeheader:
        writemode = "wb"
        headerline = []
    else:
        writemode = "a"

    with open(args.output_csv, writemode) as output_csv:
        writer = csv.writer(output_csv)

        if args.full_output:
            sample_keys = sample.__dict__.keys()
            skim_keys = skim.__dict__.keys()
            if "files" in skim_keys:
                skim_keys.remove("files")
        else:
            sample_keys = args.sampleCols
            skim_keys = args.skimCols
        line = []
        headerline = []
        looptuple = ( (sample,"sa_", sample_keys), (skim, "sk_", skim_keys) )
        for (db_obj,prefix,keydict) in looptuple:
            for key in keydict:
                if writeheader:
                    headerline.append( prefix + key )
                line.append( getattr(db_obj, key ) )
        if writeheader:
            writer.writerow( headerline )
        writer.writerow( line )

def read_csv_query_file( args ):
    """ Create a list of criteria dicts for multiple queries and an update
        dict for values to change."""

    header_dict = {}
    header_update_dict = {}
    criteria_dict_list = []
    with open(args.input_csv) as input_csv:
        reader = csv.reader( input_csv )
        for i,row in enumerate( reader ):
            if i == 0:
                header_dict, header_update_dict = parse_csv_header( row, args )
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

def parse_csv_header( row, args ):
    ''' Create a dictionary which maps csv columns to query fields from header line'''
    header_dict = {}
    header_update_dict = {}
    for i,entry in enumerate(row):
        update_entry = False
        # Columns with identifiers starting with up_ are identified as
        # update columns
        if entry.startswith("up_"):
            entry = entry.split("up_")[1]
            update_entry = True
        if not hasattr(args, entry):
            warning_msg = "Field identifier %s in csv headerdoes not exists."
            warning_msg +="Field will be ignored in query construction"
            log.warning( warning_msg )
            continue
        if update_entry:
            header_update_dict[i] = entry
        else:
            header_dict[i] = entry
    return header_dict, header_update_dict

def list_fields(skimTuple, sampleTuple):
    '''Print a list of available database fields for queries'''
    skim_common_fields, skim_mc_fields, skim_data_fields = skimTuple
    sample_common_fields, sample_mc_fields, sample_data_fields = sampleTuple
    print "Common sample fields (MC+Data)"
    for field in sample_common_fields:
        print "  --sa_%s" % field
    print "MC only sample fields "
    for field in sample_mc_fields:
        print "  --sa_%s" % field
    print "Data only sample fields "
    for field in sample_data_fields:
        print "  --sa_%s" % field

    print "Common skim fields (MC+Data)"
    for field in skim_common_fields:
        print "  --sk_%s" % field
    print "MC only skim fields "
    for field in skim_mc_fields:
        print "  --sk_%s" % field
    print "Data only skim fields "
    for field in skim_data_fields:
        print "  --sk_%s" % field
    sys.exit(1)

def parseCommandlineOptions( skimTuple, sampleTuple, default_max_skimid = 0 ):
    skim_common_fields, skim_mc_fields, skim_data_fields = skimTuple
    sample_common_fields, sample_mc_fields, sample_data_fields = sampleTuple
    #~ print sample_common_fields
    descr = 'Simple tool to bulk update samples & skims in aix3adb.'
    descr += ' Use % for wildards in strings.'
    descr += ' Use alternative operators ("<", "<=" etc.) at the beginning of your search value (no space) for numerical fields'
    # list to fill with items for usage line. Custom way otherwise it gets too long
    usage_items =[]
    parser = argparse.ArgumentParser( description= descr)
    # list of groups which should be added to usage
    data_choices = [ 'auto', 'only', 'none' ]
    parser.add_argument("-d", "--data", choices= data_choices, default= data_choices[0],
        help="Run query for data (only) or mc (none) or autodetection. default: auto choices: %s" % " ".join( data_choices ),  )
    parser.add_argument( '--debug', metavar='LEVEL', default='INFO',
        choices=[ 'ERROR', 'WARNING', 'INFO', 'DEBUG' ], help='Set the debug level. default: %(default)s' )
    general_group = parser.add_argument_group(title="General options")
    parser.add_argument( "-q", "--quiet" , action="store_true",
        help = "Do not show a summary line")
    mutual_excl_inputs = parser.add_mutually_exclusive_group()
    helpstring = "Use an input csv file with one query per line."
    helpstring += "The First line should contain field identifiers as used in the aix3adbManager options"
    helpstring += "e.g. sa_name."
    helpstring += "The criteria lists in the csv file can be supplemented with the cli options."
    helpstring += "In this case the cli option overwrites the information in the csv for every row."
    helpstring += "You may also use the input csv files to update fields. "
    helpstring += "Simply add a column with an additional up_ befor the header identifier, e.g. up_sa_crosssection."
    mutual_excl_inputs.add_argument("-i", "--input-csv", metavar="FILE", help = helpstring)
    usage_items+=["-i --input-csv"]
    mutual_excl_inputs.add_argument("--ram-config", metavar="FILE",
        help = "Get infos for all samples / skims specified in ram config file")
    general_group.add_argument("-o", "--output-csv", metavar="FILE",
        help= "Output selected sample / skims in output csv file")
    usage_items+=["-o --output-csv"]
    general_group.add_argument("--full-output", action="store_true",
        help = "Dump all fields to output cdv not just fields specified in skimCols/sampleCols")
    usage_items+=["--full-output"]
    general_group.add_argument("--list-fields", action="store_true",
        help = " list of available query fields and exit")
    usage_items+=["--list-fields"]

    #general_group.add_argument("--csv", help="Dump skim / sample info to csv (Extends existing files)")
    general_group.add_argument("--site", type=str, default = "T2_DE_RWTH",
        help ="Site to use in file related actions ( This is also the source site for mirroring). default: %(default)s" )
    usage_items+=["--site"]
    general_group.add_argument("--latest-only", action="store_true",
        help="Consider only the latest skim if several skims are available")
    general_group.add_argument("--skimMin", type=int,default=0,
        help="lowest skimid to check. Not used if queries works on MC and data. default: %(default)s")
    general_group.add_argument("--skimMax", type=int, default=default_max_skimid,
        help="highest skimid to check. Not used if queries works on MC and data. default: %(default)s",)
    usage_items+=["--skimMin"]
    usage_items+=["--skimMax"]
    general_group.add_argument("--skimCols", type=str, nargs='+',
        default = ["id", "skimmer_version", "skimmer_globaltag"],
        help="Displayed columns for skims in summary output (verbose mode). default: %(default)s")
    general_group.add_argument("--sampleCols", type=str, nargs='+', default = ["id", "name"],
        help="Displayed columns for samples in summary output (verbose mode). default: %(default)s" )
    usage_items+=["--skimCols"]
    usage_items+=["--sampleCols"]

    update_group = parser.add_argument_group( title="Update options",
        description= "Options used to update fields in skims / samples")
    usage_items+=["update_options"]
    update_group.add_argument("--updateFiles", action="store_true", help="Update file lists")
    update_group.add_argument("--forceUpdateFiles", action="store_true",
        help="Same as updateFiles, but includes skims with existing file entries")
    update_group.add_argument("--updateSkimmerVersion", help="Mark all selected samples as ")
    update_group.add_argument("--updateXs", help="Set cross_section order ")
    update_group.add_argument("--updateXsRef",
        help="Set cross_section reference ( required if updateXs is used) ")
    update_group.add_argument("--updateXsOrder", help="Set cross_section order ")
    update_group.add_argument("--updateAtSite", choices = ["T2_DE_RWTH", "T2_DE_DESY"],
        help="Mark that skims are available at chosen site", default=None)
    update_group.add_argument("--updateNotAtSite", choices = ["T2_DE_RWTH", "T2_DE_DESY"],
    help="Mark that skims are available at chosen site", default=None)

    deprecation_group = parser.add_argument_group( title="Deprecation workflow options",
        description= "Options used for the deprecation workflow")
    usage_items+=["deprecation_options"]
    deprecation_group.add_argument("--markDeprecated", action="store_true",
        help="Mark all selected skims as deprecated")
    deprecation_group.add_argument("--markPrivate", action="store_true",
        help="Mark this sample as private for current user")
    deprecation_group.add_argument("--unmarkPrivate", action="store_true",
        help="Remove private marker for current user ")
    deprecation_group.add_argument("--deleteDeprecated", action="store_true",
        help="Delete files for depreacted samples without private mark for skims in query ")
    deprecation_group.add_argument("--prepareDeprecation", action="store_true",
        help="Produce information for skims which should be deprecated and removed ")
    deprecation_group.add_argument("--processDeletionLog",
        help="Update database based on information ")
    deprecation_group.add_argument("--createDeletionInput",
        help="Produce input file for dCache deletion")
    deprecation_group.add_argument("--all-sites", default =["T2_DE_RWTH", "T2_DE_DESY"], nargs='+',
        help="All sites considered when deleting files. Files are only removed from skim if they are deleted in all sites")


    mirror_group = parser.add_argument_group( title="File mirror workflow options",
        description= "Options used for the mirroring of files between sites" )
    mirror_group.add_argument("--mirror-files", action="store_true",
        help= "Switch if files for selected skims should be mirrored to destination site" )
    mirror_group.add_argument("--dest-site", default = "T2_DE_DESY",
        help= "The destination site for file mirroring. default: %(default)s" )
    usage_items+=["mirror_options"]
    usage_items+=["sample_criteria"]
    usage_items+=["skim_criteria"]

    for field in sample_common_fields:
        parser.add_argument("--sa_%s" % field,help=argparse.SUPPRESS)

    for field in sample_mc_fields:
        parser.add_argument("--sa_%s" % field,help=argparse.SUPPRESS)

    for field in sample_data_fields:
        parser.add_argument("--sa_%s" % field,help=argparse.SUPPRESS)

    for field in skim_common_fields:
        parser.add_argument("--sk_%s" % field,help=argparse.SUPPRESS)

    for field in skim_mc_fields:
        parser.add_argument("--sk_%s" % field,help=argparse.SUPPRESS)

    for field in skim_data_fields:
        parser.add_argument("--sk_%s" % field,help=argparse.SUPPRESS)

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

    setupLogging(args)
    if args.deleteDeprecated:
        gridlib.util.voms_proxy_ensure()
    if args.list_fields:
        list_fields(skimTuple, sampleTuple)
    args_dict = vars( args )
    args.hasData = False
    args.hasMC = False
    for key in args_dict:
        if args_dict[key] is None: continue
        pattern = key
        pattern = pattern.replace( "sa_", "").replace( "sk_", "")
        if pattern in sample_mc_fields: args.hasMC = True
        if pattern in skim_mc_fields: args.hasMC = True
        if pattern in sample_data_fields: args.hasData = True
        if pattern in skim_data_fields: args.hasData = True
    if not args.hasData and not args.hasMC and not args.data == 'none':
        log.warning( "Unable to determine which query should be run. Data & MC possible." )
        log.warning( "Getting Samples skims for both!" )
    elif args.hasData and args.hasMC:
        log.error( "your query options use exclusive data and mc fields mixed !" )
        log.error( "Used options:" )
        print args
        sys.exit(1)

    # we replace the site options with site element options from gridlib
    if args.site:
        args.site = gridlib.se.StorageElement(args.site)
    if args.dest_site:
        args.dest_site = gridlib.se.StorageElement(args.dest_site)

    if args.forceUpdateFiles: args.updateFiles = True
    return args

def setupLogging(args):
    #setup logging
    format = '%(levelname)s: %(message)s'
    logging.basicConfig( level=logging._levelNames[ args.debug ], format=format)
    log.setLevel(logging._levelNames[ args.debug ])
    formatter = logging.Formatter( format )


def initEventCounter():

    # check if proxy exists
    gridlib.util.voms_proxy_ensure()

    if os.environ.get('PXLANA') == None:
        print "PXLANA not set, please source TAPAS!"
        sys.exit(1)

    runDir = os.getcwd()
    pxlana_dir = os.environ.get('PXLANA')
    os.chdir( pxlana_dir )
    # make sure we have the event counter analysis available
    if not os.path.exists( os.path.join( pxlana_dir, "EventCounter") ):
        command = "git clone ssh://git@gitlab.cern.ch:7999/aachen-3a/dummy-ana.git EventCounter"
        sh_command( command )
    # go to dummy ana and check out event_counter branch
    os.chdir( os.path.join( pxlana_dir, "EventCounter") )
    command = "git rev-parse --abbrev-ref HEAD"
    string_out,string_err = sh_command( command )
    if not "event_counter" in string_out:
        command = "git fetch origin"
        sh_command( command )
        command = "git checkout -b event_counter origin/event_counter"
        sh_command( command )

    myenv = os.environ.copy()
    myenv['MYPXLANA'] = 'EventCounter'
    # go back to main analysis folder
    os.chdir( pxlana_dir )
    p = subprocess.Popen( "make -j4", shell=True, env = myenv,
                    stdout=subprocess.PIPE,stderr=subprocess.PIPE )
    p.communicate()
    log.info( "EventCounter set up as PxlAna" )
    os.chdir( runDir )

def cleanupEventCounter():
    runDir = os.getcwd()
    pxlana_dir = os.environ.get('PXLANA')
    os.chdir( pxlana_dir )
    p = subprocess.Popen( "make -j4", shell=True,
                    stdout=subprocess.PIPE,stderr=subprocess.PIPE )
    p.communicate()
    os.chdir( runDir )

def sh_command( command, required = True ):
    p = subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE,shell=True)
    (string_out,string_err) = p.communicate()
    if p.returncode != 0:
        log.error("Unable to perform %s" % command)
        if required:
            sys.exit( 1 )
    return string_out,string_err

def updateFiles( sample, skim, args ):

    outlfn = os.path.join( "/store",
                           "user",
                           skim.owner,
                           "PxlSkim",
                           skim.skimmer_version ,
                           sample.name )

    doUpdate = True
    if not hasattr(skim, 'files'):
        doUpdate = True
    elif len(skim.files) > 0 and not args.forceUpdateFiles:
        doUpdate = False
        if not skim.files[0]['path']:
            doUpdate = True
        elif any( [ n['nevents'] == -1 for n in skim.files ] ):
            doUpdate = True

    if doUpdate:
        ce_paths = []
        lfn_paths = [path for path in args.site.find( "pxlio", outlfn ) if not "/failed" in path ]
        for path in lfn_paths:
            path = args.site.get_site_path(path)
            path = path.replace("dcap://grid-dcap","dcap://grid-dcap-extern")
            ce_paths.append( path )
        pathlist = zip( [ os.path.join("/store","user", path) for path in lfn_paths], ce_paths)
        filesList = submitMulti( pathlist )
        skim.files = filesList
        return True
    return False

def submitMulti( joblist ):

    pool = multiprocessing.Pool(multiprocessing.cpu_count() - 1)
    results = pool.map_async(run_process, joblist)
    remainingOut = 0
    while True:
        time.sleep(120)
        remaining = results._number_left
        if remainingOut!=remaining:
            print "Waiting for", remaining, "tasks to complete..."
            remainingOut=remaining
        if not pool._cache: break
    results = results.get()
    pool.close()
    return results

def run_process(args):
    path, ce_path = args
    runCommand = "music"
    runPath = ""
    cmd = r"%s "%runCommand
    cmd += "--CONFIG $PXLANA/$MYPXLANA/config.cfg "
    cmd += "--PXLIO_FILE %s " %ce_path
    myenv = os.environ.copy()
    myenv['MYPXLANA'] = 'EventCounter'
    #~ print cmd
    p = subprocess.Popen( cmd,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,
                          env=myenv,
                          shell=True)
    (string_out,string_err) = p.communicate()
    nev = parseJobOutput( string_out)
    return {'path':path,'nevents':nev}

def parseJobOutput( stdout ):
    nEvents = -1
    for line in stdout.split("\n"):
        if "elapsed CPU time:" in line:
            try:
                nEvents = int(line.split("Analyzed")[1].split("Events")[0].strip())
            except ValueError as e:
                print "Can not parse number of events to int"
            except:
                print "Error parsing number of events"
            break
    if nEvents==-1:print stdout[:-15]
    return nEvents

def deleteSingleFile( fileInfo, skim, args ):
    '''Function for deletion of a single file on all storage elements in deprecation workflow'''
    for site_name in args.all_sites:
        # check if the sample exists for site and delete it if True
        if not int(getattr(skim, site_name)):
            log.debug( "skim.%s %d" % (site_name,int(getattr(skim, site_name) ) ) )
            continue
        se = gridlib.se.StorageElement( site_name )
        path = fileInfo['path']

        if not path.startswith('/store/'):
            path = os.path.join('/store/', path.split('/store/')[-1] )
        if not path.endswith('.pxlio'):
            return False
        try:
            log.debug("deleting file " + path )
            se.rm( path )
        except IOError as e:
            log.error(e)
            return False
    return True

def mirrorSingleFile( fileInfo, args ):

    source_path = args.site.get_srm_path( path = fileInfo['path'] )
    dest_path = args.dest_site.get_srm_path( path = fileInfo['path'] )
    log.debug("Mirroring files for skim to " + args.site.site)
    # check if file already exists at destination
    try:
        ls_out = args.dest_site.ls(path)
        if len(ls_out) > 1:
            return True
    except:
        pass

    # copy not existing file
    tries = 0
    while( tries < 3):
        try:
            args.site.cp( source_path, dest_path )
            break
        except:
            tries+=1
    # perform final check if copy was successful
    try:
        ls_out = args.dest_site.ls(path)
        if len(ls_out) > 1:
            return True
    except:
        return False

if __name__ == '__main__':
    main()
