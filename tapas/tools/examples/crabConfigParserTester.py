

from crabConfigParser import CrabConfigParser

#variables to be set in previous part of script
requestName = 'ParserTestName'
workArea = '/user/pook/crab/'
#The python config file cmsRun will use as an argument
mypset = 'testpset.py'
datasetpath = '/T_tW-channel-DR_Tune4C_13TeV-CSA14-powheg-tauola/Spring14miniaod-PU20bx25_POSTLS170_V5-v1/MINIAODSIM'
unitsPerJob = 10

config = CrabConfigParser()


config.add_section('General')
config.set( 'General', 'requestName', requestName )
config.set( 'General', 'workArea', workArea )

config.add_section('JobType')
config.set( 'JobType', 'pluginName', 'Analysis' )
# For MC production with crab
# config.set( 'JobType', 'pluginName', 'PrivateMC' )
config.set( 'JobType', 'psetName', mypset )

config.add_section('Data')
config.set( 'Data', 'inputDataset', datasetpath )
config.set( 'Data', 'dbsUrl', 'global' )
config.set( 'Data', 'splitting', 'FileBased' )
config.set( 'Data', 'unitsPerJob', '%d'%unitsPerJob )
config.set( 'Data', 'publication', 'False' )

config.add_section('Site')
config.set( 'Site', 'storageSite', 'T2_DE_RWTH' )

config.writeCrabConfig('testConf.py')


