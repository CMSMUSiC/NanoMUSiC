#!/usr/bin/env python


import logging
log = logging.getLogger( "merger" )

import optparse
import os
import subprocess
import glob
import multiprocessing
import time
import re
from random import randint
import shutil

# main function is called at the end.
#
def main():

    usage = '%prog [options]'
    parser = optparse.OptionParser( usage = usage )
    parser.add_option( '-i','--inputFolder' , metavar = 'FOLDER', default=None,
                       help = 'Merge all subfolders in these folders, which can be a comma-separated list.[default: %default]' )
    parser.add_option( '--debug', metavar = 'LEVEL', default = 'INFO',
                       help= 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default = %default]' )
    parser.add_option( '-o','--output', metavar= "OUTFOLDER", default='RunAnaOutDir/merged',
                         help= 'Set the output dir [default = %default]' )
    parser.add_option( '-f', '--force',action = 'store_true', default = False,
                             help = 'If this option is specifed, all root files will be remerged. [default = %default]' )
    parser.add_option( '-c', '--clean',action = 'store_true', default = False,
                             help = 'If this option is specifed, the folders will be cleand up. [default = %default]' )
    parser.add_option( '-k', '--keep',metavar= 'keep', default = "METParked",
                             help = 'Folders with this name will be kept  [default = %default]' )
    ( options, args ) = parser.parse_args()

    format = '%(levelname)s %(name)s (%(asctime)s): %(message)s'
    date = '%H:%M:%S'
    logging.basicConfig( level = logging._levelNames[ options.debug ], format = format, datefmt = date )

    if options.inputFolder is None:
        if len(args)>0:
             options.inputFolder=args[0]

    call='cat {0}/*/grid*/err* | grep -v "WARNING: Overwriting value of key:" | grep -v "From configuration file:" | grep -v "cp: cannot stat" | grep -v "WARNING: In non-interactive mode release " | sort -u'.format(options.inputFolder)
    log.info( "Combined err output:")
    log.info( "---------------------------------------")
    subprocess.call(call,shell=True)
    log.info( "---------------------------------------")
    log.info( 'look for the hadd in "---" to see which file')
    call='grep "error" {0}*/grid*/err.txt | grep -v "copy failed with the error"'.format(options.inputFolder)
    subprocess.call(call,shell=True)
    log.info( "---------------------------------------")



    extractRootFiles(options)
    megeRootFiles(options)
    alljdls=glob.glob(options.inputFolder+"/*/job*.jdl")
    try:
        shutil.copyfile(alljdls[0], os.path.join(options.inputFolder,"example.jdl"))
    except:
        pass
    if options.clean:
        cleanUp(options)

def multiextractRoot(splited):
    thidir=os.getcwd()
    os.chdir( splited[0] )
    if os.path.getsize(splited[1])==0.:
        return
    if "SpecialHistos.root" in os.listdir(splited[0]):
        if os.path.getsize(splited[0]+"/SpecialHistos.root") >os.path.getsize(splited[1])*1.2:
            return
    #print os.listdir(splited[0])
    subprocess.call("tar -xzf %s MusicOutDir/events_after_cuts.txt"%(splited[1]),shell=True)
    subprocess.call("tar -xzf %s MusicOutDir/SpecialHistos.root"%(splited[1]),shell=True)
    subprocess.call("mv MusicOutDir/SpecialHistos.root SpecialHistos.root",shell=True)
    if not os.path.exists("SpecialHistos.root"):
        print "missing hist in %s"%(splited[0])
    os.chdir( thidir )

def extractRootFiles(options):
    log.info("Extracting Files.....")

    for i in glob.glob(options.inputFolder+"/*/*/"):
        if len(glob.glob(i+"MusicOutDir.tar.gz"))==0 and "Exe" not in i and "/bak/" not in i:
            log.warning("There is some thing worng in %s no MusicOutDir.tar.gz"%(i))
    if len(glob.glob(options.inputFolder+"/*/*/*tail")):
        log.warning("There is some thing worng:")
        for i in glob.glob(options.inputFolder+"/*/*/*tail"):
            log.warning("Found tail in %s"%(i))


    allFiles=[]
    for i in glob.glob(options.inputFolder+"/*/*/*.tar.gz"):
        splited=os.path.split(i)
        #if "SpecialHistos.root" in os.listdir(splited[0]):
            #continue
        allFiles.append(splited)
    if len(allFiles)>0:
        #for i in allFiles:
            #multiextractRoot(i)
        pool = multiprocessing.Pool()
        pool.map_async(multiextractRoot, allFiles)
        while True:
            time.sleep(1)
            if not pool._cache: break
    log.info("Done")



def megeRootFiles(options):
    replaceItems=["data__","mc__","mc_tauTrig__","data_tauTrig__"]
    listOfSamples=[]

    outputFolder=(options.output+"/").replace("RunAnaOutDir",options.inputFolder)
    if not os.path.exists(outputFolder):
        os.makedirs(outputFolder)
    for sample in glob.glob(options.inputFolder+"/*"):
        if os.path.isfile(sample):
            continue
        if "merged" in sample or "Exe" in sample or ".sh" in sample:
            continue
        csample=sample.split("/")[-1]
        #for i in replaceItems:
            #csample=csample.replace(i,"")
        csample = re.sub(r"\w*__","",csample)
        listOfSamples.append([outputFolder+csample+".root",sample+"/*/",sample,options])

    log.info("Now merging files:")
    if len(listOfSamples)>0:
        #now merge all samples
        pool = multiprocessing.Pool(5)
        pool.map_async(hadd, listOfSamples)
        while True:
            time.sleep(5)
            if not pool._cache: break
    hasData=False
    dataSamples=[]
    for data in glob.glob(outputFolder+"/"+"Data_*.root"):
        hasData=True
        data=data.split("_")[-1]
        data=data.replace(".root","")
        #data=data.split("_")[0].replace("Parked","")
        if data not in dataSamples:
            dataSamples.append(data)
    for dataSample in dataSamples:
        command= "hadd -f6 "+outputFolder+"/"+"Data_%s.root "%(dataSample)+outputFolder+"/"+"Data_*_%s.root"%(dataSample)
        print command
        p = subprocess.Popen(command,shell=True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        out, err = p.communicate()
        log.debug(out)
        log.debug(err)
    if hasData:
        all_merged_data_files=[outputFolder+"/"+"Data_%s.root "%(dataSample)  for dataSample in dataSamples]
        command= "hadd -f6 "+outputFolder+"/"+"allData.root "+" ".join(all_merged_data_files)
        print command
        p = subprocess.Popen(command,shell=True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
        out, err = p.communicate()
        log.debug(out)
        log.debug(err)
    log.info("Done")


#method to add the Files at the end
def hadd(item):
    time.sleep(float(randint(0,10))/10)
    out, err="",""
    overwrite=item[3].force
    if (not os.path.exists(item[0])) or overwrite:
        if len(glob.glob(item[1]+"/*.root"))==1:
            calling= "cp "+(glob.glob(item[1]+"/*.root")[0])+" "+item[0]
        else:
            if not "data" in item[2]:
                calling="hadd -f6 "+item[0]+" "+item[1]+"/*.root"
            elif("dataDriven" in item[2]):
                calling="hadd -f6 "+item[0]+"_dataDriven.root "+item[1]+"/*dataDriven_*.root"
            else:
                calling="hadd -f6 "+item[0]+" "+item[1]+"/*.root"
        print(calling)
        p = subprocess.Popen(calling,shell=True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )
        out, err = p.communicate()
        if ("Zomie" in out) or ("Error" in out):
            print "------------------------------------"
            print calling
            print "------------------------------------"
        log.debug(out)
        log.debug(err)
    return [out, err]

def cleanUp(options):
    log.info('Will delete all input files!!!')
    for sample in glob.glob(options.inputFolder+"/*"):
        if options.output in sample or options.keep in sample:
            continue
        log.debug("Delete",sample)
        for i in os.listdir(sample):
            for j in os.listdir(sample+"/"+i):

                os.remove(sample+"/"+i+"/"+j)
            os.rmdir(sample+"/"+i)
        os.rmdir(sample)

    for sample in glob.glob(options.inputFolder+"/*/*"):
        if options.output in sample or options.keep in sample:
            continue
        os.rmdir(sample)



if __name__ == "__main__":
    main()
