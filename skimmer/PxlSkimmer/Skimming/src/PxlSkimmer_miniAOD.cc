// -*- C++ -*-
// Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>
//
// Package:    PxlSkimmer
// Class:      PxlSkimmer_miniAOD
//
// \class PxlSkimmer_miniAOD PxlSkimmer/Skimming/src/PxlSkimmer_miniAOD.cc
//
// Description: Data and MC Skimmer for the Model Unspecific Search in CMS
//
// Implementation:
//
//
// Original Authors: Carsten Hof, Philipp Biallass, Holger Pieta, Paul Papacz
//         Created:  Mo Okt 30 12:03:52 CET 2006
// $Id$
//
//
// Own header file.
#include "PxlSkimmer/Skimming/interface/PxlSkimmer_miniAOD.h"
#include "PxlSkimmer/Skimming/interface/IsolationComputer.h"
#include "PxlSkimmer/Skimming/interface/MultiIsolation.h"

// System include files.
#include <iostream>
#include <algorithm>    // std::set_intersection, std::set_difference

// Message Logger for Debug etc.
#include "FWCore/MessageLogger/interface/MessageLogger.h"

// Exceptions. Do *not* use edm::LogError(), use cms::Exception() instead!
#include "FWCore/Utilities/interface/Exception.h"

// Necessary objects.
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

// For GenParticles.
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/METReco/interface/GenMET.h"
#include "DataFormats/METReco/interface/GenMETCollection.h"
#include "SimDataFormats/GeneratorProducts/interface/LesHouches.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
//#include "GeneratorInterface/LHEInterface/interface/LHEEvent.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

// Special stuff for sim truth of converted photons.
#include "SimDataFormats/Track/interface/SimTrackContainer.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"
#include "SimDataFormats/Vertex/interface/SimVertexContainer.h"

// PDF stuff.
#include "SimDataFormats/GeneratorProducts/interface/PdfInfo.h"

// PAT related stuff.
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Tau.h"

// EGamma stuff.
#include "EgammaAnalysis/ElectronTools/interface/PFIsolationEstimator.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EgammaCandidates/interface/ConversionFwd.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrackFwd.h"
// this is for the ID selection
// #include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
// #include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"

// For Muon stuff.
#include "DataFormats/MuonReco/interface/MuonCocktails.h"
#include "DataFormats/MuonReco/interface/MuonIsolation.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "RecoVertex/KalmanVertexFit/interface/KalmanVertexFitter.h"
#include "RecoVertex/VertexTools/interface/InvariantMassFromVertex.h"
#include "TrackingTools/Records/interface/TransientTrackRecord.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "TrackingTools/TransientTrack/interface/TransientTrackBuilder.h"


// Jet stuff.
#include "SimDataFormats/JetMatching/interface/JetFlavourMatching.h"
#include "PhysicsTools/SelectorUtils/interface/Selector.h"
#include "PhysicsTools/SelectorUtils/interface/strbitset.h"
// #include "DataFormats/JetReco/interface/CATopJetTagInfo.h"
#include "DataFormats/BTauReco/interface/CATopJetTagInfo.h"

// Tau stuff

#include "DataFormats/PatCandidates/interface/TauPFEssential.h"

// MET stuff.
#include "DataFormats/METReco/interface/PFMETCollection.h"

// For Trigger Bits:
#include "DataFormats/Common/interface/TriggerResults.h"

// Misc.
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
// #include "DataFormats/Common/interface/ValueMap.h"

// Math stuff from Physics tools.
#include "DataFormats/Math/interface/deltaR.h"

// Private ParticleMatcher
#include "PxlSkimmer/Skimming/interface/ParticleMatcher.h"

//
// constructors and destructor
//
PxlSkimmer_miniAOD::PxlSkimmer_miniAOD(edm::ParameterSet const &iConfig) :
    FileName_(iConfig.getUntrackedParameter< string >("FileName")),
    fastSim_(iConfig.getParameter<bool>("FastSim")),
    // Photon Isolations
    multiIsolation_(iConfig.getParameter<edm::ParameterSet>("isolationDefinitions"),consumesCollector())
    {
    // now  do what ever initialization is needed
    pxl::Core::initialize();
    fePaxFile = new pxl::OutputFile(FileName_);
    fePaxFile->setCompressionMode(6);

    // Get Physics process
    Process_ = iConfig.getUntrackedParameter<string>("Process");
    // Get dataset name.
    Dataset_ = iConfig.getUntrackedParameter<string>("Dataset");
    // Gen-Only or also Rec-information
    GenOnly_ = iConfig.getUntrackedParameter<bool>("GenOnly");
    // Use SIM info
    UseSIM_ = iConfig.getUntrackedParameter<bool>("UseSIM");
    // Current pdfsets for the Pdf set mapping
    PdfSetFileName_ = iConfig.getParameter<edm::FileInPath>("PdfSetFileName");
    // The labels used in cfg-file
    genParticleCandidatesLabel_   = iConfig.getParameter<edm::InputTag>("genParticleCandidatesLabel");
    //consumes(std::vector<reco::GenParticle>,genParticleCandidatesLabel_);

    // Get run year
    Year_ = iConfig.getUntrackedParameter<int>("Year");

    VertexRecoLabel_              = iConfig.getUntrackedParameter<string>("VertexRecoLabel");
    consumes<reco::VertexCollection>(VertexRecoLabel_);

    // Get the main particle lables
    patMuonLabel_                = iConfig.getParameter<edm::InputTag>("patMuonLabel");
    consumes<std::vector<pat::Muon> >(patMuonLabel_);
    patElectronLabel_            = iConfig.getParameter<edm::InputTag>("patElectronLabel");
    consumes<vector< pat::Electron > >(patElectronLabel_);
    // used in analyzeRecElectrons
    consumes<edm::View<pat::Electron>  >(patElectronLabel_);
    patGammaLabel_               = iConfig.getParameter<edm::InputTag>("patGammaLabel");
    consumes<vector< pat::Photon > >(patGammaLabel_);
    consumes< edm::View<pat::Photon> >(patGammaLabel_);
    patTauTag_                   = iConfig.getParameter<edm::InputTag>("patTauTag");
    consumes<pat::TauCollection>(patTauTag_);
    patTauBoostedTag_                   = iConfig.getParameter<edm::InputTag>("patTauBoostedTag");
    consumes<pat::TauCollection>(patTauBoostedTag_);
    // LOR TRY TO REMOVE. MOVE TO DeepTau. FOR CMSSW106X
    mvaIsolationToken_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1raw"));
    mvaIsolationTokenNewDM_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1rawNewDM"));
    mvaIsolationVLooseToken_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1VLoose"));
    mvaIsolationLooseToken_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1Loose"));
    mvaIsolationMediumToken_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1Medium"));
    mvaIsolationTightToken_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1Tight"));
    mvaIsolationVTightToken_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1VTight"));
    mvaIsolationVVTightToken_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1VVTight"));
    mvaIsolationVLooseTokenNewDM_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1VLooseNewDM"));
    mvaIsolationLooseTokenNewDM_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1LooseNewDM"));
    mvaIsolationMediumTokenNewDM_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1MediumNewDM"));
    mvaIsolationTightTokenNewDM_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1TightNewDM"));
    mvaIsolationVTightTokenNewDM_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1VTightNewDM"));
    mvaIsolationVVTightTokenNewDM_ = consumes<pat::PATTauDiscriminator>(edm::InputTag("rerunDiscriminationByIsolationMVArun2v1VVTightNewDM"));
   
    patMETTag_                   = iConfig.getParameter<edm::InputTag>("patMETTag");
    consumes<pat::METCollection>(patMETTag_);
    // used in analyzeRecPatMET
    consumes< edm::View<pat::MET> >(patMETTag_);
    PUPPIMETTag_                 = iConfig.getParameter<edm::InputTag>("PUPPIMETTag");
    consumes<pat::METCollection>(PUPPIMETTag_);
    consumes< edm::View<pat::MET> >(PUPPIMETTag_);
    patMETEleCorrTag_            = iConfig.getParameter<edm::InputTag>("patMETEleCorrTag");
    consumes<pat::METCollection>(patMETEleCorrTag_);
    consumes< edm::View<pat::MET> >(patMETEleCorrTag_);
    patPFCandiates_              = iConfig.getParameter<edm::InputTag>("patPFCandiates");
    consumes<pat::PackedCandidateCollection>(patPFCandiates_);

    genEvtInfoToken_             = consumes<GenEventInfoProduct>(edm::InputTag("generator"));

    consumes< reco::BeamSpot >( edm::InputTag("offlineBeamSpot") );
    // additonal collections
    //reducedSuperClusterCollection_ = iConfig.getParameter<edm::InputTag>("reducedSuperClusterCollection");
    //reducedEBClusterCollection_    = iConfig.getParameter<edm::InputTag>("reducedEBClusterCollection");


    conversionsTag_          = iConfig.getParameter<edm::InputTag>("conversionsTag");
    consumes<reco::ConversionCollection>(conversionsTag_);
    //conversionsSingleLegTag_ = iConfig.getParameter<edm::InputTag>("conversionsSingleLegTag");

    rhos_                    = iConfig.getParameter<VInputTag>("rhos");

    for (VInputTag::const_iterator rhos_label = rhos_.begin(); rhos_label != rhos_.end(); ++rhos_label) {
        consumes<double>(*rhos_label);
    }

    METFilter_ = consumes<edm::TriggerResults>(iConfig.getParameter< edm::InputTag >("METFilterTag"));
    METFilterAlternative_ = consumes<edm::TriggerResults>(iConfig.getParameter< edm::InputTag >("METFilterAlternativeTag"));
    ecalBadCalibFilterUpdate_= consumes<bool>(edm::InputTag("ecalBadCalibReducedMINIAODFilter"));

    prefweight_token = consumes< double >(edm::InputTag("prefiringweight:nonPrefiringProb"));
    prefweightup_token = consumes< double >(edm::InputTag("prefiringweight:nonPrefiringProbUp"));
    prefweightdown_token = consumes< double >(edm::InputTag("prefiringweight:nonPrefiringProbDown"));

    genParticleCandidatesToken_= consumes< std::vector<reco::GenParticle> >(genParticleCandidatesLabel_);

    puInfoToken_ = consumes<std::vector< PileupSummaryInfo > >(edm::InputTag("slimmedAddPileupInfo"));
    lheProducerToken = consumes< LHERunInfoProduct, edm::InRun >(edm::InputTag("externalLHEProducer"));
    consumes< LHEEventProduct >(edm::InputTag("externalLHEProducer"));


    // get the PSet that contains all jet PSets
    edm::ParameterSet jets_pset = iConfig.getParameter< edm::ParameterSet >("jets");
    // get the names of all sub-PSets
    vector< string > jet_names;
    jets_pset.getParameterSetNames(jet_names);
    // loop over the names of the jet PSets
    for (vector< string >::const_iterator jet_name = jet_names.begin(); jet_name != jet_names.end(); ++jet_name) {
        jet_def jet;
        jet.name = *jet_name;

        edm::ParameterSet one_jet = jets_pset.getParameter< edm::ParameterSet >(jet.name);
        jet.MCToken   = mayConsume<reco::GenJetCollection>(one_jet.getParameter< edm::InputTag >("MCLabel"));
        jet.RecoToken = consumes< vector<pat::Jet> >(one_jet.getParameter< edm::InputTag >("RecoLabel"));
        vector< string > id_names = one_jet.getParameter< vector< string > >("IDs");
        jet.isPF = one_jet.getParameter< bool >("isPF");
        if(one_jet.getParameter< edm ::InputTag >("MCLabel").label()==""){
            jet.recoOnly=true;
        }else{
            jet.recoOnly=false;
        }

        for (auto& id_name : id_names)
        {
            edm::ParameterSet jet_id_cfg;
            jet_id_cfg.addParameter("version", one_jet.getParameter<std::string>("version"));
            jet_id_cfg.addParameter("quality", id_name);
            pair< std::string, ::Selector<pat::Jet>* > ID(id_name, new PFJetIDSelectionFunctor(jet_id_cfg));
            jet.IDs.push_back(ID);
        }

        jet_infos.push_back(jet);
    }

    // Triggers
    // -------
    // all settings / variables / etc related to trigger
    triggerBits_ = consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("bits"));
    triggerPrescales_ = consumes<pat::PackedTriggerPrescales>(iConfig.getParameter<edm::InputTag>("prescales"));

    // Filters
    // -------
    // This is based on the triggers handling from above because the information
    // from filters that ran are accessed with help of the edm::TriggerResults.
    // Basically it is foreseen (but not used atm.) to use more than one filter combination.
    //
    edm::ParameterSet filter_pset = iConfig.getParameter< edm::ParameterSet >("filters");

    vector< string > filter_paths;
    filter_pset.getParameterSetNames(filter_paths);

    for (vector< string >::const_iterator filter_path = filter_paths.begin(); filter_path != filter_paths.end(); ++filter_path) {
        trigger_group filter;
        filter.name = *filter_path;

        edm::ParameterSet one_filter = filter_pset.getParameter< edm::ParameterSet >(filter.name);
        filter.process = one_filter.getParameter< string >("process");

        filter.results = edm::InputTag(one_filter.getParameter< string >("results"), "");
        consumes<edm::TriggerResults>(filter.results);

        vstring const tmp_paths = one_filter.getParameter< vstring >("paths");
        filter.triggers_names = sstring(tmp_paths.begin(), tmp_paths.end());

        filters.push_back(filter);
    }


    // cuts
    edm::ParameterSet cut_pset = iConfig.getParameter< edm::ParameterSet >("cuts");
    min_tau_pt = cut_pset.getParameter< double >("min_tau_pt");
    min_muon_pt = cut_pset.getParameter< double >("min_muon_pt");
    min_ele_pt = cut_pset.getParameter< double >("min_ele_pt");
    min_gamma_pt = cut_pset.getParameter< double >("min_gamma_pt");
    min_jet_pt = cut_pset.getParameter< double >("min_jet_pt");
    min_met = cut_pset.getParameter< double >("min_met");
    max_eta = cut_pset.getParameter< double >("max_eta");
    vertex_minNDOF = cut_pset.getParameter< double >("vertex_minNDOF");
    vertex_maxZ = cut_pset.getParameter< double >("vertex_maxZ");
    vertex_maxR = cut_pset.getParameter< double >("vertex_maxR");
    PV_minNDOF = cut_pset.getParameter< double >("PV_minNDOF");
    PV_maxZ = cut_pset.getParameter< double >("PV_maxZ");
    PV_maxR = cut_pset.getParameter< double >("PV_maxR");

    // Initialise isolators.
    // Alternative way to compute PF based isolation for photons and electrons.
    // m_eleIsolator = new PFIsolationEstimator();
    // m_eleIsolator->setConeSize(0.3);
    // m_eleIsolator->initializeElectronIsolation(kTRUE);

    // m_phoIsolator = new PFIsolationEstimator();
    // m_phoIsolator->setConeSize(0.3);
    // m_phoIsolator->initializePhotonIsolation(kTRUE);

    //  //!!!!!!!!!!!!!!!Not in mini AOD at the moment!!!!
    // PU corrected isolation for electrons, according to:
    // https:// twiki.cern.ch/twiki/bin/view/CMS/EgammaPFBasedIsolation#Example_for_photons
    // http:// cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/EGamma/EGammaAnalysisTools/test/ElectronIsoAnalyzer.cc
    // if     (m_eleEffAreaTargetLabel == "NoCorr"     ) m_eleEffAreaTarget = ElectronEffectiveArea::kEleEANoCorr;
    // else if (m_eleEffAreaTargetLabel == "Data2011"   ) m_eleEffAreaTarget = ElectronEffectiveArea::kEleEAData2011;
    // else if (m_eleEffAreaTargetLabel == "Data2012"   ) m_eleEffAreaTarget = ElectronEffectiveArea::kEleEAData2012;
    // else if (m_eleEffAreaTargetLabel == "Summer11MC") m_eleEffAreaTarget = ElectronEffectiveArea::kEleEASummer11MC;
    // else if (m_eleEffAreaTargetLabel == "Fall11MC"   ) m_eleEffAreaTarget = ElectronEffectiveArea::kEleEAFall11MC;
    // else throw cms::Exception("Configuration") << "Unknown effective area " << m_eleEffAreaTargetLabel << endl;

    // m_eleEffAreaType = ElectronEffectiveArea::kEleGammaAndNeutralHadronIso03;

    Matcher = new ParticleMatcher();
    fNumEvt = 0;

    ElectronHcalHelper::Configuration hcalCfg;

    hcalCfg.hOverEConeSize = 0.15;
    hcalCfg.useTowers      = true;
    // hcalCfg.hcalTowers     = edm::InputTag("towerMaker");  // old way to access the collection
    hcalCfg.hcalTowers     = consumes< CaloTowerCollection >(edm::InputTag("towerMaker"));
    hcalCfg.hOverEPtMin    = 0;

    // m_hcalHelper = new ElectronHcalHelper(hcalCfg);
    }

// ------------ MIS Destructor  ------------

PxlSkimmer_miniAOD::~PxlSkimmer_miniAOD() {
    // delete m_eleIsolator;
    // delete m_phoIsolator;
    // do anything here that needs to be done at desctruction time
    // (e.g. close files, deallocate resources etc.)
    delete Matcher;
}

// ------------ method called when starting to processes a run ------------

void
PxlSkimmer_miniAOD::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{
    /// In the begin of the run, we have to save all the pdfset relevant informations, in case our sample is NLO, so we can calculate the uncertainties later
    /// First, we create the list of pdf set identifyers which are used to determine the pdfset used during the production of the MC sample

    //std::cout <<"Init beginRun" << std::endl;
    edm::Handle<LHERunInfoProduct> run;
    typedef std::vector<LHERunInfoProduct::Header>::const_iterator headers_const_iterator;
    
    // consumes< edm::Handle<LHERunInfoProduct> >(run);
    
    //The line below GENERATES A WARNING/ERROR when the you run the PxlSkimmer!!!!!!!!
    //Ideally you should not take care about the warning as explained in: 
    //https://twiki.cern.ch/twiki/bin/view/CMS/GeneratorFAQ#How_to_read_the_LHERunInfoProduc 
    iRun.getByLabel( "externalLHEProducer", run );
    // iRun.getByToken(lhe_handle_, run );
    //std::cout <<"Ho fatto getByLabel(externalLHEProducer)" << std::endl;

    if (run.isValid()) {//Begin of LHE PDF stuff preparing
      //std::cout <<"Sono dentro: run.isValid()" << std::endl;
        LHERunInfoProduct myLHERunInfoProduct = *(run.product());
        vector<int> pdf_set_numbers;
        vector<std::string> pdf_set_names;
        ///Read them from the input file, people are welcome to improve this (make it work on the grid? :) )
        ifstream infile(PdfSetFileName_.fullPath(),ios::in);
        //int number=0;  //Variable to hold each number as it is read

        int start = 0;
        int length = 0;

        while(!infile.eof()) // reads file to end of *file*, not line
            {
            std::string line;
            while (std::getline(infile,line)){
                start = line.find(" ");
                length = line.find("\n")-start+1;
                all_lhapdf_pdf_sets.push_back(atoi(line.substr(0,start).c_str()));
                all_lhapdf_pdf_sets_names.push_back(line.substr(start+1,length).c_str());
            }
	}

       //Close the file stream
        infile.close();

        for (unsigned int i=0; i< all_lhapdf_pdf_sets.size();i++){
            if (i==0) continue;
            //if (all_lhapdf_pdf_sets[i-1] == all_lhapdf_pdf_sets[i] -1) continue;
            Range temp_range(all_lhapdf_pdf_sets[i-1], all_lhapdf_pdf_sets[i] -1); ///make sure that no id appears twice, so substract one from the upper threshold
            range_map_lhapdf.insert(std::pair<Range,std::string>(temp_range, all_lhapdf_pdf_sets_names[i-1])); ///the name of the pdfset is the one from the lower bound
        }

        std::string xmltag_start = "<";
        std::string xmltag_end = ">";
        std::string id_delimeter = "id=";
        std::string scale_muR_delimeter = "muR=";
        std::string scale_muF_delimeter = "muF=";
        std::string pdf_delimeter ="";
        std::string scale_weight_identifier = "scale_variation";

        bool alternative_xmltags = false;
        bool alternative_scale_weight_identifier = false;

        // Get Madgraph version
        for (headers_const_iterator iter=myLHERunInfoProduct.headers_begin(); iter!=myLHERunInfoProduct.headers_end(); iter++){
            for (auto& line : iter->lines()){
                if (line.find("&lt;") != std::string::npos){
                    alternative_xmltags = true;
                }
                if (line.find("Central scale variation") != std::string::npos){
                    alternative_scale_weight_identifier = true;
                }
            }
        }

        if (alternative_xmltags)
        {
            xmltag_start = "&lt;";
            xmltag_end = "&gt;";
        }
        if (alternative_scale_weight_identifier)
        {
            scale_weight_identifier = "Central scale variation";
        }

        std::string endofline_delimeter = xmltag_start + "/weight" + xmltag_end;

        // Function to parse a vlue inside quotation marks after a given identifier
        // e.g. a line containing MUR="0.5" can be parsed by passising MUR= as identifier
        const auto parse_value_in_quotation_marks = [] (const std::string& str, const std::string& identifier){
            const auto start = str.find(identifier);
            const auto first_quotation_mark = str.find("\"", start) + 1;
            const auto second_quotation_mark = str.find("\"", first_quotation_mark);
            if (start == std::string::npos or
                first_quotation_mark == std::string::npos or
                second_quotation_mark == std::string::npos)
                throw cms::Exception("Parsing Error") << "Cannot parse the string " << str << " for the value of " << identifier << std::endl;
            const auto result = str.substr(first_quotation_mark, second_quotation_mark - first_quotation_mark);
            return result;
        };

        const auto ltrim = [] (std::string s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [] (int ch) {
                return !std::isspace(ch);
            }));
            return s;
        };

        for (headers_const_iterator iter=myLHERunInfoProduct.headers_begin(); iter!=myLHERunInfoProduct.headers_end(); iter++){
            std::vector<std::string> lines = iter->lines();

            bool reading_scale = false;
            bool reading_pdfvar = false;

            int current_id = 0;
            int current_pdf_set= 0;
            string muR = "";
            string muF = "";
            separator = ";";

            for (const auto& line : lines) {
                TString Tline(line);
                ///After getting the line, we set the corresponding bools to the let the skimmer know what to save
                // Madgraph 26 uses another identifier
                if (Tline.Contains(xmltag_start + "weightgroup"))
                {
                    if (Tline.Contains("envelope") && Tline.Contains(scale_weight_identifier)) {
                        reading_scale = true;
                        continue;
                    }
                    // for madgraph26 no common tag. Only write NNPDF
                    if ((Tline.Contains("PDF_variation") or Tline.Contains("NNPDF")) and scale_ids.size() > 0){
                        reading_pdfvar = true;
                        continue;
                    }
                }
                ///This is the way every group ends
                if (Tline.Contains(xmltag_start + "/weightgroup" + xmltag_end)){
                    // Madgraph 26 has an endgroup directly after the start of the scale variations
                    // Also the nominal value mur=muf=1 is written before the start of the group
                    if (alternative_scale_weight_identifier and reading_scale and scale_ids.size() == 0)
                        continue;
                    reading_scale = false;
                    reading_pdfvar = false;
                    continue;
                }
                if (reading_scale){
                    ///Very nice and well-arragned parsing of strings in C++ (first we wind the start of the value by finding the delimeter and adding the delimeter length, then we calculate the length,
                    ///The values are parsed and pushed into corresponding vectors
                    ///The needed information is store in global vectors, scale_ids, all_muR, all_muF, pdf_ids and pdf_sets which will be later used in each event
                    current_id = atoi(parse_value_in_quotation_marks(line, id_delimeter).c_str());
                    scale_ids.push_back(current_id);

                    if (Tline.Contains("MUR")){
		      //std::cout <<"Tline Contains MUR. Tline is :" << Tline << std::endl;
		      //string a_muR;
		      //string a_muF;
			//a_muR=parse_value_in_quotation_marks(line, "MUR") + separator;
			//a_muF=parse_value_in_quotation_marks(line, "MUF") + separator;
			//std::cout <<"The value of a_MUR is :" << a_muR << std::endl;
			//std::cout <<"The value of a_MUF is :" << a_muF << std::endl;
                        all_muR.append(parse_value_in_quotation_marks(line, "MUR") + separator);
                        all_muF.append(parse_value_in_quotation_marks(line, "MUF") + separator);
                    }
                    else {
                        if (Tline.Contains("renscfact="))
                        {
                            scale_muR_delimeter = "renscfact=";
                            scale_muF_delimeter = "facscfact=";
                        }
                        start = line.find(scale_muR_delimeter);
                        length = line.find(scale_muF_delimeter)-start - scale_muR_delimeter.size()-1;
                        muR = line.substr(start+scale_muR_delimeter.size(), length);
                        all_muR.append(muR+separator);

                        start  = line.find(scale_muF_delimeter);
                        length = line.find(endofline_delimeter)-start - scale_muF_delimeter.size()-1;
                        muF = line.substr(start+scale_muF_delimeter.size(), length);
                        all_muF.append(muF+separator);
			//std::cout <<"Tline    DO NOT    Contains MUR. Tline is :" << Tline << std::endl;
			//std::cout <<"   muF   is :  " << muF << std::endl;
			//std::cout <<"   muR   is :  " << muR << std::endl;
                    }
                }
                if (reading_pdfvar){
                    int current_id = atoi(parse_value_in_quotation_marks(line, id_delimeter).c_str());
                    pdf_ids.push_back(current_id);
                    if(Tline.Contains("PDF set = ")){
                        pdf_delimeter = "PDF set = ";
                    }
                    if(Tline.Contains("pdfset=")){
                        pdf_delimeter = "pdfset=";
                    }
                    if(Tline.Contains("PDF=")){
                        pdf_delimeter = "PDF=";
                    }
                    if(Tline.Contains("lhapdf=")){
                        pdf_delimeter = "lhapdf=";
                    }
                    if (Tline.Contains("PDF=\"")) {
                        current_pdf_set = atoi(parse_value_in_quotation_marks(line, "PDF=").c_str());
                    }
                    else {
                        start  = line.find(pdf_delimeter);
                        auto substr = ltrim(line.substr(start+pdf_delimeter.size()));
                        current_pdf_set = atoi(substr.substr(0, substr.find(" ")).c_str());
                    }
                    pdf_sets.push_back(current_pdf_set);
                }
            }
        }
    }
    //std::cout <<"End of beginRun" << std::endl;
}


string PxlSkimmer_miniAOD::find_inmap(int item, const ranges_type& ranges)
{
  for (auto& it: ranges){
      if(it.first.fits_in(item)){
          return it.second;
    }
  }
  std::cout <<"Range not found, auauaua !" << std::endl;
  return "";
}

void
PxlSkimmer_miniAOD::endRun(edm::Run const&, edm::EventSetup const&)
{
    scale_ids.clear();
    pdf_ids.clear();
    all_lhapdf_pdf_sets.clear();
    range_map_lhapdf.clear();
    all_lhapdf_pdf_sets_names.clear();
    pdf_sets.clear();
    //std::cout << "endRun Done" << endl;
}



// ------------ method called to for each event  ------------

void PxlSkimmer_miniAOD::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    edm::LogInfo("PxlSkimmer_miniAOD|EventInfo") << "Run " << iEvent.id().run()
                                                   << ", EventID = " << iEvent.id().event()
                                                   << ", is MC = " << !iEvent.isRealData();

    // set event counter
    fNumEvt++;
    // Owner of all Pxl Objects
    pxl::Event event;

    // event-specific data
    bool IsMC =  !iEvent.isRealData();
    event.setUserRecord("MC", IsMC);  // distinguish between MC and data
    event.setUserRecord("Run", iEvent.run());
    event.setUserRecord("LumiSection", iEvent.luminosityBlock());
    event.setUserRecord("EventNum", static_cast<uint64_t>(iEvent.id().event()));
    event.setUserRecord("BX", iEvent.bunchCrossing());
    event.setUserRecord("Orbit", iEvent.orbitNumber());
    event.setUserRecord("Dataset", Dataset_);

    pxl::EventView* RecEvtView = event.create<pxl::EventView>();
    event.setIndex("Rec", RecEvtView);
    pxl::EventView* GenEvtView = event.create<pxl::EventView>();
    event.setIndex("Gen", GenEvtView);
    pxl::EventView* TrigEvtView = event.create<pxl::EventView>();
    event.setIndex("Trig", TrigEvtView);
    pxl::EventView* FilterEvtView = event.create<pxl::EventView>();
    event.setIndex("Filter", FilterEvtView);
    GenEvtView->setName("Gen");
    RecEvtView->setName("Rec");
    TrigEvtView->setName("Trig");
    FilterEvtView->setName("Filter");

    GenEvtView->setUserRecord("Type", (string) "Gen");
    RecEvtView->setUserRecord("Type", (string) "Rec");
    TrigEvtView->setUserRecord("Type", (string) "Trig");
    FilterEvtView->setUserRecord("Type", (string) "Filter");

    // maps for matching
    std::map< const reco::Candidate*, pxl::Particle* > genmap;
    std::map< const reco::Candidate*, pxl::Particle* > genjetmap;

    // set process name
    GenEvtView->setUserRecord("Process", Process_);
    RecEvtView->setUserRecord("Process", Process_);

    // Isolation Functor for miniIsolation and multiIsolation
    multiIsolation_.init(iEvent);

    // Generator stuff
    if (IsMC) {
        analyzeGenRelatedInfo(iEvent, GenEvtView);  // PDFInfo, Process ID, scale, pthat
        analyzeGenInfo(iEvent, GenEvtView, genmap);
        for (vector< jet_def >::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info) {
            if(not jet_info->recoOnly){
                analyzeGenJets(iEvent, GenEvtView, genjetmap, *jet_info);
            }
        }
        analyzeGenMET(iEvent, GenEvtView);

        if (UseSIM_) {
            analyzeSIM(iEvent, GenEvtView);
        }
    }
    // store Rec Objects only if requested
    if (!GenOnly_) {
        // We need the PFCandidates later for isolation computation, so get it here once
        // per event!
        edm::Handle< pat::PackedCandidateCollection > pfCandidates;
        iEvent.getByLabel(patPFCandiates_,pfCandidates);

        // Same for the vertices.
        edm::Handle< reco::VertexCollection > vertices;
        iEvent.getByLabel(VertexRecoLabel_, vertices);

        // Median pt per area for each event.
        // See also:
        // https:// twiki.cern.ch/twiki/bin/view/CMS/EgammaEARhoCorrection#Rho_for_2011_Effective_Areas
        // https:// twiki.cern.ch/twiki/bin/view/CMS/Vgamma2011PhotonID#Recommended_cuts
        //
        // get all rhos from the miniAOD

        vector< double > rhos;
        double rhoFixedGrid = 0.;
        for (VInputTag::const_iterator rho_label = rhos_.begin(); rho_label != rhos_.end(); ++rho_label) {
            edm::Handle< double > rho;
            iEvent.getByLabel(*rho_label, rho);
            RecEvtView->setUserRecord(rho_label->label(), *rho);
            if (rho_label->label() == "fixedGridRhoFastjetAll") rhoFixedGrid = *rho;
            rhos.push_back(*rho);
        }

        double rho25 = rhos[0];


        // Trigger bits
        if (analyzeTrigger(iEvent, TrigEvtView) == false) {
          return;
        }

        // Filters more info see above.
        //
        for (vector< trigger_group >::iterator filt = filters.begin(); filt != filters.end(); ++filt) {
            analyzeFilter(iEvent, iSetup, FilterEvtView, *filt);
        }

        analyseMETFilter(iEvent,FilterEvtView);

        // Reconstructed stuff
        analyzeRecVertices(iEvent, RecEvtView);
        analyzeRecTaus(iEvent, RecEvtView);
        analyzeRecMuons(iEvent, iSetup, RecEvtView, IsMC, genmap, vertices->at(0));
        analyzeRecElectrons(iEvent, RecEvtView, IsMC, genmap, vertices, pfCandidates, rhoFixedGrid);
        for (vector< jet_def >::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info) {
            analyzeRecJets(iEvent, RecEvtView, IsMC, genjetmap, *jet_info);
        }

        analyzeRecMETs(iEvent, RecEvtView);

        analyzeRecGammas(iEvent, RecEvtView, IsMC, genmap, vertices, pfCandidates, rho25);

        if (Year_ != 2018) {
            analyzePrefiringWeights(iEvent, RecEvtView);
        }
    }

    if (IsMC && !GenOnly_) {
        const string met_name = "MET";
        Matcher->matchObjects(GenEvtView, RecEvtView, jet_infos, met_name);
    }

    printEventContent(GenEvtView, RecEvtView, IsMC);

    fePaxFile->writeEvent(&event);
}




// ------------ reading Generator related Stuff ------------

void PxlSkimmer_miniAOD::analyzeGenRelatedInfo(const edm::Event& iEvent, pxl::EventView* EvtView) {

    /// Get the PDF weights for calculating uncertainties in NLO samples and store them in the userrecord

    edm::Handle<LHEEventProduct> lheInfoHandel;
    iEvent.getByLabel("externalLHEProducer" , lheInfoHandel);

    if (lheInfoHandel.isValid()) {//Begin of LHE PDF storing code
        ///We have to take care of the fact that in the weights vector, there are at first the scale variation weights and then the pdf variation weights.
        ///We put the scale weigths into a string (since the userrecord will contain them all as a string anyway)
        ///For pdf weigths, we deal with a vector because we have to sort the weights into the pdf groups using the nice map we created in beginRun()

        std::string scale_weights;
        std::vector< double > pdf_weights;
        std::vector<std::string> pdf_weights_id;
        std::map<int,int> pdf_ids_to_pdf_sets;
        std::map<int,double> weight_ids_to_weights;
        std::map<int, double> pdf_sets_to_weights;

        ///We want to store the pdf ids mapped to the pdf sets
        for (unsigned int i = 0; i < pdf_sets.size(); i++){
            pdf_ids_to_pdf_sets.insert(std::pair<int, int> (pdf_ids[i],pdf_sets[i]));
        }

        for (unsigned int i=0; i<lheInfoHandel->weights().size();i++){
            if (std::stoi(lheInfoHandel->weights()[i].id) >= scale_ids[0] and std::stoi(lheInfoHandel->weights()[i].id) <= scale_ids[scale_ids.size()-1]){
                scale_weights.append(std::to_string(lheInfoHandel->weights()[i].wgt)+separator);
            }else {
                pdf_weights.push_back(lheInfoHandel->weights()[i].wgt);
                pdf_weights_id.push_back(lheInfoHandel->weights()[i].id);
                /// We like to store the weight ID mapped to the values of the weights
                weight_ids_to_weights.emplace( std::stoi( lheInfoHandel->weights()[i].id ), lheInfoHandel->weights()[i].wgt );
            }
        }

        ///Comparison between the two maps to get only those weights that are connected to a PDF set -> stored in a third map
        for (auto const pdf_iter : pdf_ids_to_pdf_sets) {
            for (auto const weights_iter : weight_ids_to_weights){
                if (pdf_iter.first == weights_iter.first){
                    pdf_sets_to_weights.emplace(pdf_iter.second,weights_iter.second);
                    weight_ids_to_weights.erase(weights_iter.first);
                    break;
                }
            }
        }

	//std::cout << " scale_weights are :    " << scale_weights << endl;
	//for (unsigned int i = 0; i < pdf_weights.size(); i++){
	//    std::cout << " pdf_weights at position :    " << i  <<  "  is    " << pdf_weights[i] << endl;
	//
	//}
	
        EvtView->setUserRecord("scale_variation_n",scale_ids.size());
        EvtView->setUserRecord("scale_variation",scale_weights);
        EvtView->setUserRecord("muF",all_muF);
        EvtView->setUserRecord("muR",all_muR);

        ///We will store the pdf weight information in this map with <pdfsetname,weight;weight;weight;...>
        std::map< std::string, std::string > all_pdfsets_to_weights;

        std::string pdf_set_name = "";

        bool ignore_pdf_weights = false;
        if (pdf_sets.size() != pdf_sets_to_weights.size()) {
            EvtView->setUserRecord("Incomplete_PDF_weights",true);
            ignore_pdf_weights = true;
        }

        if( !ignore_pdf_weights )
        {
            for (auto const iter : pdf_sets_to_weights){
                pdf_set_name = find_inmap(iter.first,range_map_lhapdf);
                if(all_pdfsets_to_weights.find(pdf_set_name)==all_pdfsets_to_weights.end()){
                    ///If the key (pdf set name) does not exists, we create it
                    all_pdfsets_to_weights.emplace(pdf_set_name,std::to_string(iter.second)+separator);
                }else{
                    ///If the key (pdf set name) exists, we just append the next weight to it
                    all_pdfsets_to_weights.find(pdf_set_name)->second.append(std::to_string(iter.second)+separator);
                }
            }
        }
        ///Finally, run over the map and write the key and the corresponding value into the userrecord
        for(auto const mapiterator : all_pdfsets_to_weights) {
	  //std::cout << mapiterator.first << std::endl;
	  //std::cout << mapiterator.second << std::endl;
            EvtView->setUserRecord(mapiterator.first,mapiterator.second);
        }
    }//End of LHE PDF storing code



    // this works at least for RECO. Need to check if this works on AOD or PAT-Ntuplee

    edm::Handle< GenEventInfoProduct > genEvtInfo;
    iEvent.getByToken(genEvtInfoToken_, genEvtInfo);

    // LOR ADDING central weight information for alpha_s weight. See https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideDataFormatGeneratorInterface#Using_other_products_AODSIM
    std::vector<double> evtWeightsVct = genEvtInfo->weights();
    double originalXWGTUP = evtWeightsVct.at(1);
    EvtView->setUserRecord("originalXWGTUP", originalXWGTUP);
    //END LOR ADDING




    // if the sample is binned, there should be a binning value. so save it, otherwise just save a 0
    EvtView->setUserRecord("binScale", genEvtInfo->hasBinningValues() ? genEvtInfo->binningValues()[0] : 0);

    EvtView->setUserRecord("Weight", genEvtInfo->weight());

    
    unsigned int ID = genEvtInfo->signalProcessID();
    EvtView->setUserRecord("processID", ID);

    // don't save PDF infos for processes without partons
    if (genEvtInfo->hasPDF() && !(91 <= ID && ID <= 95)) {
        const gen::PdfInfo *pdf = genEvtInfo->pdf();
        fpdf_vec.push_back(*pdf);

        int id1 = pdf->id.first;
        int id2 = pdf->id.second;

        // reset the code for a gluon, at least SHERPA got a problem there
        if (abs(id1) == 9 || abs(id1) == 21) {
            id1 = 0;
            fpdf_vec.back().id.first = 0;
        }
        if (abs(id2) == 9 || abs(id2) == 21) {
            id2 = 0;
            fpdf_vec.back().id.second = 0;
        }

        EvtView->setUserRecord("x1", pdf->x.first);
        EvtView->setUserRecord("x2", pdf->x.second);
        EvtView->setUserRecord("Q", pdf->scalePDF);
        EvtView->setUserRecord("f1", id1);
        EvtView->setUserRecord("f2", id2);
        EvtView->setUserRecord("pdf1", pdf->xPDF.first);
        EvtView->setUserRecord("pdf2", pdf->xPDF.second);


        if (abs(id1) > 6 || abs(id2) > 6) {
            throw cms::Exception("PDF error") << "PDF information corrupted in a non-diffractive event." << endl
                                              << "Process ID " << genEvtInfo->signalProcessID() << " is not in list of diffractive processes (91 <= ID <= 95)." << endl
                                              << "Scale: " << pdf->scalePDF << endl
                                              << "x1 = " << pdf->x.first << "; x2 = " << pdf->x.second << endl
                                              << "ID 1: " << id1 << endl
                                              << "ID 2: " << id2 << endl;
        }
    } else {
        gen::PdfInfo pdf;
        pdf.scalePDF = 0;

        fpdf_vec.push_back(pdf);
    }

    stringstream info;
    info << "Event Scale (i.e. pthat) = "
         << (genEvtInfo->hasBinningValues() ? genEvtInfo->binningValues()[0] : 0)
         << ", EventWeight = " << genEvtInfo->weight() << endl;
    if (genEvtInfo->hasPDF()) {
        info << "PDFInfo: " << endl
             << "========" << endl;
        info << "Momentum of first incoming parton: (id/flavour = "
             << genEvtInfo->pdf()->id.first  << ") "
             << genEvtInfo->pdf()->x.first  << endl
             << "Momentum of second incoming parton: (id/flavour = "
             << genEvtInfo->pdf()->id.second << ") "
             << genEvtInfo->pdf()->x.second << endl
             << "Scale = " << genEvtInfo->pdf()->scalePDF << endl;
    } else {
        info << "No PDFInfo in this event." << endl;
    }
    edm::LogVerbatim("PxlSkimmer_miniAOD|PDFInfo") << info.str();
}


// ------------ reading the Generator Stuff ------------

void PxlSkimmer_miniAOD::analyzeGenInfo(const edm::Event& iEvent,
                                          pxl::EventView* EvtView,
                                          std::map< const reco::Candidate*,
                                          pxl::Particle* >& genmap) {
    // LHE INFO for binned samples
    edm::Handle<LHEEventProduct> lheInfoHandel;
    iEvent.getByLabel("externalLHEProducer" , lheInfoHandel);

    if (lheInfoHandel.isValid()) {
        lhef::HEPEUP lheParticleInfo = lheInfoHandel->hepeup();
        // get the five vector
        // (Px, Py, Pz, E and M in GeV)
        std::vector<lhef::HEPEUP::FiveVector> allParticles = lheParticleInfo.PUP;
        std::vector<int> statusCodes = lheParticleInfo.ISTUP;

        double ht = 0;
        for (unsigned int i = 0; i < statusCodes.size(); i++) {
            if (statusCodes[i] == 1) {
                if (abs(lheParticleInfo.IDUP[i]) < 11 || abs(lheParticleInfo.IDUP[i]) > 16 || abs(lheParticleInfo.IDUP[i]) > 22) {
                    ht += sqrt(pow(allParticles[i][0], 2) + pow(allParticles[i][1], 2));
                }
            }
        }
        EvtView->setUserRecord("genHT", ht);
    }

    // gen particles
    edm::Handle<reco::GenParticleCollection> genParticleHandel;
    iEvent.getByToken(genParticleCandidatesToken_, genParticleHandel);


    const reco::GenParticle* p = (const reco::GenParticle*) &(*genParticleHandel->begin());  // this is the incoming proton
    pxl::Vertex* GenVtx = EvtView->create<pxl::Vertex>();
    GenVtx->setName("PV");
    // mind that clearly the following line crashes in case of ParticleGun RelVal like single photon
    // therefore add a protection :-) C.H. 20.04.09
    if (p->daughter(0) != 0) {
        GenVtx->setXYZ(p->daughter(0)->vx(), p->daughter(0)->vy(), p->daughter(0)->vz());  // need daughter since first particle (proton) has position zero
    } else {
        GenVtx->setXYZ(p->vx(), p->vy(), p->vz());  // if we do not have pp collisions
    }
    EvtView->setUserRecord("NumVertices", 1);

    int numTauMC = 0;
    int numMuonMC = 0;
    int numEleMC = 0;
    int numGammaMC = 0;
    int GenId = 0;
    double BeamEnergy = 0.;

    // save mother of stable particle
    const reco::GenParticle* p_mother;
    // const reco::GenParticle* p_mother_used;
    std::map< const reco::Candidate*, pxl::Particle* > genMatchMap;
    // loop over all particles
    for (reco::GenParticleCollection::const_iterator pa = genParticleHandel->begin(); pa != genParticleHandel->end(); ++pa) {
        // cast iterator into GenParticleCandidate
        const reco::GenParticle* p = (const reco::GenParticle*) &(*pa);

        // the following is interesting for GEN studies
        if (fabs(p->pdgId()) == 2212 && fabs(p->pz()) > BeamEnergy) {
            BeamEnergy = fabs(p->pz());
        }

        // find the two partons from the pdf or the mother conected to them
        if (p->numberOfMothers() > 0
            || fabs(fabs(p->pz())-EvtView->getUserRecord("x1").toDouble()*BeamEnergy) < 0.1
            || fabs(fabs(p->pz())-EvtView->getUserRecord("x2").toDouble()*BeamEnergy) < 0.1) {
            // look for the rest of the mothers
            vector<const reco::GenParticle*> mothers = runGenDecayTree(p, genMatchMap);
            if (mothers.size() == 0 && !(genMatchMap.size() < 2)) {
                continue;
            }
            p_mother = (const reco::GenParticle*) p->mother(0);
            if (p_mother!= 0 && p_mother->pdgId() == p->pdgId()) {
                continue;
            }

            pxl::Particle* part = EvtView->create< pxl::Particle >();
            genMatchMap[p] = part;
            part->setName("gen");
            part->setUserRecord("Accepted", false);
            part->setP4(p->px(), p->py(), p->pz(), p->energy());
            int p_id = p->pdgId();
            part->setPdgNumber(p_id);
            part->setUserRecord("Status", p->status());
            part->setUserRecord("isPromptFinalState", p->isPromptFinalState());
            part->setUserRecord("isPromptDecayed", p->isPromptDecayed());
            part->setUserRecord("isDirectPromptTauDecayProductFinalState", p->isDirectPromptTauDecayProductFinalState());
            reco::GenStatusFlags gsf = p->statusFlags();
            part->setUserRecord("isTauDecayProduct", gsf.isTauDecayProduct());
            part->setUserRecord("isPromptTauDecayProduct", gsf.isPromptTauDecayProduct());
            part->setUserRecord("isDirectTauDecayProduct", gsf.isDirectTauDecayProduct());
            part->setUserRecord("isDirectPromptTauDecayProduct", gsf.isDirectPromptTauDecayProduct());
            part->setUserRecord("isDirectHadronDecayProduct", gsf.isDirectHadronDecayProduct());

            // if there are more than 2 mothers the event is still fine, but it is not viewable in tree view of pxl!!
            for (size_t imother = 0; imother < mothers.size(); imother++) {
                part->linkMother(genMatchMap[mothers[imother]]);
            }
        }

        // fill Gen Muons passing some basic cuts
        if (abs((p)->pdgId()) == 13) {
            if (MuonMC_cuts(p)) {
                // set a soft link if the particle is already stored
                if (genMatchMap.end() == genMatchMap.find(p)) {
                    continue;
                }
                genmap[p] = genMatchMap[p];  // fill genmap
                genMatchMap[p]->setUserRecord("Accepted", true);
                genMatchMap[p]->setCharge(p->charge());
                genMatchMap[p]->setUserRecord("Vtx_X", p->vx());
                genMatchMap[p]->setUserRecord("Vtx_Y", p->vy());
                genMatchMap[p]->setUserRecord("Vtx_Z", p->vz());

                numMuonMC++;
            }
        }

        // fill Gen Electrons passing some basic cuts
        if (abs(p->pdgId()) == 11) {
            if (EleMC_cuts(p)) {
                // set a soft link if the particle is already stored
                if (genMatchMap.end() == genMatchMap.find(p)) {
                    continue;
                }
                genmap[p] = genMatchMap[p];  // fill genmap
                genMatchMap[p]->setUserRecord("Accepted", true);
                genMatchMap[p]->setCharge(p->charge());
                genMatchMap[p]->setUserRecord("Vtx_X", p->vx());
                genMatchMap[p]->setUserRecord("Vtx_Y", p->vy());
                genMatchMap[p]->setUserRecord("Vtx_Z", p->vz());
                // set a soft link if the particle is already stored
                numEleMC++;
            }
        }

        // fill Gen Gammas passing some basic cuts
        if (abs(p->pdgId()) == 22) {
            if (GammaMC_cuts(p)) {
                // set a soft link if the particle is already stored
                if (genMatchMap.end() == genMatchMap.find(p)) {
                    continue;
                }
                genmap[p] = genMatchMap[p];  // fill genmap
                genMatchMap[p]->setUserRecord("Accepted", true);
                genMatchMap[p]->setCharge(0);
                numGammaMC++;
            }
        }

        // fill Gen Taus passing some basic cuts -> status? What kind of taus can be found?
        if (abs(p->pdgId()) == 15 && TauMC_cuts(p)) {
            // check whether the tau is final or radiates a tau
            bool isfinal = true;
            for (reco::GenParticle::const_iterator daughter = p->begin(); daughter != p->end(); ++daughter) {
                if (abs(daughter->pdgId()) == 15) {
                    isfinal = false;
                    break;
                }
            }
            if (isfinal) {
                genmap[ p ] = genMatchMap[p];  // fill genmap
                genMatchMap[p]->setUserRecord("Accepted", true);
                genMatchMap[p]->setCharge(p->charge());
                genMatchMap[p]->setUserRecord("Vtx_X", p->vx());
                genMatchMap[p]->setUserRecord("Vtx_Y", p->vy());
                genMatchMap[p]->setUserRecord("Vtx_Z", p->vz());
                numTauMC++;
            }
        }

        GenId++;
    }  // end of loop over generated particles


    edm::LogInfo("PxlSkimmer_miniAOD|GenInfo") << "MC Found: " << numMuonMC <<  " muon(s), " << numEleMC << " electron(s), " << numGammaMC << " gamma(s)";
    EvtView->setUserRecord("NumMuon", numMuonMC);
    EvtView->setUserRecord("NumEle", numEleMC);
    EvtView->setUserRecord("NumTau", numTauMC);
    EvtView->setUserRecord("NumGamma", numGammaMC);

    // take care of the pile-up in the event
    //
    edm::Handle< std::vector< PileupSummaryInfo > >  PUInfo;

    iEvent.getByLabel(edm::InputTag("slimmedAddPileupInfo"), PUInfo);

    vector< PileupSummaryInfo >::const_iterator PUiter;

    // loop over all PU info object in an event and get the number of
    // primary vertices for in-time and out-of-time pile-up
    // See also:
    // https:// twiki.cern.ch/twiki/bin/view/CMS/Pileup_2011_Reweighting
    //
    for (PUiter = PUInfo->begin(); PUiter != PUInfo->end(); ++PUiter) {
        int BX      = (*PUiter).getBunchCrossing();
        int num     = (*PUiter).getPU_NumInteractions();

        if (BX == -1) {
            EvtView->setUserRecord("NumVerticesPULastBX", num);
        } else if (BX == 0) {
            EvtView->setUserRecord("NumVerticesPU", num);
            // The true number of interactions (i.e., the mean used in the Poisson
            // distribution) should be the same for in-time and out-of-time
            // pile-up as the actual number is drawn from the same Poisson distribution.
            //
            EvtView->setUserRecord("NumVerticesPUTrue", (*PUiter).getTrueNumInteractions());
        } else if (BX == 1) {
            EvtView->setUserRecord("NumVerticesPUNextBX", num);
        }
    }
}

// ------------ reading the Generator Jets ------------

void PxlSkimmer_miniAOD::analyzeGenJets(const edm::Event &iEvent, pxl::EventView *EvtView, std::map< const reco::Candidate*, pxl::Particle* > &genjetmap, const jet_def &jet_info) {
    // Get the GenJet collections
    edm::Handle<reco::GenJetCollection> GenJets;
    iEvent.getByToken(jet_info.MCToken, GenJets);

    // get the flavours
    // edm::Handle< reco::JetFlavourMatchingCollection > algoFlavour, physicsFlavour;
    // iEvent.getByLabel(jet_info.name+"GenJetFlavourAlgo", algoFlavour);
    // iEvent.getByLabel(jet_info.name+"GenJetFlavourPhysics", physicsFlavour);


    // counter
    size_t jet_index = 0;
    int numJetMC = 0;
    // double constit_pT = 5.;  // here we have a hardcoded cut, but do we really need cfg-parameter for this?...
    // Loop over GenJets
    for (reco::GenJetCollection::const_iterator genJet = GenJets->begin(); genJet != GenJets->end(); ++genJet, jet_index++) {
        if (JetMC_cuts(genJet)) {
            // get the reference
            edm::RefToBase< reco::Jet > jetRef(edm::RefToBaseProd< reco::Jet >(GenJets), jet_index);

            pxl::Particle *part = EvtView->create< pxl::Particle >();

            // cast iterator into GenParticleCandidate
            const reco::GenParticle *p = dynamic_cast< const reco::GenParticle* >(&(*genJet));
            genjetmap[p] = part;
            part->setName(jet_info.name);
            part->setP4(genJet->px(), genJet->py(), genJet->pz(), genJet->energy());
            // fill additional jet-related infos
            part->setUserRecord("EmE", genJet->emEnergy());
            part->setUserRecord("HadE", genJet->hadEnergy());
            part->setUserRecord("InvE", genJet->invisibleEnergy());
            part->setUserRecord("AuxE", genJet->auxiliaryEnergy());
            numJetMC++;

            // save number of GenJet-constituents fulfilling some cuts
            // int numGenJetConstit_withcuts = 0;
            // const vector< const reco::GenParticle* > &genJetConstit = genJet->getGenConstituents();
            // for (std::vector< const reco::GenParticle* >::const_iterator constit = genJetConstit.begin(); constit != genJetConstit.end(); ++constit) {
            //  // raise counter if cut passed
            // if ((*constit)->pt() > constit_pT) numGenJetConstit_withcuts++;
            // }
            part->setUserRecord("GenJetConstit", genJet->nConstituents());
            // part->setUserRecord("algoFlavour",    (*algoFlavour)   [ jetRef ].getFlavour());
            // part->setUserRecord("physicsFlavour", (*physicsFlavour)[ jetRef ].getFlavour());
        }
    }
    EvtView->setUserRecord("Num"+jet_info.name, numJetMC);
    edm::LogInfo("PxlSkimmer_miniAOD|GenInfo") << "MC Found: " << numJetMC << " jet(s) of type: " << jet_info.name;
}

void PxlSkimmer_miniAOD::analyzeGenMET(edm::Event const &iEvent,
                                         pxl::EventView *GenEvtView) const {
    // take the genMET() refference from the pat met
    edm::Handle< pat::METCollection > METHandle;
    iEvent.getByLabel(patMETTag_, METHandle);

    // There should be only one MET in the event, so take the first element.
    pat::METCollection::const_iterator met = (*METHandle).begin();

    const reco::GenMET* genmet =  met->genMET();

    int numMETMC = 0;

    pxl::Particle *part = GenEvtView->create< pxl::Particle >();
    part->setName(patMETTag_.label()+"_gen");
    part->setP4(genmet->px(), genmet->py(), genmet->pz(), genmet->energy());
    part->setUserRecord("sumEt",  genmet->sumEt());
    part->setUserRecord("mEtSig", genmet->mEtSig());
    // fill additional jet-related infos
    part->setUserRecord("EmE",  genmet->emEnergy());
    part->setUserRecord("HadE", genmet->hadEnergy());
    part->setUserRecord("InvE", genmet->invisibleEnergy());

    edm::LogInfo("PxlSkimmer_miniAOD|GenInfo") << "GenMET before muon corr: Px = " << part->getPx()
                                                 << ", Py = " << part->getPy()
                                                 << ", Pt = " << part->getPt();
    // Get systmetShifts:
    for (int uncert = 0 ; uncert < pat::MET::METUncertaintySize; uncert++) {
        pxl::Particle *part = GenEvtView->create< pxl::Particle >();
        part->setName(patMETTag_.label()+"uncert_"+ to_string(uncert));
        part->setP4(met->shiftedPx((pat::MET::METUncertainty)uncert), met->shiftedPy((pat::MET::METUncertainty)uncert), 0., met->shiftedPt((pat::MET::METUncertainty)uncert));
    }


    // Perform Muon Corrections!
    // loop over muons and subtract them
    // Only correct 'genMetCalo'!
    // if (genMETTag.label() == "genMetCalo" and GenEvtView->getUserRecord("NumMuon") > 0) {
    // vector< pxl::Particle* > GenMuons;
    // pxl::ParticleFilter::apply(GenEvtView->getObjectOwner(), GenMuons, pxl::ParticlePtEtaNameCriterion ("Muon"));
    // for (vector< pxl::Particle* >::const_iterator muon = GenMuons.begin(); muon != GenMuons.end(); ++muon) {
    // edm::LogInfo("PxlSkimmer_miniAOD|GenInfo") << "Correcting with " << (*muon)->getName()
    // << ", Px = " << (*muon)->getPx()
    // << ", Py = " << (*muon)->getPy();
    // *part -= **muon;
    // }
    // edm::LogInfo("PxlSkimmer_miniAOD|GenInfo") << "GenMET after muon corr: Px = " << part->getPx()
    // << ", Py = " << part->getPy()
    // << ", Pt = " << part->getPt();
    // }
    if (METMC_cuts(part)) numMETMC++;
    GenEvtView->setUserRecord("Num" + patMETTag_.label()+"_gen", numMETMC);
    if (numMETMC) edm::LogInfo("PxlSkimmer_miniAOD|GenInfo") << "Event contains MET";
}


// ----------------- SIM -------------------
void PxlSkimmer_miniAOD::analyzeSIM(const edm::Event& iEvent, pxl::EventView* EvtView) {
    edm::Handle<edm::SimVertexContainer> simVtcs;
    iEvent.getByLabel("g4SimHits", simVtcs);
    edm::SimVertexContainer::const_iterator simVertex;

    edm::Handle<edm::SimTrackContainer> simTracks;
    iEvent.getByLabel("g4SimHits", simTracks);
    edm::SimTrackContainer::const_iterator simTrack;
    edm::SimTrackContainer::const_iterator simTrack2;

    vector<unsigned int> ParentVec;

    for (simTrack = simTracks->begin(); simTrack != simTracks->end(); ++simTrack) {
        // int TrackID         = simTrack->trackId();
        // std::cout << "TrackID: " << TrackID << endl;
        int TrackType = simTrack->type();
        if ((TrackType == 11) || (TrackType == -11)) {
            // double TrackPt = sqrt(simTrack->momentum().perp2());
            // std::cout << "TrackType: " << TrackType << "TrackPt: " << TrackPt << endl;
            int VtxIndex = simTrack->vertIndex();
            unsigned int ParentTrack = (*simVtcs)[VtxIndex].parentIndex();
            vector<unsigned int>::iterator where = find(ParentVec.begin(), ParentVec.end(), ParentTrack);
            if (where == ParentVec.end()) {
                ParentVec.push_back(ParentTrack);
                // std::cout << "ParentTrack " << ParentTrack << endl;
                for (simTrack2 = simTracks->begin(); simTrack2 != simTracks->end(); ++simTrack2) {
                    if (simTrack2->trackId() == ParentTrack && (simTrack2->type() == 22) && (sqrt(simTrack2->momentum().perp2()) > 15.0)) {
                        // do not save photons without corresponding gen particle
                        if (!(simTrack2->noGenpart())) {
                            // int ParentType = simTrack2->type();
                            // double ParentPt = sqrt(simTrack2->momentum().perp2());
                            // std::cout << "TrackType: " << TrackType << "TrackPt: " << TrackPt << endl;
                            // std::cout << "ParentTrack " << ParentTrack << endl;
                            // std::cout << "found conversion: " << ParentType << " with pt: " << ParentPt << endl;
                            pxl::Particle* part = EvtView->create<pxl::Particle>();
                            part->setName("SIMConvGamma");
                            part->setP4(simTrack2->momentum().px(), simTrack2->momentum().py(), simTrack2->momentum().pz(), simTrack2->momentum().energy());
                            part->setUserRecord("TrackId", ParentTrack);
                            // std::cout << "found conversion with energy: " << simTrack2->momentum().energy() << " pt: " << part->getPt() << " eta: " << part->getEta() << " phi: " << part->getPhi() << endl;
                            // std::cout << "------------------" << endl;
                        }
                    }
                }
            }
        }
    }

    // std::cout << "---------NEW EVENT ---------" << endl;
}


// ------------ reading the Reconstructed MET ------------

void PxlSkimmer_miniAOD::analyzeRecMETs(edm::Event const &iEvent, pxl::EventView *RecEvtView) const {
    analyzeRecPatMET(iEvent, patMETTag_, RecEvtView);
    analyzeRecPUPPIMET(iEvent, PUPPIMETTag_, RecEvtView);
}


void PxlSkimmer_miniAOD::analyzeRecPatMET(edm::Event const &iEvent,
                                            edm::InputTag const &patMETTag,
                                            pxl::EventView *RecEvtView) const {
    edm::Handle<edm::View<pat::MET> > METHandle;
    //edm::Handle< pat::METCollection > METHandle;
    //std::cout<<patMETTag<<std::endl;
    iEvent.getByLabel(patMETTag, METHandle);
    edm::Handle< pat::METCollection > METEleCorrHandle;
    iEvent.getByLabel(patMETEleCorrTag_, METEleCorrHandle);

    // There should be only one MET in the event, so take the first element.
    edm::View<pat::MET>::const_iterator met = (*METHandle).begin();
    std::vector<pat::MET>::const_iterator metEleCorr = (*METEleCorrHandle).begin();

    int numPatMET = 0;
    pxl::Particle *part = RecEvtView->create< pxl::Particle >();
    if (std::string::npos != patMETTag.label().find("MuEGClean")) {
        part->setName("slimmedMETs");
    } else {
        part->setName(patMETTag.label());
    }
    part->setP4(met->px(), met->py(), met->pz(), met->energy());

    // High Pt Ele corrected met
    part->setUserRecord("eleCorrPx", metEleCorr->px());
    part->setUserRecord("eleCorrPy", metEleCorr->py());
    part->setUserRecord("eleCorrPz", metEleCorr->pz());
    part->setUserRecord("eleCorrE", metEleCorr->energy());

    part->setUserRecord("sumEt",  met->sumEt());
    part->setUserRecord("metSignificance",  met->metSignificance());
    part->setUserRecord("mEtSig", met->mEtSig());

    part->setUserRecord("T1XYPhi", met->corPhi(pat::MET::METCorrectionLevel::Type1XY));
    part->setUserRecord("T1XYPt", met->corPt(pat::MET::METCorrectionLevel::Type1XY));

    part->setUserRecord("uncorrectedPhi", met->uncorPhi());
    part->setUserRecord("uncorrectedPt", met->uncorPt());
    part->setUserRecord("caloPhi", met->caloMETPhi());
    part->setUserRecord("caloPt", met->caloMETPt());

    if (MET_cuts(part)) numPatMET++;
    RecEvtView->setUserRecord("Num" + patMETTag.label(), numPatMET);
}


void PxlSkimmer_miniAOD::analyzeRecPUPPIMET(edm::Event const &iEvent,
                                            edm::InputTag const &recoPUPPIMETTag,
                                            pxl::EventView *RecEvtView) const {
    edm::Handle< pat::METCollection > METHandle;
    iEvent.getByLabel(recoPUPPIMETTag, METHandle);

    // There should be only one MET in the event, so take the first element.
    pat::METCollection::const_iterator met = (*METHandle).begin();

    int numPuppiMET = 0;
    pxl::Particle *part = RecEvtView->create< pxl::Particle >();
    part->setName(recoPUPPIMETTag.label());
    part->setP4(met->px(), met->py(), met->pz(), met->energy());
    part->setUserRecord("sumEt",  met->sumEt());
    part->setUserRecord("mEtSig", met->mEtSig());

    if (MET_cuts(part)) numPuppiMET++;
    RecEvtView->setUserRecord("Num" + recoPUPPIMETTag.label(), numPuppiMET);
}

void PxlSkimmer_miniAOD::initializeFilter(edm::Event const &event,
                                            edm::EventSetup const &setup,
                                            trigger_group &filter) const {
    // Store the wanted filters and if the event has passed them.
    // This is uses edm::TriggerResults, so a trigger_def object is needed.
    for (sstring::const_iterator flt_name = filter.triggers_names.begin(); flt_name != filter.triggers_names.end(); ++flt_name) {
        // Get the number of the filter path.
        unsigned int index = filter.config.triggerIndex(*flt_name);

        // Check if that's a valid number.
        if (index < filter.config.size()) {
            trigger_def flt;
            flt.name = *flt_name;
            flt.ID = index;
            flt.active = true;
            filter.trigger_infos.push_back(flt);
        } else {
            // The number is invalid, the filter path is not in the config.
            edm::LogWarning("PxlSkimmer_miniAOD|TriggerWarning") << "In run " << event.run() << " filter "<< *flt_name
                                                                   << " not found in config, not added to filter list (so not used).";
        }
    }
}

void PxlSkimmer_miniAOD::analyseMETFilter(const edm::Event &iEvent,
                                         //const edm::EventSetup &iSetup,
                                         pxl::EventView *EvtView
    ) {

    edm::Handle< edm::TriggerResults > filterResultsHandle;
    edm::TriggerNames names;

    try
    {
        iEvent.getByToken(METFilter_, filterResultsHandle);
        names = iEvent.triggerNames(*filterResultsHandle);
    }
    catch (const edm::Exception &exc)
    {
        if (exc.categoryCode() == edm::errors::ProductNotFound)
        {
            iEvent.getByToken(METFilterAlternative_, filterResultsHandle);
            names = iEvent.triggerNames(*filterResultsHandle);
        }
        else
            throw exc;
    }

    for (unsigned int i = 0, n = filterResultsHandle->size(); i < n; ++i) {
        EvtView->setUserRecord(names.triggerName(i), filterResultsHandle->accept(i));
    }

    try
    {
        edm::Handle<bool> passecalBadCalibFilterUpdate ;
        iEvent.getByToken(ecalBadCalibFilterUpdate_, passecalBadCalibFilterUpdate);
        bool _passecalBadCalibFilterUpdate = (*passecalBadCalibFilterUpdate );
        EvtView->setUserRecord("Flag_ecalBadCalibReducedMINIAODFilter", _passecalBadCalibFilterUpdate);
    }
    catch (const edm::Exception &exc)
    {
        if (exc.categoryCode() != edm::errors::ProductNotFound)
        {
            throw exc;
        }
    }

}



void PxlSkimmer_miniAOD::analyzeFilter(const edm::Event &iEvent,
                                         const edm::EventSetup &iSetup,
                                         pxl::EventView *EvtView,
                                         trigger_group &filter
    ) {
    // Check if the trigger config, test for error and read it, if something changed!
    bool changed = true;
    if (!filter.config.init(iEvent.getRun(), iSetup, filter.process, changed)) {
        throw cms::Exception("FILTERS ERROR") << "Initialization of filter config failed.";
    }

    if (changed) {
        edm::LogInfo("PxlSkimmer_miniAOD|FilterInfo") << "TRIGGER INFO: HLT table changed in run " << iEvent.run()
                                                        << ", building new filter map for process " << filter.process;
        initializeFilter(iEvent, iSetup, filter);
    }

    edm::Handle< edm::TriggerResults > filterResultsHandle;
    iEvent.getByLabel(filter.results, filterResultsHandle);

    for (vector< trigger_def >::iterator filt = filter.trigger_infos.begin(); filt != filter.trigger_infos.end(); ++filt) {
        if (!filt->active) continue;

        bool wasrun = filterResultsHandle->wasrun(filt->ID);
        bool error  = filterResultsHandle->error(filt->ID);

        if (wasrun && !error) {
            EvtView->setUserRecord(filter.name + "_" + filt->name, filterResultsHandle->accept(filt->ID));

            if (filterResultsHandle->accept(filt->ID))
                edm::LogInfo("PxlSkimmer_miniAOD|FilterInfo") << "Event in process: '" << filter.process << "' passed filter: '" << filt->name << "'.";
        } else {
            // either error or was not run
            if (!wasrun) edm::LogWarning("FilterWarning") << "Filter: " << filt->name << " in process " << filter.process << " was not executed!";
            if (error)   edm::LogWarning("FilterWarning") << "An error occured during execution of Filter: " << filt->name << " in process " << filter.process;
        }
    }
}


bool PxlSkimmer_miniAOD::analyzeTrigger(const edm::Event &iEvent,
                                        pxl::EventView* EvtView) {
    edm::Handle<edm::TriggerResults> triggerBits;
    edm::Handle<pat::PackedTriggerPrescales> triggerPrescales;

    iEvent.getByToken(triggerBits_, triggerBits);
    iEvent.getByToken(triggerPrescales_, triggerPrescales);

    const edm::TriggerNames &names = iEvent.triggerNames(*triggerBits);
    if (triggerBits->size() == 0)
        return false;

    for (unsigned int i=0; i<triggerBits->size(); ++i) {
        if (triggerBits->accept(i))
           EvtView->setUserRecord(names.triggerName(i), triggerPrescales->getPrescaleForIndex(i));
    }
    return true;
}

// ------------ reading Reconstructed Primary Vertices ------------

void PxlSkimmer_miniAOD::analyzeRecVertices(const edm::Event& iEvent, pxl::EventView* EvtView) {
    edm::Handle<reco::VertexCollection> vertices;
    iEvent.getByLabel(VertexRecoLabel_, vertices);

    // get the beamspot
    edm::Handle<reco::BeamSpot> recoBeamSpotHandle;
    iEvent.getByLabel("offlineBeamSpot", recoBeamSpotHandle);
    const reco::BeamSpot &beamspot = *recoBeamSpotHandle;

    // store the BeamSpot
    pxl::Vertex* bs = EvtView->create< pxl::Vertex >();
    bs->setName("BeamSpot");
    bs->setXYZ(beamspot.x0(), beamspot.y0(), beamspot.z0());

    // save the BS for further purpose
    the_beamspot = beamspot.position();

    // get the PV
    const reco::Vertex &PV = *(vertices->begin());

    // save the primary vertex postion for later use
    // use the BeamSpot in case the PV is shit
    the_Pvertex = PV.position();

    int numVertices = 0;

    bool found_good_vertex = false;
    for (reco::VertexCollection::const_iterator  vertex = vertices->begin(); vertex != vertices->end(); ++vertex) {
        // only fill primary vertex if cuts passed
        if (Vertex_cuts(vertex)) {
            pxl::Vertex* vtx = EvtView->create<pxl::Vertex>();
            vtx->setName("PV");
            vtx->setXYZ(vertex->x(), vertex->y(), vertex->z());
            // errors
            vtx->setUserRecord("xErr", vertex->xError());
            vtx->setUserRecord("yErr", vertex->yError());
            vtx->setUserRecord("zErr", vertex->zError());
            // chi2 of vertex-fit
            vtx->setUserRecord("chi2", vertex->chi2());
            vtx->setUserRecord("ndof", vertex->ndof());
            // is valid?
            vtx->setUserRecord("IsValid", vertex->isValid());
            vtx->setUserRecord("IsFake", vertex->isFake());
            numVertices++;
        }
        if (not found_good_vertex and PV_vertex_cuts(vertex)) {
            the_vertex = vertex->position();
            found_good_vertex = true;
        }
    }
    if (not found_good_vertex) {
        the_vertex = beamspot.position();
    }
    EvtView->setUserRecord("NumVertices", numVertices);
}


// void PxlSkimmer_miniAOD::analyzeRecTracks(edm::Event const &iEvent,
// pxl::EventView *RecEvtView
// ) const {
// edm::Handle< reco::TrackCollection > tracksHandle;
// iEvent.getByLabel(m_recoTracksTag, tracksHandle);

//  // Store the number of tracks in each event!
// RecEvtView->setUserRecord("Num" + m_recoTracksTag.label(), tracksHandle->size());
// }


// ------------ reading Reconstructed Taus------------

void PxlSkimmer_miniAOD::analyzeRecTaus(edm::Event const &iEvent,
                                        pxl::EventView *RecEvtView) const {
  //for (VInputTag::const_iterator patTauTag = patTauTag_.begin(); patTauTag != patTauTag_.end();++patTauTag) {
    // FIXME: Add Taus // LOR TRY TO UNCOMMENT ALL THESE LINES UP AND DOWN
    analyzeRecPatTaus(iEvent, patTauTag_, RecEvtView);
    //analyzeRecPatTaus(iEvent, patTauBoostedTag_, RecEvtView);
    // }
}


void PxlSkimmer_miniAOD::analyzeRecPatTaus(edm::Event const &iEvent,
                                             edm::InputTag const &tauTag,
                                             pxl::EventView *RecEvtView) const {
    // Get the wanted pat::Tau's from event:
    edm::Handle< pat::TauCollection > tauHandle;
    iEvent.getByLabel(tauTag, tauHandle);
    //cout << "tauHandle: " << tauHandle << endl;
    //cout << "tauTag: " << tauTag << endl;
    pat::TauCollection const &taus = *tauHandle;
     //LOR TRY TO REMOVE. MOVE TO DeepTau. FOR CMSSW106X
    edm::Handle<pat::PATTauDiscriminator> mvaIsoRaw;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoVLoose;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoLoose;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoMedium;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoTight;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoVTight;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoVVTight;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoRawNewDM;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoVLooseNewDM;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoLooseNewDM;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoMediumNewDM;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoTightNewDM;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoVTightNewDM;
    edm::Handle<pat::PATTauDiscriminator> mvaIsoVVTightNewDM;
    iEvent.getByToken(mvaIsolationToken_,mvaIsoRaw);
    iEvent.getByToken(mvaIsolationVLooseToken_,mvaIsoVLoose);
    iEvent.getByToken(mvaIsolationLooseToken_,mvaIsoLoose);
    iEvent.getByToken(mvaIsolationMediumToken_,mvaIsoMedium);
    iEvent.getByToken(mvaIsolationTightToken_,mvaIsoTight);
    iEvent.getByToken(mvaIsolationVTightToken_,mvaIsoVTight);
    iEvent.getByToken(mvaIsolationVVTightToken_,mvaIsoVVTight);
    iEvent.getByToken(mvaIsolationTokenNewDM_,mvaIsoRawNewDM);
    iEvent.getByToken(mvaIsolationVLooseTokenNewDM_,mvaIsoVLooseNewDM);
    iEvent.getByToken(mvaIsolationLooseTokenNewDM_,mvaIsoLooseNewDM);
    iEvent.getByToken(mvaIsolationMediumTokenNewDM_,mvaIsoMediumNewDM);
    iEvent.getByToken(mvaIsolationTightTokenNewDM_,mvaIsoTightNewDM);
    iEvent.getByToken(mvaIsolationVTightTokenNewDM_,mvaIsoVTightNewDM);
    iEvent.getByToken(mvaIsolationVVTightTokenNewDM_,mvaIsoVVTightNewDM);
    
    int numPatTaus = 0;
    for (pat::TauCollection::const_iterator tau = taus.begin();
         tau != taus.end();
         ++tau
        ) {
        if (Tau_cuts(*tau)) {
            pxl::Particle *part = RecEvtView->create< pxl::Particle >();
            // The label defines the name of this pxl object!

            part->setName(tauTag.label());
            part->setCharge(tau->charge());
            part->setP4(tau->px(), tau->py(), tau->pz(), tau->energy());
            part->setUserRecord("Vtx_X", tau->vx());
            part->setUserRecord("Vtx_Y", tau->vy());
            part->setUserRecord("Vtx_Z", tau->vz());

            part->setUserRecord("dxy", tau->dxy());
            part->setUserRecord("dxy_error", tau->dxy_error());
            part->setUserRecord("dxy_Sig", tau->dxy_Sig());

            for (std::vector< pat::Tau::IdPair >::const_iterator it = tau->tauIDs().begin(); it != tau->tauIDs().end(); ++it) {
                part->setUserRecord(it->first, it->second);
            }
            // Set DeepTau ID UserRecords #LOR TRY v2 and not v2p1. v2 should work for UL2017
            part->setUserRecord("byDeepTau2017v2p1VSjetraw", tau->tauID("byDeepTau2017v2p1VSjetraw"));
            part->setUserRecord("byDeepTau2017v2p1VSeraw", tau->tauID("byDeepTau2017v2p1VSeraw"));
            part->setUserRecord("byDeepTau2017v2p1VSmuraw", tau->tauID("byDeepTau2017v2p1VSmuraw"));
            for (const auto& wp_temp : {"VVVLoose", "VVLoose", "VLoose", "Loose", "Medium", "Tight", "VTight", "VVTight"}) {
                const std::string wp = wp_temp;
                part->setUserRecord("by" + wp + "DeepTau2017v2p1VSjet", tau->tauID("by" + wp + "DeepTau2017v2p1VSjet"));
                part->setUserRecord("by" + wp + "DeepTau2017v2p1VSe", tau->tauID("by" + wp + "DeepTau2017v2p1VSe"));
            }
            for (const auto& wp_temp : {"VLoose", "Loose", "Medium", "Tight"}) {
                const std::string wp = wp_temp;
                part->setUserRecord("by" + wp + "DeepTau2017v2p1VSmu", tau->tauID("by" + wp + "DeepTau2017v2p1VSmu"));
            }
	    
	    //LOR TRY TO UPDATE. NEEDSTO FIX! HOWEVER, WE MOVE TO DeepTau. FOR CMSSW106X
	    //https://twiki.cern.ch/twiki/bin/viewauth/CMS/TauIDRecommendationForRun2#Decay_Mode_Reconstruction
            //int TauIndex = std::distance(taus.begin(), tau);
            
	    /*
	    pat::TauRef tauForMVA(tauHandle,TauIndex);
            float mvaValue = (*mvaIsoRaw)[tauForMVA];
            float mvaValueNewDM = (*mvaIsoRawNewDM)[tauForMVA];

            // Backup old ID for one WP to compare new with old ID
            
	    part->setUserRecord("byMediumIsolationMVArun2v1DBnewDMwLT2015", part->getUserRecord("byMediumIsolationMVArun2v1DBnewDMwLT"));
            part->setUserRecord("byMediumIsolationMVArun2v1DBoldDMwLT2015", part->getUserRecord("byMediumIsolationMVArun2v1DBoldDMwLT"));

            // Now get the new ID WP from the recalculated boosted decision tree
            part->setUserRecord("byIsolationMVArun2v1DBoldDMwLTraw", mvaValue);
            part->setUserRecord("byVLooseIsolationMVArun2v1DBoldDMwLT", (*mvaIsoVLoose)[tauForMVA]);
            part->setUserRecord("byLooseIsolationMVArun2v1DBoldDMwLT", (*mvaIsoLoose)[tauForMVA]);
            part->setUserRecord("byMediumIsolationMVArun2v1DBoldDMwLT", (*mvaIsoMedium)[tauForMVA]);
            part->setUserRecord("byTightIsolationMVArun2v1DBoldDMwLT", (*mvaIsoTight)[tauForMVA]);
            part->setUserRecord("byVTightIsolationMVArun2v1DBoldDMwLT", (*mvaIsoVTight)[tauForMVA]);
            part->setUserRecord("byVVTightIsolationMVArun2v1DBoldDMwLT", (*mvaIsoVVTight)[tauForMVA]);
            part->setUserRecord("byIsolationMVArun2v1DBnewDMwLTraw", mvaValueNewDM);
            part->setUserRecord("byVLooseIsolationMVArun2v1DBnewDMwLT", (*mvaIsoVLooseNewDM)[tauForMVA]);
            part->setUserRecord("byLooseIsolationMVArun2v1DBnewDMwLT", (*mvaIsoLooseNewDM)[tauForMVA]);
            part->setUserRecord("byMediumIsolationMVArun2v1DBnewDMwLT", (*mvaIsoMediumNewDM)[tauForMVA]);
            part->setUserRecord("byTightIsolationMVArun2v1DBnewDMwLT", (*mvaIsoTightNewDM)[tauForMVA]);
            part->setUserRecord("byVTightIsolationMVArun2v1DBnewDMwLT", (*mvaIsoVTightNewDM)[tauForMVA]);
            part->setUserRecord("byVVTightIsolationMVArun2v1DnewDMwLT", (*mvaIsoVVTightNewDM)[tauForMVA]);
	    //*/
            part->setUserRecord("decayMode", tau->decayMode());


	    //LOR TRY TO ADD NEW MVA decay mode id. See https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePFTauID#Running_of_the_DeepTauIDs_ver_20 --->
	    // Running of the MVA decay mode identification on MiniAOD event content
	    /*
	    part->setUserRecord("MVADM2017v1", tau->tauID("MVADM2017v1"));
            part->setUserRecord("MVADM2017v1DM0raw",tau->tauID("MVADM2017v1DM0raw"));
            part->setUserRecord("MVADM2017v1DM1raw", tau->tauID("MVADM2017v1DM1raw"));
            part->setUserRecord("MVADM2017v1DM2raw", tau->tauID("MVADM2017v1DM2raw"));
            part->setUserRecord("MVADM2017v1DM10raw", tau->tauID("MVADM2017v1DM10raw"));
            part->setUserRecord("MVADM2017v1DM11raw", tau->tauID("MVADM2017v1DM11raw"));
            part->setUserRecord("MVADM2017v1DMotherraw", tau->tauID("MVADM2017v1DMotherraw"));
	    */
            // Pt of the Leading Charged Hadron of the Jet
            // at the moment we don't need any infos from them if so use pat::PackedCandidate
            reco::CandidatePtr const &leadChargedHadrCand = tau->leadChargedHadrCand();
            if (leadChargedHadrCand.isNonnull()) {
                part->setUserRecord("LeadingHadronPt", leadChargedHadrCand->pt());
                part->setUserRecord("LeadingHadronP", leadChargedHadrCand->p());
            } else {
                part->setUserRecord("LeadingHadronPt", -1.0);
                part->setUserRecord("LeadingHadronP", -1.);
            }

            reco::TrackRefVector const &signalTracks = tau->signalTracks();
            if (signalTracks.isNonnull())
                part->setUserRecord("NumSignalTracks", signalTracks.size());
            else
                part->setUserRecord("NumSignalTracks", -1);
            // GRRRR there is no way to get the jet link at the moment!! Will be fixed in CMSSW_7_1_X!!
            // Information from jet used to reconstruct the tau:
            // (Uncorrected jet pt.)
            const pat::tau::TauPFEssential  &pfEssential = tau->pfEssential();
             part->setUserRecord("tauJetpx", pfEssential.p4Jet_.px());
             part->setUserRecord("tauJetpy", pfEssential.p4Jet_.py());
             part->setUserRecord("tauJetpz", pfEssential.p4Jet_.pz());
             part->setUserRecord("tauJetE",  pfEssential.p4Jet_.energy());

            reco::CandidatePtr const &leadNeutralCand = tau->leadNeutralCand();
            if (leadNeutralCand.isNonnull())
                part->setUserRecord("LeadingNeutralPt", leadNeutralCand->pt());
            else
                part->setUserRecord("LeadingNeutralPt", -1.0);


            // part->setUserRecord("EMFraction", tau->emFraction());  // Ecal/Hcal Cluster Energy

            // GRRRR there is no way to get the jet link at the moment!!
            // part->setUserRecord("HcalEoverLeadChargedP", tau->hcalTotOverPLead());  // total Hcal Cluster E / leadPFChargedHadron P

            // reco::VertexRef const &tau_primary_vertex = tau->primaryVertex();
            // part->setUserRecord("PrimVtx_X", tau_primary_vertex->x());
            // part->setUserRecord("PrimVtx_Y", tau_primary_vertex->y());
            // part->setUserRecord("PrimVtx_Z", tau_primary_vertex->z());
            //only write out the tau signal candidates if a loose id is fullfilled!
            if(tau->tauID("decayModeFindingNewDMs")>0.5){
                for(size_t i = 0; i < tau->signalChargedHadrCands().size(); i++){
                    pxl::Particle *part_tmp = RecEvtView->create<pxl::Particle>();
                    part_tmp->setName("signalChargedHadrCands");
                    part_tmp->setP4(tau->signalChargedHadrCands()[i]->px(),
                                    tau->signalChargedHadrCands()[i]->py(),
                                    tau->signalChargedHadrCands()[i]->pz(),
                                    tau->signalChargedHadrCands()[i]->energy());
                    part->linkFlat(part_tmp);
                }

                for(size_t i = 0; i < tau->signalNeutrHadrCands().size(); i++){
                    pxl::Particle *part_tmp = RecEvtView->create<pxl::Particle>();
                    part_tmp->setName("signalNeutrHadrCands");
                    part_tmp->setP4(tau->signalNeutrHadrCands()[i]->px(),
                                    tau->signalNeutrHadrCands()[i]->py(),
                                    tau->signalNeutrHadrCands()[i]->pz(),
                                    tau->signalNeutrHadrCands()[i]->energy());
                    part->linkFlat(part_tmp);
                }

                for(size_t i = 0; i < tau->signalGammaCands().size(); i++){
                    pxl::Particle *part_tmp = RecEvtView->create<pxl::Particle>();
                    part_tmp->setName("signalGammaCands");
                    part_tmp->setP4(tau->signalGammaCands()[i]->px(),
                                    tau->signalGammaCands()[i]->py(),
                                    tau->signalGammaCands()[i]->pz(),
                                    tau->signalGammaCands()[i]->energy());
                    part->linkFlat(part_tmp);
                }
            }

            reco::CandidatePtrVector const &signalGammaCands = tau->signalGammaCands();
            try {
                part->setUserRecord("NumPFGammaCands", signalGammaCands.size());
            }
            catch(...) {
                part->setUserRecord("NumPFGammaCands", -1);
            }
            reco::CandidatePtrVector  const &signalChargedHadrCands = tau->signalChargedHadrCands();
            try {
                part->setUserRecord("NumPFChargedHadrCands", signalChargedHadrCands.size());
            }
            catch(...) {
                part->setUserRecord("NumPFChargedHadrCands", -1);
            }
            reco::CandidatePtrVector const &signalNeutrHadrCands = tau->signalNeutrHadrCands();
            try {
                part->setUserRecord("NumPFNeutralHadrCands", signalNeutrHadrCands.size());
            }
            catch(...) {
                part->setUserRecord("NumPFNeutralHadrCands", -1);
            }

            part->setUserRecord("NumPFPiZeroCands", tau->signalPFGammaCands().size());

            numPatTaus++;
        }
    }

    RecEvtView->setUserRecord("Num" + tauTag.label(), numPatTaus);
}

// ------------ reading Reconstructed Muons ------------

void PxlSkimmer_miniAOD::analyzeRecMuons(edm::Event const &iEvent,
                                         edm::EventSetup const &iSetup,
                                           pxl::EventView *RecView,
                                           bool const &MC,
                                           std::map< reco::Candidate const*, pxl::Particle* > &genmap,
                                           reco::Vertex const &PV) const {
    // get pat::Muon's from event
    edm::Handle<std::vector<pat::Muon> > muonHandle;
    iEvent.getByLabel(patMuonLabel_, muonHandle);
    const std::vector<pat::Muon>& muons = *muonHandle;

    // count muons
    int numMuonRec = 0;

    // vector for muon refits
    std::vector<std::pair<const pat::Muon *, pxl::Particle *>> refit_vec;

    // loop over all pat::Muon's
    for (std::vector<pat::Muon>::const_iterator muon = muons.begin();  muon != muons.end(); ++muon) {
        if (Muon_cuts(*muon)) {
            pxl::Particle* part = RecView->create<pxl::Particle>();
            // make pair of pxl and pat muon for vertex refitting
            refit_vec.push_back(std::make_pair(&(*muon), part));
            // store id of physical particle
            // can be used to identify a particle after copying pxl object
            part->setUserRecord("particleId", part->getId().toString());

            part->setName("Muon");
            part->setCharge(muon->charge());
            part->setP4(muon->px(), muon->py(), muon->pz(), muon->energy());
            part->setUserRecord("Vtx_X", muon->vx());
            part->setUserRecord("Vtx_Y", muon->vy());
            part->setUserRecord("Vtx_Z", muon->vz());

            // Particle-Flow muons out-of-the-box.
            part->setUserRecord("isPFMuon", muon->isPFMuon());
            if (muon->isPFMuon()) {
                part->setUserRecord("PFpx", muon->pfP4().Px());
                part->setUserRecord("PFpy", muon->pfP4().Py());
                part->setUserRecord("PFpz", muon->pfP4().Pz());
                part->setUserRecord("PFE",  muon->pfP4().E());
            }

            // store PAT matching info if MC
            if (MC) {
                std::map< const reco::Candidate*, pxl::Particle* >::const_iterator it = genmap.find(muon->genLepton());
                if (it != genmap.end()) {
                    part->linkSoft(it->second, "pat-match");
                }
            }


            // check if muon is Global/Tracker/StandAlone -Muon
            part->setUserRecord("isGlobalMuon", muon->isGlobalMuon());
            part->setUserRecord("isTrackerMuon", muon->isTrackerMuon());
            part->setUserRecord("isStandAloneMuon", muon->isStandAloneMuon());

            // Check muon-IDs via bitmap. For details see:
            // https://github.com/cms-sw/cmssw/blob/CMSSW_9_4_X/DataFormats/MuonReco/interface/Muon.h#L188-L212
            // https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2?rev=43#Muon_selectors_Since_9_4_X
            // will only work for CMSSW version >= 9_4_X
            part->setUserRecord("isLooseMuon", muon->passed(reco::Muon::CutBasedIdLoose));
            part->setUserRecord("isMediumMuon", muon->passed(reco::Muon::CutBasedIdMedium));
            part->setUserRecord("isMediumPromptMuon", muon->passed(reco::Muon::CutBasedIdMediumPrompt));
            part->setUserRecord("isTightMuon", muon->passed(reco::Muon::CutBasedIdTight));
            part->setUserRecord("isHighPtMuon", muon->passed(reco::Muon::CutBasedIdGlobalHighPt));
            part->setUserRecord("isTrackerHighPtMuon", muon->passed(reco::Muon::CutBasedIdTrkHighPt));
            part->setUserRecord("PFIsoVeryLoose", muon->passed(reco::Muon::PFIsoVeryLoose));
            part->setUserRecord("PFIsoLoose", muon->passed(reco::Muon::PFIsoLoose));
            part->setUserRecord("PFIsoMedium", muon->passed(reco::Muon::PFIsoMedium));
            part->setUserRecord("PFIsoTight", muon->passed(reco::Muon::PFIsoTight));
            part->setUserRecord("PFIsoVeryTight", muon->passed(reco::Muon::PFIsoVeryTight));
            //Only in CMSSW101
            //part->setUserRecord("PFIsoVeryVeryTight", muon->passed(reco::Muon::PFIsoVeryVeryTight));
            part->setUserRecord("TkIsoLoose", muon->passed(reco::Muon::TkIsoLoose));
            part->setUserRecord("TkIsoTight", muon->passed(reco::Muon::TkIsoTight));
            part->setUserRecord("isSoftMuon", muon->passed(reco::Muon::SoftCutBasedId));
            //Only in CMSSW101
            //part->setUserRecord("isSoftMVAMuon", muon->passed(reco::Muon::SoftMvaId));
            part->setUserRecord("isLooseMVAMuon", muon->passed(reco::Muon::MvaLoose));
            part->setUserRecord("isMediumMVAMuon", muon->passed(reco::Muon::MvaMedium));
            part->setUserRecord("isTightMVAMuon", muon->passed(reco::Muon::MvaTight));
            part->setUserRecord("MiniIsoLoose", muon->passed(reco::Muon::MiniIsoLoose));
            part->setUserRecord("MiniIsoMedium", muon->passed(reco::Muon::MiniIsoMedium));
            part->setUserRecord("MiniIsoTight", muon->passed(reco::Muon::MiniIsoTight));
            part->setUserRecord("MiniIsoVeryTight", muon->passed(reco::Muon::MiniIsoVeryTight));
            //Only in CMSSW101
            part->setUserRecord("TriggerIdLoose", muon->passed(reco::Muon::TriggerIdLoose));
            part->setUserRecord("InTimeMuon", muon->passed(reco::Muon::InTimeMuon));
            part->setUserRecord("MultiIsoLoose", muon->passed(reco::Muon::MultiIsoLoose));
            part->setUserRecord("MultiIsoMedium", muon->passed(reco::Muon::MultiIsoMedium));

            // Further muon variables used in IDs
            // for medium ID:
            part->setUserRecord("chi2LocalPosition", muon->combinedQuality().chi2LocalPosition);
            part->setUserRecord("trkKink", muon->combinedQuality().trkKink);
            part->setUserRecord("SegComp", muon::segmentCompatibility(*muon));
            // for tight/high-pT ID:
            // store the number of muon stations containing segments
            part->setUserRecord("NMatchedStations", muon->numberOfMatchedStations());
            part->setUserRecord("HitInFirstLayer", muon->stationMask() == 1 ||
                                muon->stationMask() == 16);
            part->setUserRecord("MatchedRPCLayers", muon->numberOfMatchedRPCLayers());
            // for soft ID:
            part->setUserRecord("isGoodTMOneST", muon::isGoodMuon(*muon, muon::TMOneStationTight));

            // additional muon variables which are not part of any ID
            // may be good for additional cross-checks (?)
            // --> possibly not needed
            part->setUserRecord("CaloComp", muon->caloCompatibility());
            part->setUserRecord("NumChambers", muon->numberOfChambers());
            part->setUserRecord("NumMatches", muon->numberOfMatches());
            part->setUserRecord("EMDeposit", muon->calEnergy().em);
            part->setUserRecord("HCALDeposit", muon->calEnergy().had);
            part->setUserRecord("isGood", muon::isGoodMuon(*muon, muon::GlobalMuonPromptTight));
            part->setUserRecord("isGoodLastST", muon::isGoodMuon(*muon, muon::TMLastStationTight));
            part->setUserRecord("dBPV3D", muon->dB(pat::Muon::PV3D));
            part->setUserRecord("edBPV3D", muon->edB(pat::Muon::PV3D));


            // Muon Isolation
            // CaloIso and TrkIso
            // Def:  aMuon.setCaloIso(aMuon.isolationR03().emEt + aMuon.isolationR03().hadEt + aMuon.isolationR03().hoEt);
            part->setUserRecord("CaloIso", muon->caloIso());
            part->setUserRecord("TrkIso", muon->trackIso());
            part->setUserRecord("ECALIso", muon->ecalIso());
            part->setUserRecord("HCALIso", muon->hcalIso());
            // save constituents for CaloIso and TrkIso with deltaR = 0.3
            const reco::MuonIsolation& muonIsoR03 = muon->isolationR03();
            part->setUserRecord("IsoR3SumPt", muonIsoR03.sumPt);
            part->setUserRecord("IsoR3EmEt", muonIsoR03.emEt);
            part->setUserRecord("IsoR3HadEt", muonIsoR03.hadEt);
            part->setUserRecord("IsoR3HoEt", muonIsoR03.hoEt);
            part->setUserRecord("IsoR3NTracks", muonIsoR03.nTracks);
            part->setUserRecord("IsoR3NJets", muonIsoR03.nJets);
            // save constituents for CaloIso and TrkIso with deltaR = 0.5
            const reco::MuonIsolation& muonIsoR05 = muon->isolationR05();
            part->setUserRecord("IsoR5SumPt", muonIsoR05.sumPt);
            part->setUserRecord("IsoR5EmEt", muonIsoR05.emEt);
            part->setUserRecord("IsoR5HadEt", muonIsoR05.hadEt);
            part->setUserRecord("IsoR5HoEt", muonIsoR05.hoEt);
            part->setUserRecord("IsoR5NTracks", muonIsoR05.nTracks);
            part->setUserRecord("IsoR5NJets", muonIsoR05.nJets);

            // Particle Flow based Isolation
            const reco::MuonPFIsolation muonPFIso03 = muon->pfIsolationR03();
            const reco::MuonPFIsolation muonPFIso04 = muon->pfIsolationR04();
            // Save constituents of PF Iso for deltaR = 0.3 and deltaR = 0.4
            // Sum Pt of the charged Hadrons.
            part->setUserRecord("PFIsoR03ChargedHadrons", muonPFIso03.sumChargedHadronPt);
            part->setUserRecord("PFIsoR04ChargedHadrons", muonPFIso04.sumChargedHadronPt);
            // Sum Pt of all charged particles (including PF electrons and muons).
            part->setUserRecord("PFIsoR03ChargeParticles", muonPFIso03.sumChargedParticlePt);
            part->setUserRecord("PFIsoR04ChargeParticles", muonPFIso04.sumChargedParticlePt);
            // Sum Et of the neutral hadrons.
            part->setUserRecord("PFIsoR03NeutralHadrons", muonPFIso03.sumNeutralHadronEt);
            part->setUserRecord("PFIsoR04NeutralHadrons", muonPFIso04.sumNeutralHadronEt);
            // Sum Et of PF photons.
            part->setUserRecord("PFIsoR03Photons", muonPFIso03.sumPhotonEt);
            part->setUserRecord("PFIsoR04Photons", muonPFIso04.sumPhotonEt);
            // Sum Pt of the charged particles in the cone of interest but with particles not originating from the primary vertex(for PU corrections).
            part->setUserRecord("PFIsoR03PU", muonPFIso03.sumPUPt);
            part->setUserRecord("PFIsoR04PU", muonPFIso04.sumPUPt);
            // Sum of the PF photons Et with higher threshold (1 GeV instead of 0.5).
            part->setUserRecord("PFIso03PhotonsHighThres", muonPFIso03.sumPhotonEtHighThreshold);
            part->setUserRecord("PFIso04PhotonsHighThres", muonPFIso04.sumPhotonEtHighThreshold);

            // mini Isolation
            // weird call needed to call a non const function in a const one
            part->setUserRecord("miniIsoEA", multiIsolation_.getAbsMiniIso(*muon));
            double ptRatio, ptRel, laJetPt, laJetEta, laJetPhi, laJetM, laJetJECRaw, laJetJECL1;
            std::tie(ptRatio, ptRel, laJetPt, laJetEta, laJetPhi, laJetM, laJetJECRaw, laJetJECL1) = multiIsolation_.getPtRatioPtRel(*muon);
            part->setUserRecord("ptRatio", ptRatio);
            part->setUserRecord("ptRel", ptRel);
            part->setUserRecord("lepawareJetPt", laJetPt);
            part->setUserRecord("lepawareJetEta", laJetEta);
            part->setUserRecord("lepawareJetPhi", laJetPhi);
            part->setUserRecord("lepawareJetM", laJetM);
            part->setUserRecord("lepawareJetJECToRaw", laJetJECRaw);
            part->setUserRecord("lepawareJetJECToL1", laJetJECL1);

            // Muon Tracks:
            // save info about quality of track-fit for combined muon (muon system + tracker)
            reco::TrackRef globalTrack = muon->globalTrack();
            reco::TrackRef innerTrack = muon->innerTrack();
            reco::TrackRef outerTrack = muon->outerTrack();
            reco::TrackRef bestTrack = muon->muonBestTrack();

            // Some variables may be needed for different track types or with respect to the beamspot.
            // In these casesm the following abbreviations are added to the names:
            // PV --> primary vertex
            // BS --> beamspot
            // IT --> inner track
            // GT --> global track
            // BT --> best track
            if(globalTrack.isAvailable()){
                // get pt and pt error
                part->setUserRecord("ptError",     globalTrack->ptError());
                part->setUserRecord("pt",          globalTrack->pt());
                // Store the pt and error from the global track.
                // (qoverpError() is the same as error(0) for a track.)
                part->setUserRecord("qoverp",      globalTrack->qoverp());
                part->setUserRecord("qoverpError", globalTrack->qoverpError());

                // Need chi^2 and n.d.f. to calculate fit probability.
                part->setUserRecord("chi2", globalTrack->chi2());
                part->setUserRecord("ndof", globalTrack->ndof());
                part->setUserRecord("normalizedChi2", globalTrack->normalizedChi2());
                part->setUserRecord("VHitsMuonSys", globalTrack->hitPattern().numberOfValidMuonHits());

                // Store information of the impact parameters with respect to the PV
                part->setUserRecord("DzGT", globalTrack->dz(the_Pvertex));
                part->setUserRecord("DxyGT", globalTrack->dxy(the_Pvertex));

                // Store information of the impact parameters with respect to the Best Vertex
                part->setUserRecord("DzGTBV", globalTrack->dz(the_vertex));
                part->setUserRecord("DxyGTBV", globalTrack->dxy(the_vertex));

                // Store information of the impact parameters with respect to the BS
                part->setUserRecord("DzGTBS", globalTrack->dz(the_beamspot));
                part->setUserRecord("DxyGTBS", globalTrack->dxy(the_beamspot));

                // additional muon track variables which are not part of any ID
                // may be good for additional cross-checks (?)
                // --> possibly not needed
                part->setUserRecord("VHits", globalTrack->hitPattern().numberOfValidHits());
                part->setUserRecord("VHitsTracker", globalTrack->hitPattern().numberOfValidTrackerHits());
                part->setUserRecord("LHits", globalTrack->hitPattern().numberOfLostHits(reco::HitPattern::HitCategory::TRACK_HITS));
            }
            if(innerTrack.isAvailable()){
                // get pt and pt error even for slim muons:
                part->setUserRecord("ptErrorTracker",     innerTrack->ptError());
                part->setUserRecord("ptTracker",          innerTrack->pt());

                // Store also the pt error from the inner track.
                // (qoverpError() is the same as error(0) for a track.)
                part->setUserRecord("qoverpTracker",      innerTrack->qoverp());
                part->setUserRecord("qoverpErrorTracker", innerTrack->qoverpError());

                // Store info from HitPattern of the global and inner track.
                part->setUserRecord("PixelLayersWithMeas",   innerTrack->hitPattern().pixelLayersWithMeasurement());
                part->setUserRecord("TrackerLayersWithMeas", innerTrack->hitPattern().trackerLayersWithMeasurement());
                part->setUserRecord("VHitsPixel", innerTrack->hitPattern().numberOfValidPixelHits());

                // Store info about the quality of the inner track.
                part->setUserRecord("validFraction", innerTrack->validFraction());
                part->setUserRecord("QualityInnerTrack", innerTrack->quality(reco::TrackBase::highPurity));

                // Store information of the impact parameters with respect to the PV
                part->setUserRecord("DzIT", innerTrack->dz(the_Pvertex));
                part->setUserRecord("DxyIT", innerTrack->dxy(the_Pvertex));

                // Store information of the impact parameters with respect to the Best Vertex
                part->setUserRecord("DzITBV", innerTrack->dz(the_vertex));
                part->setUserRecord("DxyITBV", innerTrack->dxy(the_vertex));
            }
            if(bestTrack.isAvailable()){
                // get pt and pt error even for slim muons:
                part->setUserRecord("ptErrorBT",     bestTrack->ptError());
                part->setUserRecord("ptBT",          bestTrack->pt());
                part->setUserRecord("Dz", bestTrack->dz(the_Pvertex));
                part->setUserRecord("Dxy", bestTrack->dxy(the_Pvertex));
                part->setUserRecord("DzBV", bestTrack->dz(the_vertex));
                part->setUserRecord("DxyBV", bestTrack->dxy(the_vertex));
                part->setUserRecord("DzBS", bestTrack->dz(the_beamspot));
                part->setUserRecord("DxyBS", bestTrack->dxy(the_beamspot));
                // dB returns almost the same value as Dxy, but is more accurate. For more details see:
                // https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonId2015?rev=13
                part->setUserRecord("dB",    muon->dB());  // Causes the jobs to fail on the grid
            }


            // Store information for "cocktail" high energy refit. These are needed
            // for the HighPT Muon ID, for more details see:
            // https:// twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId?rev = 48#New_Version_recommended
            //
            // Get the optimal cocktail muon track using the improved version of Tune P.
            reco::TrackRef pmcTrack = muon->tunePMuonBestTrack();

            if (pmcTrack.isAvailable()) {
                part->setUserRecord("validCocktail", true);
                part->setUserRecord("pxCocktail", pmcTrack->px());
                part->setUserRecord("pyCocktail", pmcTrack->py());
                part->setUserRecord("pzCocktail", pmcTrack->pz());

                part->setUserRecord("ptErrorCocktail",     pmcTrack->ptError());
                part->setUserRecord("ptCocktail",          pmcTrack->pt());

                part->setUserRecord("qoverpCocktail",      pmcTrack->qoverp());
                part->setUserRecord("qoverpErrorCocktail", pmcTrack->qoverpError());


                part->setUserRecord("NormChi2Cocktail", pmcTrack->normalizedChi2());

                part->setUserRecord("LHitsCocktail",        pmcTrack->hitPattern().numberOfLostHits(reco::HitPattern::HitCategory::TRACK_HITS));
                part->setUserRecord("VHitsCocktail",        pmcTrack->hitPattern().numberOfValidHits());
                part->setUserRecord("VHitsPixelCocktail",   pmcTrack->hitPattern().numberOfValidPixelHits());
                part->setUserRecord("VHitsTrackerCocktail", pmcTrack->hitPattern().numberOfValidTrackerHits());
                part->setUserRecord("VHitsMuonSysCocktail", pmcTrack->hitPattern().numberOfValidMuonHits());

                part->setUserRecord("DzCocktail",    pmcTrack->dz(the_Pvertex));
                part->setUserRecord("DszCocktail",   pmcTrack->dsz(the_Pvertex));
                part->setUserRecord("DxyCocktail",   pmcTrack->dxy(the_Pvertex));

                part->setUserRecord("DzBVCocktail",    pmcTrack->dz(the_vertex));
                part->setUserRecord("DszBVCocktail",   pmcTrack->dsz(the_vertex));
                part->setUserRecord("DxyBVCocktail",   pmcTrack->dxy(the_vertex));

                // part->setUserRecord("DzBSCocktail",  pmcTrack->dz(the_beamspot));
                // part->setUserRecord("DszBSCocktail", pmcTrack->dsz(the_beamspot));
                // part->setUserRecord("DxyBSCocktail", pmcTrack->dxy(the_beamspot));

                part->setUserRecord("TrackerLayersWithMeasCocktail", pmcTrack->hitPattern().trackerLayersWithMeasurement());
                part->setUserRecord("PixelLayersWithMeasCocktail",   pmcTrack->hitPattern().pixelLayersWithMeasurement());
            } else {
                part->setUserRecord("validCocktail", false);
            }

            if (MC && muon->isLooseMuon()) {
              const reco::GenParticle *gen_muon = muon->genLepton();
              if (gen_muon == nullptr) {
                part->setUserRecord("gen_match", false);
              } else {
                part->setUserRecord("gen_match", true);
                part->setUserRecord("gen_isPromptFinalState", gen_muon->isPromptFinalState());
                part->setUserRecord("gen_isPromptDecayed", gen_muon->isPromptDecayed());
                part->setUserRecord("gen_isDirectPromptTauDecayProductFinalState", gen_muon->isDirectPromptTauDecayProductFinalState());
                reco::GenStatusFlags genflags = gen_muon->statusFlags();
                part->setUserRecord("gen_isTauDecayProduct", genflags.isTauDecayProduct());
                part->setUserRecord("gen_isPromptTauDecayProduct", genflags.isPromptTauDecayProduct());
                part->setUserRecord("gen_isDirectTauDecayProduct", genflags.isDirectTauDecayProduct());
                part->setUserRecord("gen_isDirectPromptTauDecayProduct", genflags.isDirectPromptTauDecayProduct());
                part->setUserRecord("gen_isDirectHadronDecayProduct", genflags.isDirectHadronDecayProduct());
              }
            }


            numMuonRec++;
        }
    }
    RecView->setUserRecord("NumMuon", numMuonRec);
    edm::LogInfo("PxlSkimmer_miniAOD|RecInfo") << "Rec Muons: " << numMuonRec;

    // Refit muon pairs to a single vertex
    if (numMuonRec >= 2) {
        edm::ESHandle<TransientTrackBuilder> ttkb;
        iSetup.get<TransientTrackRecord>().get("TransientTrackBuilder", ttkb);

        for (std::vector<std::pair<const pat::Muon *, pxl::Particle *>>::iterator it1 = refit_vec.begin();
             it1 != refit_vec.end() - 1; ++it1) {
            const pat::Muon * muon1 = it1->first;
            // Get the track reference for the first muon
            const reco::TrackRef& tk1 = muon1->tunePMuonBestTrack().isAvailable() ?
                    muon1->tunePMuonBestTrack() :
                    muon1->globalTrack();
            if (!tk1.isAvailable() || tk1->pt() < 20.0)
                continue;

            for (std::vector<std::pair<const pat::Muon *, pxl::Particle *>>::iterator it2 = it1 + 1;
                 it2 != refit_vec.end(); ++it2) {
                const pat::Muon * muon2 = it2->first;
                // Get the track reference for the second muon
                const reco::TrackRef& tk2 = muon2->tunePMuonBestTrack().isAvailable() ?
                        muon2->tunePMuonBestTrack() :
                        muon2->globalTrack();
                if (!tk2.isAvailable() || tk2->pt() < 20.0)
                    continue;

                std::vector<reco::TransientTrack> ttv;
                ttv.push_back(ttkb->build(tk1));
                ttv.push_back(ttkb->build(tk2));

                KalmanVertexFitter kvf(true);
                CachingVertex<5> cv = kvf.vertex(ttv);

                if (!cv.isValid())
                    continue;

                // Store the information in a vertex
                pxl::Vertex * vtx = RecView->create<pxl::Vertex>();
                vtx->setName("RefitVtx");
                vtx->setXYZ(cv.position().x(), cv.position().y(), cv.position().z());
                vtx->setUserRecord("chi2", cv.totalChiSquared());
                vtx->setUserRecord("ndof", cv.degreesOfFreedom());

                InvariantMassFromVertex imfv;
                const double muon_mass = 0.1056583;
                InvariantMassFromVertex::LorentzVector p4 = imfv.p4(cv, muon_mass);
                Measurement1D mass = imfv.invariantMass(cv, muon_mass);

                vtx->setUserRecord("px", p4.X());
                vtx->setUserRecord("py", p4.Y());
                vtx->setUserRecord("pz", p4.Z());

                vtx->setUserRecord("mass", mass.value());
                vtx->setUserRecord("massError", mass.error());

                // set soft relations to muons
                vtx->setUserRecord("daughterId1", (it1->second)->getUserRecord("particleId").toString());
                vtx->setUserRecord("daughterId2", (it2->second)->getUserRecord("particleId").toString());
            }
        }
    }
}



// ------------ reading Reconstructed Electrons ------------

void PxlSkimmer_miniAOD::analyzeRecElectrons(const edm::Event &iEvent,
                                               pxl::EventView *RecView,
                                               const bool &MC,
                                               // EcalClusterLazyTools &lazyTools,
                                               map< const reco::Candidate*, pxl::Particle*> &genmap,
                                               // const ESHandle< CaloGeometry > &geo,
                                               const edm::Handle< reco::VertexCollection > &vertices,
                                               const edm::Handle< pat::PackedCandidateCollection > &pfCandidates,
                                               const double &rhoFixedGrid
    ) {
    int numEleRec = 0;
    int numEleAll = 0;   // for matching

    edm::Handle< vector< pat::Electron > > electronHandle;
    iEvent.getByLabel(patElectronLabel_, electronHandle);
    const vector< pat::Electron > &patElectrons = *electronHandle;

    // edm::Handle< EcalRecHitCollection > barrelRecHits;
    // iEvent.getByLabel(freducedBarrelRecHitCollection, barrelRecHits);

    // edm::Handle< EcalRecHitCollection > endcapRecHits;
    // iEvent.getByLabel(freducedEndcapRecHitCollection, endcapRecHits);

    edm::Handle< reco::ConversionCollection > conversionsHandle;
    iEvent.getByLabel(conversionsTag_, conversionsHandle);

    // const unsigned int numIsoVals = m_inputTagIsoValElectronsPFId.size();

    // typedef in PxlSkimmer_miniAOD.h
    // IsoDepositVals eleIsoValPFId(numIsoVals);

    // for (unsigned int i = 0; i < numIsoVals; ++i) {
    // iEvent.getByLabel(m_inputTagIsoValElectronsPFId.at(i), eleIsoValPFId.at(i));
    // }

    edm::Handle<edm::View<pat::Electron> > electrons;
    iEvent.getByLabel(patElectronLabel_, electrons);

    edm::View<pat::Electron>::const_iterator el = electrons->begin();
    for (vector< pat::Electron>::const_iterator patEle = patElectrons.begin() ; patEle != patElectrons.end(); ++patEle) {
        if (Ele_cuts(patEle)) {
            edm::LogInfo("PxlSkimmer_miniAOD|RecInfo") << "Electron Energy scale corrected: " << patEle->isEnergyScaleCorrected() << endl;

            // edm::Handle< EcalRecHitCollection > recHits;

            const bool isBarrel = patEle->isEB();
            const bool isEndcap = patEle->isEE();

            // if (isBarrel) recHits = barrelRecHits;
            // if (isEndcap) recHits = endcapRecHits;

            pxl::Particle *pxlEle = RecView->create< pxl::Particle >();
            pxlEle->setName("Ele");
            pxlEle->setCharge(patEle->charge());
            pxlEle->setP4(patEle->px(), patEle->py(), patEle->pz(), patEle->ecalEnergy());
            // For the sake of completeness write the HEEP definition of Et.
            // According to:
            // https:// twiki.cern.ch/twiki/bin/view/CMS/HEEPElectronID?rev = 64#Et
            pxlEle->setUserRecord("SCEt", patEle->caloEnergy() *
                                  std::sin(patEle->p4().theta()));

            pxlEle->setUserRecord("isBarrel", isBarrel);
            pxlEle->setUserRecord("isEndcap", isEndcap);
            pxlEle->setUserRecord("isGap", patEle->isEBGap() || patEle->isEEGap() || patEle->isEBEEGap());

            //get the charge info from the chargeInfo class:
            //only one charge and three bools
            pxlEle->setUserRecord("isGsfCtfScPixConsistent",  patEle->chargeInfo().isGsfCtfScPixConsistent);
            pxlEle->setUserRecord("isGsfScPixConsistent",   patEle->chargeInfo().isGsfScPixConsistent);
            pxlEle->setUserRecord("isGsfCtfConsistent",   patEle->chargeInfo().isGsfCtfConsistent);
            pxlEle->setUserRecord("scPixCharge",   patEle->chargeInfo().scPixCharge);

            //
            // Electron variables orientated to
            // https:// twiki.cern.ch/twiki/bin/view/CMS/EgammaIDInputVariables
            //

            pxlEle->setUserRecord("PErr",   patEle->trackMomentumError());
            pxlEle->setUserRecord("SCeta",  patEle->caloPosition().eta());
            pxlEle->setUserRecord("SCEErr", patEle->ecalEnergyError());

            // Isolation variables:
            //
            // The following are there to have the same variable naming for all
            // particles with isolation.
            pxlEle->setUserRecord("CaloIso", patEle->caloIso());
            pxlEle->setUserRecord("TrkIso",  patEle->trackIso());
            pxlEle->setUserRecord("ECALIso", patEle->ecalIso());
            pxlEle->setUserRecord("HCALIso", patEle->hcalIso());
            // Track iso deposit with electron footprint removed.
            pxlEle->setUserRecord("TrkIso03", patEle->dr03TkSumPt());
            pxlEle->setUserRecord("TrkIso04", patEle->dr04TkSumPt());  // (Identical to trackIso()!)
            // ECAL iso deposit with electron footprint removed.
            pxlEle->setUserRecord("ECALIso03", patEle->dr03EcalRecHitSumEt());
            pxlEle->setUserRecord("ECALIso04", patEle->dr04EcalRecHitSumEt());  // (Identical to ecalIso()!)
            // dr03HcalDepth1TowerSumEt()+dr03HcalDepth2TowerSumEt().
            pxlEle->setUserRecord("HCALIso03", patEle->dr03HcalTowerSumEt());
            // dr04HcalDepth1TowerSumEt()+dr04HcalDepth2TowerSumEt().
            pxlEle->setUserRecord("HCALIso04", patEle->dr04HcalTowerSumEt());  // (Identical to hcalIso()!)
            // HCAL depth 1 iso deposit with electron footprint removed.
            pxlEle->setUserRecord("HCALIso03d1", patEle->dr03HcalDepth1TowerSumEt());
            pxlEle->setUserRecord("HCALIso04d1", patEle->dr04HcalDepth1TowerSumEt());
            // HCAL depth 2 iso deposit with electron footprint removed.
            pxlEle->setUserRecord("HCALIso03d2", patEle->dr03HcalDepth2TowerSumEt());
            pxlEle->setUserRecord("HCALIso04d2", patEle->dr04HcalDepth2TowerSumEt());

            // mini Isolation
            pxlEle->setUserRecord("miniIsoEA", multiIsolation_.getAbsMiniIso(*patEle));

            // Track-cluster matching variables.
            //
            // The supercluster eta - track eta position at calo extrapolated from innermost track state.
            pxlEle->setUserRecord("DEtaSCVtx", patEle->deltaEtaSuperClusterTrackAtVtx());
            // The supercluster phi - track phi position at calo extrapolated from the innermost track state.
            pxlEle->setUserRecord("DPhiSCVtx", patEle->deltaPhiSuperClusterTrackAtVtx());
            // The electron cluster eta - track eta position at calo extrapolated from the outermost state.
            pxlEle->setUserRecord("DEtaSCCalo", patEle->deltaEtaEleClusterTrackAtCalo());
            // The seed cluster eta - track eta at calo from outermost state.
            pxlEle->setUserRecord("DEtaSeedTrk", patEle->deltaEtaSeedClusterTrackAtCalo());
            // The seed cluster eta - track eta at calo extrapolated from innermost track state.
            pxlEle->setUserRecord("DEtaSeedVtx", patEle->deltaEtaSeedClusterTrackAtVtx());
            // The seed cluster phi - track phi position at calo extrapolated from the outermost track state.
            pxlEle->setUserRecord("DPhiSeedTrk", patEle->deltaPhiSeedClusterTrackAtCalo());
            // The seed cluster energy / track momentum at the PCA to the beam spot.
            pxlEle->setUserRecord("ESCSeedOverP", patEle->eSeedClusterOverP());
            // The seed cluster energy / track momentum at calo extrapolated from the outermost track state.
            pxlEle->setUserRecord("ESCSeedPout", patEle->eSeedClusterOverPout());
            // The supercluster energy / track momentum at the PCA to the beam spot.
            pxlEle->setUserRecord("EoP", patEle->eSuperClusterOverP());
            // The electron cluster energy / track momentum at calo extrapolated from the outermost track state.
            pxlEle->setUserRecord("ESCOverPout", patEle->eEleClusterOverPout());

            // Calorimeter information.
            //
            pxlEle->setUserRecord("sigmaIetaIeta", patEle->sigmaIetaIeta());
            // Energy inside 1x5 in etaxphi around the seed Xtal.
            pxlEle->setUserRecord("e1x5", patEle->e1x5());
            // Energy inside 2x5 in etaxphi around the seed Xtal (max bwt the 2 possible sums).
            pxlEle->setUserRecord("e2x5", patEle->e2x5Max());
            // Energy inside 5x5 in etaxphi around the seed Xtal.
            pxlEle->setUserRecord("e5x5", patEle->e5x5());
            // hcal over ecal seed cluster energy using first hcal depth (hcal is energy of towers within dR = 0.15).



            pxlEle->setUserRecord("full5x5_e1x5", patEle->full5x5_e1x5());
            pxlEle->setUserRecord("full5x5_e2x5Max", patEle->full5x5_e2x5Max());
            pxlEle->setUserRecord("full5x5_e5x5", patEle->full5x5_e5x5());
            pxlEle->setUserRecord("full5x5_hcalDepth1OverEcal", patEle->full5x5_hcalDepth1OverEcal());
            pxlEle->setUserRecord("full5x5_hcalDepth1OverEcalBc", patEle->full5x5_hcalDepth1OverEcalBc());
            pxlEle->setUserRecord("full5x5_hcalDepth2OverEcal", patEle->full5x5_hcalDepth2OverEcal());
            pxlEle->setUserRecord("full5x5_hcalDepth2OverEcalBc", patEle->full5x5_hcalDepth2OverEcalBc());
            pxlEle->setUserRecord("full5x5_hcalOverEcal", patEle->full5x5_hcalOverEcal());
            pxlEle->setUserRecord("full5x5_hcalOverEcalBc", patEle->full5x5_hcalOverEcalBc());
            pxlEle->setUserRecord("full5x5_r9", patEle->full5x5_r9());
            pxlEle->setUserRecord("full5x5_sigmaEtaEta", patEle->full5x5_sigmaEtaEta());
            pxlEle->setUserRecord("full5x5_sigmaIetaIeta", patEle->full5x5_sigmaIetaIeta());
            pxlEle->setUserRecord("full5x5_sigmaIetaIphi", patEle->full5x5_sigmaIetaIphi());
            pxlEle->setUserRecord("full5x5_sigmaIphiIphi", patEle->full5x5_sigmaIphiIphi());


            pxlEle->setUserRecord("HCALOverECALd1", patEle->hcalDepth1OverEcal());
            // hadronicOverEm() = hcalDepth1OverEcal() + hcalDepth2OverEcal()
            const double HoEm = patEle->hadronicOverEm();
            pxlEle->setUserRecord("HoEm", HoEm);
            // Number of basic clusters inside the supercluster - 1.
            pxlEle->setUserRecord("NumBrems", patEle->numberOfBrems());

            // Track information
            //
            // The brem fraction from gsf fit:
            // (track momentum in - track momentum out) / track momentum in
            pxlEle->setUserRecord("fbrem", patEle->fbrem());
            pxlEle->setUserRecord("pin",   patEle->trackMomentumAtVtx().R());
            pxlEle->setUserRecord("pout",  patEle->trackMomentumOut().R());

            const reco::GsfTrackRef gsfTrack = patEle->gsfTrack();
            pxlEle->setUserRecord("TrackerP", gsfTrack->p());

            pxlEle->setUserRecord("GSFNormChi2", gsfTrack->normalizedChi2());
            // Save distance to the primary vertex, best vertex and the beam spot, respectively (i.e. the impact parameter).
            pxlEle->setUserRecord("Dz",  gsfTrack->dz(the_Pvertex));
            pxlEle->setUserRecord("Dsz", gsfTrack->dsz(the_Pvertex));
            pxlEle->setUserRecord("Dxy", gsfTrack->dxy(the_Pvertex));

            pxlEle->setUserRecord("DzBV",  gsfTrack->dz(the_vertex));
            pxlEle->setUserRecord("DszBV", gsfTrack->dsz(the_vertex));
            pxlEle->setUserRecord("DxyBV", gsfTrack->dxy(the_vertex));

            pxlEle->setUserRecord("DzBS",  gsfTrack->dz(the_beamspot));
            pxlEle->setUserRecord("DszBS", gsfTrack->dsz(the_beamspot));
            pxlEle->setUserRecord("DxyBS", gsfTrack->dxy(the_beamspot));

            // Store the number of *expected* crossed layers before the first trajectory's hit.
            // If this number is 0, this is the number of missing hits in that trajectory. (This is for conversion rejection.)
            pxlEle->setUserRecord("NinnerLayerLostHits", gsfTrack->hitPattern().numberOfAllHits(reco::HitPattern::HitCategory::MISSING_INNER_HITS));
            pxlEle->setUserRecord("TrackerVHits", gsfTrack->numberOfValidHits());
            pxlEle->setUserRecord("TrackerLHits", gsfTrack->numberOfLostHits());

            //to get one more acutal charge output:
            pxlEle->setUserRecord("gsfTrackCharge", gsfTrack->charge());

            // True if the electron track had an ecalDriven seed (regardless of the
            // result of the tracker driven seed finding algorithm).
            pxlEle->setUserRecord("ecalDriven", patEle->ecalDrivenSeed());
            // True if ecalDrivenSeed is true AND the electron passes the cut based
            // preselection.
            if (patEle->passingCutBasedPreselection() || patEle->passingMvaPreselection()) {
                pxlEle->setUserRecord("ecalDrivenEle", patEle->ecalDriven());
            } else {
                pxlEle->setUserRecord("ecalDrivenEle", false);
            }


            // Conversion rejection variables.
            //
            // Difference of cot(angle) with the conversion partner track.
            pxlEle->setUserRecord("convDcot", patEle->convDcot());
            // Distance to the conversion partner.
            pxlEle->setUserRecord("convDist", patEle->convDist());
            // Signed conversion radius.
            pxlEle->setUserRecord("convRadius", patEle->convRadius());

            // Vertex coordinates.
            //
            pxlEle->setUserRecord("Vtx_X", patEle->vx());
            pxlEle->setUserRecord("Vtx_Y", patEle->vy());
            pxlEle->setUserRecord("Vtx_Z", patEle->vz());

            // Electron classification: UNKNOWN = -1, GOLDEN = 0, BIGBREM = 1, OLDNARROW = 2, SHOWERING = 3, GAP = 4.
            pxlEle->setUserRecord("Class", static_cast<int>(patEle->classification()));
            // Additional cluster variables for (spike) cleaning:
            //
            // Get the supercluster (ref) of the Electron
            // a SuperClusterRef is a edm::Ref<SuperClusterCollection>
            // a SuperClusterCollection is a std::vector<SuperCluster>
            // although we get a vector of SuperClusters an electron is only made out of ONE SC
            // therefore only the first element of the vector should be available!
            const reco::SuperClusterRef SCRef = patEle->superCluster();

            const double SCenergy = SCRef->energy();
            pxlEle->setUserRecord("SCE", SCenergy);

            // Get highest energy entry (seed) and SC ID.
            // Use EcalClusterLazyTools to store ClusterShapeVariables.
            //
            // const pair< DetId, float > max_hit = lazyTools.getMaximum(*SCRef);
            // const DetId seedID = max_hit.first;
            // const double eMax  = max_hit.second;
            // const double e3x3  = lazyTools.e3x3(*SCRef);  // Energy in 3x3 around most energetic hit.

            // pxlEle->setUserRecord("Emax", eMax);
            // pxlEle->setUserRecord("E2nd", lazyTools.e2nd(*SCRef));
            // pxlEle->setUserRecord("e3x3", e3x3);
            // pxlEle->setUserRecord("r19",  eMax / e3x3);
            // const pair< DetId, float > max_hit = lazyTools.getMaximum(*SCRef);
            // const DetId seedID = max_hit.first;
            // const double eMax  = max_hit.second;
            // const double e3x3  = lazyTools.e3x3(*SCRef);  // Energy in 3x3 around most energetic hit.

            // pxlEle->setUserRecord("scE2x5Max", patEle->scE2x5Max());
            // pxlEle->setUserRecord("E2x5Max", patEle->E2x5Max());
            // pxlEle->setUserRecord("E2nd", lazyTools.e2nd(*SCRef));
            // pxlEle->setUserRecord("e3x3", patEle->e3x3);
            // pxlEle->setUserRecord("r19",  eMax / e3x3);

            // These are the covariances, if you want the sigmas, you have to sqrt them.
            // const vector< float > covariances = lazyTools.covariances(*SCRef, 4.7);
            // pxlEle->setUserRecord("covEtaEta", covariances[0]);
            // pxlEle->setUserRecord("covEtaPhi", covariances[1]);
            // pxlEle->setUserRecord("covPhiPhi", covariances[2]);


            // not possible
            // SwissCross
            //
            // double swissCross         = -1.0;
            // double swissCrossNoBorder = -1.0;

            // if (isBarrel || isEndcap) {
            // swissCross         = EcalTools::swissCross(seedID, *recHits, 0, false);
            // swissCrossNoBorder = EcalTools::swissCross(seedID, *recHits, 0, true);

            // EcalRecHitCollection::const_iterator recHit_it = recHits->find(seedID);
            // if (recHit_it != recHits->end()) {
            // pxlEle->setUserRecord("recoFlag", recHit_it->recoFlag());
            // }
            // }

            // pxlEle->setUserRecord("SwissCross",         swissCross);
            // pxlEle->setUserRecord("SwissCrossNoBorder", swissCrossNoBorder);

            // Save eta/phi and DetId info from seed-cluster to prevent duplication of Electron/Photon-Candidates (in final selection).
            pxlEle->setUserRecord("seedId", patEle->seed()->seed().rawId());
            // pxlEle->setUserRecord("seedphi", geo->getPosition(seedID).phi());
            // pxlEle->setUserRecord("seedeta", geo->getPosition(seedID).eta());

            for (auto& eleId : patEle->electronIDs()) {
                pxlEle->setUserRecord(eleId.first, (bool) eleId.second);
            }

            pxlEle->setUserRecord("HEEPisolation", patEle->dr03TkSumPtHEEP()); //LOR COR FOR UL2017 TO FIX
	    //TO upgrade to UL17 see FROM twiki:
	    //The tracker isolation is in a value map of floats named heepIDVarValueMaps::eleTrkPtIso.

            // Conversion veto for electron ID.
            // https:// twiki.cern.ch/twiki/bin/view/CMS/ConversionTools
            //
            // const bool hasMatchedConversion = ConversionTools::hasMatchedConversion(*patEle, conversionsHandle, the_beamspot);

            // pxlEle->setUserRecord("hasMatchedConversion", hasMatchedConversion);
            pxlEle->setUserRecord("passConversionVeto", patEle->passConversionVeto());

            // Scale and Smearing values and systematics
             //LOR COR FOR UL2017 TO FIX 
	    pxlEle->setUserRecord("energyScaleValue", patEle->userFloat("energyScaleValue")); //LOR COR FOR UL2017 TO FIX
            pxlEle->setUserRecord("energySigmaValue", patEle->userFloat("energySigmaValue"));//LOR COR FOR UL2017 TO FIX
            pxlEle->setUserRecord("energyScaleUp", patEle->userFloat("energyScaleUp"));//LOR COR FOR UL2017 TO FIX
            pxlEle->setUserRecord("energyScaleDown", patEle->userFloat("energyScaleDown"));//LOR COR FOR UL2017 TO FIX
            pxlEle->setUserRecord("energySigmaUp", patEle->userFloat("energySigmaUp"));//LOR COR FOR UL2017 TO FIX
            pxlEle->setUserRecord("energySigmaDown", patEle->userFloat("energySigmaDown"));//LOR COR FOR UL2017 TO FIX
	    
            // Value after correction
            //pxlEle->setUserRecord("ecalTrkEnergyPostCorr", patEle->userFloat("ecalTrkEnergyPostCorr")); //LOR COR FOR UL2017 TO FIX //SEE BELOW THE FIX

            // 2012 definition of H/E and related HCAL isolation.
            // See also:
            // https:// twiki.cern.ch/twiki/bin/view/CMS/HoverE2012?rev = 11
            //
            pxlEle->setUserRecord("HoverE2012", patEle->hcalOverEcalBc()); //LOR COR FOR UL2017 TO FIX
            pxlEle->setUserRecord("HCALIsoConeDR03_2012", patEle->dr03HcalDepth1TowerSumEtBc()); //LOR COR FOR UL2017 TO FIX
            pxlEle->setUserRecord("HCALIsoConeDR04_2012", patEle->dr04HcalDepth1TowerSumEtBc());//LOR COR FOR UL2017 TO FIX
            // vector< CaloTowerDetId > hcalTowersBehindClusters = m_hcalHelper->hcalTowersBehindClusters(*SCRef);

            // const double hcalDepth1 = m_hcalHelper->hcalESumDepth1BehindClusters(hcalTowersBehindClusters);
            // const double hcalDepth2 = m_hcalHelper->hcalESumDepth2BehindClusters(hcalTowersBehindClusters);
            // const double HoverE2012 = (hcalDepth1 + hcalDepth2) / SCenergy;

            // const double HCALIsoConeDR03_2012 = patEle->dr03HcalTowerSumEt() +
            //                                    (HoEm - HoverE2012) *
            //                                    SCenergy / cosh(SCRef->eta());
            // const double HCALIsoConeDR04_2012 = patEle->dr04HcalTowerSumEt() +
            //                                    (HoEm - HoverE2012) *
            //                                    SCenergy / cosh(SCRef->eta());

            // pxlEle->setUserRecord("HoverE2012",           HoverE2012           );
            // pxlEle->setUserRecord("HCALIsoConeDR03_2012", HCALIsoConeDR03_2012);
            // pxlEle->setUserRecord("HCALIsoConeDR04_2012", HCALIsoConeDR04_2012);

            // Default PF based isolation for charged leptons:
            pxlEle->setUserRecord("chargedHadronIso", patEle->chargedHadronIso());
            pxlEle->setUserRecord("neutralHadronIso", patEle->neutralHadronIso());
            pxlEle->setUserRecord("photonIso",        patEle->photonIso());
            pxlEle->setUserRecord("puChargedHadronIso", patEle->puChargedHadronIso());

			//for EGamma scale and Smearing corrections
			//https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2#Energy_Scale_and_Smearing
	     //LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalTrkEnergyPreCorr", patEle->userFloat("ecalTrkEnergyPreCorr")); //LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalTrkEnergyErrPreCorr", patEle->userFloat("ecalTrkEnergyErrPreCorr"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalTrkEnergyPostCorr", patEle->userFloat("ecalTrkEnergyPostCorr"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalTrkEnergyErrPostCorr", patEle->userFloat("ecalTrkEnergyErrPostCorr"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalEnergyPreCorr", patEle->userFloat("ecalEnergyPreCorr"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalEnergyErrPreCorr", patEle->userFloat("ecalEnergyErrPreCorr"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalEnergyPostCorr", patEle->userFloat("ecalEnergyPostCorr"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("ecalEnergyErrPostCorr", patEle->userFloat("ecalEnergyErrPostCorr"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("energyScaleValue", patEle->userFloat("energyScaleValue"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("energySigmaValue", patEle->userFloat("energySigmaValue"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("energySmearNrSigma", patEle->userFloat("energySmearNrSigma"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("energyScaleUp", patEle->userFloat("energyScaleUp"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("energyScaleDown", patEle->userFloat("energyScaleDown"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("energySigmaUp", patEle->userFloat("energySigmaUp"));//LOR COR FOR UL2017 TO FIX 
            pxlEle->setUserRecord("energySigmaDown", patEle->userFloat("energySigmaDown"));//LOR COR FOR UL2017 TO FIX 
	    
            // Need a Ref to access the isolation values in particleFlowBasedIsolation(...).
            //
            pat::ElectronRef eleRef(electronHandle, numEleAll);

            if (eleRef.isNull()) {
                throw cms::Exception("Reference Error") << "Could not create valid edm::Ref() to PAT electron "
                                                        << "(no. " << numEleAll << ")!";
            }

            // particleFlowBasedIsolation(eleIsoValPFId,
            // m_eleIsolator,
            // vertices,
            // pfCandidates,
            // eleRef,
            // rhoFastJet25,
            // *pxlEle
            // );
            
            // Redo HEEP ID v7 for 2018
            if (Year_ == 2018){
                // Check if barrel or endcap
                bool const barrel = pxlEle->getUserRecord( "isBarrel" );
                bool const endcap = pxlEle->getUserRecord( "isEndcap" );

                // eta
                double const abseta = fabs( pxlEle->getUserRecord( "SCeta" ).toDouble() );
                
                // Use SC Et
                double const eleEt = pxlEle->getUserRecord( "SCEt" );

                pxlEle->setUserRecord("heepElectronID-HEEPV70-2018Prompt", passHEEPID(pxlEle, eleEt, abseta, rhoFixedGrid, barrel, endcap) and passHEEP_Isolation(pxlEle, eleEt, abseta, barrel, endcap, rhoFixedGrid));

            }
            // Store PAT matching info if MC. FIXME: Do we still use this?
            if (MC) {
                map< const reco::Candidate*, pxl::Particle* >::const_iterator it = genmap.find(patEle->genLepton());
                if (it != genmap.end()) {
                    pxlEle->linkSoft(it->second, "pat-match");
                }
            }



            numEleRec++;
        }
        numEleAll++;
        el++;
    }
    RecView->setUserRecord("NumEle", numEleRec);

    edm::LogInfo("PxlSkimmer_miniAOD|RecInfo") << "Rec Eles: " << numEleRec;
}

// Check HEEP ID v7 for 2018 (has to be redone by us)

bool PxlSkimmer_miniAOD::passHEEPID( const pxl::Particle* ele,
                                double const eleEt,
                                double const abseta,
                                double const eleRho,
                                bool const eleBarrel,
                                bool const eleEndcap
                                ) const {

   // Require electron to be ECAL driven
   if(not ele->getUserRecord( "ecalDriven") ) return false;

   // These variables are checked in the barrel as well as in the endcaps.
   double const ele_absDeltaEta = fabs( ele->getUserRecord( "DEtaSeedVtx" ).toDouble() );
   double const ele_absDeltaPhi = fabs( ele->getUserRecord( "DPhiSCVtx" ).toDouble() );
   double const ele_HoEM        = ele->getUserRecord( "HoEm" );

   double ele_E=0;
   ele_E = ele->getUserRecord( "SCE" );

   double const ele_innerLayerLostHits = ele->getUserRecord( "NinnerLayerLostHits" );

   //ele in barrel
   if( eleBarrel ) {
      //delta eta between SC and track
      if( ele_absDeltaEta > 0.004 )
         return false;

      //delta phi between SC and track
      if( ele_absDeltaPhi > 0.06 )
         return false;

      //avoid division by zero in hadronic over EM
      if( ele_E == 0 )
         return false;

      //hadronic over EM
      if( ele_HoEM > ( 1. / ele_E + 0.05) )
         return false;
      //shower shape
      double e5x5=0;
      double e1x5=0;
      double e2x5=0;
      e5x5 = ele->getUserRecord( "full5x5_e5x5" );
      e1x5 = ele->getUserRecord( "full5x5_e1x5" );
      e2x5 = ele->getUserRecord( "full5x5_e2x5Max" );


      if( e1x5/e5x5 <= 0.83 and
          e2x5/e5x5 <= 0.94
          ) return false;

      if( ele_innerLayerLostHits > 1 )
         return false;

      if( fabs( ele->getUserRecord( "Dxy" ).toDouble() ) > 0.02 )
         return false;
   }

   //ele in endcap
   if( eleEndcap ) {
      //delta eta between SC and track
      if( ele_absDeltaEta > 0.006 )
         return false;

      //delta phi between SC and track
      if( ele_absDeltaPhi > 0.06 )
         return false;

      //avoid division by zero in hadronic over EM
      if( ele_E == 0 )
         return false;

      //hadronic over EM
      if( ele_HoEM > ((-0.4+0.4*abseta)*eleRho/ele_E + 0.05) )
         return false;

      //sigma iEta-iEta
      if( ele->getUserRecord( "full5x5_sigmaIetaIeta" ).toDouble() > 0.03 )
            return false;


      if( ele_innerLayerLostHits > 1 )
         return false;

      if( fabs( ele->getUserRecord( "Dxy" ).toDouble() ) > 0.05 )
         return false;
   }
   return true;
}

bool PxlSkimmer_miniAOD::passHEEP_Isolation(const pxl::Particle* ele, double const eleEt,double const abseta, bool const eleBarrel, bool const eleEndcap, double const eleRho) const {
    double const ele_TrkIso      = ele->getUserRecord( "HEEPisolation" );
    double const ele_ECALIso     = ele->getUserRecord( "ECALIso03" );
    double const ele_HCALIso     = ele->getUserRecord( "HCALIso03d1" );
    double const ele_CaloIso     = ele_ECALIso + ele_HCALIso;
    //ele in barrel
    if( eleBarrel ) {
        //Isolation
        bool iso_ok = true;
        //HCAL iso depth 1
        double const maxIso = 2 +
                            0.03 * eleEt +
                            0.28 * eleRho;

        if( iso_ok and ele_CaloIso > maxIso ) iso_ok = false;
        //Track iso
        if( iso_ok and ele_TrkIso > 5 )
            iso_ok = false;

        //now test
        if( !iso_ok ) return false;
    }
    //ele in endcap
    else if( eleEndcap ) {
        //Isolation
        bool iso_ok = true;
        //HCAL iso depth 1
        double maxIso = 2.5 +
                      (0.15 + 0.07 * abseta) * eleRho;
        //add a slope for high energy electrons
        if( eleEt > 50.0 ) maxIso += 0.03 * ( eleEt - 50.0 );
        //now test
        if( iso_ok and ele_CaloIso > maxIso )
            iso_ok = false;
        //Track iso
        if( iso_ok and ele_TrkIso > 5 )
            iso_ok = false;
        //now test
        if( not iso_ok )
            return false;
    }else{
        return false;
    }
    return true;
}
// ------------ reading Reconstructed Jets ------------

void PxlSkimmer_miniAOD::analyzeRecJets(const edm::Event &iEvent, pxl::EventView *RecView, bool &MC, std::map< const reco::Candidate*, pxl::Particle* > &genjetmap, const jet_def &jet_info) {
    int numJetRec = 0;
    // get RecoJets
    edm::Handle< std::vector< pat::Jet > > jetHandle;
    iEvent.getByToken(jet_info.RecoToken, jetHandle);
    const std::vector< pat::Jet > &RecJets = *jetHandle;

    // generator flavour matching only available in MC. Surprise!
    // edm::Handle< reco::JetFlavourMatchingCollection > physicsFlavour;
    // if (MC) {
    //   iEvent.getByLabel(jet_info.name+"RecoJetFlavourPhysics", physicsFlavour);
    // }


    // loop over the jets
    size_t jet_index = 0;
    for (std::vector< pat::Jet >::const_iterator jet = RecJets.begin(); jet != RecJets.end(); ++jet, ++jet_index) {
        if (Jet_cuts(jet)) {
            pxl::Particle* part = RecView->create<pxl::Particle>();
            part->setName(jet_info.name);
            part->setP4(jet->px(), jet->py(), jet->pz(), jet->energy());
            part->setUserRecord("isPFJet", jet->isPFJet());
            if (jet->isPFJet() or jet->isJPTJet())
            {
               part->setUserRecord("chargedHadronEnergyFraction", jet->chargedHadronEnergyFraction());
               part->setUserRecord("chargedHadronEnergy",         jet->chargedHadronEnergy());
               part->setUserRecord("neutralHadronEnergyFraction", jet->neutralHadronEnergyFraction());
               part->setUserRecord("neutralHadronEnergy",         jet->neutralHadronEnergy());
               part->setUserRecord("chargedEmEnergyFraction",     jet->chargedEmEnergyFraction());
               part->setUserRecord("chargedEmEnergy",             jet->chargedEmEnergy());
               part->setUserRecord("neutralEmEnergyFraction",     jet->neutralEmEnergyFraction());
               part->setUserRecord("neutralEmEnergy",             jet->neutralEmEnergy());
               part->setUserRecord("muonEnergy",                  jet->muonEnergy());
               part->setUserRecord("muonEnergyFraction",          jet->muonEnergyFraction());
               part->setUserRecord("chargedMultiplicity",         jet->chargedMultiplicity());
               part->setUserRecord("neutralMultiplicity",         jet->neutralMultiplicity());
            }
            part->setUserRecord("nconstituents",               jet->numberOfDaughters());
            part->setUserRecord("uncorrectedPT",               jet->pt()*jet->jecFactor("Uncorrected"));
            part->setUserRecord("jecFactorUncorrected",        jet->jecFactor("Uncorrected"));
            part->setUserRecord("jecFactorL1FastJet",          jet->jecFactor("L1FastJet"));
            if ( iEvent.isRealData() ) part->setUserRecord("jecFactorL2L3Residual", jet->jecFactor("L2L3Residual"));
            for (jet_id_list::const_iterator ID = jet_info.IDs.begin(); ID != jet_info.IDs.end(); ++ID) {
                pat::strbitset ret = ID->second->getBitTemplate();
                part->setUserRecord(ID->first, (*(ID->second))(*jet, ret));
            }

            // calculate the kinematics with a new vertex
            // is this used in any way????
            reco::Candidate::LorentzVector physP4 = reco::Jet::physicsP4(the_vertex, *jet, jet->vertex());
            part->setUserRecord("PhysEta", physP4.eta());
            part->setUserRecord("PhysPhi", physP4.phi());
            part->setUserRecord("PhysPt",  physP4.pt());
            if (jet->hasUserFloat("NjettinessAK8Puppi:tau1")) {
               part->setUserRecord("tau1", jet->userFloat("NjettinessAK8Puppi:tau1"));    //
               part->setUserRecord("tau2", jet->userFloat("NjettinessAK8Puppi:tau2"));    //  Access the n-subjettiness variables
               part->setUserRecord("tau3", jet->userFloat("NjettinessAK8Puppi:tau3"));    //
               part->setUserRecord("prunedMass",  jet->userFloat("ak8PFJetsCHSValueMap:ak8PFJetsCHSPrunedMass"));  // access to pruned mass
               part->setUserRecord("puppi_softDropMass", jet->userFloat("ak8PFJetsPuppiSoftDropMass"));

               part->setUserRecord("chs_tau1", jet->userFloat("ak8PFJetsCHSValueMap:NjettinessAK8CHSTau1"));
               part->setUserRecord("chs_tau2", jet->userFloat("ak8PFJetsCHSValueMap:NjettinessAK8CHSTau2"));
               part->setUserRecord("chs_tau3", jet->userFloat("ak8PFJetsCHSValueMap:NjettinessAK8CHSTau3"));
            }



            part->setUserRecord("fHPD", jet->jetID().fHPD);
            part->setUserRecord("fRBX", jet->jetID().fRBX);
            // store b-tag discriminator values:
            const vector< pair< string, float > > &btags = jet->getPairDiscri();
            for (vector< pair< string, float > >::const_iterator btag = btags.begin(); btag != btags.end(); ++btag) {
                part->setUserRecord(btag->first, btag->second);
                edm::LogInfo("PxlSkimmer_miniAOD|RecInfo") << "BTag name: " << btag->first << ", value: " << btag->second;
            }
            // jet IDs

            stringstream info;
            part->print(0, info);
            edm::LogInfo("PxlSkimmer_miniAOD|RecInfo") << "PXL Jet Info: " << info.str();

            // store PAT matching info if MC
            if (MC) {
                //   // to be compared with Generator Flavor:
                part->setUserRecord("algoFlavour", jet->partonFlavour());
                part->setUserRecord("hadronFlavour", jet->hadronFlavour());
                std::map< const reco::Candidate*, pxl::Particle* >::const_iterator it = genjetmap.find(jet->genJet());
                if (it != genjetmap.end()) {
                    part->linkSoft(it->second, "pat-match");
                }
            }
            numJetRec++;
        }
    }
    RecView->setUserRecord("Num"+jet_info.name, numJetRec);
    edm::LogInfo("PxlSkimmer_miniAOD|RecInfo") << "Found Rec Jets:  " << numJetRec << " of Type " << jet_info.name;
}

// ------------ reading Reconstructed Gammas ------------

void PxlSkimmer_miniAOD::printCutFlowResult(const vid::CutFlowResult &cutflow){

    printf("    CutFlow name= %s    decision is %d\n",
    cutflow.cutFlowName().c_str(),
    (int) cutflow.cutFlowPassed());
    int ncuts = cutflow.cutFlowSize();
    printf(" Index                               cut name              isMasked    value-cut-upon     pass?\n");
    for(int icut = 0; icut<ncuts; icut++){
        printf("  %d       %50s    %d        %f          %d\n", icut,
        cutflow.getNameAtIndex(icut).c_str(),
        (int)cutflow.isCutMasked(icut),
        cutflow.getValueCutUpon(icut),
        (int)cutflow.getCutResultByIndex(icut));
  }
  //printf("    WARNING: the value-cut-upon is bugged in 7.4.7, it is always 1.0\n");

}

void PxlSkimmer_miniAOD::analyzeRecGammas(const edm::Event &iEvent,
                                            pxl::EventView *RecView,
                                            const bool &MC,
                                            // EcalClusterLazyTools &lazyTools,
                                            map< const reco::Candidate*, pxl::Particle* > &genmap,
                                            // const ESHandle< CaloGeometry > &geo,
                                            const edm::Handle< reco::VertexCollection > &vertices,
                                            const edm::Handle< pat::PackedCandidateCollection > &pfCandidates,
                                            const double &rhoFastJet25
    ) {
    // Get Photon Collection.
    edm::Handle< vector< pat::Photon > > photonHandle;
    iEvent.getByLabel(patGammaLabel_, photonHandle);
    const vector< pat::Photon > &patPhotons = *photonHandle;

    // edm::Handle< EcalRecHitCollection > barrelRecHits;
    // iEvent.getByLabel(freducedBarrelRecHitCollection, barrelRecHits);

    // edm::Handle< EcalRecHitCollection > endcapRecHits;
    // iEvent.getByLabel(freducedEndcapRecHitCollection, endcapRecHits);

    // edm::Handle< reco::ConversionCollection > conversionsHandle;
    // iEvent.getByLabel(m_conversionsTag, conversionsHandle);

    // edm::Handle< reco::GsfElectronCollection > electronsHandle;
    // iEvent.getByLabel(m_gsfElectronsTag, electronsHandle);

    // const unsigned int numIsoVals = m_inputTagIsoValPhotonsPFId.size();

    // typedef in PxlSkimmer_miniAOD.h
    // IsoDepositVals phoIsoValPFId(numIsoVals);

    // for (unsigned int i = 0; i < numIsoVals; ++i) {
    // iEvent.getByLabel(m_inputTagIsoValPhotonsPFId.at(i), phoIsoValPFId.at(i));
    // }

    int numGammaRec = 0;
    int numGammaAll = 0;
    edm::Handle<edm::View<pat::Photon> > photons;
    iEvent.getByLabel(patGammaLabel_, photons);
    edm::View<pat::Photon>::const_iterator ph = photons->begin();
    for (vector< pat::Photon >::const_iterator patPhoton = patPhotons.begin(); patPhoton != patPhotons.end(); ++patPhoton) {
        if (Gamma_cuts(patPhoton)) {
            edm::Handle< EcalRecHitCollection > recHits;

            const bool isBarrel = patPhoton->isEB();
            const bool isEndcap = patPhoton->isEE();

            // if (isBarrel) recHits = barrelRecHits;
            // if (isEndcap) recHits = endcapRecHits;

            pxl::Particle *pxlPhoton = RecView->create< pxl::Particle >();
            pxlPhoton->setName("Gamma");
            pxlPhoton->setCharge(0);
            pxlPhoton->setP4(patPhoton->px(), patPhoton->py(), patPhoton->pz(), patPhoton->energy());

            pxlPhoton->setUserRecord("isBarrel", isBarrel);
            pxlPhoton->setUserRecord("isEndcap", isEndcap);
            pxlPhoton->setUserRecord("Gap", patPhoton->isEBGap() || patPhoton->isEEGap() || patPhoton->isEBEEGap());


            // Write ID decisions to particle
            for (auto& phoId : patPhoton->photonIDs()) {
                pxlPhoton->setUserRecord(phoId.first, (bool) phoId.second);
            }

            // Scale and Smearing values and systematics
	    //LOR COR FOR UL2017 TO FIX
            pxlPhoton->setUserRecord("energyScaleValue", patPhoton->userFloat("energyScaleValue"));
            pxlPhoton->setUserRecord("energySigmaValue", patPhoton->userFloat("energySigmaValue"));
            pxlPhoton->setUserRecord("energyScaleUp", patPhoton->userFloat("energyScaleUp"));
            pxlPhoton->setUserRecord("energyScaleDown", patPhoton->userFloat("energyScaleDown"));
            pxlPhoton->setUserRecord("energySigmaUp", patPhoton->userFloat("energySigmaUp"));
            pxlPhoton->setUserRecord("energySigmaDown", patPhoton->userFloat("energySigmaDown"));
	    
            // Value after correction
            //pxlPhoton->setUserRecord("ecalEnergyPostCorr", patPhoton->userFloat("ecalEnergyPostCorr")); //LOR COR FOR UL2017 TO FIX

            // 2012 definition of H/E and related HCAL isolation.
            // Store Photon info corrected for primary vertex (this changes direction but leaves energy of SC unchanged).
            pat::Photon localPho(*patPhoton);
            // Set event vertex
            localPho.setVertex(the_Pvertex);
            pxlPhoton->setUserRecord("PhysEta", localPho.eta());
            pxlPhoton->setUserRecord("PhysPhi", localPho.phi());
            pxlPhoton->setUserRecord("PhysPt",  localPho.pt());

            // Store Photon info corrected for best vertex (this changes direction but leaves energy of SC unchanged).
            pat::Photon localBVPho(*patPhoton);
            // Set event vertex
            localBVPho.setVertex(the_vertex);
            pxlPhoton->setUserRecord("PhysBVEta", localBVPho.eta());
            pxlPhoton->setUserRecord("PhysBVPhi", localBVPho.phi());
            pxlPhoton->setUserRecord("PhysBVPt",  localBVPho.pt());

            //
            // Photon variables orientated to
            // https:// twiki.cern.ch/twiki/bin/view/CMS/EgammaIDInputVariables
            //

            pxlPhoton->setUserRecord("SCeta",  patPhoton->caloPosition().eta());

            // Isolation variables:
            //
            // The following are there to have the same variable naming for all
            // particles with isolation.
            pxlPhoton->setUserRecord("CaloIso", patPhoton->caloIso());
            pxlPhoton->setUserRecord("TrkIso",  patPhoton->trackIso());
            pxlPhoton->setUserRecord("ECALIso", patPhoton->ecalIso());
            pxlPhoton->setUserRecord("HCALIso", patPhoton->hcalIso());
            // Sum of track pT in a hollow cone of outer radius, inner radius.
            pxlPhoton->setUserRecord("TrkIsoHollow03", patPhoton->trkSumPtHollowConeDR03());
            pxlPhoton->setUserRecord("TrkIsoHollow04", patPhoton->trkSumPtHollowConeDR04());
            // Sum of track pT in a cone of dR.
            pxlPhoton->setUserRecord("TrkIso03", patPhoton->trkSumPtSolidConeDR03());
            pxlPhoton->setUserRecord("TrkIso04", patPhoton->trkSumPtSolidConeDR04());  // (Identical to trackIso()!)
            // EcalRecHit isolation.
            pxlPhoton->setUserRecord("ECALIso03", patPhoton->ecalRecHitSumEtConeDR03());
            pxlPhoton->setUserRecord("ECALIso04", patPhoton->ecalRecHitSumEtConeDR04());  // (Identical to ecalIso()!)
            // HcalDepth1Tower isolation.
            pxlPhoton->setUserRecord("HCALIso03", patPhoton->hcalTowerSumEtConeDR03());
            pxlPhoton->setUserRecord("HCALIso04", patPhoton->hcalTowerSumEtConeDR04());  // (Identical to hcalIso()!)
            // Number of tracks in a cone of dR.
            pxlPhoton->setUserRecord("TrackNum03", patPhoton->nTrkSolidConeDR03());
            pxlPhoton->setUserRecord("TrackNum04", patPhoton->nTrkSolidConeDR04());

            // Store miniAOD photon IDs


            // Store old ID PAT information.
            //const vector< pair< string, bool > > &photonIDs = patPhoton->photonIDs();
            //for (vector< pair< string, bool > >::const_iterator photonID = photonIDs.begin(); photonID != photonIDs.end(); ++photonID) {
            //    pxlPhoton->setUserRecord(photonID->first, photonID->second);
            //}
            pxlPhoton->setUserRecord("pfMVA", patPhoton->pfMVA());

            pxlPhoton->setUserRecord("seedId", patPhoton->seed()->seed().rawId());


            const double HoEm = patPhoton->hadronicOverEm();
            pxlPhoton->setUserRecord("HoEm", HoEm);

            // Additional cluster variables for (spike) cleaning:
            //
            // Get the supercluster (ref) of the Electron
            // a SuperClusterRef is a edm::Ref<SuperClusterCollection>
            // a SuperClusterCollection is a std::vector<SuperCluster>
            // although we get a vector of SuperClusters an electron is only made out of ONE SC
            // therefore only the first element of the vector should be available!
            const reco::SuperClusterRef SCRef = patPhoton->superCluster();

            // 2012 definition of H/E and related HCAL isolation.
            // See also:
            // https:// twiki.cern.ch/twiki/bin/view/CMS/HoverE2012?rev = 11
            //
            // const vector< CaloTowerDetId > hcalTowersBehindClusters = m_hcalHelper->hcalTowersBehindClusters(*SCRef);
            const double hadTowOverEm           = patPhoton->hadTowOverEm();

            const double HCALIsoConeDR03_2012 = patPhoton->hcalTowerSumEtConeDR03() +
                (HoEm - hadTowOverEm) *
                SCRef->energy() / cosh(SCRef->eta());

            const double HCALIsoConeDR04_2012 = patPhoton->hcalTowerSumEtConeDR04() +
                (HoEm - hadTowOverEm) *
                SCRef->energy() / cosh(SCRef->eta());

            pxlPhoton->setUserRecord("hadTowOverEm", hadTowOverEm);
            pxlPhoton->setUserRecord("HCALIsoConeDR03_2012", HCALIsoConeDR03_2012);
            pxlPhoton->setUserRecord("HCALIsoConeDR04_2012", HCALIsoConeDR04_2012);



            // Default PF based isolation for charged leptons (deprecated by value maps for now)
            pxlPhoton->setUserRecord("chargedHadronIso", patPhoton->chargedHadronIso()); 
            pxlPhoton->setUserRecord("neutralHadronIso", patPhoton->neutralHadronIso()); 
            pxlPhoton->setUserRecord("photonIso",        patPhoton->photonIso()); 
            pxlPhoton->setUserRecord("puChargedHadronIso", patPhoton->puChargedHadronIso());
            // Isolations from value maps as recommeded by EGamma POG
            // see https://hypernews.cern.ch/HyperNews/CMS/get/egamma/1736/1.html
            // and https://github.com/ikrav/EgammaWork/blob/master/PhotonNtupler/plugins/SimplePhotonNtupler.cc
            //LOR COR FOR UL2017 TO FIX
	    //pxlPhoton->setUserRecord("chargedHadronIso", patPhoton->userFloat("phoChargedIsolation"));
            //pxlPhoton->setUserRecord("neutralHadronIso", patPhoton->userFloat("phoNeutralHadronIsolation"));
            //pxlPhoton->setUserRecord("photonIso", patPhoton->userFloat("phoPhotonIsolation"));
	    
            // Store PAT matching info if MC. FIXME: Do we still use this?
            if (MC) {
                std::map< const reco::Candidate*, pxl::Particle* >::const_iterator it = genmap.find(patPhoton->genPhoton());
                if (it != genmap.end()) {
                    pxlPhoton->linkSoft(it->second, "pat-match");
                }
            }


            // Whether or not the SuperCluster has a matched pixel seed (electron veto).
            pxlPhoton->setUserRecord("HasSeed", patPhoton->hasPixelSeed());

            // Store information about converted state.
            pxlPhoton->setUserRecord("Converted", patPhoton->hasConversionTracks());

            pxlPhoton->setUserRecord("etaWidth", SCRef->etaWidth());
            pxlPhoton->setUserRecord("phiWidth", SCRef->phiWidth());
            // Set hadronic over electromagnetic energy fraction.

            // Raw uncorrected energy (sum of energies of component BasicClusters).
            pxlPhoton->setUserRecord("rawEnergy", SCRef->rawEnergy());
            // Energy deposited in preshower.
            pxlPhoton->setUserRecord("preshowerEnergy", SCRef->preshowerEnergy());

            // Shower shape variables
            pxlPhoton->setUserRecord("e1x5", patPhoton->e1x5());
            pxlPhoton->setUserRecord("e2x5", patPhoton->e2x5());
            pxlPhoton->setUserRecord("e3x3", patPhoton->e3x3());
            pxlPhoton->setUserRecord("e5x5", patPhoton->e5x5());
            pxlPhoton->setUserRecord("maxEnergyXtal", patPhoton->maxEnergyXtal());
            pxlPhoton->setUserRecord("sigma_Eta_Eta", patPhoton->sigmaEtaEta());
            pxlPhoton->setUserRecord("sigma_iEta_iEta", patPhoton->sigmaIetaIeta());
            pxlPhoton->setUserRecord("r1x5", patPhoton->r1x5());
            pxlPhoton->setUserRecord("r2x5", patPhoton->r2x5());
            pxlPhoton->setUserRecord("r9",        patPhoton->r9());
            // full5x5 Shower shape variables
            pxlPhoton->setUserRecord("full5x5_e1x5", patPhoton->full5x5_e1x5());
            pxlPhoton->setUserRecord("full5x5_e2x5", patPhoton->full5x5_e2x5());
            pxlPhoton->setUserRecord("full5x5_e3x3", patPhoton->full5x5_e3x3());
            pxlPhoton->setUserRecord("full5x5_e5x5", patPhoton->full5x5_e5x5());
            pxlPhoton->setUserRecord("full5x5_maxEnergyXtal", patPhoton->full5x5_maxEnergyXtal());
            pxlPhoton->setUserRecord("full5x5_sigma_Eta_Eta", patPhoton->full5x5_sigmaEtaEta());
            pxlPhoton->setUserRecord("full5x5_sigma_iEta_iEta", patPhoton->full5x5_sigmaIetaIeta());
            pxlPhoton->setUserRecord("full5x5_r1x5", patPhoton->full5x5_r1x5());
            pxlPhoton->setUserRecord("full5x5_r2x5", patPhoton->full5x5_r2x5());
            pxlPhoton->setUserRecord("full5x5_r9",        patPhoton->full5x5_r9());

			//for EGamma scale and Smearing corrections
			//https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2#Energy_Scale_and_Smearing
             //LOR COR FOR UL2017 TO FIX
	    pxlPhoton->setUserRecord("ecalEnergyPreCorr", patPhoton->userFloat("ecalEnergyPreCorr"));
            pxlPhoton->setUserRecord("ecalEnergyErrPreCorr", patPhoton->userFloat("ecalEnergyErrPreCorr"));
            pxlPhoton->setUserRecord("ecalEnergyPostCorr", patPhoton->userFloat("ecalEnergyPostCorr"));
            pxlPhoton->setUserRecord("ecalEnergyErrPostCorr", patPhoton->userFloat("ecalEnergyErrPostCorr"));
            pxlPhoton->setUserRecord("energyScaleValue", patPhoton->userFloat("energyScaleValue"));
            pxlPhoton->setUserRecord("energySigmaValue", patPhoton->userFloat("energySigmaValue"));
            pxlPhoton->setUserRecord("energySmearNrSigma", patPhoton->userFloat("energySmearNrSigma"));
            pxlPhoton->setUserRecord("energyScaleUp", patPhoton->userFloat("energyScaleUp"));
            pxlPhoton->setUserRecord("energyScaleDown", patPhoton->userFloat("energyScaleDown"));
            pxlPhoton->setUserRecord("energySigmaUp", patPhoton->userFloat("energySigmaUp"));
            pxlPhoton->setUserRecord("energySigmaDown", patPhoton->userFloat("energySigmaDown"));
	    

            // pxlPhoton->setUserRecord("scE2x5Max", patPhoton->scE2x5Max());
            // pxlPhoton->setUserRecord("E2x5Max",   patpatPhotonEle->E2x5Max());



            // FIXME still needed??? Is there a way to get it from mini AOD??
            // Conversion safe electron veto for photon ID.
            // ("Conversion-safe" since it explicitly checks for the presence of a
            // reconstructed conversion.)
            // See also:
            // https:// twiki.cern.ch/twiki/bin/view/CMS/ConversionTools
            //
            // const bool hasMatchedPromptElectron = ConversionTools::hasMatchedPromptElectron(SCRef,
            // electronsHandle,
            // conversionsHandle,
            // the_beamspot);
            // pxlPhoton->setUserRecord("hasMatchedPromptElectron", hasMatchedPromptElectron);
            pxlPhoton->setUserRecord("passElectronVeto", patPhoton->passElectronVeto());

            //pxlPhoton->setUserRecord("MVA_wp90_value", patPhoton->userFloat("PhotonMVAEstimatorRunIIFall17v1Values")); //LOR COR FOR UL2017 TO FIX
            //pxlPhoton->setUserRecord("MVA_wp90_category", patPhoton->userInt("PhotonMVAEstimatorRunIIFall17v1Categories")); //LOR COR FOR UL2017 TO FIX
            pxlPhoton->setUserRecord("MVA_wp90v1p1_value", patPhoton->userFloat("PhotonMVAEstimatorRunIIFall17v1p1Values"));
            pxlPhoton->setUserRecord("MVA_wp90v1p1_category", patPhoton->userInt("PhotonMVAEstimatorRunIIFall17v1p1Categories"));
	    pxlPhoton->setUserRecord("MVA_wp90v2_value", patPhoton->userFloat("PhotonMVAEstimatorRunIIFall17v2Values")); //LOR COR FOR UL2017 TO FIX
	    pxlPhoton->setUserRecord("MVA_wp90v2_category", patPhoton->userInt("PhotonMVAEstimatorRunIIFall17v2Categories")); //LOR COR FOR UL2017 TO FIX

            numGammaRec++;
        }
        numGammaAll++;
        ph++;
    }
    RecView->setUserRecord("NumGamma", numGammaRec);

    edm::LogInfo("PxlSkimmer_miniAOD|RecInfo") << "Rec Gammas: " << numGammaRec;
}

void PxlSkimmer_miniAOD::analyzePrefiringWeights(edm::Event const &iEvent, pxl::EventView *RecEvtView) const {
    edm::Handle< double > theprefweight;
    iEvent.getByToken(prefweight_token, theprefweight);
    RecEvtView->setUserRecord("prefiring_scale_factor", *theprefweight);

    edm::Handle< double > theprefweightup;
    iEvent.getByToken(prefweightup_token, theprefweightup);
    RecEvtView->setUserRecord("prefiring_scale_factor_up", *theprefweightup);

    edm::Handle< double > theprefweightdown;
    iEvent.getByToken(prefweightdown_token, theprefweightdown);
    RecEvtView->setUserRecord("prefiring_scale_factor_down", *theprefweightdown);
}

// ------------ method called once each job just after ending the event loop  ------------

void PxlSkimmer_miniAOD::endJob() {
    std::cout << "++++++++++++++++++++++++++++++++++++++" << endl;
    std::cout << "analyzed " << fNumEvt << " events " << endl;
    // close output file:
    fePaxFile->close();

    // write a single EOF byte at the end of the file
    // that doesn't hurt PXL, but should avoid the "file has zero size" stage-out problem
    system(("echo -e \\0004 >> "+FileName_).c_str());
}
// ------------ method to define MC-TAU-cuts

bool PxlSkimmer_miniAOD::TauMC_cuts(const reco::GenParticle *MCtau) const {
    if (MCtau->pt() < min_tau_pt) return false;
    if (fabs(MCtau->eta()) > max_eta) return false;
    return true;
}

// ------------ method to define MC-MUON-cuts

bool PxlSkimmer_miniAOD::MuonMC_cuts(const reco::GenParticle *MCmuon) const {
    if (MCmuon->pt() < min_muon_pt) return false;
    if (fabs(MCmuon->eta()) > max_eta) return false;
    return true;
}



// ------------ method to define MC-Electron-cuts

bool PxlSkimmer_miniAOD::EleMC_cuts(const reco::GenParticle *MCele) const {
    if (MCele->pt() < min_ele_pt) return false;
    if (fabs(MCele->eta()) > max_eta) return false;
    return true;
}

// ------------ method to define MC-Gamma-cuts

bool PxlSkimmer_miniAOD::GammaMC_cuts(const reco::GenParticle *MCgamma) const {
    if (MCgamma->pt() < min_gamma_pt) return false;
    if (fabs(MCgamma->eta()) > max_eta) return false;
    return true;
}

// ------------ method to define MC-Jet-cuts

bool PxlSkimmer_miniAOD::JetMC_cuts(reco::GenJetCollection::const_iterator MCjet) const {
    if (MCjet->pt() < min_jet_pt) return false;
    if (fabs(MCjet->eta()) > max_eta) return false;
    return true;
}

// ------------ method to define MC-MET-cuts

bool PxlSkimmer_miniAOD::METMC_cuts(const pxl::Particle* MCmet) const {
    if (MCmet->getPt() < min_met) return false;
    return true;
}

// ------------ method to define RecVertex-cuts
bool PxlSkimmer_miniAOD::Vertex_cuts(reco::VertexCollection::const_iterator vertex) const {
    return (vertex->ndof() >= vertex_minNDOF
            && fabs(vertex->z()) <= vertex_maxZ
            && vertex->position().rho() <= vertex_maxR);
}

bool PxlSkimmer_miniAOD::PV_vertex_cuts(reco::VertexCollection::const_iterator vertex) const {
    return (vertex->ndof() >= PV_minNDOF
            && fabs(vertex->z()) <= PV_maxZ
            && vertex->position().rho() <= PV_maxR);
}


// ------------ method to define TAU-cuts

bool PxlSkimmer_miniAOD::Tau_cuts(const pat::Tau &tau) const {
    // basic preselection cuts
    if (tau.pt() < min_tau_pt)  return false;
    if (fabs(tau.eta()) > max_eta) return false;
    return true;
}

// ------------ method to define MUON-cuts

bool PxlSkimmer_miniAOD::Muon_cuts(const pat::Muon& muon) const {
    // basic preselection cuts

    if (muon.pt() < min_muon_pt)  return false;
    if (fabs(muon.eta()) > max_eta) return false;

    if (muon.hasUserInt("muonsCleaned:oldPF")) {
        if ((bool)muon.userInt("muonsCleaned:oldPF") != muon.isPFMuon()) {
            return false;
        }
    }

    return true;
}


// ------------ method to define ELECTRON-cuts

bool PxlSkimmer_miniAOD::Ele_cuts(std::vector<pat::Electron>::const_iterator ele) const {
    if (ele->pt() < min_ele_pt) return false;
    // both SCEta and PAT eta should be smaller than 3 to keep particle
    if (fabs(ele->eta()) > max_eta && fabs(ele->caloPosition().eta()) > max_eta) return false;
    return true;
}

// ------------ method to define JET-cuts

bool PxlSkimmer_miniAOD::Jet_cuts(std::vector<pat::Jet>::const_iterator jet) const {
    if (jet->pt() < min_jet_pt) return false;
    if (fabs(jet->eta()) > max_eta) return false;
    return true;
}


// ------------ method to define GAMMA-cuts

bool PxlSkimmer_miniAOD::Gamma_cuts(std::vector<pat::Photon>::const_iterator photon) const {
    if (photon->pt() < min_gamma_pt) return false;
    if (fabs(photon->eta()) > max_eta && fabs(photon->caloPosition().eta()) > max_eta) return false;
    return true;
}


// ------------ method to define MET-cuts

bool PxlSkimmer_miniAOD::MET_cuts(const pxl::Particle* met) const {
    if (met->getPt() < min_met) return false;
    return true;
}

// ------------------------------------------------------------------------------


// recrusive mother searcher for gen trees
vector<const reco::GenParticle*> PxlSkimmer_miniAOD::runGenDecayTree(const reco::GenParticle* part,  std::map< const reco::Candidate*, pxl::Particle* > genMatchMap) {
    vector<const reco::GenParticle*> mothers;

    if ((part->numberOfMothers()) > 2) {
        return mothers;
    }
    for (size_t jmother = 0; jmother < part->numberOfMothers(); jmother++) {
        const reco::GenParticle* mother_part = (const reco::GenParticle*) part->mother(jmother);

        if (genMatchMap.end() == genMatchMap.find(mother_part)) {
            vector<const reco::GenParticle*> tmp = runGenDecayTree(mother_part, genMatchMap);
            for (size_t kmother = 0; kmother < tmp.size(); kmother++) {
                mothers.push_back(tmp[kmother]);
            }
        } else {
            mothers.push_back(mother_part);
        }
    }
    return mothers;
}
// Accessing ParticleFlow based isolation (both methods):
// (See also: https:// twiki.cern.ch/twiki/bin/view/CMS/EgammaPFBasedIsolation)
//
// template< typename T >
// void PxlSkimmer_miniAOD::particleFlowBasedIsolation(IsoDepositVals const &isoValPFId,
// PFIsolationEstimator *isolator,
// edm::Handle< reco::VertexCollection > const &vertices,
// edm::Handle< reco::PFCandidateCollection > const &pfCandidates,
// Ref< T > const &ref,
// double const &rhoFastJet25,
// pxl::Particle &part,
// bool const useIsolator
// ) const {
//  // The first method works but is NOT recommended for photons!
//  // Instead use the alternative method with PFIsolationEstimator.
//  // const double pfIsoCharged = (*isoValPFId.at(0))[ ref ];
//  // const double pfIsoPhoton  = (*isoValPFId.at(1))[ ref ];
//  // const double pfIsoNeutral = (*isoValPFId.at(2))[ ref ];

//  // part.setUserRecord("PFIso03ChargedHadron", pfIsoCharged);
//  // part.setUserRecord("PFIso03NeutralHadron", pfIsoNeutral);
//  // part.setUserRecord("PFIso03Photon",        pfIsoPhoton  );

//  // PU corrected isolation for electrons, according to:
//  // http:// cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/EGamma/EGammaAnalysisTools/test/ElectronIsoAnalyzer.cc
// if (ref->isElectron()) {
//  // const double absEta  = fabs(ref->superCluster()->eta());
//  // const double effArea = ElectronEffectiveArea::GetElectronEffectiveArea(m_eleEffAreaType, absEta, m_eleEffAreaTarget);

//  // const double PFIsoPUCorrected = pfIsoCharged + max(0.0, (pfIsoPhoton + pfIsoNeutral) - effArea * rhoFastJet25);

//  // part.setUserRecord("EffectiveArea",      effArea          );
//  // part.setUserRecord("PFIso03PUCorrected", PFIsoPUCorrected);
// }

//  // This is the recommended method for photons!
// if (useIsolator) {
// const PFCandidateCollection thePFCollection = *pfCandidates;

//  // Primary Vertex
// const reco::VertexRef vtxRef(vertices, 0);

// isolator->fGetIsolation(&*ref, &thePFCollection, vtxRef, vertices);

// part.setUserRecord("PFIso03ChargedHadronFromIsolator", isolator->getIsolationCharged());
// part.setUserRecord("PFIso03NeutralHadronFromIsolator", isolator->getIsolationNeutral());
// part.setUserRecord("PFIso03PhotonFromIsolator",        isolator->getIsolationPhoton()  );
// }
// }


void PxlSkimmer_miniAOD::printEventContent(pxl::EventView const *GenEvtView,
                                             pxl::EventView const *RecEvtView,
                                             bool const &IsMC) const {
    if (!GenOnly_) {
        std::string const ele = "ele";
        std::string const muo = "muo";
        std::string const tau = "tau";
        std::string const gam = "gam";
        std::string const s   = "   ";

        stringstream info;
        info << "Found the following objects: " << endl;
        if (IsMC) {
            // Header:
            info << "Gen: " << ele + s + muo + s + tau + s + gam + s;

            for (std::vector< jet_def >::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info) {
                info << jet_info->name + s;
            }

            // for (VInputTag::const_iterator genMET = m_genMETTags.begin(); genMET != m_genMETTags.end(); ++genMET) {
            // info << (*genMET).label() + s;
            // }
            info << endl;

            // Actual numbers:
            info << "     ";
            if(GenEvtView->hasUserRecord("NumEle")){
                info << setw(ele.size()) << GenEvtView->getUserRecord("NumEle") << s;
            }
            if(GenEvtView->hasUserRecord("NumMuon")){
                info << setw(muo.size()) << GenEvtView->getUserRecord("NumMuon") << s;
            }
            if(GenEvtView->hasUserRecord("NumTau")){
                info << setw(tau.size()) << GenEvtView->getUserRecord("NumTau") << s;
            }
            if(GenEvtView->hasUserRecord("NumGamma")){
                info << setw(gam.size()) << GenEvtView->getUserRecord("NumGamma") << s;
            }
            // for (std::vector< jet_def >::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info) {
            //   info << setw((jet_info->name).size()) << GenEvtView->getUserRecord("Num" + jet_info->name) << s;
            // }

            // for (VInputTag::const_iterator genMET = m_genMETTags.begin(); genMET != m_genMETTags.end(); ++genMET) {
            // info << setw((*genMET).label().size()) << GenEvtView->getUserRecord("Num" + (*genMET).label()) << s;
            // }
            info << endl;
        }

        // Header:
        info << "Rec: " << ele + s + muo + s;

        // for (VInputTag::const_iterator patTau = patTauTags_.begin();
        // patTau != patTauTags_.end();
        // ++patTau
        // ) {
        // info << (*patTau).label() + s;
        // }
        info << patTauTag_.label() + s;

        info << gam + s;

        for (std::vector< jet_def >::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info) {
            info << jet_info->name + s;
        }

        info << patMETTag_.label() + s;

        // for (VInputTag::const_iterator patMET = m_patMETTags.begin(); patMET != m_patMETTags.end(); ++patMET) {
        // info << (*patMET).label() + s;
        // }

        // for (VInputTag::const_iterator recoPFMET = m_recoPFMETTags.begin(); recoPFMET != m_recoPFMETTags.end(); ++recoPFMET) {
        // info << (*recoPFMET).label() + s;
        // }
        info << endl;

        // Actual numbers:
        info << "     ";
        if(RecEvtView->hasUserRecord("NumEle")){
            info << setw(ele.size()) << RecEvtView->getUserRecord("NumEle") << s;
        }
        if(RecEvtView->hasUserRecord("NumMuon")){
            info << setw(muo.size()) << RecEvtView->getUserRecord("NumMuon") << s;
        }

        // for (VInputTag::const_iterator patTau = patTauTags_.begin();
        //     patTau != patTauTags_.end();
        //     ++patTau
        //     ) {
        //   info << setw((*patTau).label().size());
        //   info << RecEvtView->getUserRecord("Num" + (*patTau).label());
        //   info << s;
        // }
        if(RecEvtView->hasUserRecord("NumGamma")){
            info << setw(gam.size()) << RecEvtView->getUserRecord("NumGamma") << s;
        }

        for (std::vector< jet_def >::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info) {
            if(RecEvtView->hasUserRecord("Num" + jet_info->name)){
                info << setw((jet_info->name).size()) << RecEvtView->getUserRecord("Num" + jet_info->name) << s;
            }
        }

        // for (VInputTag::const_iterator patMET = m_patMETTags.begin(); patMET != m_patMETTags.end(); ++patMET) {
        // info << setw((*patMET).label().size()) << RecEvtView->getUserRecord("Num" + (*patMET).label()) << s;
        // }

        // for (VInputTag::const_iterator recoPFMET = m_recoPFMETTags.begin(); recoPFMET != m_recoPFMETTags.end(); ++recoPFMET) {
        // info << setw((*recoPFMET).label().size()) << RecEvtView->getUserRecord("Num" + (*recoPFMET).label()) << s;
        // }
        info << endl;

        edm::LogVerbatim("PxlSkimmer_miniAOD|EventInfo") << info.str();
    }
}


#include "FWCore/Framework/interface/MakerMacros.h"

// define this as a plug-in
DEFINE_FWK_MODULE(PxlSkimmer_miniAOD);
                                                                                                                                                                                                                                                                                                                     
