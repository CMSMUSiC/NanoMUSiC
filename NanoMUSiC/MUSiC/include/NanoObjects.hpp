#ifndef MUSIC_NANOOBJECTS
#define MUSIC_NANOOBJECTS

#include "fmt/format.h"
#include <ROOT/RVec.hxx>
#include <variant>

constexpr float Muon_mass = 105.6583755 / 1000.;
constexpr float Electron_mass = 0.51099895000 / 1000.;
constexpr float Tau_mass = 1776.86 / 1000.;

using namespace ROOT;
using namespace ROOT::VecOps;

namespace LorentzVectorHelper
{

auto mass(const float &pt1,
          const float &eta1,
          const float &phi1,
          const float &mass1,
          const float &pt2,
          const float &eta2,
          const float &phi2,
          const float &mass2) -> float;

auto pt(const float &pt1, const float &phi1, const float &pt2, const float &phi2) -> float;

} // namespace LorentzVectorHelper

namespace NanoObjects
{
struct EventInfo
{
    UInt_t run;
    UInt_t lumi;
    ULong64_t event;
    float Pileup_nTrueInt;
    float genWeight;
    float L1PreFiringWeight_Nom;
    float L1PreFiringWeight_Up;
    float L1PreFiringWeight_Dn;
    int PV_npvsGood;
    bool Flag_goodVertices;
    bool Flag_globalSuperTightHalo2016Filter;
    bool Flag_HBHENoiseFilter;
    bool Flag_HBHENoiseIsoFilter;
    bool Flag_EcalDeadCellTriggerPrimitiveFilter;
    bool Flag_BadPFMuonFilter;
    bool Flag_BadPFMuonDzFilter;
    bool Flag_eeBadScFilter;
    bool Flag_ecalBadCalibFilter;
    float rho;
    bool HLT_IsoMu27;
    bool HLT_IsoMu24;
    bool HLT_IsoTkMu24;
    bool HLT_Mu50;
    bool HLT_TkMu50;
    bool HLT_TkMu100;
    bool HLT_OldMu100;
    bool HLT_Ele27_WPTight_Gsf;
    bool HLT_Ele35_WPTight_Gsf;
    bool HLT_Ele32_WPTight_Gsf;
    bool HLT_Photon200;
    bool HLT_Photon175;
    bool HLT_Ele115_CaloIdVT_GsfTrkIdT;

    EventInfo(const UInt_t &_run = 0,
              const UInt_t &_lumi = 0,
              const ULong64_t &_event = 0,
              const float &_Pileup_nTrueInt = 0,
              const float &_genWeight = 1.,
              const float &_L1PreFiringWeight_Nom = 1.,
              const float &_L1PreFiringWeight_Up = 1.,
              const float &_L1PreFiringWeight_Dn = 1.,
              const int &_PV_npvsGood = 0,
              const bool &_Flag_goodVertices = false,
              const bool &_Flag_globalSuperTightHalo2016Filter = false,
              const bool &_Flag_HBHENoiseFilter = false,
              const bool &_Flag_HBHENoiseIsoFilter = false,
              const bool &_Flag_EcalDeadCellTriggerPrimitiveFilter = false,
              const bool &_Flag_BadPFMuonFilter = false,
              const bool &_Flag_BadPFMuonDzFilter = false,
              const bool &_Flag_eeBadScFilter = false,
              const bool &_Flag_ecalBadCalibFilter = false,
              const float &_rho = 0.,
              const bool &_HLT_IsoMu27 = false,
              const bool &_HLT_IsoMu24 = false,
              const bool &_HLT_IsoTkMu24 = false,
              const bool &_HLT_Mu50 = false,
              const bool &_HLT_TkMu50 = false,
              const bool &_HLT_TkMu100 = false,
              const bool &_HLT_OldMu100 = false,
              const bool &_HLT_Ele27_WPTight_Gsf = false,
              const bool &_HLT_Ele35_WPTight_Gsf = false,
              const bool &_HLT_Ele32_WPTight_Gsf = false,
              const bool &_HLT_Photon200 = false,
              const bool &_HLT_Photon175 = false,
              const bool &_HLT_Ele115_CaloIdVT_GsfTrkIdT = false);
};

struct GeneratorInfo
{
    float binvar;
    float scalePDF;
    float weight;
    float x1;
    float x2;
    float xpdf1;
    float xpdf2;
    int id1;
    int id2;

    GeneratorInfo(const float &_binvar = 0.,
                  const float &_scalePDF = 0.,
                  const float &_weight = 1.,
                  const float &_x1 = 0.,
                  const float &_x2 = 0.,
                  const float &_xpdf1 = 0.,
                  const float &_xpdf2 = 0.,
                  const int &_id1 = 0,
                  const int &_id2 = 0);
};

struct LHEInfo
{
    std::size_t nLHEPdfWeight;
    RVec<float> LHEPdfWeight;
    std::size_t nLHEScaleWeight;
    RVec<float> LHEScaleWeight;
    float originalXWGTUP;
    float HT;
    float HTIncoming;

    LHEInfo(const RVec<float> &_LHEPdfWeight = {},
            const RVec<float> &_LHEScaleWeight = {},
            const float &_originalXWGTUP = 1.,
            const float &_HT = 0.,
            const float &_HTIncoming = 0.);
};

struct GenParticles
{
    std::size_t nGenParticles;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<float> mass;
    RVec<int> genPartIdxMother;
    RVec<int> pdgId;
    RVec<int> status;
    RVec<int> statusFlags;

    GenParticles(const RVec<float> &_pt = {},
                 const RVec<float> &_eta = {},
                 const RVec<float> &_phi = {},
                 const RVec<float> &_mass = {},
                 const RVec<int> &_genPartIdxMother = {},
                 const RVec<int> &_pdgId = {},
                 const RVec<int> &_status = {},
                 const RVec<int> &_statusFlags = {});
};

struct LHEParticles
{
    std::size_t nLHEParticles;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<float> mass;
    RVec<float> incomingpz;
    RVec<int> pdgId;
    RVec<int> status;

    LHEParticles(const RVec<float> _pt = {},
                 const RVec<float> _eta = {},
                 const RVec<float> _phi = {},
                 const RVec<float> _mass = {},
                 const RVec<float> _incomingpz = {},
                 const RVec<int> _pdgId = {},
                 const RVec<int> _status = {});
};

struct GenJets
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;

    GenJets(const RVec<float> _pt = {}, const RVec<float> _eta = {}, const RVec<float> _phi = {});
};

struct Muons
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<bool> tightId;
    RVec<unsigned char> highPtId;
    RVec<float> pfRelIso04_all;
    RVec<float> tkRelIso;
    RVec<bool> highPurity;

    Muons(const RVec<float> &_pt = {},
          const RVec<float> &_eta = {},
          const RVec<float> &_phi = {},
          const RVec<bool> &_tightId = {},
          const RVec<unsigned char> &_highPtId = {},
          const RVec<float> &_pfRelIso04_all = {},
          const RVec<float> &_tkRelIso = {},
          const RVec<bool> &_highPurity = {});
};

struct Electrons
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<int> cutBased;
    RVec<bool> cutBased_HEEP;
    RVec<float> deltaEtaSC;

    Electrons(const RVec<float> &_pt = {},
              const RVec<float> &_eta = {},
              const RVec<float> &_phi = {},
              const RVec<int> &_cutBased = {},
              const RVec<bool> &_cutBased_HEEP = {},
              const RVec<float> &_deltaEtaSC = {});
};

struct Photons
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<int> cutBased;
    RVec<bool> pixelSeed;
    RVec<bool> isScEtaEB;
    RVec<bool> isScEtaEE;

    Photons(const RVec<float> &_pt = {},
            const RVec<float> &_eta = {},
            const RVec<float> &_phi = {},
            const RVec<int> &_cutBased = {},
            const RVec<bool> &_pixelSeed = {},
            const RVec<bool> &_isScEtaEB = {},
            const RVec<bool> &_isScEtaEE = {});
};

struct Taus
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;

    Taus(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {});
};

struct Jets
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<float> mass;
    RVec<int> jetId;
    RVec<float> btagDeepFlavB;
    RVec<int> hadronFlavour;
    RVec<int> genJetIdx;
    RVec<float> rawFactor;
    RVec<float> area;

    // Corrections variations
    RVec<float> pt_JES_up;
    RVec<float> mass_JES_up;
    RVec<float> pt_JES_down;
    RVec<float> mass_JES_down;
    RVec<float> pt_JER_up;
    RVec<float> mass_JER_up;
    RVec<float> pt_JER_down;
    RVec<float> mass_JER_down;

    Jets(const RVec<float> &_pt = {},
         const RVec<float> &_eta = {},
         const RVec<float> &_phi = {},
         const RVec<float> &_mass = {},
         const RVec<int> &_jetId = {},
         const RVec<float> &_btagDeepFlavB = {},
         const RVec<int> &_hadronFlavour = {},
         const RVec<int> &_genJetIdx = {},
         const RVec<float> &_rawFactor = {},
         const RVec<float> &_area = {});
};

using BJets = Jets;

struct MET
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;

    // Corrections variations
    RVec<float> JES_up;
    RVec<float> JES_down;
    RVec<float> JER_up;
    RVec<float> JER_down;

    MET(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {});
};

struct TrgObjs
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    // ID of the object: 11 = Electron (PixelMatched e/gamma), 22 = Photon (PixelMatch-vetoed e/gamma), 13 =
    // Muon, 15 = Tau, 1 = Jet, 6 = FatJet, 2 = MET, 3 = HT, 4 = MHT
    RVec<int> id;
    RVec<int> filterBits;

    TrgObjs(const RVec<float> &_pt = {},
            const RVec<float> &_eta = {},
            const RVec<float> &_phi = {},
            const RVec<int> &_id = {},
            const RVec<int> &_filterBits = {});
};
using NanoAODObjects_t = std::tuple<Muons, Electrons, Photons, Taus, BJets, Jets, MET>;

} // namespace NanoObjects

#endif /*MUSIC_NANOOBJECTS*/
