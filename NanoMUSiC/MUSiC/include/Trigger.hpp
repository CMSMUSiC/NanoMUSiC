#ifndef MUSIC_TRIGGER
#define MUSIC_TRIGGER

#include <string>
#include <string_view>

#include "CorrectionSets.hpp"
#include "Enumerate.hpp"
#include "NanoObjects.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>

#include "ROOT/RVec.hxx"
using namespace ROOT;
using namespace ROOT::VecOps;

using namespace std::literals;

namespace Trigger
{
constexpr auto HLTPath = make_enumerate("SingleMuonLowPt"sv,      //
                                        "SingleMuonHighPt"sv,     //
                                        "SingleElectronLowPt"sv,  //
                                        "SingleElectronHighPt"sv, //
                                        "DoubleMuon"sv,           //
                                        "DoubleElectron"sv,       //
                                        "Photon"sv,               //
                                        "Tau"sv,                  //
                                        "BJet"sv,                 //
                                        "Jet"sv,                  //
                                        "MET"sv);
constexpr auto kTotalPaths = HLTPath.size();

constexpr auto ActivatedHLTPath = make_enumerate("SingleMuonLowPt"sv,     //
                                                 "SingleMuonHighPt"sv,    //
                                                 "SingleElectronLowPt"sv, //
                                                 "SingleElectronHighPt"sv);

// constexpr auto kTotalActivatedPaths = ActivatedHLTPath && (nano_objects.pt <= pt_min).size();

inline std::string get_year_for_muon_sf(Year year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return "2016preVFP_UL"s;
    case Year::Run2016:
        return "2016postVFP_UL"s;
    case Year::Run2017:
        return "2017_UL"s;
    case Year::Run2018:
        return "2018_UL"s;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

template <typename T1, typename T2>
std::pair<RVec<float>, RVec<float>> get_matches(const T1 &trigger_objects_pt,  //
                                                const T1 &trigger_objects_eta, //
                                                const T1 &trigger_objects_phi, //
                                                const T2 &nano_objects_pt,     //
                                                const T2 &nano_objects_eta,    //
                                                const T2 &nano_objects_phi)
{
    auto matches_distances = RVec<float>(nano_objects_pt.size(), std::numeric_limits<float>::max());
    auto matches_relative_pT = RVec<float>(nano_objects_pt.size(), std::numeric_limits<float>::max());

    for (std::size_t idx = 0; idx < nano_objects_pt.size(); idx++)
    {
        // DeltaR
        matches_distances.at(idx) =
            VecOps::Min(VecOps::sqrt(VecOps::pow((trigger_objects_eta - nano_objects_eta[idx]), 2.) +
                                     +VecOps::pow(VecOps::DeltaPhi(trigger_objects_phi, nano_objects_phi[idx]), 2.)));

        // Relative pT diff
        matches_relative_pT.at(idx) =
            VecOps::Min(VecOps::abs(nano_objects_pt[idx] - trigger_objects_phi) / (nano_objects_pt[idx]));
    }

    return std::make_pair(matches_distances, matches_relative_pT);
}

template <typename T1, typename T2>
constexpr std::tuple<bool, float, float> trigger_matcher(const T1 &trigger_objects_pt,  //
                                                         const T1 &trigger_objects_eta, //
                                                         const T1 &trigger_objects_phi, //
                                                         const T2 &nano_objects_pt,     //
                                                         const T2 &nano_objects_eta,    //
                                                         const T2 &nano_objects_phi,    //
                                                         const float max_delta_r)
{
    bool has_trigger_match = false;
    float matched_nanoobject_pt = 0.;
    float matched_nanoobject_eta = 0.;

    if (nano_objects_pt.size() > 0 and trigger_objects_pt.size() > 0)
    {
        auto [matches_distances, matches_rel_pT] = get_matches(trigger_objects_pt,  //
                                                               trigger_objects_eta, //
                                                               trigger_objects_phi, //
                                                               nano_objects_pt,     //
                                                               nano_objects_eta,    //
                                                               nano_objects_phi);

        auto good_delta_r = matches_distances <= max_delta_r;
        if (VecOps::Any(good_delta_r))
        {
            has_trigger_match = true;

            // if there is any match, them we take the first object.
            // it should be the largest pT
            matched_nanoobject_pt = nano_objects_pt[0];
            matched_nanoobject_eta = nano_objects_eta[0];
        }
    }

    //////////////////////////////////////////////////////
    // DEBUG
    // fmt::print("###### DEBUG ############\n");
    // fmt::print("Has a match? : {}\n", has_trigger_match);
    // fmt::print("Distances: : {}\n", matches_distances);
    // fmt::print("Rel_pT: : {}\n", matches_rel_pT);
    ///////////////////////////////////////

    return std::make_tuple(has_trigger_match, matched_nanoobject_pt, matched_nanoobject_eta);
}

inline RVec<int> check_bit(const RVec<int> &trigger_bits, const int &bit)
{
    // fmt::print("trigger bits: {} - bit: {}\n", trigger_bits, bit);
    return (trigger_bits & bit) / bit;
}

///////////////////////////////////////////////////////////
// Ref: https://hlt-config-editor-dev-confdbv3.app.cern.ch/
/// Check the `trigger_bit` of a given TrigObj for Single Muon - Low pT
// Run2017 configurations
// hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p07 -> HLT_IsoMu27 - bit: 8
// hltL3crIsoL1sSingleMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p07 -> HLT_IsoMu24 - bit: 8
///
inline RVec<int> SingleMuonLowPtBits(const RVec<int> &triggerobj_bit, const Year &year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8));
    case Year::Run2016:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8));
    case Year::Run2017:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8));
    case Year::Run2018:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8));
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
};

///////////////////////////////////////////////////////////
/// Check the `trigger_bit` of a given TrigObj for Single Muon - High pT
// Ref: https://hlt-config-editor-dev-confdbv3.app.cern.ch/
// Run2017 configurations
// hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q -> HLT_Mu50 - bit: 1024
// hltL3fL1sMu22Or25L1f0L2f10QL3Filtered100Q -> HLT_OldMu100 - bit: 2048
// hltL3fL1sMu25f0TkFiltered100Q -> HLT_OldMu100 - bit: 2048
///
inline RVec<int> SingleMuonHighPtBits(const RVec<int> &triggerobj_bit, const Year &year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return (check_bit(triggerobj_bit, 1024) || check_bit(triggerobj_bit, 2048));
    case Year::Run2016:
        return (check_bit(triggerobj_bit, 1024) || check_bit(triggerobj_bit, 2048));
    case Year::Run2017:
        return (check_bit(triggerobj_bit, 1024) || check_bit(triggerobj_bit, 2048));
    case Year::Run2018:
        return (check_bit(triggerobj_bit, 1024) || check_bit(triggerobj_bit, 2048));
    default:
        throw std::runtime_error(
            fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).\n", year));
    }
};

//////////////////////////////////////////////////////////////////////
/// Seems to be the same for all three years
// Ref: https://hlt-config-editor-dev-confdbv3.app.cern.ch/
/// 1 = CaloIdL_TrackIdL_IsoVL
/// 2 = 1e (WPTight)
/// 4 = 1e (WPLoose)
/// 8 = OverlapFilter PFTau
/// 16 = 2e
/// 32 = 1e-1mu
/// 64 = 1e-1tau
/// 128 = 3e
/// 256 = 2e-1mu
/// 512 = 1e-2mu
/// 1024 = 1e (32_L1DoubleEG_AND_L1SingleEGOr)
/// 2048 = 1e (CaloIdVT_GsfTrkIdT)
/// 4096 = 1e (PFJet)
/// 8192 = 1e (Photon175_OR_Photon200) for Electron (PixelMatched e/gamma)
inline RVec<int> SingleElectronLowPtBits(const RVec<int> &triggerobj_bit, const Year &year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192));
    case Year::Run2016:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192));
    case Year::Run2017:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192));
    case Year::Run2018:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192));
    default:
        throw std::runtime_error(
            fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).\n", year));
    }
};

//////////////////////////////////////////////////////////////////////
/// Seems to be the same for all three years
// Ref: https://hlt-config-editor-dev-confdbv3.app.cern.ch/
/// 1 = CaloIdL_TrackIdL_IsoVL
/// 2 = 1e (WPTight)
/// 4 = 1e (WPLoose)
/// 8 = OverlapFilter PFTau
/// 16 = 2e
/// 32 = 1e-1mu
/// 64 = 1e-1tau
/// 128 = 3e
/// 256 = 2e-1mu
/// 512 = 1e-2mu
/// 1024 = 1e (32_L1DoubleEG_AND_L1SingleEGOr)
/// 2048 = 1e (CaloIdVT_GsfTrkIdT)
/// 4096 = 1e (PFJet)
/// 8192 = 1e (Photon175_OR_Photon200) for Electron (PixelMatched e/gamma)
inline RVec<int> SingleElectronHighPtBits(const RVec<int> &triggerobj_bit, const Year &year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192) || check_bit(triggerobj_bit, 2048));
    case Year::Run2016:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192) || check_bit(triggerobj_bit, 2048));
    case Year::Run2017:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192) || check_bit(triggerobj_bit, 2048));
    case Year::Run2018:
        return (check_bit(triggerobj_bit, 2) || check_bit(triggerobj_bit, 8192) || check_bit(triggerobj_bit, 2048));
    default:
        throw std::runtime_error(
            fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).\n", year));
    }
};

//////////////////////////////////////////////////////////////////////
/// Seems to be the same for all three years.
/// NOTE: So far, we do not trigger solo on photons.
// Ref: https://hlt-config-editor-dev-confdbv3.app.cern.ch/
/// 1 = CaloIdL_TrackIdL_IsoVL
/// 2 = 1e (WPTight)
/// 4 = 1e (WPLoose)
/// 8 = OverlapFilter PFTau
/// 16 = 2e
/// 32 = 1e-1mu
/// 64 = 1e-1tau
/// 128 = 3e
/// 256 = 2e-1mu
/// 512 = 1e-2mu
/// 1024 = 1e (32_L1DoubleEG_AND_L1SingleEGOr)
/// 2048 = 1e (CaloIdVT_GsfTrkIdT)
/// 4096 = 1e (PFJet)
/// 8192 = 1e (Photon175_OR_Photon200) for Electron (PixelMatched e/gamma)
inline RVec<int> PhotonBits(const RVec<int> &triggerobj_bit, const Year &year)
{
    switch (year)
    {
    case Year::Run2016APV:
        return check_bit(triggerobj_bit, 8192);
    case Year::Run2016:
        return check_bit(triggerobj_bit, 8192);
    case Year::Run2017:
        return check_bit(triggerobj_bit, 8192);
    case Year::Run2018:
        return check_bit(triggerobj_bit, 8192);
    default:
        throw std::runtime_error(
            fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).\n", year));
    }
};

////////////////////////////////////////////////////////////////////////
/// Check the `trigger_bit` of a given TrigObj, `path` and `year`.
///
inline RVec<int> check_trigger_bit(const RVec<int> &triggerobj_bit, const std::string_view &path, const Year &year)
{
    if (path == std::string_view("SingleMuonLowPt"))
    {
        return SingleMuonLowPtBits(triggerobj_bit, year);
    }
    else if (path == std::string_view("SingleMuonHighPt"))
    {
        return SingleMuonHighPtBits(triggerobj_bit, year);
    }
    else if (path == std::string_view("SingleElectronLowPt"))
    {
        return SingleElectronLowPtBits(triggerobj_bit, year);
    }
    else if (path == std::string_view("SingleElectronHighPt"))
    {
        return SingleElectronHighPtBits(triggerobj_bit, year);
    }
    else if (path == std::string_view("Photon"))
    {
        return PhotonBits(triggerobj_bit, year);
    }
    else
    {
        throw std::runtime_error(
            fmt::format("\'check_trigger_bit\' is not implemented for the requested trigger path ({}).\n", path));
    }
}
} // namespace Trigger

class TrgObjMatcher
{
  public:
    const std::string_view hlt_path;
    const int id;
    const double max_deltar_r;
    const double pt_min;
    const Year year;
    const bool is_data;
    const Corrector trigger_sf_correctors;

    TrgObjMatcher(const std::string_view &_hlt_path, double _max_deltar_r, double _pt_min, int _id, Year _year,
                  bool _is_data)
        : hlt_path(_hlt_path),
          id(_id),
          max_deltar_r(_max_deltar_r),
          pt_min(_pt_min),
          year(_year),
          is_data(_is_data),
          trigger_sf_correctors(Corrector(hlt_path, year, is_data))
    {
        // sanity checks ...
        // the key above should match the defined Activated HLT paths ...
        if (std::find(Trigger::ActivatedHLTPath.cbegin(), Trigger::ActivatedHLTPath.cend(), hlt_path) ==
            Trigger::ActivatedHLTPath.cend())
        {
            throw std::runtime_error(
                fmt::format("The Trigger SF name ({}) is not present in the array of defined HLT paths ({}).\n",
                            hlt_path, Trigger::ActivatedHLTPath));
        }
    }

    template <typename T>
    auto operator()(const NanoObjects::TrgObjs &trgobjs, const T &nano_objects, RVec<int> nano_objs_mask) const
        -> std::tuple<bool, double, double, double>
    {
        ////////////////////////////////////////
        // DEBUG
        // fmt::print("----------------------------- DEBUG ----------------------\n");
        // fmt::print("HLT Path: {}\n", hlt_path);
        /////////////////////////////////////////////////////
        double trigger_sf_nominal = 1.;
        double trigger_sf_up = 1.;
        double trigger_sf_down = 1.;

        RVec<int> trgobj_mask = (                                            //
            (trgobjs.id == id) &&                                            //
            (Trigger::check_trigger_bit(trgobjs.filterBits, hlt_path, year)) //
            && (trgobjs.pt >= pt_min)                                        //
        );

        auto nano_objects_pt_mask = nano_objects.pt >= pt_min;

        // if (hlt_path == "SingleElectronLowPt")
        // {
        //     fmt::print("----------------------------- DEBUG ----------------------\n");
        //     fmt::print("HLT Path: {}\n", hlt_path);
        //     fmt::print("check_trigger_bit: {}\n", Trigger::check_trigger_bit(trgobjs.filterBits, hlt_path, year));
        //     fmt::print("trgobj_mask: {}\n", trgobj_mask);

        //     fmt::print("Id: {}\n", trgobjs.id);
        //     fmt::print("Id mask: {}\n", (trgobjs.id == id));
        //     fmt::print("Bit : {}\n", trgobjs.filterBits);
        //     fmt::print("Bit mask: {}\n", Trigger::check_trigger_bit(trgobjs.filterBits, hlt_path, year));
        //     fmt::print("---------\n");
        //     fmt::print("Trigger Pt: {}\n", trgobjs.eta[trgobj_mask]);
        //     fmt::print("Obj Eta: {}\n", nano_objects.pt[nano_objs_mask && nano_objects_pt_mask]);
        //     fmt::print("Trigger Pt: {}\n", trgobjs.pt[trgobj_mask]);
        //     fmt::print("Obj Eta: {}\n", nano_objects.eta[nano_objs_mask && nano_objects_pt_mask]);
        //     fmt::print("Trigger Phi: {}\n", trgobjs.phi[trgobj_mask]);
        //     fmt::print("Obj Phi: {}\n", nano_objects.phi[nano_objs_mask && nano_objects_pt_mask]);
        //     fmt::print("---------\n");
        //     fmt::print("MASK: {}\n", nano_objs_mask && nano_objects_pt_mask);
        //     fmt::print("RAW Obj pt: {}\n", nano_objects.pt);
        //     fmt::print("RAW Obj eta: {}\n", nano_objects.eta);
        //     fmt::print("RAW Obj Phi: {}\n", nano_objects.phi);
        // }

        const auto [has_trigger_match, _matched_nano_object_pt, _matched_nano_object_eta] =
            Trigger::trigger_matcher(trgobjs.pt[trgobj_mask],                                  //
                                     trgobjs.eta[trgobj_mask],                                 //
                                     trgobjs.phi[trgobj_mask],                                 //
                                     nano_objects.pt[nano_objs_mask && nano_objects_pt_mask],  //
                                     nano_objects.eta[nano_objs_mask && nano_objects_pt_mask], //
                                     nano_objects.phi[nano_objs_mask && nano_objects_pt_mask], //
                                     max_deltar_r);

        // will run if a trigger is found and matched
        if (has_trigger_match)
        {
            if (id == PDG::Muon::Id)
            {
                trigger_sf_nominal = //
                    trigger_sf_correctors({Trigger::get_year_for_muon_sf(year), fabs(_matched_nano_object_eta),
                                           _matched_nano_object_pt, "sf"});
                trigger_sf_up = //
                    trigger_sf_correctors({Trigger::get_year_for_muon_sf(year), fabs(_matched_nano_object_eta),
                                           _matched_nano_object_pt, "systup"});
                trigger_sf_down = //
                    trigger_sf_correctors({Trigger::get_year_for_muon_sf(year), fabs(_matched_nano_object_eta),
                                           _matched_nano_object_pt, "systdown"});
            }
            else if (id == PDG::Electron::Id)
            {
                trigger_sf_nominal = //
                    trigger_sf_correctors();
                trigger_sf_up = //
                    trigger_sf_correctors();
                trigger_sf_down = //
                    trigger_sf_correctors();
            }
            else
            {
                throw std::runtime_error(
                    fmt::format("ERROR: There is no trigger scale factor corrector defined for path: {}\n", hlt_path));
            }

            // fmt::print("SF: {} - {} - {}\n", trigger_sf_nominal, trigger_sf_up, trigger_sf_down);
        }
        return std::make_tuple(has_trigger_match, trigger_sf_nominal, trigger_sf_up, trigger_sf_down);
    }
};

inline auto make_trgobj_matcher(Year year, bool is_data) -> std::map<std::string_view, TrgObjMatcher>
{
    std::map<std::string_view, TrgObjMatcher> matchers;
    // for (auto &&hlt_path : Trigger::HLTPath)
    for (auto &&hlt_path : Trigger::ActivatedHLTPath)
    {
        double _max_delta_r = 0.;
        double _pt_min = std::numeric_limits<double>::max();
        unsigned int _id = std::numeric_limits<int>::max();

        if (hlt_path.find("Muon") != std::string::npos)
        {
            _max_delta_r = 0.1;
            switch (year)
            {
            case Year::Run2016APV:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 26.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 52.; // driven by scale factor lower bound
                }
                break;
            case Year::Run2016:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 26.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 52.; // driven by scale factor lower bound
                }
                break;
            case Year::Run2017:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 29.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 52.; // driven by scale factor lower bound
                }
                break;
            case Year::Run2018:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 26.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 52.; // driven by scale factor lower bound
                }
                break;
            default:
                throw std::runtime_error("Year (" + std::to_string(year) +
                                         ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
            }
            _id = PDG::Muon::Id;
        }
        else if (hlt_path.find("Electron") != std::string::npos)
        {
            _max_delta_r = 0.3;
            switch (year)
            {
            case Year::Run2016APV:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 29.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 118.; // driven by scale factor lower bound
                }
                break;
            case Year::Run2016:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 29.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 118.; // driven by scale factor lower bound
                }
                break;
            case Year::Run2017:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 37.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 118.; // driven by scale factor lower bound
                }
                break;
            case Year::Run2018:
                if (hlt_path.find("Low") != std::string::npos)
                {
                    _pt_min = 34.; // driven by scale factor lower bound
                }
                else
                {
                    _pt_min = 118.; // driven by scale factor lower bound
                }
                break;
            default:
                throw std::runtime_error("Year (" + std::to_string(year) +
                                         ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
            }
            _id = PDG::Electron::Id;
        }

        matchers.emplace(std::string_view(hlt_path),
                         TrgObjMatcher(hlt_path, _max_delta_r, _pt_min, _id, year, is_data));
    }
    return matchers;
}

struct TriggerBits
{
    std::bitset<Trigger::kTotalPaths> trigger_bits;

    TriggerBits &set(unsigned int path, bool value)
    {
        trigger_bits.set(path, value);
        return *this;
    }

    TriggerBits &set(const std::string_view &path, bool value)
    {
        trigger_bits.set(Trigger::HLTPath.index_of(path), value);
        return *this;
    }

    bool pass(const std::string_view &path) const
    {
        return trigger_bits.test(Trigger::HLTPath.index_of(path));
    }

    bool pass(unsigned int path) const
    {
        return trigger_bits.test(path);
    }

    bool any() const
    {
        return trigger_bits.any();
    }

    auto as_ulong() const
    {
        return trigger_bits.to_ulong();
    }

    auto as_ulonglong() const
    {
        return trigger_bits.to_ullong();
    }

    auto as_uint() const
    {
        return static_cast<unsigned int>(trigger_bits.to_ullong());
    }

    std::string_view as_string() const
    {
        return trigger_bits.to_string();
    }
};

#endif /*MUSIC_TRIGGER*/
