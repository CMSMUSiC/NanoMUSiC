#ifndef MUSIC_NANOOBJECTS
#define MUSIC_NANOOBJECTS

#include "ROOT/RVec.hxx"

constexpr double Muon_mass = 105.6583755 / 1000.;
constexpr double Electron_mass = 0.51099895000 / 1000.;
constexpr double Tau_mass = 1776.86 / 1000.;

using namespace ROOT;
using namespace ROOT::VecOps;

namespace NanoObjects
{

struct EventInfo
{
    std::reference_wrapper<const UInt_t> run;
    std::reference_wrapper<const UInt_t> lumi;
    std::reference_wrapper<const ULong64_t> event;
    std::reference_wrapper<const float> Pileup_nTrueInt;
    std::reference_wrapper<const float> genWeight;
    std::reference_wrapper<const int> PV_npvsGood;
    std::reference_wrapper<const bool> Flag_goodVertices;
    std::reference_wrapper<const bool> Flag_globalSuperTightHalo2016Filter;
    std::reference_wrapper<const bool> Flag_HBHENoiseFilter;
    std::reference_wrapper<const bool> Flag_HBHENoiseIsoFilter;
    std::reference_wrapper<const bool> Flag_EcalDeadCellTriggerPrimitiveFilter;
    std::reference_wrapper<const bool> Flag_BadPFMuonFilter;
    std::reference_wrapper<const bool> Flag_BadPFMuonDzFilter;
    std::reference_wrapper<const bool> Flag_eeBadScFilter;
    std::reference_wrapper<const bool> Flag_ecalBadCalibFilter;
    std::reference_wrapper<const bool> HLT_IsoMu27;
    std::reference_wrapper<const bool> HLT_Mu50;
    std::reference_wrapper<const bool> HLT_TkMu100;
    std::reference_wrapper<const bool> HLT_OldMu100;

    EventInfo(const UInt_t &_run = 0, const UInt_t &_lumi = 0, const ULong64_t &_event = 0, const float &_Pileup_nTrueInt = 0,
              const float &_genWeight = 1., const int &_PV_npvsGood = 0, const bool &_Flag_goodVertices = false,
              const bool &_Flag_globalSuperTightHalo2016Filter = false, const bool &_Flag_HBHENoiseFilter = false,
              const bool &_Flag_HBHENoiseIsoFilter = false, const bool &_Flag_EcalDeadCellTriggerPrimitiveFilter = false,
              const bool &_Flag_BadPFMuonFilter = false, const bool &_Flag_BadPFMuonDzFilter = false,
              const bool &_Flag_eeBadScFilter = false, const bool &_Flag_ecalBadCalibFilter = false,
              const bool &_HLT_IsoMu27 = false, const bool &_HLT_Mu50 = false, const bool &_HLT_TkMu100 = false,
              const bool &_HLT_OldMu100 = false)
        : run(_run), lumi(_lumi), event(_event), Pileup_nTrueInt(_Pileup_nTrueInt), genWeight(_genWeight),
          PV_npvsGood(_PV_npvsGood), Flag_goodVertices(_Flag_goodVertices),
          Flag_globalSuperTightHalo2016Filter(_Flag_globalSuperTightHalo2016Filter), Flag_HBHENoiseFilter(_Flag_HBHENoiseFilter),
          Flag_HBHENoiseIsoFilter(_Flag_HBHENoiseIsoFilter),
          Flag_EcalDeadCellTriggerPrimitiveFilter(_Flag_EcalDeadCellTriggerPrimitiveFilter),
          Flag_BadPFMuonFilter(_Flag_BadPFMuonFilter), Flag_BadPFMuonDzFilter(_Flag_BadPFMuonDzFilter),
          Flag_eeBadScFilter(_Flag_eeBadScFilter), Flag_ecalBadCalibFilter(_Flag_ecalBadCalibFilter), HLT_IsoMu27(_HLT_IsoMu27),
          HLT_Mu50(_HLT_Mu50), HLT_TkMu100(_HLT_TkMu100), HLT_OldMu100(_HLT_OldMu100)
    {
    }
};

struct Muons
{
    std::size_t size;
    std::reference_wrapper<const RVec<float>> pt;
    std::reference_wrapper<const RVec<float>> eta;
    std::reference_wrapper<const RVec<float>> phi;
    std::reference_wrapper<const RVec<bool>> tightId;
    std::reference_wrapper<const RVec<UChar_t>> highPtId;
    std::reference_wrapper<const RVec<float>> pfRelIso03_all;
    std::reference_wrapper<const RVec<float>> tkRelIso;

    Muons(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {},
          const RVec<bool> &_tightId = {}, const RVec<UChar_t> &_highPtId = {}, const RVec<float> &_pfRelIso03_all = {},
          const RVec<float> &_tkRelIso = {})
        : size(_pt.size()), pt(_pt), eta(_eta), phi(_phi), tightId(_tightId), highPtId(_highPtId),
          pfRelIso03_all(_pfRelIso03_all), tkRelIso(_tkRelIso)
    {
    }

    // template <typename T, typename = std::enable_if<std::is_convertible<T, bool>::value>>
    // Muons filter(RVec<T> filter_mask)
    // {
    //     return Muons(pt[filter_mask], eta[filter_mask], phi[filter_mask], tightId[filter_mask], highPtId[filter_mask],
    //                  pfRelIso03_all[filter_mask], tkRelIso[filter_mask]);
    // }

    // // be carefull!!! This is slower than calling filter (the method above)...
    // template <typename T>
    // auto operator[](T &&filter_mask)
    // {
    //     return filter(std::forward<T>(filter_mask));
    // }
};

struct Electrons
{
    std::size_t size;
    std::reference_wrapper<const RVec<float>> pt;
    std::reference_wrapper<const RVec<float>> eta;
    std::reference_wrapper<const RVec<float>> phi;

    Electrons(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {})
        : size(_pt.size()), pt(_pt), eta(_eta), phi(_phi)
    {
    }

    // template <typename T, typename = std::enable_if<std::is_convertible<T, bool>::value>>
    // Electrons filter(RVec<T> filter_mask)
    // {
    //     return Electrons(pt[filter_mask], eta[filter_mask], phi[filter_mask]);
    // }

    // // be carefull!!! This is slow!
    // template <typename T>
    // auto operator[](T &&filter_mask)
    // {
    //     return filter(std::forward<T>(filter_mask));
    // }
};

struct Photons
{
    std::size_t size;
    std::reference_wrapper<const RVec<float>> pt;
    std::reference_wrapper<const RVec<float>> eta;
    std::reference_wrapper<const RVec<float>> phi;

    Photons(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {})
        : size(_pt.size()), pt(_pt), eta(_eta), phi(_phi)
    {
    }

    // template <typename T, typename = std::enable_if<std::is_convertible<T, bool>::value>>
    // Photons filter(RVec<T> filter_mask)
    // {
    //     return Photons(pt[filter_mask], eta[filter_mask], phi[filter_mask]);
    // }

    // // be carefull!!! This is slow!
    // template <typename T>
    // auto operator[](T &&filter_mask)
    // {
    //     return filter(std::forward<T>(filter_mask));
    // }
};

struct Taus
{
    std::size_t size;
    std::reference_wrapper<const RVec<float>> pt;
    std::reference_wrapper<const RVec<float>> eta;
    std::reference_wrapper<const RVec<float>> phi;

    Taus(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {})
        : size(_pt.size()), pt(_pt), eta(_eta), phi(_phi)
    {
    }

    // template <typename T, typename = std::enable_if<std::is_convertible<T, bool>::value>>
    // Taus filter(RVec<T> filter_mask)
    // {
    //     return Taus(pt[filter_mask], eta[filter_mask], phi[filter_mask]);
    // }

    // // be carefull!!! This is slow!
    // template <typename T>
    // auto operator[](T &&filter_mask)
    // {
    //     return filter(std::forward<T>(filter_mask));
    // }
};

struct Jets
{
    std::size_t size;
    std::reference_wrapper<const RVec<float>> pt;
    std::reference_wrapper<const RVec<float>> eta;
    std::reference_wrapper<const RVec<float>> phi;

    Jets(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {})
        : size(_pt.size()), pt(_pt), eta(_eta), phi(_phi)
    {
    }

    // template <typename T, typename = std::enable_if<std::is_convertible<T, bool>::value>>
    // Jets filter(RVec<T> filter_mask)
    // {
    //     return Jets(pt[filter_mask], eta[filter_mask], phi[filter_mask]);
    // }

    // // be carefull!!! This is slow!
    // template <typename T>
    // auto operator[](T &&filter_mask)
    // {
    //     return filter(std::forward<T>(filter_mask));
    // }
};

using BJets = Jets;

struct MET
{
    std::size_t size;
    std::reference_wrapper<const RVec<float>> pt;
    std::reference_wrapper<const RVec<float>> eta;
    std::reference_wrapper<const RVec<float>> phi;

    MET(const RVec<float> &_pt = {}, const RVec<float> &_eta = {}, const RVec<float> &_phi = {})
        : size(_pt.size()), pt(_pt), eta(_eta), phi(_phi)
    {
    }

    // template <typename T, typename = std::enable_if<std::is_convertible<T, bool>::value>>
    // MET filter(RVec<T> filter_mask)
    // {
    //     return MET(pt[filter_mask], phi[filter_mask]);
    // }

    // // be carefull!!! This is slow!
    // template <typename T>
    // auto operator[](T &&filter_mask)
    // {
    //     return filter(std::forward<T>(filter_mask));
    // }
};

using NanoAODObjects_t = std::tuple<Muons, Electrons, Photons, Taus, BJets, Jets, MET>;

} // namespace NanoObjects

#endif /*MUSIC_NANOOBJECTS*/
