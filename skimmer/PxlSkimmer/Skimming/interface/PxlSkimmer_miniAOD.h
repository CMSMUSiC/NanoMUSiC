// -*- C++ -*-
// Copyright [2015] <RWTH Aachen, III. Phys. Inst. A>

#ifndef SKIMMING_INTERFACE_PXLSKIMMER_MINIAOD_H_
#define SKIMMING_INTERFACE_PXLSKIMMER_MINIAOD_H_

// LHAPDF stuff
extern "C" {
    void initpdfset_(char *, int len);
    void initpdfsetm_(int &, char *);
    void initpdf_(int &);
    void evolvepdf_(double &, double &, double *);
    void numberpdf_(int &);
}

// EDM related stuff
// (Don't need headers here, forward declarations are enough!)
namespace edm {
class Event;
class EventSetup;
class ParameterSet;
template< class T > class ESHandle;
}

// GEN stuff.
namespace gen {
class PdfInfo;
}

// RECO forward declarations.
namespace reco {
class Candidate;
class GenParticle;
class Vertex;
}

// PAT related stuff.
namespace pat {
class Electron;
class Muon;
class Photon;
class Jet;
class Tau;
class PackedCandidate;
}

// No namespace for these classes.
class CaloGeometry;
class ElectronHcalHelper;
// class EcalClusterLazyTools;
class ParticleMatcher;
class PFIsolationEstimator;

// STL
#include <map>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include <assert.h>

// CMSSW includes
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Utilities/interface/InputTag.h"

// GenJetCollection and related typedefs and forward declarations.
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHERunInfoProduct.h"

// PFCandidates
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

// TauDiscriminators
#include "DataFormats/PatCandidates/interface/PATTauDiscriminator.h"

// Beam Spot.
#include "DataFormats/BeamSpot/interface/BeamSpot.h"

// HLT stuff.
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"
#include "RecoEgamma/EgammaElectronAlgos/interface/ElectronHcalHelper.h"

// Jet selectors (need to be included here, otherwise there are (namespace) problems).
#include "PhysicsTools/SelectorUtils/interface/JetIDSelectionFunctor.h"
#include "PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h"

// Vertex typedefs / forward declarations.
#include "DataFormats/VertexReco/interface/VertexFwd.h"

// Pile-Up information.
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

// Private collection defintions.
#include "PxlSkimmer/Skimming/interface/collection_def.h"

// EGamma stuff.
#include "EgammaAnalysis/ElectronTools/interface/ElectronEffectiveArea.h"
#include "DataFormats/PatCandidates/interface/VIDCutFlowResult.h"
// Trigger stuff
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"

// multi isolation
#include "PxlSkimmer/Skimming/interface/MultiIsolation.h"

// PXL stuff
// Has to be included as the last header otherwise there will be a warning concerning the
// zlib. According to Steffen there are two different zlib and ROOT can only deal with one of them
// but PXL can deal with both of them
// #include "PxlSkimmer/Pxl/interface/Pxl.h"
// #include "Pxl/Pxl/interface/Pxl.h"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

class Range
{
public:
  //explicit Range(int item): mLow(item),mHigh(item){}  // [item,item]
  Range(int low, int high): mLow(low), mHigh(high){}  // [low,high]

  bool operator<(const Range& rhs) const
  {
    if (mLow < rhs.mLow)
    {
	  assert(mHigh < rhs.mLow); // sanity check
      return true;
    }
    return false;
  } // operator<
  bool fits_in(const int item) const{
	  if (mLow<=item && mHigh>=item){
		  return true;
	  }
	  return false;
  }
  int low() const { return mLow; }
  int high() const { return mHigh; }

private:
  int mLow;
  int mHigh;
}; // class Range

typedef std::map<Range, std::string> ranges_type;

class PxlSkimmer_miniAOD : public edm::EDAnalyzer {
  public:
    // Why explicit?
    // The explicit keyword prevents the constructor from being invoked
    // implicitly as a conversion (what can be done for constructors with one
    // argument).
    explicit PxlSkimmer_miniAOD(const edm::ParameterSet&);
    ~PxlSkimmer_miniAOD();

  private:
    typedef std::set< std::string > sstring;
    typedef std::vector< std::string > vstring;
    typedef std::vector< edm::InputTag > VInputTag;
    // for PF isolation
    typedef std::vector< edm::Handle< edm::ValueMap< double > > > IsoDepositVals;
    // information about one single trigger
    struct trigger_def {
        std::string name;
        unsigned int ID;
        bool active;
    };
    typedef std::vector< trigger_def > vtrigger_def;
    // information about one trigger group
    struct trigger_group {
        std::string   name;
        std::string   process;
        edm::InputTag results;
        edm::InputTag event;
        HLTConfigProvider config;
        // std::set because duplicates make no sense here.
        sstring triggers_names;
        sstring datastreams;
        vtrigger_def trigger_infos;
        // Map the triggers to the corresponding datastream.
        std::map< string, sstring > triggers_by_datastream;
        // Map the trigger_def objects to the corresponding datastream.
        std::map< string, vtrigger_def > trigger_infos_by_datastream;
    };

    virtual void beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
    virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
    virtual string  find_inmap(int item, const ranges_type& ranges);
    virtual void analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup);
    virtual void endJob();
    virtual void analyzeGenInfo(const edm::Event &iEvent, pxl::EventView *EvtView, std::map< const reco::Candidate*, pxl::Particle* > &genmap);
    virtual void analyzeGenRelatedInfo(const edm::Event&, pxl::EventView*);
    virtual void analyzeGenJets(const edm::Event &iEvent, pxl::EventView *GenEvtView, std::map< const reco::Candidate*, pxl::Particle* > &genjetmap, const jet_def &jet_info);

    virtual void analyzeGenMET(edm::Event const &iEvent,
                               pxl::EventView *EvtView) const;

    virtual void analyzeSIM(const edm::Event&, pxl::EventView*);

    virtual void initializeFilter(edm::Event const &event,
                                  edm::EventSetup const &setup,
                                  trigger_group &filter) const;

    virtual void analyzeFilter(const edm::Event &iEvent,
                               const edm::EventSetup &iSetup,
                               pxl::EventView *EvtView,
                               trigger_group &filter);

    virtual void analyseMETFilter(const edm::Event &iEvent,pxl::EventView *EvtView);

    virtual bool analyzeTrigger(const edm::Event &iEvent,
                                pxl::EventView *EvtView);

    virtual void analyzeRecVertices(const edm::Event&, pxl::EventView*);

    // virtual void analyzeRecTracks(edm::Event const &iEvent,
    // pxl::EventView *RecEvtView
    // ) const;

    virtual void analyzeRecTaus(edm::Event const &iEvent,
                                pxl::EventView *RecEvtView) const;

    virtual void analyzeRecPatTaus(edm::Event const &iEvent,
                                   edm::InputTag const &tauTag,
                                   pxl::EventView *RecEvtView) const;

    virtual void analyzeRecMuons(edm::Event const &iEvent,
                                 edm::EventSetup const &iSetup,
                                 pxl::EventView *RecView,
                                 bool const &MC,
                                 std::map< reco::Candidate const*, pxl::Particle* > &genmap,
                                 reco::Vertex const &PV) const;

    // virtual void analyzeRecMuons(const edm::Event &iEvent, pxl::EventView *RecView, const bool &MC, std::map< const reco::Candidate*, pxl::Particle* > &genmap);
    virtual void analyzeRecElectrons(const edm::Event &iEvent,
                                     pxl::EventView *RecView,
                                     const bool &MC,
                                     // EcalClusterLazyTools &lazyTools,
                                     std::map< const reco::Candidate*, pxl::Particle* > &genmap,
                                     // const edm::ESHandle< CaloGeometry > &geo,
                                     const edm::Handle< reco::VertexCollection > &vertices,
                                     const edm::Handle< pat::PackedCandidateCollection > &pfCandidates,
                                     const double &rhoFixedGrid);

    virtual void analyzeRecJets(const edm::Event &iEvent, pxl::EventView *RecView, bool &MC, std::map< const reco::Candidate*, pxl::Particle* > &genjetmap, const jet_def &jet_info);

    virtual void analyzeRecMETs(edm::Event const &iEvent,
                                pxl::EventView *RecEvtView) const;

    virtual void analyzeRecPatMET(edm::Event const &iEvent,
                                  edm::InputTag const &patMETTag,
                                  pxl::EventView *RecEvtView) const;

    virtual void analyzeRecPUPPIMET(edm::Event const &iEvent,
                                    edm::InputTag const &recoPUPPIMETTag,
                                    pxl::EventView *RecEvtView) const;

    virtual void analyzeRecGammas(const edm::Event &iEvent,
                                  pxl::EventView *RecView,
                                  const bool &MC,
                                  // EcalClusterLazyTools &lazyTools,
                                  std::map< const reco::Candidate*, pxl::Particle* > &genmap,
                                  // const edm::ESHandle< CaloGeometry > &geo,
                                  const edm::Handle< reco::VertexCollection > &vertices,
                                  // const edm::Handle< reco::PFCandidateCollection > &pfCandidates,
                                  const edm::Handle< pat::PackedCandidateCollection > &pfCandidates,
                                  const double &rhoFastJet25);

    void analyzePrefiringWeights(edm::Event const &iEvent,
                                 pxl::EventView *RecEvtView) const;

    bool TauMC_cuts(const reco::GenParticle *MCtau) const;
    bool MuonMC_cuts(const reco::GenParticle* MCmuon) const;
    bool EleMC_cuts(const reco::GenParticle* MCele) const;
    bool GammaMC_cuts(const reco::GenParticle* MCgamma) const;
    bool JetMC_cuts(reco::GenJetCollection::const_iterator MCjet) const;
    bool METMC_cuts(const pxl::Particle* MCmet) const;
    bool Vertex_cuts(reco::VertexCollection::const_iterator vertex) const;
    bool PV_vertex_cuts(reco::VertexCollection::const_iterator vertex) const;
    bool Tau_cuts(const pat::Tau &tau) const;
    bool Muon_cuts(const pat::Muon& muon) const;
    bool Ele_cuts(std::vector<pat::Electron>::const_iterator ele) const;
    bool Gamma_cuts(std::vector<pat::Photon>::const_iterator photon) const;
    bool Jet_cuts(std::vector<pat::Jet>::const_iterator jet) const;
    bool MET_cuts(const pxl::Particle* met) const;

    bool passHEEPID(const pxl::Particle* ele,
					double const eleEt,
					double const abseta,
					double const eleRho,
					bool const eleBarrel,
					bool const eleEndcap
					) const;

    bool passHEEP_Isolation(const pxl::Particle* ele,
					double const eleEt,
					double const abseta,
					bool const eleBarrel,
					bool const eleEndcap,
					double const eleRho
					) const;
    vector<const reco::GenParticle*> runGenDecayTree(const reco::GenParticle* part ,  std::map< const reco::Candidate*, pxl::Particle* > genMatchMap);

    // Generic function to write ParticleFlow based isolation into (PXL) photons and
    // electrons. Could be extended to other particles as well.
    //
    // template< typename T >
    // void particleFlowBasedIsolation(IsoDepositVals const &isoValPFId,
    // PFIsolationEstimator *isolator,
    // edm::Handle< reco::VertexCollection > const &vertices,
    // //edm::Handle< reco::PFCandidateCollection > const &pfCandidates,
    // const edm::Handle< pat::PackedCandidateCollection > &pfCandidates,
    // edm::Ref< T > const &ref,
    // double const &rhoFastJet25,
    // pxl::Particle &part,
    // bool const useIsolator = true
    // ) const;

    void printEventContent(pxl::EventView const *GenEvtView,
                           pxl::EventView const *RecEvtView,
                           bool const &IsMC) const;

    // ----------member data ---------------------------

    int fNumEvt;  // used to count the number of events
	int Year_; 
    std::string FileName_;
    std::string Process_;
    std::string Dataset_;

    bool const fastSim_;
    bool GenOnly_;
    bool UseSIM_;

    bool allMuonInfos_;
    bool allElectronInfos_;
    bool allGammaInfos_;
    bool allTauInfos_;

    edm::InputTag  genParticleCandidatesLabel_;
    edm::EDGetTokenT< std::vector<reco::GenParticle> > genParticleCandidatesToken_;
    string VertexRecoLabel_;

    edm::FileInPath PdfSetFileName_;
    // Get the main particle lables
    edm::InputTag patMuonLabel_;
    edm::InputTag patElectronLabel_;
    edm::InputTag patGammaLabel_;
    edm::InputTag patTauTag_;
    edm::InputTag patTauBoostedTag_;
    edm::InputTag patMETTag_;
    edm::InputTag PUPPIMETTag_;
    edm::InputTag patMETEleCorrTag_;
    edm::InputTag patJetTag_;
    edm::InputTag patPFCandiates_;

    edm::EDGetTokenT<GenEventInfoProduct> genEvtInfoToken_;

    std::vector<int> scale_ids;
    std::vector<int> pdf_ids;
    std::vector<int> all_lhapdf_pdf_sets;
    std::string separator;
    ranges_type range_map_lhapdf;
    std::vector<std::string> all_lhapdf_pdf_sets_names;
    std::vector<int> pdf_sets;
    std::string all_muR;
    std::string all_muF;

    // additonal collections
    edm::InputTag reducedSuperClusterCollection_;
    edm::InputTag reducedEBClusterCollection_;


    edm::InputTag conversionsTag_;
    edm::InputTag conversionsSingleLegTag_;

    std::vector< edm::InputTag > rhos_;

    void printCutFlowResult(const vid::CutFlowResult &cutflow);

    // Tau MVA Token
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationToken_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationVLooseToken_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationLooseToken_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationMediumToken_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationTightToken_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationVTightToken_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationVVTightToken_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationTokenNewDM_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationVLooseTokenNewDM_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationLooseTokenNewDM_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationMediumTokenNewDM_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationTightTokenNewDM_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationVTightTokenNewDM_;
    edm::EDGetTokenT<pat::PATTauDiscriminator> mvaIsolationVVTightTokenNewDM_;

    // HCAL noise
    edm::InputTag hcal_noise_label_;
    edm::EDGetTokenT<edm::TriggerResults> METFilter_;
    edm::EDGetTokenT<edm::TriggerResults> METFilterAlternative_;
    edm::EDGetTokenT<bool> ecalBadCalibFilterUpdate_;

    edm::EDGetTokenT<std::vector< PileupSummaryInfo > > puInfoToken_;
    edm::EDGetTokenT<LHERunInfoProduct> lheProducerToken;

    MultiIsolation multiIsolation_;

    // Prefiring Weights
    edm::EDGetTokenT< double > prefweight_token;
    edm::EDGetTokenT< double > prefweightup_token;
    edm::EDGetTokenT< double > prefweightdown_token;

    // // Jets
    std::vector< jet_def > jet_infos;
    // //JetIDs
    typedef std::vector< std::pair< std::string, Selector<pat::Jet>* > > jet_id_list;


    // All filters.

    std::vector< trigger_group > filters;

    // Triggers
    edm::EDGetTokenT<edm::TriggerResults> triggerBits_;
    edm::EDGetTokenT<pat::PackedTriggerPrescales> triggerPrescales_;

    ParticleMatcher* Matcher;
    // to be used for ePax output
    pxl::OutputFile *fePaxFile;
    std::vector<gen::PdfInfo> fpdf_vec;
    double xfx(const double &x, const double &Q, int fl) {
        double f[13], mx = x, mQ = Q;
        evolvepdf_(mx, mQ, f);
        return f[fl+6];
    }

    PFIsolationEstimator* m_eleIsolator;
    PFIsolationEstimator* m_phoIsolator;

    ElectronEffectiveArea::ElectronEffectiveAreaTarget m_eleEffAreaTarget;
    ElectronEffectiveArea::ElectronEffectiveAreaType m_eleEffAreaType;

    // cuts
    double min_muon_pt,
        min_ele_pt,
        min_gamma_pt,
        min_jet_pt,
        min_met,
        min_tau_pt,
        max_eta,
        vertex_minNDOF,
        vertex_maxZ,
        vertex_maxR,
        PV_minNDOF,
        PV_maxZ,
        PV_maxR;

    // vertex for physics eta, phi, pt
    reco::BeamSpot::Point the_Pvertex;
    reco::BeamSpot::Point the_vertex;
    reco::BeamSpot::Point the_beamspot;
};

#endif  // SKIMMING_INTERFACE_PXLSKIMMER_MINIAOD_H_
