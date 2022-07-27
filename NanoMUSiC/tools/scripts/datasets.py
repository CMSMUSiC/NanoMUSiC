#!/usr/bin/env python

import argparse
import json
import shutil
import os
import re
import urllib

import dbutilscms

def main():
    options = commandline_parsing()

    dataset_tags = read_dataset_tags(options)

    with open( options.output , 'w' ) as out_file:
        out_file.write('isData = 1\n')
        out_file.write( 'config = %s \n'%(options.config))
        out_file.write('\n')
        das = dbutilscms.DasClient()

        for dataset_tag in dataset_tags:
            print ("Working on",dataset_tag)
            clean_dataset_tag = dataset_tag.split("/")[0]
            dcs_raw_filename = get_dcs_only_file(options, dataset_tag)
            run_min_file, run_max_file = get_json_runrange(dcs_raw_filename)
            for datastream in options.datastreams:
                runs_das = das.get_runrange(os.path.join("/" + datastream,
                                                dataset_tag ))

                run_min = max((min(runs_das), run_min_file))
                run_max = min((max(runs_das), run_max_file))
                dcs_filename = "-". join( ["DCS", str(run_min),
                                        str(run_max)] )
                dcs_filename += ".json"
                shutil.copyfile( dcs_raw_filename, os.path.join(os.getcwd(),
                                                            dcs_filename))
                string  = "_".join(['Data',
                                    clean_dataset_tag,
                                    str(run_min),
                                    str(run_max),
                                    datastream])
                string += ':/' + datastream
                string += '/' + dataset_tag + ';' + dcs_filename
                out_file.write(string + "\n")

def get_dcs_only_file(options, dataset_tag):
    year = re.search(ur"Run20(\d{2})", dataset_tag).groups()[0]
    print ("Year",year)
    out_filename = "DCSONLY_Collisions{year}".format(year=year)
    if not os.path.exists(out_filename):
        print "If an error releated to urlib appears, maybe the url is not working. Try to copy paste the json_DCSONLY.txt file from lxplus to your working directory. Please, inspect datasets.py to more information"
        #As you understand from the code you can just copy the json_DCSONLY.txt from lxplus and rename it as DCSONLY_Collisions{year}.
        #To know how please visit https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmV2017Analysis#Data subsection Data
        url = "https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/certification/Collisions{year}/13TeV/DCSOnly/json_DCSONLY.txt".format(year=year)
        #url = "https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/CMSCOMM/COMM_DQM/certification/Collisions{year}/13TeV/DCSOnly/json_DCSONLY.txt".format(year=year)#LOR NEW
        urllib.urlretrieve(url, out_filename)
    return out_filename

def read_dataset_tags(options):
    dataset_tags = []
    with open( options.dataset_tag_file, 'r' ) as input_file:
        for line in input_file:
            line = line.strip()
            if line.startswith("/"):
                dataset_tags.append( line[1:] )
            else:
                dataset_tags.append( line )
    return dataset_tags

def get_json_runrange(json_path):
     ''' Return run range from a path to a json file '''
     with open(json_path, "r") as json_file:
          run_json = json.load(json_file)
          runs = [ int(run) for run in run_json]
     return min(runs), max(runs)

def commandline_parsing():
    datastreams = [
                #'Jet',
                #'MET',
                #'METBTag',
                #'MuEG',
                # "BTagCSV",
                # "BTagMu",
                "DoubleEG",
                #"DoubleMuon",
                #"DoubleMuonLowMass",
                # "EGamma",
                # "HTMHT",
                # "Jet",
                # "JetHT",
                # "MET",
                # "MuonEG",
                #"SingleElectron",
                #"SingleMu",
                #"SingleMuon",
                #"SinglePhoton",
                # "Tau",
                ]

    description  = "This script helps to create a .txt file with information needed "
    description += "for data skimming. It assignes the given run jsons (usually DCS only json) to the "
    description += "datastreams depending on the run range saved in das."

    parser = argparse.ArgumentParser(description = description)
    hstr = "File containing one dataset tag per line. This means the datasetpath without the trigger stream tag "
    hstr += "e.g. Run2016B-PromptReco-v2/MINIAOD"
    parser.add_argument("dataset_tag_file", help=hstr)

    hstr = "Path to a json files containing lumi section."
    hstr = "The matching DCS only file is chosen for the required collison eras, if this argument is missing."
    hstr += "Use DCSonly json unless you have very good reasons!"
    parser.add_argument("--jsonfiles", nargs= '+', help=hstr)


    parser.add_argument( '-o', '--output', metavar = 'FILENAME',
        default = 'data.txt',
        help = 'Set the filename where the results shall be stored.'\
               '[default = %s{default}]' )
    parser.add_argument( '-c', '--config',
                         metavar='CONFIGNAME',
                         default = 'data_miniAOD_cfg.py',
        help = 'Set the name of the config file that shall be used for'\
               'this dataset(s). [default = %s{default}]' )
    parser.add_argument( '-d', '--datastreams',
        metavar = 'DATASTREAMS',
        nargs='+',
        default = datastreams,
        help = 'Set the datastream(s) (aka. Primary Datasets)  you want'\
               ' to skim. [default = %s]' % ' '.join( datastreams ))

    options = parser.parse_args()
    return options

if __name__=="__main__":
    main()
