#!/bin/env python

import cesubmit
from gridFunctions import getdcachelist
import binConfig_example as binConfig
import checkEnvironment
from datetime import datetime
import optparse,os,time,cPickle,subprocess,shutil,sys
import logging
log = logging.getLogger( 'remote' )

def getFilesfromFile(cfgFile):
    sampleList={}
    file = open(cfgFile,'r')
    user,tag,sample,config,mainFolder=["","","","",""]

    known_folders=dict()
    if not os.path.exists("data"):
        os.mkdir("data")

    #if os.path.exists("data/fileList.pkl"):
        #known_folders=readDcachePickle("data/fileList.pkl")

    for line in file:
        if line[0]=="#" or len(line.split())==0:
            continue
        if "tag=" in line:
            tag=line.split("=")[1].strip()
            continue
        if "user=" in line:
            user=line.split("=")[1].strip()
            continue
        if "config=" in line:
            config=line.split("=")[1].strip()
            continue
        if "mainFolder=" in line:
            mainFolder=line.split("=")[1].strip()
            continue
        sample=line.strip()
        log.debug( " ".join([user,tag,sample,config]))
        if mainFolder=="":
            mainFolder="MUSiC"
        folder="/%s/%s/%s/%s" % (user,mainFolder,tag,sample)
        sampleFolder=folder.replace("/","")
        sampleFolder+=".pkl"
        if os.path.exists("data/"+sampleFolder):
            known_folders=readDcachePickle("data/"+sampleFolder)
            file_lists_1=known_folders[folder]
            file_lists=[]
            for j in file_lists_1:
                #file_lists.append([ i.replace("dcap://grid-dcap.physik.rwth-aachen.de/pnfs","/pnfs") for i in j])
                file_lists.append([ i for i in j])

        else:
            #time.sleep(4)
            file_lists=[]
            if "Data" in sample:
                file_lists_1 = getdcachelist( folder, sample,mem_limit = 1000000000 )
            else:
                file_lists_1 = getdcachelist( folder, sample,mem_limit = 4000000000 )
            for j in file_lists_1:
                #file_lists.append([ i.replace("dcap://grid-dcap.physik.rwth-aachen.de/pnfs","/pnfs") for i in j])
                file_lists.append([ i for i in j])
            outfile = open( "data/"+sampleFolder, 'a+b' )
            cPickle.dump( {folder:file_lists}, outfile, -1 )
            outfile.close()
        if len(file_lists)>0:

            file_lists=filter(lambda x: "failed" not in x, file_lists)
            sampleList.update({sample:[file_lists,config]})
        else:
            print sample
            #raise IOError( 'No sample in List for folder '+sample )
    return sampleList

def readDcachePickle(file):
    infile = open(file, 'rb' )
    known_folders = dict()
    try:
        while True:
            try:
                folder = cPickle.load( infile )
                known_folders.update( folder )
            except KeyError as e:
                log.error( "KeyError: "+e)
            except IndexError as e:
                log.error( "IndexError: "+e)
                infile.close()
                return known_folders
            except cPickle.UnpicklingError as e:
                log.error( "cPickle.UnpicklingError: "+e)
                infile.close()
                return known_folders
            except ValueError as e:
                log.error( "ValueError: "+e)
    except EOFError:
        infile.close()
        return known_folders
    except:
        raise

def makeExe(user,options,md5sum):
    from string import Template
    exe="""
    echo Copying pack...
    #Try 10 times to copy the pack file with help of srmcp.
    success=false
    #cp /pnfs/physik.rwth-aachen.de/cms/store/user/$USER/$PROGAM/share/$PROGRAM .



    success=false
    #if ! `cp /pnfs/physik.rwth-aachen.de/cms/store/user/$USER/$PROGAM/share/$PROGRAM .`; then
    if ! `srmcp gsiftp://grid-se114.physik.rwth-aachen.de:2811/pnfs/physik.rwth-aachen.de/cms/store/user/$USER/$PROGAM/share/$PROGRAM file:///.`; then
        for i in {1..2}; do
           if srmcp gsiftp://grid-se114.physik.rwth-aachen.de:2811/pnfs/physik.rwth-aachen.de/cms/store/user/$USER/$PROGAM/share/$PROGRAM file:///.; then
              success=true
              break
           fi
        done
        if ! $success; then
           echo Copying of pack file \\\'gsiftp://grid-se114.physik.rwth-aachen.de:2811/pnfs/physik.rwth-aachen.de/cms/store/user/$USER/$PROGAM/share$PROGRAM\\\' failed! 1>&2
           echo Did you forget to \\\'remix --copy\\\'? 1>&2
        fi
    fi
    md5sum $PROGRAM
    echo $MD5SUM
    tar xzvf $PROGRAM
    export PXLANA=$PWD
    export MUSIC_BASE=$PWD
    export MYPXLANA=specialAna
    export LD_LIBRARY_PATH=$PWD/extra_libs:$LD_LIBRARY_PATH
    export LD_LIBRARY_PATH=$LHAPATHREPLACE/lib:$LD_LIBRARY_PATH
    #echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH
    #echo Setting LHAPATH to $LHAPATHREPLACE
    export LHAPATH=$LHAPATHREPLACE2

    ldd bin/music
    echo "$@"
    bin/music "$@"

    tar czf MusicOutDir.tar.gz MusicOutDir
    """



    #this should be done as a fuction parameter
    d = dict(
            USER=user,
            PROGAM="MUSiC",
            LHAPATHREPLACE="/cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/lhapdf6/6.1.4/",
            LHAPATHREPLACE2="/cvmfs/cms.cern.ch/slc6_amd64_gcc481/external/lhapdf6/6.1.4/share/LHAPDF/PDFsets",
            PROGRAM=options.program,
            MD5SUM=md5sum,
        )
    exe=Template(exe).safe_substitute(d)
    exeFile=open("runtemp.sh","w+")
    exeFile.write(exe)
    exeFile.close()

def prepare_teli(options):
    import tempfile
    import TimedCall
    log.info("Copy file to dache..")
    cpFiles=binConfig.cpFiles
    PathtoExecutable=binConfig.PathtoExecutable
    os.makedirs(options.Output+"/Exe")
    #tempdir = tempfile.mkdtemp( prefix='televisionExe-' )
    tempdir = options.Output+"/Exe"
    for i in cpFiles:
        if os.path.isdir("%s"%(os.path.join(PathtoExecutable,i))):
            command='rsync -av --exclude ".*/" --exclude=".git" --exclude="*.o" %s/%s %s'%(PathtoExecutable,i,tempdir)
        else:
            os.makedirs("%s/%s"%(tempdir,i.split("/")[0]))
            command="cp -r %s/%s %s/%s"%(PathtoExecutable,i,tempdir,i.split("/")[0])
        retcode, output2=TimedCall.retry(3,300,command.split(" "))
        if retcode!=0:
            log.error("Could not create a local copy check arguments!!\n %s"%(command))
            sys.exit(1)
    thidir=os.getcwd()
    os.chdir( tempdir )

    retcode, output2=TimedCall.retry(3,300,['tar', 'czf' , options.program]+os.listdir(tempdir) )
    if retcode!=0:
        log.error("Could not create a local copy check arguments!!")
        sys.exit(1)

    retcode, output2=TimedCall.retry(3,300,['md5sum' , options.program])



    user = options.user
    path = "srm://grid-srm.physik.rwth-aachen.de:8443/pnfs/physik.rwth-aachen.de/cms/store/user/%s/MUSiC/share/"%(user)
    cmd1 = "lcg-cp"
    cmd2 = "file:///%s/%s"% (tempdir,options.program)
    cmd3 = "%s%s"% (path,options.program)
    command = [cmd1,cmd2,cmd3]
    command2 = ["uberftp","grid-ftp",r"rm /pnfs/physik.rwth-aachen.de/cms/store/user/%s/MUSiC/share/%s"%(user,options.program)]
    counter=0
    log.debug( " ".join(command2))
    log.debug(" ".join(command))
    retcode,retcode2=0,0
    while counter<3:
        retcode2, output2=TimedCall.retry( 3, 600, command2 )
        retcode, output=TimedCall.retry( 3, 600, command )
        if retcode+retcode2==0:
            break
        if (retcode+retcode2)!=0:
            log.info("Could not copy file to dcache")
            log.info(" ".join(command2))
            log.info(" ".join(command))
            log.info(output2)
            log.info(output)
    if (retcode+retcode2)!=0:
        log.info("Could not copy file to dcache")
        log.info(command2)
        log.info(command)
        log.info(output2)
        log.info(output)
        sys.exit(1)

    retcode, output=TimedCall.retry( 3, 600, ("md5sum %s/%s"%(tempdir,options.program)).split(" ") )
    md5sum=output.split()[0]
    log.info("File "+tempdir+"/%s  copied to dcache"%(options.program))
    os.chdir(thidir)
    return md5sum


def main():

    date_time = datetime.now()
    usage = '%prog [options] CONFIG_FILE'
    parser = optparse.OptionParser( usage = usage )
    parser.add_option( '-u', '--user', default = os.getenv( 'LOGNAME' ),
                            help = 'which user on dcache [default = %s]'%(os.getenv( 'LOGNAME' )))
    parser.add_option( '-o', '--Output', default = '%s'%(binConfig.outDir).replace("USER",os.getlogin())+"/TAG", metavar = 'DIRECTORY',
                            help = 'Define the output directory. [default = %default]')
    parser.add_option( '-p', '--program', default = "program.tar.gz", metavar = 'program',
                            help = 'Define name of the program on the dcache [default = %default]')
    parser.add_option( '-f', '--force',action = 'store_true', default = False,
                            help = 'Force the output folder to be overwritten. [default = %default]')
    parser.add_option( '-l', '--local',action = 'store_true', default = False,
                            help = 'run localy over the files [default = %default]')
    parser.add_option( '--debug', metavar = 'LEVEL', default = 'INFO',
                       help= 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default = %default]' )
    parser.add_option( '-t', '--Tag', default = "output%s_%s_%s_%s_%s"%(date_time.year,
                                                                        date_time.month,
                                                                        date_time.day,
                                                                        date_time.hour,
                                                                        date_time.minute), metavar = 'DIRECTORY',
                        help = 'Define a Tag for the output directory. [default = %default]' )

    ( options, args ) = parser.parse_args()
    if len( args ) != 1:
        parser.error( 'Exactly one CONFIG_FILE required!' )
    options.Output=options.Output.replace("TAG",options.Tag)


    format = '%(levelname)s from %(name)s at %(asctime)s: %(message)s'
    date = '%F %H:%M:%S'
    logging.basicConfig( level = logging._levelNames[ options.debug ], format = format, datefmt = date )
    log.info("Welcome to the wonders of color!")

    try:
       pxlana_base ,cmssw_version, cmssw_base, scram_arch = checkEnvironment.checkEnvironment()
    except EnvironmentError, err:
        log.error( err )
        log.info( 'Exiting...' )
        sys.exit( err.errno )


    cfgFile = args[ 0 ]
    if os.path.exists(options.Output) and not options.force:
        log.error("The outpath "+options.Output+" already exists pick a new one or use --force")
        sys.exit(3)
    else:
        os.makedirs(options.Output)
    sampleList=getFilesfromFile(cfgFile)
    md5sum=prepare_teli(options)
    makeExe(options.user,options,md5sum)

    thisdir=os.getcwd()

    shutil.copyfile(thisdir+"/runtemp.sh",options.Output+"/runtemp.sh")
    shutil.copyfile(os.path.join(thisdir,cfgFile),os.path.join(options.Output,cfgFile))
    os.remove(thisdir+"/runtemp.sh")


    n_jobs=0
    for sample in sampleList:
        n_jobs+=len(sampleList[sample][0])
    #raw_input("There will be %d jobs in total"%n_jobs)
    print("There will be %d jobs in total"%n_jobs)

    sbumittedjobs=0
    for sample in sampleList:
        task=cesubmit.Task(sample,options.Output+"/"+sample,scramArch=scram_arch, cmsswVersion=cmssw_version)

        task.executable=options.Output+"/runtemp.sh"
        task.inputfiles=[]
        task.outputfiles=["MusicOutDir.tar.gz"]

        #usage: bin/music [--DumpECHistos] [--NoSpecialAna] [--NoCcControl] [--NoEventClassFactory] [-h] [-o value] [-N value] [-x value] [-p value] [--debug value] [-M value] a1...


        standardArg=["-o","MusicOutDir",sampleList[sample][1]]
        for f in sampleList[sample][0]:
            job=cesubmit.Job()
            job.arguments=standardArg+f
            task.addJob(job)
        #log.info("start submitting "+sample+" "+str(len(sampleList[sample][0])) )
        log.info("start submitting %s %d  %d/%d"%(sample,len(sampleList[sample][0]),sbumittedjobs,n_jobs) )
        sbumittedjobs+=len(sampleList[sample][0])
        numberOfJobs=8
        if options.local:
            numberOfJobs=30
        task.submit(processes=numberOfJobs,local=options.local)


    log.info("Thanks for zapping in, bye bye")
    log.info("The out files will be in "+options.Output+"/*")



if __name__ == '__main__':
    main()
