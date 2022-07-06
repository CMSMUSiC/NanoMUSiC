#!/bin/env python

import cesubmit
from gridFunctions import getdcachelist
import binConfig
import checkEnvironment
from datetime import datetime
import optparse,os,time,cPickle,subprocess,shutil,sys,random
import logging
log = logging.getLogger( 'remote' )




def makeExe(user):
    from string import Template
    exe="""
#!/bin/bash -e
echo Copying pack...

name=$1
file=$2
seed=$3
echo "------------------"
echo $name
echo $file
echo $seed
echo "------------------"

sed -i "s/PROC_NAME/$name/g" hadronizer_match_pu_2_cfg.py
sed -i "s/File_NAME/$file/g" hadronizer_match_pu_2_cfg.py
sed -i "s/SEED/$seed/g" hadronizer_match_pu_2_cfg.py

cmsRun hadronizer_match_pu_2_cfg.py

outfileName="${file%.lhe}.root"

uberftp grid-ftp.physik.rwth-aachen.de "rm /pnfs/physik.rwth-aachen.de/cms/store/user/padeken/MC/DMV/AOD/$name/$outfileName"
#Try 10 times to copy the pack file with help of srmcp.
success=false
for i in {1..10}; do
   if lcg-cp file:///`pwd`/test.root srm://grid-srm.physik.rwth-aachen.de:8443/pnfs/physik.rwth-aachen.de/cms/store/user/padeken/MC/DMV/AOD/$name/$outfileName; then
      success=true
      break
   fi
done
if ! $success; then
   echo Copying of pack file \\\'lcg-cp file:///`pwd`/test.root srm://grid-srm.physik.rwth-aachen.de:8443/pnfs/physik.rwth-aachen.de/cms/store/user/padeken/MC/DMV/$name/$outfileName\\\' failed! 1>&2
   echo Did you forget to \\\'remix --copy\\\'? 1>&2
fi





"""

    exeFile=open("runtemp.sh","w+")
    exeFile.write(exe)
    exeFile.close()


def main():

    date_time = datetime.now()
    usage = '%prog [options] CONFIG_FILE'
    parser = optparse.OptionParser( usage = usage )
    parser.add_option( '-u', '--user', default = os.getenv( 'LOGNAME' ),
                            help = 'which user on dcache [default = %s]'%(os.getenv( 'LOGNAME' )))
    parser.add_option( '-o', '--Output', default = '%s'%(binConfig.outDir).replace("USER",os.getlogin())+"/TAG", metavar = 'DIRECTORY',
                            help = 'Define the output directory. [default = %default]')
    parser.add_option( '-f', '--force', default = "force the output to overwrite", metavar = 'DIRECTORY',
                            help = 'Define the output directory. [default = %default]')
    parser.add_option( '--debug', metavar = 'LEVEL', default = 'INFO',
                       help= 'Set the debug level. Allowed values: ERROR, WARNING, INFO, DEBUG. [default = %default]' )
    parser.add_option( '-t', '--Tag', default = "output%s_%s_%s_%s_%s"%(date_time.year,
                                                                        date_time.month,
                                                                        date_time.day,
                                                                        date_time.hour,
                                                                        date_time.minute), metavar = 'DIRECTORY',
                        help = 'Define a Tag for the output directory. [default = %default]' )

    ( options, args ) = parser.parse_args()
    #if len( args ) != 1:
        #parser.error( 'Exactly one CONFIG_FILE required!' )
    options.Output=options.Output.replace("TAG",options.Tag)


    format = '%(levelname)s from %(name)s at %(asctime)s: %(message)s'
    date = '%F %H:%M:%S'
    logging.basicConfig( level = logging._levelNames[ options.debug ], format = format, datefmt = date )

    try:
       cmssw_version, cmssw_base, scram_arch = checkEnvironment.checkEnvironment()
    except EnvironmentError, err:
        log.error( err )
        log.info( 'Exiting...' )
        sys.exit( err.errno )



    sampleList={


    "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0333333":
    ["DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_01.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_08.lhe",
    "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_15.lhe",    "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_02.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_09.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_16.lhe" ,   "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_03.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_10.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_17.lhe" ,   "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_04.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_11.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_18.lhe" ,   "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_05.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_12.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_19.lhe" ,   "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_06.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_13.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_20.lhe" ,   "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_07.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_m1_widthfactor_0.333333_14.lhe"
    ],

    "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0333333":
    ["DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_01.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_08.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_15.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_02.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_09.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_16.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_03.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_10.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_17.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_04.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_11.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_18.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_05.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_12.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_19.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_06.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_13.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_20.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_07.lhe", "DM_DMAV_2000_Mxi_Wchi_666_g_p1_widthfactor_0.333333_14.lhe"],

    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0333333":
    ["DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_01.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_06.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_11.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_16.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_02.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_07.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_12.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_17.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_03.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_08.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_13.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_18.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_04.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_09.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_14.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_19.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_05.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_10.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_15.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_m1_widthfactor_0.333333_20.lhe"],

    "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0333333":
    ["DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_01.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_06.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_11.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_16.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_02.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_07.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_12.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_17.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_03.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_08.lhe",
    "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_13.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_18.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_04.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_09.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_14.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_19.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_05.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_10.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_15.lhe", "DM_DMAV_500_Mxi_Wchi_166_g_p1_widthfactor_0.333333_20.lhe"],

    "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0333333":
    ["DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_01.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_06.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_11.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_16.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_02.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_07.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_12.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_17.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_03.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_08.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_13.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_18.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_04.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_09.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_14.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_19.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_05.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_10.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_15.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_m1_widthfactor_0.333333_20.lhe"],

    "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0333333":
    ["DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_01.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_06.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_11.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_16.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_02.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_07.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_12.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_17.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_03.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_08.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_13.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_18.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_04.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_09.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_14.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_19.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_05.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_10.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_15.lhe", "DM_DMAV_50_Mxi_Wchi_16_g_p1_widthfactor_0.333333_20.lhe"]
    }


    makeExe(options.user)

    thisdir=os.getcwd()
    if os.path.exists(options.Output) or not options.force:
        log.error("The outpath "+options.Output+" already exists pick a new one or use --force")
        sys.exit(3)
    else:
        os.makedirs(options.Output)
    shutil.copyfile(thisdir+"/runtemp.sh",options.Output+"/runtemp.sh")
    os.remove(thisdir+"/runtemp.sh")

    for sample in sampleList:
        #task=cesubmit.Task(sample,options.Output+"/"+sample,scramArch=scram_arch, cmsswVersion=cmssw_version , ceId='grid-ce.physik.rwth-aachen.de:8443/cream-pbs-cms')
        #task=cesubmit.Task(sample,options.Output+"/"+sample,scramArch=scram_arch, cmsswVersion=cmssw_version , ceId='grid-cr0.desy.de:8443/cream-pbs-cms')
        task=cesubmit.Task(sample,options.Output+"/"+sample,scramArch=scram_arch, cmsswVersion=cmssw_version )
        #ceId="dcache-se-cms.desy.de:8443/cream-pbs-cms")
        #'grid-ce.physik.rwth-aachen.de:8443/cream-pbs-cms'

        task.executable=options.Output+"/runtemp.sh"
        task.inputfiles=[thisdir+"/hadronizer_match_pu_2_cfg.py"]
        #task.outputfiles=[""]

        #usage: bin/music [--DumpECHistos] [--NoSpecialAna] [--NoCcControl] [--NoCcEventClass] [-h] [-o value] [-N value] [-x value] [-p value] [--debug value] [-M value] a1...


        standardArg=[sample]

        for f in sampleList[sample]:
            job=cesubmit.Job()
            job.arguments=standardArg+[f]+["%d"%(random.randint(1,1000000))]
            task.addJob(job)
        log.info("start submitting "+sample)
        task.submit(6)


    log.info("Thanks for zapping in, bye bye")
    log.info("The out files will be in "+options.Output)



if __name__ == '__main__':
    main()
