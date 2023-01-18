- Rechte für diesen Ordner an alle geben: `chmod 777 .`
- nach jeder .cc Änderung in CMSSW einmal den Befehl: `scram build -j` ausführen
-  nach jeder .cc Änderung in PxlAnalyser einmal den Befehl: `make clean` ausführen und dann `make -j`
- lx3acms2 -> 128 Kerne

#### ParseSampleList
- `parseSampleList.py new_files.txt -c /home/home1/institut_3a/ykaiser/TAPAS_2022/CMSSW_10_6_20/src/PxlSkimmer/Skimming/test/mc_miniAOD_UL2018_cfg.py -e 13`

#### Data parse...
- `datasets.py name.txt`
- **WICHTIG:** datasets.py muss für jedes Datenset einzeln editiert werden

##### Datein von CMS lokal speichern
- `xrdcp root://cms-xrd-global.cern.ch//store/mc/RunIISummer20UL18MiniAODv2/WZTo1L1Nu2Q_4f_TuneCP5_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/106X_upgrade2018_realistic_v16_L1v1-v2/70000/D1312141-ED22-E345-99E3-E2798024286E.root .`

#####  Crosssection für PxlSkimmer selber berechnen:
- login to -> `ssh -Y ykaiser@lxplus.cern.ch`
- [https://twiki.cern.ch/twiki/bin/view/CMS/HowToGenXSecAnalyzer#Running_the_GenXSecAnalyzer_on_a](https://twiki.cern.ch/twiki/bin/view/CMS/HowToGenXSecAnalyzer#Running_the_GenXSecAnalyzer_on_a)
- **WICHTIG:** Nutze den richtigen Tag (_z.B. Summer20UL18_)
 
##### Login
- `voms-proxy-init --voms cms:/cms --valid 192:0`
- `voms-proxy-init --voms cms:/cms/dcms --valid 168:00`
- `voms-proxy-init --voms cms --voms cms:/cms/dcms --valid 168:00`
- `ssh -Y lx3acms2`
- `ssh -Y ykaiser@lxplus.cern.ch`


##### dcache
- Login:  `uberftp grid-ftp`
- Mount: `gfalFS -s ~/dcache_mount "srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/ykaiser"`
- Unmount: `gfalFS_umount ~/dcache_moun`

##### Crab
- Submitting (mit allen Samples - Dauer ca. 5 Tage + 2-3 extra Tage falls man resubmitten muss):
	- `music_crab3.py -g 106X_upgrade2018_realistic_v16_L1v1 mc_powheg.txt`
	- Exit Codes: https://twiki.cern.ch/twiki/bin/view/CMSPublic/JobExitCodes
- Lokal: 
	- `cmsRun /home/home1/institut_3a/ykaiser/TAPAS_2022/CMSSW_10_6_29/src/PxlSkimmer/Skimming/test/data_miniAOD_UL2018_cfg.py`
- Status überprüfen:
	- `crab status *Path*`
- Stoppen der failed/toRetry Jobs auf crab:
	- `crab kill -d *PathofthefailedJobs*`
- Sende die Ergebnisse zur Database (mit Watchdog):
	- `watchdog.py --debug DEBUG --addIncomplete -f --only FOLDER_TAG`

##### Luigi Task Visualiser
- `ssh -L 8082:lx3acms2:8082 lx3acms2`
- `ssh -J ykaiser@portal.physik.rwth-aachen.de -L 8082:lx3acms2:8082 ykaiser@lx3acms2`
- http://localhost:8082/static/visualiser/index.html
##### Start the Scheduler
- `cd AnaMSc/dependencies/TAPAS/MUSIC-Utils/python/luigi-music`
- `./daemon.sh restart`
- `./daemon.sh status     #checks if luigi is running`

##### Full Monecarlo ==Scan==:
- `ECWorkflows.py scan --mc /home/home1/institut_3a/ykaiser/TAPAS/Working/bg.root --data /home/home1/institut_3a/ykaiser/TAPAS/Working/data.root --distributions MET --class-types exclusive --campaign-id ECWoTest_Scan_26_10 --remote-dir /net/scratch_cms3a/ykaiser/working/ECWoTest_Scan_26_10/`
###### 2018
- `ECWorkflows.py classify --myskims-path /.automount/home/home__home1/institut_3a/silva/music/tapas/MUSiC-Configs/MC/myskims_ram_LOR_SKIM2_LG1_Jan22_test_felipe_2022_07_07.yaml --campaign-id EC_test_2022_07_12 --remote-dir /disk1/silva/music/classification_test_2022_07_11/EC_test_2022_07_13`


##### Full Montecarlo ==Classification==:
- `ECWorkflows.py classify --myskims-path /.automount/home/home__home1/institut_3a/ykaiser/TAPAS/MUSiC-Configs/MC/myskims_ram_test1.yaml --campaign-id ECWoTest_Classify_28_10 --remote-dir /net/scratch_cms3a/ykaiser/working/ECWoTest_Classify_28_10/ `

##### Merging the Classification Outputfiles:
- `ECMerger.py /disk1/roemer/MUSiC_Run/Data/12/Data/* --final --merge=all --data=True -o /user/roemer/mergedOutputs/data_2016_6_1.root`
- `ECMerger.py /disk1/ykaiser/working/Masterarbeit/Classification/SemiFULLClassification_09_08_2022_edited/mc/* --final --merge=all --data=False -o /disk1/ykaiser/working/Masterarbeit/Classification/SemiFULLClassification_09_08_2022_edited/mc_merged/bg.root -j 120`


##### Zeige Status im Grid:
- `condor_q`


##### Zeige gridpacks im d_cach:
- `srmls srm://grid-srm.physik.rwth-aachen.de:8443/srm/managerv2?SFN=/pnfs/physik.rwth-aachen.de/cms/store/user/ykaiser/luigi_gridpacks/`


##### ==Scan== one:
- `luigi_wrapper.sh scan --mc /net/scratch_cms3a/pook/MUSiC/MUSiCout/171229/bg.root --data /net/scratch_cms3a/pook/MUSiC/MUSiCout/171229/data.root --nrounds 10000`


##### Überprüfe den Status von scans oder classifications mit ==Television==:
- `television.py ECWoTest_FULL_4_11/Scan_MET_ECWoTest_FULL_4_11/Rec_1Muon_1MET_c703d9fc78c51842857d46efbb46930e64f618cf/`


##### ==Classify== one:
- `music /.automount/home/home__home1/institut_3a/ghosh/TAPAS/MUSiC-Configs/configs/MC.cfg dcap://grid-dcap-extern.physik.rwth-aachen.de/pnfs/physik.rwth-aachen.de/cms/store/user/saghosh/PxlSkim/CMSSW_8_0_v2.10/DiPhotonJetsBox_M40_80_13TeV_SP/DiPhotonJetsBox_M40_80-Sherpa/80X_mcRun2_asymptotic_2016_TrancheIV_v8/171114_165821/0000/DiPhotonJetsBox_M40_80_13TeV_SP_47.pxlio -x /.automount/home/home__home1/institut_3a/ghosh/TAPAS/MUSiC-Configs/MC/scales.txt -o test1`


##### Open a ROOT File:
- `rootM filename`
- `TBrowser h`
- `Exit with .q`


##### Linking File:
- `ln -s /net/scratch_cms3a/ykaiser/working/FULLClassification_20_11/bg.root bg.root`


##### ==Scan== Signal:
- Kopiere Timing z.B. `/timing/timing_exclusive_InvMass.json`
- Kopiere bg.root & data.root/signal.root
- Erstelle CAMPAIGNID_YEAR_MONTH_DAY Ordner (z.B. Scan_3000_19_09_2022_09_19)
- WPrime
	- `ECWorkflows.py scan --mc bg.root --signal signalWprime4000.root --distributions InvMass --class-types exclusive --campaign-id Scan_4000_2_12 --remote-dir /net/scratch_cms3a/ykaiser/working/Scan_4000_2_12/`
- Sphaleron
	- `ECWorkflows.py scan --mc /net/scratch_cms3a/ykaiser/working/Sphaleron/001/bg.root --signal /net/scratch_cms3a/ykaiser/working/Sphaleron/001/Output.root --distributions SumPt --class-types jet-inclusive --campaign-id Sphaleron001 --remote-dir /net/scratch_cms3a/ykaiser/working/Sphaleron/001/Scan/`

	- `ECWorkflows.py scan --mc /user/ykaiser/working/Sphaleron/005_2/bg.root --signal /user/ykaiser/working/Sphaleron/005_2/Output.root --distributions SumPt --class-types jet-inclusive --campaign-id Sphaleron005_new2 --remote-dir /user/ykaiser/working/Sphaleron/005_2/Scan`

##### Hash:
- `sdb.py --database Scan_4000_2_12_2020_12_02/scan_exclusive_InvMass.sqlite list`


##### Finde die Eventclasses mit dem geringsten p value(Hash vom Signal):
- `sdb.py --database Scan_4000_2_12_2020_12_02/scan_exclusive_InvMass.sqlite significant b4712dbaa15fc69da2acb19e17be75d2c22d924a`


##### P tilde ==Plot== (Grosse abweichung zeigt, da kann was sein und wenn der p Wert gering ist dann ist die Wahrscheinlichkeit gross, dass es nicht vom Background kommt):
- `ECFinalPlot.py -j 20 --scan-db-path WPRIME3000_2020_01_21/scan_exclusive_InvMass.sqlite --signal-hash 149e7d82b2603021298d48ebb408d70bc5fbc9c9 --mc-hash 41cb9d392a4cead31dc22c032dce1974d1a733f4 --out TestPlots --scale-bands --header-subtitle "Simulation Preliminary" --lumi 137000.0`

- `ECFinalPlot.py -j 40 --scan-db-path Scan_3000_4_12_2020_12_04/scan_exclusive_InvMass.sqlite --signal-hash fff0a681449da8feb546d8a3a345d4761c1e4b7e --mc-hash ddac0d3d110cfe3d2e418da052594b600ae30965 --out TestBA4 --scale-bands --header-subtitle "Simulation Private" --lumi 137000.0 --canvas-width 1600 --canvas-height 1400`


##### Ein Eventclass ==plotten==:
- `ECPlot_root.py  --filter-systematics "*/*qcdWeightGamma*" "*/*qcdWeightQCD*" "*/*xs*NLO" --mc bg.root --signal ../../Sensitvity_nov14_Wprime/WPRIME3000/WPRIME3000.root  -j 10 --filter Rec_1Muon_1MET --out TestPlots --detailed-groups strong --legend-include-yield False --ymin 0.001 --ymax 100000 --legend-number-columns 1 --header-subtitle " Simulation Preliminary" --text-size-scale-factor 1.4 --legend-label-uncert 'Bkg. uncert.' --ymin 0.01 --ymax 500000 --legend-number-columns 1 --legend-xlow 0.75 --legend-xup 0.92 --legend-yup 0.9  --canvas-width 1600 --canvas-height 1400`

#MUSiC #Commands 