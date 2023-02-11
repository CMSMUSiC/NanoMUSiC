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

inline auto mass(const float &pt1,
                 const float &eta1,
                 const float &phi1,
                 const float &mass1,
                 const float &pt2,
                 const float &eta2,
                 const float &phi2,
                 const float &mass2) -> float
{
    // Conversion from (pt, eta, phi, mass) to (x, y, z, e) coordinate system
    const auto x1 = pt1 * std::cos(phi1);
    const auto y1 = pt1 * std::sin(phi1);
    const auto z1 = pt1 * std::sinh(eta1);
    const auto e1 = std::sqrt(x1 * x1 + y1 * y1 + z1 * z1 + mass1 * mass1);

    const auto x2 = pt2 * std::cos(phi2);
    const auto y2 = pt2 * std::sin(phi2);
    const auto z2 = pt2 * std::sinh(eta2);
    const auto e2 = std::sqrt(x2 * x2 + y2 * y2 + z2 * z2 + mass2 * mass2);

    // Addition of particle four-vector elements
    const auto e = e1 + e2;
    const auto x = x1 + x2;
    const auto y = y1 + y2;
    const auto z = z1 + z2;

    return std::sqrt(e * e - x * x - y * y - z * z);
}

inline auto pt(const float &pt1, const float &phi1, const float &pt2, const float &phi2) -> float
{
    const auto x1 = pt1 * std::cos(phi1);
    const auto y1 = pt1 * std::sin(phi1);

    const auto x2 = pt2 * std::cos(phi2);
    const auto y2 = pt2 * std::sin(phi2);

    // Addition of particle 2d-vector components
    const auto x = x1 + x2;
    const auto y = y1 + y2;

    return std::sqrt(x * x + y * y);
}

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
              const bool &_HLT_Ele115_CaloIdVT_GsfTrkIdT = false)
        : run(_run),
          lumi(_lumi),
          event(_event),
          Pileup_nTrueInt(_Pileup_nTrueInt),
          genWeight(_genWeight),
          L1PreFiringWeight_Nom(_L1PreFiringWeight_Nom),
          L1PreFiringWeight_Up(_L1PreFiringWeight_Up),
          L1PreFiringWeight_Dn(_L1PreFiringWeight_Dn),
          PV_npvsGood(_PV_npvsGood),
          Flag_goodVertices(_Flag_goodVertices),
          Flag_globalSuperTightHalo2016Filter(_Flag_globalSuperTightHalo2016Filter),
          Flag_HBHENoiseFilter(_Flag_HBHENoiseFilter),
          Flag_HBHENoiseIsoFilter(_Flag_HBHENoiseIsoFilter),
          Flag_EcalDeadCellTriggerPrimitiveFilter(_Flag_EcalDeadCellTriggerPrimitiveFilter),
          Flag_BadPFMuonFilter(_Flag_BadPFMuonFilter),
          Flag_BadPFMuonDzFilter(_Flag_BadPFMuonDzFilter),
          Flag_eeBadScFilter(_Flag_eeBadScFilter),
          Flag_ecalBadCalibFilter(_Flag_ecalBadCalibFilter),
          HLT_IsoMu27(_HLT_IsoMu27),
          HLT_IsoMu24(_HLT_IsoMu24),
          HLT_IsoTkMu24(_HLT_IsoTkMu24),
          HLT_Mu50(_HLT_Mu50),
          HLT_TkMu50(_HLT_TkMu50),
          HLT_TkMu100(_HLT_TkMu100),
          HLT_OldMu100(_HLT_OldMu100),
          HLT_Ele27_WPTight_Gsf(_HLT_Ele27_WPTight_Gsf),
          HLT_Ele35_WPTight_Gsf(_HLT_Ele35_WPTight_Gsf),
          HLT_Ele32_WPTight_Gsf(_HLT_Ele32_WPTight_Gsf),
          HLT_Photon200(_HLT_Photon200),
          HLT_Photon175(_HLT_Photon175),
          HLT_Ele115_CaloIdVT_GsfTrkIdT(_HLT_Ele115_CaloIdVT_GsfTrkIdT)
    {
    }
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
                  const int &_id2 = 0)
        : binvar(_binvar),
          scalePDF(_scalePDF),
          weight(_weight),
          x1(_x1),
          x2(_x2),
          xpdf1(_xpdf1),
          xpdf2(_xpdf2),
          id1(_id1),
          id2(_id2)
    {
    }
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
            const float &_HTIncoming = 0.)
        : nLHEPdfWeight(_LHEPdfWeight.size()),
          LHEPdfWeight(_LHEPdfWeight),
          nLHEScaleWeight(_LHEScaleWeight.size()),
          LHEScaleWeight(_LHEScaleWeight),
          originalXWGTUP(_originalXWGTUP),
          HT(_HT),
          HTIncoming(_HTIncoming)
    {
    }
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
                 const RVec<int> &_statusFlags = {})
        : nGenParticles(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          mass(_mass),
          genPartIdxMother(_genPartIdxMother),
          pdgId(_pdgId),
          status(_status),
          statusFlags(_statusFlags)
    {
    }
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
                 const RVec<int> _status = {})
        : nLHEParticles(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          mass(_mass),
          incomingpz(_incomingpz),
          pdgId(_pdgId),
          status(_status)
    {
    }
};

struct Muons
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<bool> tightId;
    RVec<UChar_t> highPtId;
    RVec<float> pfRelIso04_all;
    RVec<float> tkRelIso;

    Muons(const RVec<float> &_pt = {},
          const RVec<float> &_eta = {},
          const RVec<float> &_phi = {},
          const RVec<bool> &_tightId = {},
          const RVec<UChar_t> &_highPtId = {},
          const RVec<float> &_pfRelIso04_all = {},
          const RVec<float> &_tkRelIso = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          tightId(_tightId),
          highPtId(_highPtId),
          pfRelIso04_all(_pfRelIso04_all),
          tkRelIso(_tkRelIso)
    {
    }
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
              const RVec<float> &_deltaEtaSC = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          cutBased(_cutBased),
          cutBased_HEEP(_cutBased_HEEP),
          deltaEtaSC(_deltaEtaSC)
    {
    }
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
            const RVec<bool> &_isScEtaEE = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          cutBased(_cutBased),
          pixelSeed(_pixelSeed),
          isScEtaEB(_isScEtaEB),
          isScEtaEE(_isScEtaEE)
    {
    }
};

struct Taus
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;

    Taus(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi)
    {
    }
};

struct Jets
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<int> jetId;
    RVec<float> btagDeepFlavB;
    RVec<int> hadronFlavour;

    Jets(const RVec<float> &_pt = {},
         const RVec<float> &_eta = {},
         const RVec<float> &_phi = {},
         const RVec<int> &_jetId = {},
         const RVec<float> &_btagDeepFlavB = {},
         const RVec<int> &_hadronFlavour = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          jetId(_jetId),
          btagDeepFlavB(_btagDeepFlavB),
          hadronFlavour((_pt.size() > 0 and _hadronFlavour.size() == 0) ? RVec<int>(_pt.size()) : _hadronFlavour)
    {
    }
};

using BJets = Jets;

struct MET
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;

    MET(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi)
    {
    }
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
            const RVec<int> &_filterBits = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          id(_id),
          filterBits(_filterBits)
    {
    }
};
using NanoAODObjects_t = std::tuple<Muons, Electrons, Photons, Taus, BJets, Jets, MET>;

} // namespace NanoObjects

#endif /*MUSIC_NANOOBJECTS*/
