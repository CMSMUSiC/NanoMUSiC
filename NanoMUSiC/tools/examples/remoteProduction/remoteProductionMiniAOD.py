#!/bin/env python

import cesubmit
from listFiles import getdcachelist
import binConfig
import checkEnvironment
from datetime import datetime
import optparse,os,time,cPickle,subprocess,shutil,sys
import logging
log = logging.getLogger( 'remote' )




def makeExe(user):
    from string import Template
    exe="""
echo Copying pack...

mkdir -p CMSSW_7_0_6_patch1/src/A/A/data/

name=$1
file=$2
echo $name
echo $file

sed -i "s/PROC_NAME/$name/g" EXO-Spring14miniaod-private_cfg.py
sed -i "s/File_NAME/$file/g" EXO-Spring14miniaod-private_cfg.py

cat EXO-Spring14miniaod-private_cfg.py
cmsRun EXO-Spring14miniaod-private_cfg.py


outfileName="$file"


uberftp grid-ftp.physik.rwth-aachen.de mkdir /pnfs/physik.rwth-aachen.de/cms/store/user/padeken/MCminiAOD/$name/$outfileName

#uberftp grid-ftp.physik.rwth-aachen.de rm /pnfs/physik.rwth-aachen.de/cms/store/user/padeken/MCminiAOD/$name/$outfileName
lcg-cp file:///`pwd`/DM_out.root srm://grid-srm.physik.rwth-aachen.de:8443/pnfs/physik.rwth-aachen.de/cms/store/user/padeken/MCminiAOD/$name/$outfileName


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



    sampleList={'DM_EFT_400_Mchi_Lambda_600_g_p0':[
    #'DM_EFT_400_Mchi_Lambda_600_g_p0_06.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_07.root',]
     'DM_EFT_400_Mchi_Lambda_600_g_p0_02.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_14.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_12.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_19.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_08.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_11.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_05.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_18.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_09.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_01.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_16.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_17.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_20.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_03.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_04.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_13.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_15.root', 'DM_EFT_400_Mchi_Lambda_600_g_p0_10.root',],
    'DM_EFT_50_Mchi_Lambda_600_g_m1':[ 'DM_EFT_50_Mchi_Lambda_600_g_m1_01.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_10.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_15.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_19.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_20.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_18.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_14.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_12.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_03.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_13.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_05.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_16.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_09.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_02.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_17.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_06.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_08.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_04.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_07.root', 'DM_EFT_50_Mchi_Lambda_600_g_m1_11.root', ],
    'DM_EFT_600_Mchi_Lambda_600_g_p1':[ 'DM_EFT_600_Mchi_Lambda_600_g_p1_05.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_02.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_14.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_11.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_20.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_17.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_13.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_04.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_10.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_08.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_06.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_01.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_09.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_18.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_07.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_16.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_12.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_03.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_19.root', 'DM_EFT_600_Mchi_Lambda_600_g_p1_15.root', ],
    'DM_EFT_1000_Mchi_Lambda_600_g_p0':[ 'DM_EFT_1000_Mchi_Lambda_600_g_p0_10.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_01.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_13.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_02.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_18.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_03.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_17.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_11.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_05.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_06.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_12.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_04.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_15.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_14.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_19.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_16.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_07.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_09.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_08.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p0_20.root', ],
    'DM_EFT_600_Mchi_Lambda_600_g_p0':[ 'DM_EFT_600_Mchi_Lambda_600_g_p0_01.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_20.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_08.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_19.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_05.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_10.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_07.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_09.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_17.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_11.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_03.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_06.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_13.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_14.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_18.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_16.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_15.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_02.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_12.root', 'DM_EFT_600_Mchi_Lambda_600_g_p0_04.root', ],
    'DM_EFT_1000_Mchi_Lambda_600_g_p1':[ 'DM_EFT_1000_Mchi_Lambda_600_g_p1_12.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_15.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_08.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_04.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_16.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_06.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_02.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_14.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_18.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_01.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_09.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_07.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_19.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_11.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_20.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_10.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_13.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_17.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_05.root', 'DM_EFT_1000_Mchi_Lambda_600_g_p1_03.root', ],
    'DM_EFT_2000_Mchi_Lambda_600_g_p1':[ 'DM_EFT_2000_Mchi_Lambda_600_g_p1_07.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_01.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_03.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_19.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_14.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_10.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_15.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_16.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_04.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_13.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_20.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_09.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_08.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_18.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_17.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_12.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_06.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_05.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_11.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p1_02.root', ],
    'DM_EFT_2000_Mchi_Lambda_600_g_m1':[ 'DM_EFT_2000_Mchi_Lambda_600_g_m1_13.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_19.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_12.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_15.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_20.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_04.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_02.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_10.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_03.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_01.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_16.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_11.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_17.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_08.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_18.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_05.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_06.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_14.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_09.root', 'DM_EFT_2000_Mchi_Lambda_600_g_m1_07.root', ],
    'DM_EFT_400_Mchi_Lambda_600_g_m1':[ 'DM_EFT_400_Mchi_Lambda_600_g_m1_06.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_18.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_07.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_02.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_15.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_01.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_05.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_03.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_13.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_19.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_08.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_14.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_20.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_11.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_04.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_12.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_10.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_09.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_17.root', 'DM_EFT_400_Mchi_Lambda_600_g_m1_16.root', ],
    'DM_EFT_3000_Mchi_Lambda_600_g_m1':[ 'DM_EFT_3000_Mchi_Lambda_600_g_m1_18.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_02.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_09.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_14.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_19.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_05.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_13.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_16.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_06.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_01.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_12.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_15.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_17.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_08.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_03.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_07.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_10.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_11.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_04.root', 'DM_EFT_3000_Mchi_Lambda_600_g_m1_20.root', ],
    'DM_EFT_2000_Mchi_Lambda_600_g_p0':[ 'DM_EFT_2000_Mchi_Lambda_600_g_p0_14.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_11.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_03.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_17.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_16.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_04.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_20.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_07.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_08.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_09.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_15.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_05.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_01.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_06.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_18.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_10.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_02.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_19.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_12.root', 'DM_EFT_2000_Mchi_Lambda_600_g_p0_13.root', ],
    'DM_EFT_1000_Mchi_Lambda_600_g_m1':[ 'DM_EFT_1000_Mchi_Lambda_600_g_m1_18.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_15.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_13.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_16.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_14.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_03.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_17.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_10.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_02.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_04.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_06.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_01.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_09.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_11.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_08.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_07.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_12.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_19.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_05.root', 'DM_EFT_1000_Mchi_Lambda_600_g_m1_20.root', ],
    'DM_EFT_4000_Mchi_Lambda_600_g_p0':[ 'DM_EFT_4000_Mchi_Lambda_600_g_p0_18.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_03.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_06.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_07.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_05.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_09.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_02.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_04.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_20.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_13.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_16.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_17.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_10.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_01.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_12.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_14.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_15.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_11.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_08.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p0_19.root', ],
    'DM_EFT_400_Mchi_Lambda_600_g_p1':[ 'DM_EFT_400_Mchi_Lambda_600_g_p1_04.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_07.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_01.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_12.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_15.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_14.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_05.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_17.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_20.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_09.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_08.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_18.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_11.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_19.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_16.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_10.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_03.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_13.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_06.root', 'DM_EFT_400_Mchi_Lambda_600_g_p1_02.root', ],
    'DM_EFT_4000_Mchi_Lambda_600_g_p1':[ 'DM_EFT_4000_Mchi_Lambda_600_g_p1_15.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_12.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_17.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_13.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_06.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_14.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_02.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_10.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_03.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_09.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_01.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_08.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_11.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_05.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_19.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_20.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_16.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_07.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_04.root', 'DM_EFT_4000_Mchi_Lambda_600_g_p1_18.root', ],
    'DM_EFT_3000_Mchi_Lambda_600_g_p0':[ 'DM_EFT_3000_Mchi_Lambda_600_g_p0_01.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_05.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_20.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_18.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_13.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_07.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_10.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_19.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_02.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_03.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_14.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_09.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_15.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_04.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_08.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_16.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_12.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_06.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_17.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p0_11.root', ],
    'DM_EFT_600_Mchi_Lambda_600_g_m1':[ 'DM_EFT_600_Mchi_Lambda_600_g_m1_10.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_12.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_11.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_14.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_05.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_18.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_13.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_04.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_19.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_09.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_03.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_17.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_07.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_20.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_02.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_01.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_16.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_15.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_06.root', 'DM_EFT_600_Mchi_Lambda_600_g_m1_08.root', ],
    'DM_EFT_50_Mchi_Lambda_600_g_p0':[ 'DM_EFT_50_Mchi_Lambda_600_g_p0_03.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_01.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_14.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_16.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_08.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_12.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_05.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_17.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_10.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_02.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_18.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_04.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_19.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_06.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_20.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_15.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_09.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_07.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_13.root', 'DM_EFT_50_Mchi_Lambda_600_g_p0_11.root', ],
    'DM_EFT_4000_Mchi_Lambda_600_g_m1':[ 'DM_EFT_4000_Mchi_Lambda_600_g_m1_05.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_01.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_06.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_08.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_10.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_04.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_20.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_19.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_15.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_16.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_09.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_03.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_12.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_17.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_07.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_18.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_02.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_11.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_14.root', 'DM_EFT_4000_Mchi_Lambda_600_g_m1_13.root', ],
    'DM_EFT_50_Mchi_Lambda_600_g_p1':[ 'DM_EFT_50_Mchi_Lambda_600_g_p1_06.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_08.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_13.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_04.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_10.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_05.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_12.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_11.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_03.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_16.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_07.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_17.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_01.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_14.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_15.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_02.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_20.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_19.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_09.root', 'DM_EFT_50_Mchi_Lambda_600_g_p1_18.root', ],
    'DM_EFT_3000_Mchi_Lambda_600_g_p1':[ 'DM_EFT_3000_Mchi_Lambda_600_g_p1_06.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_04.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_03.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_13.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_18.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_11.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_07.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_08.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_09.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_12.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_20.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_14.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_17.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_10.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_15.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_01.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_05.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_16.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_02.root', 'DM_EFT_3000_Mchi_Lambda_600_g_p1_19.root', ]
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
        task=cesubmit.Task(sample,options.Output+"/"+sample,scramArch=scram_arch, cmsswVersion=cmssw_version)

        task.executable=options.Output+"/runtemp.sh"
        task.inputfiles=[thisdir+"/EXO-Spring14miniaod-private_cfg.py"]
        task.outputfiles=[""]


        standardArg=[sample]

        for f in sampleList[sample]:
            job=cesubmit.Job()
            job.arguments=standardArg+[f]
            task.addJob(job)
        log.info("start submitting "+sample)
        task.submit(6)


    log.info("Thanks for zapping in, bye bye")
    log.info("The out files will be in "+options.Output)



if __name__ == '__main__':
    main()
