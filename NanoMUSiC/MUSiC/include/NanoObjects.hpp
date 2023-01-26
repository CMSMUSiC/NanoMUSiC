#ifndef MUSIC_NANOOBJECTS
#define MUSIC_NANOOBJECTS

#include "fmt/core.h"
#include <ROOT/RVec.hxx>
#include <variant>

constexpr float Muon_mass = 105.6583755 / 1000.;
constexpr float Electron_mass = 0.51099895000 / 1000.;
constexpr float Tau_mass = 1776.86 / 1000.;

using namespace ROOT;
using namespace ROOT::VecOps;

namespace NanoObjects
{

struct EventInfo
{
    UInt_t run;
    UInt_t lumi;
    ULong64_t event;
    float Pileup_nTrueInt;
    float genWeight;
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

    EventInfo(const UInt_t &_run = 0, const UInt_t &_lumi = 0, const ULong64_t &_event = 0,
              const float &_Pileup_nTrueInt = 0, const float &_genWeight = 1., const int &_PV_npvsGood = 0,
              const bool &_Flag_goodVertices = false, const bool &_Flag_globalSuperTightHalo2016Filter = false,
              const bool &_Flag_HBHENoiseFilter = false, const bool &_Flag_HBHENoiseIsoFilter = false,
              const bool &_Flag_EcalDeadCellTriggerPrimitiveFilter = false, const bool &_Flag_BadPFMuonFilter = false,
              const bool &_Flag_BadPFMuonDzFilter = false, const bool &_Flag_eeBadScFilter = false,
              const bool &_Flag_ecalBadCalibFilter = false, const bool &_HLT_IsoMu27 = false,
              const bool &_HLT_IsoMu24 = false, const bool &_HLT_IsoTkMu24 = false, const bool &_HLT_Mu50 = false,
              const bool &_HLT_TkMu50 = false, const bool &_HLT_TkMu100 = false, const bool &_HLT_OldMu100 = false,
              const bool &_HLT_Ele27_WPTight_Gsf = false, const bool &_HLT_Ele35_WPTight_Gsf = false,
              const bool &_HLT_Ele32_WPTight_Gsf = false, const bool &_HLT_Photon200 = false,
              const bool &_HLT_Photon175 = false, const bool &_HLT_Ele115_CaloIdVT_GsfTrkIdT = false)
        : run(_run),
          lumi(_lumi),
          event(_event),
          Pileup_nTrueInt(_Pileup_nTrueInt),
          genWeight(_genWeight),
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

struct Muons
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<bool> tightId;
    RVec<UChar_t> highPtId;
    RVec<float> pfRelIso03_all;
    RVec<float> tkRelIso;

    Muons(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {},
          const RVec<bool> &_tightId = {}, const RVec<UChar_t> &_highPtId = {}, const RVec<float> &_pfRelIso03_all = {},
          const RVec<float> &_tkRelIso = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          tightId(_tightId),
          highPtId(_highPtId),
          pfRelIso03_all(_pfRelIso03_all),
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

    Electrons(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {},
              const RVec<int> &_cutBased = {}, const RVec<bool> &_cutBased_HEEP = {},
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
    Photons(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {},
            const RVec<int> &_cutBased = {}, const RVec<bool> &_pixelSeed = {}, const RVec<bool> &_isScEtaEB = {},
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

    Jets(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {},
         const RVec<int> &_jetId = {}, const RVec<float> &_btagDeepFlavB = {})
        : size(_pt.size()),
          pt(_pt),
          eta(_eta),
          phi(_phi),
          jetId(_jetId),
          btagDeepFlavB(_btagDeepFlavB)
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

    TrgObjs(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {},
            const RVec<int> &_id = {}, const RVec<int> &_filterBits = {})
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
