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



#MUSiC #PxlAnalyzer #Masterarbeit #MUSiC-Configs 