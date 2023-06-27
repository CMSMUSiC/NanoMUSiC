#!/usr/bin/env python
import os,sys
import subprocess
import argparse
import pandas

import dbutilscms
import aachen3adb

def load_eventlist(basepath, ec_name, distribution):
    filename = ec_name + "_" + distribution + ".txt"
    return pandas.read_table(os.path.join(basepath, filename), sep= "\s+")

class DasFileFinder(dbutilscms.DasClient):

    def data_sample2raw_datasetpath(self, sample):
        datasetpath = []
        if "SingleElectron" in sample:
            datasetpath.append("/SingleElectron")
        if "DoubleEG" in sample:
            datasetpath.append("/DoubleEG")
        if "SingleMuon" in sample:
            datasetpath.append("/SingleMuon")
        if "DoubleMuon" in sample:
            datasetpath.append("/DoubleMuon")
        run_eras = ["B", "C" ,"D", "E", "F", "G", "H"]
        print
        for run_era in run_eras:
            if "Run2016" + run_era in sample:
                chunk = "Run2016" + run_era
                vs = ["1", "2"]
                if run_era in ["B"]:
                    for v in vs:
                        if "ver" + v in sample:
                            chunk +="-v"
                            chunk += v
                elif run_era in ["C", "D", "E"]:
                    chunk += "-v2"
                elif run_era in ["F", "G", "H"]:
                    chunk += "-v1"
                datasetpath.append(chunk)
                break
        datasetpath.append("RAW")
        return os.path.join(*datasetpath)

    def get_file(self, sample, run, lumi, get_raw=False, get_aod=False):
        db = aachen3adb.ObjectDatabaseConnection()
        print "sample" , sample
        if get_raw:
            datasetpath = self.data_sample2raw_datasetpath(sample)
        else:
            sample = db.get_objects(sample_criteria={'name' : sample }, data=True)
            datasetpath = sample[0].skims[0].datasetpath
            if get_aod:
                datasetpath = datasetpath.replace("MINIAOD","AOD")

        query = "file dataset=%s run=%s lumi=%s" % (datasetpath,
                                                    run,
                                                    lumi)
        jsondict = self.query(query)

        try:
            return jsondict[0]['file'][0]['name']
        except KeyError:
            import pprint
            print "ERROR: Unable to parse das output!"
            pprint.pprint(jsondict)
            sys.exit(1)
        except IndexError:
            import pprint
            print "ERROR: Unable to parse das output!"
            print datasetpath
            pprint.pprint(jsondict)
            sys.exit(1)


def pickEvents(run, eventnumber, lfn, tag, norder, raw=False, aod=False):
    cmd = ["edmCopyPickMerge"]
    if raw:
        cmd+= ["outputFile=%s" % "pick_%s_%d_%s_%s_raw" % (tag, norder, run, eventnumber)]
    elif aod:
        cmd+= ["outputFile=%s" % "pick_%s_%d_%s_%s_AOD" % (tag, norder, run, eventnumber)]
    else:
        cmd+= ["outputFile=%s" % "pick_%s_%d_%s_%s" % (tag, norder, run, eventnumber)]
    cmd += ["eventsToProcess=%s:%s" % (run, eventnumber)]
    cmd += ["inputFiles=%s" % (lfn)]
    print " ".join(cmd)
    subprocess.check_call(cmd)

def main():
    parser = argparse.ArgumentParser()
    file_input_group = parser.add_argument_group('Input from eventlists')
    file_input_group =  parser.add_argument("-e" , "--ecs", help="Event classes to pick", nargs='+')
    file_input_group = parser.add_argument("-n" , "--nevents", default=5, type=int,
                        help="Number of events to pick starting from top")
    file_input_group = parser.add_argument("-b" , "--basepath", default="Eventlists")
    file_format_group = parser.add_mutually_exclusive_group()
    file_format_group.add_argument("-r" , "--raw", action="store_true", help="Pick raw")
    file_format_group.add_argument("-a" , "--aod", action="store_true", help="Pick AOD")
    file_input_group = parser.add_argument("-d" ,
                        "--distribution", choices= ['SumPt', 'InvMass', 'MET','DEta'],
                        help="Number of events to pick starting from top",
                        default='SumPt')

    args = parser.parse_args()
    das = DasFileFinder()
    for ec_name in args.ecs:
        df = load_eventlist(args.basepath, ec_name, args.distribution)
        for i in range(args.nevents):
            sample = "_".join(df['pxliofile'][i].split("_")[:-1])
            lfn = das.get_file(sample,
                               df['run'][i],
                               df['lumi'][i],
                               get_raw=args.raw,
                               get_aod=args.aod)
            pickEvents(df['run'][i],
                       df['eventnumber'][i],
                       lfn,
                       sample.split("_")[-1] + sample.split("_")[1] + "_" + ec_name + "_" + args.distribution,
                       i,
                       raw=args.raw,
                       aod=args.aod)



if __name__ == '__main__':
    main()

