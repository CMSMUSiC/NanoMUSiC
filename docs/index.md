# NanoMUSiC 

![music](https://raw.githubusercontent.com/CMSMUSiC/NanoMUSiC/main/docs/images/music_t.svg)   


**MUSiC - Model Unspecific Search in CMS**

📶 Run 2 - Ultra Legacy, using NanoAOD 📶

## About 

This repository stands on the shoulders of the work done by many brilliant students and scientists of III. Physikalisches Institut A und B (RWTH Aachen University), that along many years touched this code. Their contributions are mainly hosted in the legacy repositories below:

- https://gitlab.cern.ch/aachen-3a/tapas
- https://gitlab.cern.ch/aachen-3a/pxlanalyzer
- https://gitlab.cern.ch/aachen-3a/PxlSkimmer
- https://gitlab.cern.ch/MUSiC/MUSiC-EventClassFactory
- https://gitlab.cern.ch/MUSiC/MUSiC-Configs
- https://gitlab.cern.ch/MUSiC/MUSiC-Utils
- https://gitlab.cern.ch/MUSiC/MUSiC-RoIScanner
- https://gitlab.cern.ch/aachen-3a/lstar-analyzer/-/tree/ghosh_202107_updates

This repo is just a convolution of the algorithms and tools mentioned above, including:
- modernized C++ and Python code;
- GPU implementations of computing demanding procedures;
- simpler workflows and environment setups;
- adaptations to the NanoAOD format, supported by the CMS experiment.

### License

All the credit goes, to the authors of the legacy repositories.

The LICENSING inherits from them, also. When not this is not applicable, MIT license applies.


## The MUSiC algorithm in a nutsheld
### Kinematical distributions of interest

#### Sum pT: $\sum_{i}|\vec{p_{i}}_{T}|$

$$\sum_{i}|\vec{p_{i}}_{T}|$$

It is calculated taking into account the $|\vec{p}_{T}|$ of all object explicitly state in the 

**The Cauchy-Schwarz Inequality**

$$\left( \sum_{k=1}^n a_k b_k \right)^2 \leq \left( \sum_{k=1}^n a_k^2 \right) \left( \sum_{k=1}^n b_k^2 \right)$$




## Setup and instructiond

# Setup

The instructions here described are designed tested for the computing environment of RWTH-3A. In principle, it should be no problem to setup and run the analysis code at other sites, with CentOS7 and CVMFS mounted,e.g. `lxplus`.

## Dependencies

There are no dependencies on CMSSW. The needed softwares are provided by LCG views (view 102b - `/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/setup.sh`).

LCG list of available software: https://lcginfo.cern.ch/release_packages/x86_64-centos7-gcc11-opt/102b/

## Setup the environment (only once)

```
git clone git@github.com:CMSMUSiC/NanoMUSiC.git
cd NanoMUSiC
./bootstrap_music.sh
```

## Configuring the environment (for every new session)

```
source setenv.sh
```

## Compilation

```
cd build
```

If it does not exists, just create it (`mkdir build`). You can play around with this directory at will.


To configure and build (and install):

```
cmake ..
ninja install
```

Other options are:

- `ninja lut` &rarr; Only creates the p-values LUTs. Usually takes some time to run, but, in principle, one should do it only right before start a classification.
- `make_docs.sh` &rarr; Produces this documentation.
- `ninja clean` &rarr; Clear compilations. 

## Installations directories

- Executables: `bin/`  
- Shared libraries: `libs/`
- Useful scripts: `scripts/`

# Running the analysis

The nominal MUSiC analysis, following the approach of the Run2016 [arXiv:2010.02984](https://arxiv.org/abs/2010.02984), is divided in three stages:

1 - Skimming

Data and MC NanoAOD samples are skimmed in order to:
- Build a **per-event** class model stored in TTrees, with \(p_{T}\), \($m_{inv/T}$\), $MET$ and weights calculated for all systematics variations and classes.
- The skimmed samples, since is per-event based, can be used to explore other methods of p-value estimations.

2 - Classification

- Events sharing the same classes are merged and a Event Class Model (`TEventClass`) is build.
- Data and MC ROOT files are merged in a format compatible with the legacy MUSiC code.

3 - Scan

- Calculates p-values for integrated distributions.
- Perform the RoI (Region of Interest) Scan, per-class, per-distribution.
- Calculates the p-values for selected RoIs, as well as estimates the Look Elsewhere Effect (LEE).
- Estimates the p-value distribution, corrected by the LEE: p-tilde ($\tilde{p}$-value).

## Skim

### Run Config File

Inside `./config`, different run configuration files can by found.

#### Single process

To run a test/debug Skimming:

```
nano_music --run-config <path_to_config_file>
```
The possible options are:

- `--batch` (optional): will run in batch mode. No colors, pretty-printings or flushes.
- `--run-config <path_to_config_file>` (mandatory): a run config should be provided.




### Data Samples

https://docs.google.com/spreadsheets/d/1azJoopSLTqZSY_pcu9P0RvMbmbreHtEAUKvrwhd1870/edit?usp=sharing

## References

### NanoAOD file Content

Content of CMS NanoAOD files per year. Could have variariations from sample to sample.

Reference:
https://cms-nanoaod-integration.web.cern.ch/autoDoc/



### PDF recomendations

https://cms-pdmv.gitbook.io/project/mccontact/info-for-mc-production-for-ultra-legacy-campaigns-2016-2017-2018#sample-collection-through-mcm-and-monitoring-via-pmp

### HLT Bits

### 2017

```
1 = CaloIdL_TrackIdL_IsoVL
--  2 = 1e (WPTight)
--  4 = 1e (WPLoose)
--  8 = OverlapFilter PFTau
--  16 = 2e
--  32 = 1e-1mu
--  64 = 1e-1tau
--  128 = 3e
--  256 = 2e-1mu
--  512 = 1e-2mu
--  1024 = 1e (32_L1DoubleEG_AND_L1SingleEGOr)
--  2048 = 1e (CaloIdVT_GsfTrkIdT)
--  4096 = 1e (PFJet)
--  8192 = 1e (Photon175_OR_Photon200) for Electron (PixelMatched e/gamma)

 1 = TrkIsoVVL
--  2 = Iso
--  4 = OverlapFilter PFTau
--  8 = 1mu
--  16 = 2mu
--  32 = 1mu-1e
--  64 = 1mu-1tau
--  128 = 3mu
--  256 = 2mu-1e
--  512 = 1mu-2e
--  1024 = 1mu (Mu50)
--  2048 = 1mu (Mu100) for Muon

 1 = LooseChargedIso
--  2 = MediumChargedIso
--  4 = TightChargedIso
--  8 = TightID OOSC photons
--  16 = HPS
--  32 = single-tau + tau+MET
--  64 = di-tau
--  128 = e-tau
--  256 = mu-tau
--  512 = VBF+di-tau for Tau

 Jet bits: bit 0 for VBF cross-cleaned from loose iso PFTau
--  bit 1 for hltBTagCaloCSVp087Triple
--  bit 2 for hltDoubleCentralJet90
--  bit 3 for hltDoublePFCentralJetLooseID90
--  bit 4 for hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet
--  bit 5 for hltQuadCentralJet30
--  bit 6 for hltQuadPFCentralJetLooseID30
--  bit 7 for hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF or hltL1sQuadJetCIorTripleJetVBFIorHTT
--  bit 8 for hltQuadCentralJet45
--  bit 9 for hltQuadPFCentralJetLooseID45
--  bit 10  for hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet or hltL1sQuadJetC50to60IorHTT280to500IorHTT250to340QuadJet bit 11 for hltBTagCaloCSVp05Double or hltBTagCaloDeepCSVp17Double
--  bit 12 for hltPFCentralJetLooseIDQuad30
--  bit 13 for hlt1PFCentralJetLooseID75
--  bit 14 for hlt2PFCentralJetLooseID60
--  bit 15 for hlt3PFCentralJetLooseID45
--  bit 16 for hlt4PFCentralJetLooseID40
--  bit 17 for hltBTagPFCSVp070Triple or hltBTagPFDeepCSVp24Triple or hltBTagPFDeepCSV4p5Triple  for Jet

 HT bits: bit 0 for hltL1sTripleJetVBFIorHTTIorDoubleJetCIorSingleJet
--  bit 1 for hltL1sQuadJetC50IorQuadJetC60IorHTT280IorHTT300IorHTT320IorTripleJet846848VBFIorTripleJet887256VBFIorTripleJet927664VBF or hltL1sQuadJetCIorTripleJetVBFIorHTT
--  bit 2 for hltL1sQuadJetC60IorHTT380IorHTT280QuadJetIorHTT300QuadJet or hltL1sQuadJetC50to60IorHTT280to500IorHTT250to340QuadJet
--  bit 3 for hltCaloQuadJet30HT300 or hltCaloQuadJet30HT320
--  bit 4 for hltPFCentralJetsLooseIDQuad30HT300 or hltPFCentralJetsLooseIDQuad30HT330 for HT

 MHT bits: bit 0 for hltCaloQuadJet30HT300 or hltCaloQuadJet30HT320
--  bit 1 for hltPFCentralJetsLooseIDQuad30HT300 or hltPFCentralJetsLooseIDQuad30HT330 for MHT
```

#### Muons

```
hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p07 -> HLT_IsoMu27 - bit: 8
hltL3crIsoL1sSingleMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p07 -> HLT_IsoMu24 - bit: 8
hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q -> HLT_Mu50 - bit: 1024
hltL3fL1sMu22Or25L1f0L2f10QL3Filtered100Q -> HLT_OldMu100 - bit: 2048
hltL3fL1sMu25f0TkFiltered100Q -> HLT_OldMu100 - bit: 2048
```


### Run2 Generator Output

https://indico.cern.ch/event/459797/contributions/1961581/attachments/1181555/1800214/mcaod-Feb15-2016.pdf

### Electron Trigger Scale Factors

Run2 Electron Trigger Scale Factors were only provided in the form of a ROOT file. In order to avoid their inclusion in the git repo (it is a very bad pratice to add binary files to git repositories), they were dumped to HEX using `xxd`.

```
xxd -i root_file_name.root > foo.hpp
```

The `-i` option dumps the file content inthe form of a C/C++ library that can be included. The `CorrectionSets.hpp` have functions/methods to properly load those dumps into a `TMemFile`, which has the same interface of a regular file.

Documentation and references:

 - EGamma Run2 recommendations: https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations#E_gamma_IDs
 - EGamma trigger SF: https://twiki.cern.ch/twiki/bin/view/CMS/EgHLTScaleFactorMeasurements
 - Low Pt SFs: https://indico.cern.ch/event/1146225/contributions/4835158/attachments/2429997/4160813/HLTSFsWprime%20.pdf
 - High Pt SFs: https://indico.cern.ch/event/787353/#1-hlt-efficiency-measurement-f


# Notes on the Legacy Code (German)

#### Für PxlAnalyzer/ConfigFiles/Objects/ScaleFactors/ele_sf.cff
- UL18 (still using rereco):  https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations#HEEPv7_0_2018Prompt
- |eta| < 1.4442 (Barrel)
- 1.566 < |eta| < 2.5 (Endcap)


-> PythonScriptName: _create_root_histogram.py_



- File das erstellt werden soll:
	- https://gitlab.cern.ch/cms-muonPOG/muonefficiencies/-/blob/master/Run2/UL/2018/NUM_TrackerMuons_DEN_genTracks_Z_abseta_pt.json

-> PythonScriptName: _create_root_from_JSON.py_




### **PxlAnalyzer**
- ConfigFiles:
	- Objects:
		- EleID:
			- Ele_CBID* :
				- https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2#Cut_Based_Electron_ID_for_Run_2
			- Ele_HEEP_v70.cff: 
				- https://twiki.cern.ch/twiki/bin/view/CMS/HEEPElectronIdentificationRun2
		- Muon:
			- highpt.cff:
				- https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#HighPt_Muon
			- loose.cff:
				- https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#Loose_Muon 
			- medium.cff:
				- https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#Medium_Muon
			- soft.cff:
				- https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#Soft_Muon
			- tight.cff:
				- https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#Tight_Muon
		- jet.cff: 
			- https://twiki.cern.ch/twiki/bin/view/CMS/BtagRecommendation106XUL18
				- https://gitlab.cern.ch/cms-nanoAOD/jsonpog-integration/-/tree/master/POG/BTV/2018_UL
			- https://twiki.cern.ch/twiki/bin/viewauth/CMS/JECDataMC#Recommended_for_MC
				- https://github.com/cms-jet/JRDatabase/tree/master/textFiles/Summer19UL18_JRV2_MC
			- https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution#JER_Scaling_factors_and_Uncertai
		- fatjet.cff:
			- gleich wie jet.cff
			- PUPPI: https://github.com/cms-jet/PuppiSoftdropMassCorrections
		- Scale Factors: 
			- ele_sf.cff: 
				- https://twiki.cern.ch/twiki/bin/view/CMS/EgammaUL2016To2018#SFs_for_Electrons_UL_2018
				- **Temp** Ele.ScaleFactor.ID.HEEP.file: [[Create root file for electron ScaleFactor(HEEP)]]
			- gamma_sf.cff:
				- https://twiki.cern.ch/twiki/bin/view/CMS/EgammaUL2016To2018#SFs_for_Photons_UL_2018
					-> SeedVeto File is the "CSEV and pixel veto SFs" CSEV File information als neues Histogramm (.root file) erstellt
			- muon_sf.cff:
				- https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonUL2018#ISO_efficiencies
					- https://gitlab.cern.ch/cms-muonPOG/muonefficiencies/-/tree/master/Run2/UL/2018
				- Muon.ScaleFactor.Tracker.file: [[Create a root file from a JSON file]]
	- trigger.cff 
		- gleich wie bei MUSiC-Configs 
			- https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonUL2018#Trigger_efficiency_AN1 
			- https://twiki.cern.ch/twiki/bin/view/CMS/EgHLTRunIISummary#2018
	- non_particle.cff
		- https://hypernews.cern.ch/HyperNews/CMS/get/physics-validation/3689/1/1.html
- Main:
	- PDFTool.cc
		- https://lhapdfsets.web.cern.ch/current/
### **MUSiC-Configs**
- Data.cff & MC.cfg
	- Electron & Gamma: 
		- https://twiki.cern.ch/twiki/bin/view/CMS/EgHLTRunIISummary
	- Muon: 
		- https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonUL2018#Trigger_efficiency_AN1
	- Double Muon: 
		- https://twiki.cern.ch/twiki/bin/view/CMS/TopTrigger#Dilepton_triggers
	- JSON: 
		- https://twiki.cern.ch/twiki/bin/viewauth/CMS/DCUserPage
	- Flags: 
		- https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#2018_2017_data_and_MC_UL
- trigger.cff:
	- Electron & Gamma:
		- https://twiki.cern.ch/twiki/bin/view/CMS/EgHLTRunIISummary
- MC:
	- scales.txt:
		- Crossection:
			- Formel:  `sigma(final) = sigma(product) x Br(decay)`
			- https://twiki.cern.ch/twiki/bin/viewauth/CMS/StandardModelCrossSectionsat13TeV
			- TT: 
				- https://twiki.cern.ch/twiki/bin/view/LHCPhysics/TtbarNNLO 
				- https://pdglive.lbl.gov/Particle.action?node=Q007&init=0
			- Higgs Based (mH=125): 
				- https://twiki.cern.ch/twiki/bin/view/LHCPhysics/CERNYellowReportPageAt13TeV#VBF_Process
				- https://twiki.cern.ch/twiki/bin/view/LHCPhysics/CERNYellowReportPageBR#Higgs_2_gauge_bosons
		- FilterEff:
			- https://cms-pdmv.cern.ch/mcm/requests?prepid=TOP-RunIISummer20UL18wmLHEGEN-00003&shown=140737488355327
			- Dataset Namen eingeben -> auf Namen klicken -> auf All klicken -> Summer20UL18wmLHE -> drauf klicken -> Generator Parameters
		- kFator from Theory = 1.0
		- XSecOrder in den oben genannten Seiten
		- ProcessGroup: Gucke in alte scales.txt von ähnlichen Samples

### **PxlSkimmer**
- Skimming/python/Tools_miniAOD.py (Prefiring):
	- https://twiki.cern.ch/twiki/bin/view/CMS/L1PrefiringWeightRecipe#2018_UL
	- **WICHTIG:** Achte darauf den richtigen ***cms.InputTag*** zu nutzen (gucke in die letzten Jahre rein, in MUSiC sind die Namen leicht anders)


##### Vllt. Nützlich
- Ganz viele (v.Felipe): 
	- https://github.com/cms-sw/cmssw/tree/CMSSW_10_2_X/RecoEgamma/ElectronIdentification/python/Identification




1) **Alle MC & Data Samples auf [DAS](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2FZToMuMu_*%2FRunIISummer20UL18MiniAODv2*%2FMINIAODSIM*), [Grasp](https://cms-pdmv.cern.ch/grasp/samples?dataset_query=ZToMuMu_M-3500To4500_TuneCP5_13TeV-powheg-pythia8&campaign=Run3Winter22*GS,RunIISummer20UL16*GEN,RunIISummer20UL16*GENAPV,RunIISummer20UL17*GEN,RunIISummer20UL18*GEN) etc. suchen wie [hier](https://docs.google.com/spreadsheets/d/1C3wC3vG5VHEX0-bk-s6qdhGR0Hy7KcVLkG9efDYlN6Q/edit#gid=1654878574)**
------------------
2) **[[MUSiC einrichten]]**
------------------
- Immer alles **sourcen** und **VOMS** einrichten
	- `source MUSiC-StartUp.csh`
	- `voms-proxy-init --voms cms --voms cms:/cms/dcms --valid 198:0`
------------------
3) **Samples Skimmen:**
	- MC:
		- .txt mit allen DAS Namen erstellen
		- ParseSampleList benutzen
			- z.B.: `parseSampleList.py new_files.txt -c /home/home1/institut_3a/ykaiser/TAPAS_2022/CMSSW_10_6_20/src/PxlSkimmer/Skimming/test/mc_miniAOD_UL2018_cfg.py -e 13`
		- Benutze Crab:
			- z.B.: `music_crab3.py -g 106X_upgrade2018_realistic_v16_L1v1 mc_powheg.txt`
		- Bei Fails:
			- Status überprüfen:
			    - Lokal: `crab status *Path*`
			    - Global: [Grafana](https://monit-grafana.cern.ch/d/cmsTMDetail/cms-task-monitoring-task-view?orgId=11) 
			- [Exit Codes](https://twiki.cern.ch/twiki/bin/view/CMSPublic/JobExitCodes)
			- Resubmitten:
				- Single one:
					- z.B.: `crab resubmit crab_WJetsToLNu_Pt-250To400_MatchEWPDG20_13TeV_AM --maxmemory 4000 --maxjobruntime 2750`
				- Multiple:
					- `python ../resubmit_crab_jobs.py -d .`
			- Wenn Resubmitten nicht geht, dann Killen
				- Stoppen der failed/toRetry Jobs auf crab:
				    -   `crab kill -d *PathofthefailedJobs*` 
	- Data:
		- .txt mit allen DAS Namen erstellen
		- Data Parsen:
			-  `datasets.py name.txt`
			-   **WICHTIG:** datasets.py muss für jedes Datenset einzeln editiert werden
		- Benutze Crab:
			- `music_crab3.py -g 106X_dataRun2_v35 All_data.txt --maxmemory 4000 --maxJobRuntimeMin 2750 --eventsPerJob 78000`   
			- Rest wie bei MC
	- Samples zur [Database](https://cms-project-aachen3a-db.web.cern.ch/index) senden
		- Einzeln: `watchdog.py --debug DEBUG -f --addIncomplete --only FILENAME`
		- Alle im Ordner: `python ../multi_watchdog.py -d .`
		- **WICHTIG:** Manchmal entsteht ein Buffer Overflow, wenn das passiert einfach nochmal das Kommando nutzen (solange bis alle Samples einen Eintrag in der Database haben der nicht 0 ist)
------------------
4) **Classification:**
	- Kopiere den **timing** Ordner eines Vorherigen Nutzers an eine Stelle an dem du Zugriff drauf hast und editiere den PATH im Code
		- in `TAPAS/MUSiC-Utils/python/luigi_music/tasks/classification.py`
	- Erstelle einen Ordner in dem die Classification laufen soll
	- Erstelle eine data.root (`touch data.root`)
	- Erstelle einen Ordner (`mkdir NAME`) mit dem Namen der CampaignID_Datum die genutzt werden soll (Datum im Format Jahr_Monat_Tag)
	- Lösche vorhandene gridpacks im dcache zur Classification (siehe bei ERRORs)
	- Kopiere deine Cookies:
		- z.B.: /home/home1/institut_3a/ykaiser/TAPAS_2022/Condor_utils/restoring_sso_cookies.sh ~/private/CERN_UserCertificate.pem ~/private/CERN_UserCertificate.key
	- Starte die Classification:
		- z.B.: `ECWorkflows.py classify --myskims-path /.automount/home/home__home1/institut_3a/ykaiser/TAPAS_2022/TAPAS/MUSiC-Configs/MC/myskims_UL18_20_07_2022.yaml --campaign-id EC_test_2022_10_22 --remote-dir /disk1/ykaiser/working/Masterarbeit/Classification/SemiFULLClassification_22_10_2022`
	- mc Ordner mergen
		- z.B.: `ECMerger.py /disk1/ykaiser/working/Masterarbeit/Classification/SemiFULLClassification_09_08_2022_edited/mc/* --final --merge=all --data=False -o /disk1/ykaiser/working/Masterarbeit/Classification/SemiFULLClassification_09_08_2022_edited/mc_merged/bg.root -j 120`
	- ==**Überprüfung der Classification:**==
		- Television `television.py mc/*/*`
			- **WICHTIG:** Television funktioniert nur, wenn das Terminalfenster groß genug gezogen wurde, daher am besten nur im Vollbildfenstermodus ausprobieren
			- Bei der Passwortfrage einfach Enter drücken
		- [Luigi Task Visualiser](http://localhost:8082/static/visualiser/index.html)
			- Unirechner: `ssh -L 8082:lx3acms2:8082 lx3acms2`
			- Heimrechner: `ssh -J ykaiser@portal.physik.rwth-aachen.de -L 8082:lx3acms2:8082 ykaiser@lx3acms2`
		- `condor_q`
	- ==**Bei ERRORS oder Stuck:**==
		- Wenn die Classification schon einiges klassifiziert hat und nur einzelne Gruppen Failen:
			- Stoppen mittels `Strg+alt gr+ß`oder`Strg+C` oder `Strg+Z` (bei +Z mit `ps` gucken ob der Prozess noch läuft und mit `kill -9 NUMMER` killen)
			- `condor_rm USERNAME` anwenden
			- dcache Gridpacks der Classification clearen
				- z.B.: zu finden mittels: `uberftp grid-ftp`
					- `cd /pnfs/physik.rwth-aachen.de/cms/store/user/DEINUSERNAME/luigi_gridpacks`
					- `ls`
			- **ACHTUNG**: Im gleichen Ordner werden auch die Gridpacks des Scans gespeichert und mittels uberftp hat man ADMIN Rechte und könnte alle Daten löschen (auch von anderen Usern) und es gibt kein BACKUP
			- Gucken ob beim Luigi Task Visualiser alle Disabled Tasks weg sind (ca. 5 min nach dem stoppen des Kommandos und dem löschen in condor sollten die Tasks von alleine verschwinden)
			- Lokale Ordner der gefailten Gruppen löschen (in `mc/..`)
			- Classification Kommando erneut nutzen
		- Wenn die Classification zu Beginn Failed dann Fehler suchen und in einem **NEUEM** Ordner alles nochmal starten
------------------
5) **Scan:**
	- Erstelle einen Ordner in dem du den Scan laufen lassen willst
	- Kopiere den **timing** Ordner in den Scan Ordner
	- Kopiere oder **besser Linke** die `bg.root` Datei aus dem Classification `mc_merged` Ordner in den Scan Ordner (von MC `bg.root` und Data `bg_data.root`)
		- z.B.:  `ln -s /disk1/ykaiser/working/Masterarbeit/Classification/SemiFULLClassification_09_08_2022_edited/mc_merged/bg.root bg.root`
	- Erstelle einen Ordner (`mkdir NAME`) mit dem Namen der CampaignID_Datum die genutzt werden soll (Datum im Format Jahr_Monat_Tag)
	- Starte den Scan:
		- `ECWorkflows.py scan --mc bg.root --signal bg_3000.root --distributions InvMass --class-types exclusive --campaign-id Scan_30001_19_09 --remote-dir /disk1/ykaiser/working/Masterarbeit/Scan/test_04_19_09_2022_good`
	- ==**Überprüfung des Scans:**==
		- z.B.: `television.py Scan_InvMass_Scan_30001_31_08/*`
			- **WICHTIG:** Television funktioniert nur, wenn das Terminalfenster groß genug gezogen wurde, daher am besten nur im Vollbildfenstermodus ausprobieren
			- Bei der Passwortfrage einfach Enter drücken
		- [Luigi Task Visualiser](http://localhost:8082/static/visualiser/index.html)
			- Unirechner: `ssh -L 8082:lx3acms2:8082 lx3acms2`
			- Heimrechner: `ssh -J ykaiser@portal.physik.rwth-aachen.de -L 8082:lx3acms2:8082 ykaiser@lx3acms2`
		- `condor_q`
	- ==**Bei ERRORs oder Stuck:**==
		- Wenn einige Sachen schon Erfolgreich gescannt wurden:
			- Stoppen mittels `Strg+alt gr+ß`oder`Strg+C` oder `Strg+Z` (bei +Z mit `ps` gucken ob der Prozess noch läuft und mit `kill -9 NUMMER` killen)
			- `condor_rm USERNAME` anwenden
			- dcache Gridpacks des Scans clearen
				- z.B.: zu finden mittels: `uberftp grid-ftp`
					- `cd /pnfs/physik.rwth-aachen.de/cms/store/user/DEINUSERNAME/luigi_gridpacks`
					- `ls`
			- **ACHTUNG**: Im gleichen Ordner werden auch die Gridpacks der Classification gespeichert und mittels uberftp hat man ADMIN Rechte und könnte alle Daten löschen (auch von anderen Usern) und es gibt kein BACKUP
			- Gucken ob beim Luigi Task Visualiser alle Disabled Tasks weg sind (ca. 5 min nach dem stoppen des Kommandos und dem löschen in condor sollten die Tasks von alleine verschwinden)
			- Lokale Ordner der gefailten Gruppen löschen (im CampaignID-Ordner)
				- Tipp:
					- `python Scan_check.py -d test_04_19_09_2022_good/Scan_InvMass_Scan_30001_19_09`
					- `python delete_scan.py -d Scan_InvMass_Scan_30001_31_08`
						- das Lösch-Script löscht manchmal einen gefailten Ordner nicht, daher am Ende sicherheitshalber den Scancheck nochmal laufen lassen und den übrig gebliebenen Ordner in der Failes***.txt per Hand löschen
			- Scan Kommando erneut nutzen


### MUSiC installieren
- https://gitlab.cern.ch/MUSiC/MUSiC-Utils/-/wikis/Basic-Setup
	- https://twiki.cern.ch/twiki/bin/view/CMS/MUSiCBasicSetupRunII

### PxlSkimmer installieren
- https://docs.google.com/document/d/1ixJxFBO7rzhzigSjoxY14dLzW_smf27jvkye9HWlGBI/edit

### Git SSH
- https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account

### Zertifikate einrichten
- Cern Zertifikat: https://ca.cern.ch/ca/
- MUSIC Specific Path and names for CERN Certificate
	- https://docs.google.com/document/d/1NG-7iVxS9sIRzNiTieAovg-TiwJJgJHmusIt6LBEQ0Y/edit
- Cern Zertifikat mappen: https://resources.web.cern.ch/resources/Manage/Accounts/MapCertificate.aspx

### VOMS
- https://gridka-ca.kit.edu/
- https://voms2.cern.ch:8443/voms/cms/
- https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookStartingGrid#ObtainingCert

### HyperNews
- https://hypernews.cern.ch/HyperNews/CMS/add-member.pl

### Analyse Vorbereitung:
- Files die Geupdatet werden müssen
	- [[Links für die PxlAnalyser & MUSiC-Config config Files]]
	- [Felipes Notizen](https://codimd.web.cern.ch/4hoSkAGbS1ClQuCpnLIZ4A?view)

### Zusatz:
- [[Wichtige MUSiC Internetseiten]]



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


### Wichtige Seiten:
- [TAPAS](https://gitlab.cern.ch/aachen-3a/tapas/-/blob/master/README.md)
- [PxlAnalyser](https://gitlab.cern.ch/aachen-3a/pxlanalyzer/-/tree/dev_UltraLegacy)
- [PxlSkimmer](https://gitlab.cern.ch/aachen-3a/PxlSkimmer/-/tree/CMSSW_10_6_v5p9_2017)
- [MUSiC-Configs](https://gitlab.cern.ch/MUSiC/MUSiC-Configs/-/tree/dev_UL18)
- [MUSiC-Utils](https://gitlab.cern.ch/MUSiC/MUSiC-Utils/-/tree/dev_UltraLegacy)
- [MUSiC-RoIScanner](https://gitlab.cern.ch/MUSiC/MUSiC-RoIScanner/-/tree/Yannik_20201022_MUSiCfix)
- [Mattermost](https://mattermost.web.cern.ch)
- [TWiki](https://twiki.cern.ch/twiki/bin/view/CMS/WebHome)
- [DAS](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2FZToMuMu_*%2FRunIISummer20UL18MiniAODv2*%2FMINIAODSIM*)
- [GrASP](https://cms-pdmv.cern.ch/grasp/)
- [McM](https://cms-pdmv.cern.ch/mcm/requests?dataset_name=WZTo1L*&prepid=*wmLHEGEN*&page=0&shown=127)
- [Grafana](https://monit-grafana.cern.ch/d/15468761344/personal-tasks-monitoring-globalview?from=now-30d&to=now&orgId=11&var-user=ykaiser&var-site=All&var-current_url=%2Fd%2FcmsTMDetail%2Fcms_task_monitoring&var-task=All)
- [CMS-Cric](https://cms-cric.cern.ch/accounts/account/list/)
- [CMS-Talk](https://cms-talk.web.cern.ch/)
- [Database](https://cms-project-aachen3a-db.web.cern.ch/)

