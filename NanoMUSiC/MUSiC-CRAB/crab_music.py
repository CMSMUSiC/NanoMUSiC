#!/usr/bin/env python3

from datetime import datetime
import subprocess
import os

from CRABClient.UserUtilities import config 
from CRABAPI.RawCommand import crabCommand

def get_username():
    res = subprocess.check_output(['crab', 'checkusername'], stderr=subprocess.STDOUT).decode("utf-8") 
    if "Username is:" in res:
        return res.replace("Username is: ", "").replace("\n", "")
    else:
        print("[ERROR] Could not get username.")
        exit()

def build_crab_config(): 
    this_config = config()

    process_name = "DYJetsToLL_M-50_13TeV_AM"
    now = datetime.now().strftime(r"date_%Y_%m_%d_time_%H_%M_%S")

    this_config.General.requestName = process_name
    this_config.General.workArea = 'crab_nano_music_'+now
    this_config.General.transferOutputs = True 

    this_config.JobType.pluginName = 'Analysis'
    this_config.JobType.psetName = 'crab_music_pset.py'
    this_config.JobType.scriptExe = 'run_nano_music.sh'
    # this_config.JobType.scriptArgs = [r'configs/run_configs/MC_2017.toml']
    os.system("rm raw_config.toml ; cp ../../configs/run_configs/MC_2017.toml raw_config.toml")
    this_config.JobType.inputFiles   = ['task.tar.gz', 'raw_config.toml']


    this_config.Data.inputDataset = '/DYJetsToLL_M-50_TuneCP5_13TeV-amcatnloFXFX-pythia8/RunIISummer20UL17NanoAODv9-106X_mc2017_realistic_v9-v2/NANOAODSIM'
    this_config.Data.inputDBS = 'global'
    this_config.Data.splitting = 'FileBased'
    this_config.Data.unitsPerJob = 2
    this_config.Data.totalUnits = 5
    # this_config.Data.lumiMask = 'https://cms-service-dqmdc.web.cern.ch/CAF/certification/Collisions16/13TeV/Cert_271036-275783_13TeV_PromptReco_Collisions16_JSON.txt'
    # this_config.Data.runRange = '275776-275782'
    this_config.Data.publication = False
    this_config.Data.outputDatasetTag = process_name
    this_config.Data.outLFNDirBase  = '/store/user/'+get_username()+'/nano_music/'+this_config.General.workArea

    this_config.JobType.outputFiles = [
                                    'nano_music_DYJetsToLL_M-50_13TeV_AM_0.root',
                                    # f'outputs_{process_name}/nano_music_DYJetsToLL_M-50_13TeV_AM_0.classes'
                                    ]
    this_config.User.voGroup = 'dcms'
    this_config.Site.storageSite = "T2_DE_RWTH"

    return this_config

def submit(config):
        crabCommand('submit', config=config)

def build_task_tarball():
    print("Packing input files ...")
    os.system(r'rm task.tar.gz')
    os.system(r'tar --exclude="task.tar.gz" --exclude="CMSSW_*" --exclude="__pycache*" --exclude="build" --exclude="docs_BKP" --exclude="docs" --exclude="crab_nano_music_date_*" --exclude="NanoMUSiC/tools" --exclude="NanoMUSiC/PxlAnalyzer" --exclude="*.root" --exclude="NanoMUSiC/PlotLib" --exclude="NanoMUSiC/MUSiC-Configs" --exclude="NanoMUSiC/MUSiC-RoIScanner" --exclude="NanoMUSiC/MUSiC-Utils"  -zcvf task.tar.gz ../../*')
    print("")
 
def main():
    build_task_tarball()
    submit(build_crab_config())

if __name__ == "__main__":
    main()