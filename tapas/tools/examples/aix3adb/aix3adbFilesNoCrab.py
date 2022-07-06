#!/bin/env python
'''
This script is a workaround for very rare cases when crab does not
work anymore for your task for some reason.

Only use it if:
* Files were not filled in DB entries
* You already sucessfully called crab log
* In case of data skims, already called crab report

If not you need to rerun later on if crab works again  !
'''

#!/bin/env python
import crabFunctions
import aix3adb
import logging
log = logging.getLogger( 'music_crab' )

dblink = aix3adb.createDBlink( "padeken" )


#setup logging
format = '%(levelname)s (%(name)s) [%(asctime)s]: %(message)s'
logging.basicConfig( level=logging._levelNames[ "DEBUG" ], format=format)
log.setLevel(logging._levelNames[ "DEBUG" ])
formatter = logging.Formatter( format )
logging.getLogger('CRAB3').propagate = False  # Avoid any CRAB message to propagate up to the handlers of the root logger.


skimmer_version = "CMSSW_7_6_v1.0"
globalTag = "76X_mcRun2_asymptotic_v12"

listOfSampleSkim=[
[ "ZZTo4L_13TeV_AM",3979],
["ST_t-channel_4f_leptonDecays_13TeV_AM",3880],
["WJetsToLNu_13TeV_AM",3837],
["WprimeToTauNu_M-3400_13TeV_P8",3835],
["DYJetsToLL_M-100to200_13TeV_ext1_AM",3832],
["WprimeToTauNu_M-4800_13TeV_P8",3830],
["DYJetsToLL_M-10to50_13TeV_ext1_AM",3824],
["QCD_Pt_800to1000_13TeV_P8",3823],
["WprimeToTauNu_M-2200_13TeV_P8",3822],
["QCD_Pt-120to170_MuEnrichedPt5_13TeV_P8",3819],
["ST_tW_top_5f_NoFullyHadronicDecays_13TeV_PH",3817],
["WToENu_M-200_13TeV_P8",3816],
["WprimeToTauNu_M-2800_13TeV_P8",3814],
["QCD_Pt_470to600_13TeV_P8",3812],
["GG_M-8000To13000_Pt-70_13TeV_SP",3811],
["ST_tW_antitop_5f_inclusiveDecays_13TeV_PH",3810],
["QCD_Pt-170to300_MuEnrichedPt5_13TeV_P8",3809],
["WprimeToTauNu_M-4200_13TeV_P8",3806],
["QCD_Pt_80to120_13TeV_P8",3805],
["GG_M-2000To4000_Pt-70_13TeV_SP",3802],
["WprimeToTauNu_M-3000_13TeV_P8",3801],
["QCD_Pt_5to10_13TeV_P8",3800],
["WprimeToTauNu_M-1200_13TeV_P8",3799],
["WprimeToTauNu_M-3200_13TeV_P8",3798],
["QCD_Pt_170to300_13TeV_P8",3797],
["WToTauNu_M-500_13TeV_P8",3795],
["ST_tW_antitop_5f_DS_inclusiveDecays_13TeV_PH",3792],
["GJet_Pt-15To6000-Flat_13TeV_P8",3791],
["WToMuNu_M-1000_13TeV_P8",3790],
["WprimeToTauNu_M-3600_13TeV_P8",3789],
["WToENu_M-3000_13TeV_P8",3788],
["ST_t-channel_5f_leptonDecays_13TeV_AM",3783],
["ST_tW_top_5f_DS_inclusiveDecays_13TeV_PH",3782],
["QCD_Pt-300toInf_EMEnriched_13TeV_P8",3781],
["ST_t-channel_4f_leptonDecays_13TeV_ext1_AM",3780],
["MuMuG_PTG130To400_MMuMu50_13TeV_SP",3777],
["WToMuNu_M-200_13TeV_P8",3776],
["WprimeToTauNu_M-3800_13TeV_P8",3775],
["WprimeToTauNu_M-1800_13TeV_P8",3769],
["WprimeToTauNu_M-2400_13TeV_P8",3768],
["TT_TuneEE5C-amcatnlo_13TeV_HP",3767],
["WprimeToTauNu_M-2600_13TeV_P8",3766],
["WprimeToTauNu_M-1000_13TeV_P8",3764],
["WprimeToTauNu_M-4000_13TeV_P8",3762],
["GG_M-4000To8000_Pt-70_13TeV_SP",3760],
["WprimeToTauNu_M-5800_13TeV_P8",3747],
["QCD_Pt-30to50_MuEnrichedPt5_13TeV_P8",3746],
["ZZ_13TeV_P8",3742],
["WprimeToTauNu_M-5400_13TeV_P8",3741],
["WToTauNu_M-200_13TeV_P8",3740],
["WprimeToTauNu_M-2000_13TeV_P8",3739],
["WJetsToLNu_HT-600To800_13TeV_MG",3738],
["QCD_Pt_50to80_13TeV_P8",3735],
["QCD_Pt-20to30_EMEnriched_13TeV_P8",3734],
["QCD_Pt-470to600_MuEnrichedPt5_13TeV_P8",3733],
["WJetsToLNu_HT-2500ToInf_13TeV_MG",3730],
["QCD_Pt-300to470_MuEnrichedPt5_13TeV_P8",3728],
["WprimeToTauNu_M-1400_13TeV_P8",3725],
["QCD_Pt-80to120_MuEnrichedPt5_13TeV_P8",3724],
["QCD_Pt_300to470_13TeV_P8",3723],
["GJet_Pt-15to6000_Flat_13TeV_P8",3722],
["WprimeToTauNu_M-5000_13TeV_P8",3721],
["QCD_Pt_600to800_13TeV_P8",3719],
["WprimeToTauNu_M-4400_13TeV_P8",3717],
["WToMuNu_M-100_13TeV_P8",3716],
["WToENu_M-1000_13TeV_P8",3711],
["TTJets_13TeV_MG",3557],
["WZJets_13TeV_AM",3551],
["TT_13TeV_AM",3547],
["DYJetsToLL_M-10to50_13TeV_AM",3538],
["DYJetsToLL_M-1000to1500_13TeV_AM",3536],
["DYJetsToLL_M-200to400_13TeV_AM",3532],
["DYJetsToLL_M-50_HT-200to400_13TeV_MG",3527],
["DYJetsToLL_M-5to50_HT-600toInf_13TeV_MG",3524],
["TTJets_SingleLeptFromT_13TeV_MG",3521],
["WZTo1L3Nu_13TeV_AM",3516],
["DYJetsToLL_M-2000to3000_13TeV_AM",3513],
["DYJetsToLL_M-1500to2000_13TeV_AM",3512],
["DYJetsToLL_M-500to700_13TeV_AM",3511],
["ZZTo4L_13TeV_PH",3507],
["WWTo4Q_13TeV_PH",3503],
["WZTo3LNu_13TeV_PH",3485],
["WWToLNuQQ_13TeV_ext1_PH",3483],
["TTTo2L2Nu_13TeV_PH",3482],
]
#datasetpath = "/Tau/Run2015D-16Dec2015-v1/MINIAOD"
#name = "Data_Run2015D-16Dec2015_258159_260627_Tau"

for sample,skimID in listOfSampleSkim:
  skim=dblink.getMCSkim(skimID)
  name=sample

  datasetpath=skim.datasetpath
  task = crabFunctions.CrabTask( name,
                        dblink = dblink,
                        debuglevel="DEBUG",
                        initUpdate = False,
                        globalTag = globalTag,
                        datasetpath=datasetpath,
                        skimmer_version = skimmer_version )

  task.finalizeTask(debug=True)

