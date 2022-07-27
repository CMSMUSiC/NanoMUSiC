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

# main function is called at the end.
#
def main():
    
    usage = '%prog [options]'
    parser = optparse.OptionParser( usage = usage )
    parser.add_option( '-i','--inputFolder' , metavar = 'FOLDER', default='RunAnaOutDir',
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
    
    
    extractRootFiles(options)
    megeRootFiles(options)
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
    subprocess.call("tar -xzf %s MusicOutDir/SpecialHistos.root"%(splited[1]),shell=True)
    subprocess.call("mv MusicOutDir/SpecialHistos.root SpecialHistos.root",shell=True)
    os.chdir( thidir )
    
def extractRootFiles(options):
    log.info("Extracting Files.....")
    
    for i in glob.glob(options.inputFolder+"/*/*/"):
        if len(glob.glob(i+"MusicOutDir.tar.gz"))==0:
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
        if "merged" in sample:
            continue
        csample=sample.split("/")[-1]
        #for i in replaceItems:
            #csample=csample.replace(i,"")
        csample = re.sub(r"\w*__","",csample)
        listOfSamples.append([outputFolder+csample+".root",sample+"/*/",sample,options])
    
    log.info("Now merging files:")
    if len(listOfSamples)>0:
        #now merge all samples
        pool = multiprocessing.Pool(4)
        pool.map_async(hadd, listOfSamples)
        while True:
            time.sleep(5)
            if not pool._cache: break
    dataSamples=[]
    for data in glob.glob(outputFolder+"/"+"*Run*root"):
        data=data.split("/")[-1]
        data=data.split("_")[0].replace("Parked","")
        if data not in dataSamples:
            dataSamples.append(data)
    for dataSample in dataSamples:
        command= "hadd -f9 "+outputFolder+"/"+"allData%s.root "%(dataSample)+outputFolder+"/"+"%s*Run*root"%(dataSample)
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
        
        if not "data" in item[2]:
            calling="hadd -f9 "+item[0]+" "+item[1]+"/*.root"
        elif("dataDriven" in item[2]):
            calling="hadd -f9 "+item[0]+"_dataDriven.root "+item[1]+"/*dataDriven_*.root"
        else:
            calling="hadd -f9 "+item[0]+" "+item[1]+"/*.root"
        print(calling)
        p = subprocess.Popen(calling,shell=True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT )
        out, err = p.communicate()
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
