#ifndef MUSIC_TRIGGER
#define MUSIC_TRIGGER

#include <limits>
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
constexpr auto HLTPath = Enumerate::make_enumerate("SingleMuonLowPt"sv,      //
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

constexpr auto ActivatedHLTPath = Enumerate::make_enumerate("SingleMuonLowPt"sv,     //
                                                            "SingleMuonHighPt"sv,    //
                                                            "SingleElectronLowPt"sv, //
                                                            "SingleElectronHighPt"sv);

// constexpr auto kTotalActivatedPaths = ActivatedHLTPath && (nano_objects.pt <= pt_min).size();

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
            VecOps::Min(VecOps::abs(nano_objects_pt[idx] - trigger_objects_pt) / (nano_objects_pt[idx]));
    }

    return std::make_pair(matches_distances, matches_relative_pT);
}

template <typename T1, typename T2>
constexpr std::tuple<bool, std::size_t> trigger_matcher(const T1 &trigger_objects_pt,  //
                                                        const T1 &trigger_objects_eta, //
                                                        const T1 &trigger_objects_phi, //
                                                        const T2 &nano_objects_pt,     //
                                                        const T2 &nano_objects_eta,    //
                                                        const T2 &nano_objects_phi,    //
                                                        const float max_delta_r)
{
    bool has_trigger_match = false;
    std::size_t matched_index = std::numeric_limits<unsigned int>::max();

    if (nano_objects_pt.size() > 0 and trigger_objects_pt.size() > 0)
    {
        auto [matches_distances, matches_rel_pT] = //
            get_matches(                           //
                trigger_objects_pt,                //
                trigger_objects_eta,               //
                trigger_objects_phi,               //
                nano_objects_pt,                   //
                nano_objects_eta,                  //
                nano_objects_phi                   //
            );

        auto good_delta_r = matches_distances <= max_delta_r;
        if (VecOps::Any(good_delta_r))
        {
            has_trigger_match = true;
            for (std::size_t i = 0; i < good_delta_r.size(); i++)
            {
                // the match should corresponds to the first matched object in the event
                if (good_delta_r.at(i) == 1)
                {
                    matched_index = i;
                    break;
                }
            }
        }
    }

    //////////////////////////////////////////////////////
    // DEBUG
    // fmt::print("###### DEBUG ############\n");
    // fmt::print("Has a match? : {}\n", has_trigger_match);
    // fmt::print("Distances: : {}\n", matches_distances);
    // fmt::print("Rel_pT: : {}\n", matches_rel_pT);
    ///////////////////////////////////////

    return std::make_tuple(has_trigger_match, matched_index);
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

// access functions for variant forms
// Reference: https://en.cppreference.com/w/cpp/utility/variant/visit
// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

inline auto get_pt(const std::variant<NanoObjects::Muons, NanoObjects::Electrons> &var_) -> RVec<float>
{
    return std::visit(
        overloaded{
            [](auto nano_obj) -> RVec<float>
            {
                return nano_obj.pt;
            },
        },
        var_);
}

inline auto get_eta(const std::variant<NanoObjects::Muons, NanoObjects::Electrons> &var_) -> RVec<float>
{
    return std::visit(
        overloaded{
            [](auto nano_obj) -> RVec<float>
            {
                return nano_obj.eta;
            },
        },
        var_);
}

inline auto get_phi(const std::variant<NanoObjects::Muons, NanoObjects::Electrons> &var_) -> RVec<float>
{
    return std::visit(
        overloaded{
            [](auto nano_obj) -> RVec<float>
            {
                return nano_obj.phi;
            },
        },
        var_);
}

inline auto get_delta_eta_sc(const std::variant<NanoObjects::Muons, NanoObjects::Electrons> &var_) -> RVec<float>
{
    return std::visit(
        overloaded{
            [](const NanoObjects::Electrons &nano_obj) -> RVec<float>
            {
                return nano_obj.deltaEtaSC;
            },
            [](auto nano_obj) -> RVec<float>
            {
                return RVec<float>(nano_obj.size, 0.f);
            },
        },
        var_);
}

class TrgObjMatcher
{
  public:
    const std::string_view hlt_path;
    const int id;
    const double max_deltar_r;
    const double pt_min;
    const unsigned int min_number_of_matches;
    const Year year;
    const bool is_data;
    const Corrector trigger_sf_correctors;

    TrgObjMatcher(const std::string_view &_hlt_path,
                  double _max_deltar_r,
                  double _pt_min,
                  unsigned int _min_number_of_matches,
                  int _id,
                  Year _year,
                  bool _is_data)
        : hlt_path(_hlt_path),
          id(_id),
          max_deltar_r(_max_deltar_r),
          pt_min(_pt_min),
          min_number_of_matches(_min_number_of_matches),
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
                            hlt_path,
                            Trigger::ActivatedHLTPath));
        }
    }

    auto operator()(const NanoObjects::TrgObjs &trgobjs,
                    const std::variant<NanoObjects::Muons, NanoObjects::Electrons> &nano_objects,
                    RVec<int> nano_objs_mask) const -> std::tuple<bool, double, double, double>
    {
        ////////////////////////////////////////
        // DEBUG
        // fmt::print("----------------------------- DEBUG ----------------------\n");
        // fmt::print("HLT Path: {}\n", hlt_path);
        /////////////////////////////////////////////////////
        double trigger_sf_nominal = 1.;
        double trigger_sf_up = 1.;
        double trigger_sf_down = 1.;

        // is it need, if we don't match? Nope ...
        // RVec<int> trgobj_mask = (                                            //
        //     (trgobjs.id == id) &&                                            //
        //     (Trigger::check_trigger_bit(trgobjs.filterBits, hlt_path, year)) //
        //     && (trgobjs.pt >= pt_min)                                        //
        // );

        // filter out bad NanoObjects
        auto raw_pt = get_pt(nano_objects)[nano_objs_mask];
        auto raw_eta = get_eta(nano_objects)[nano_objs_mask];
        // auto raw_phi = get_phi(nano_objects)[nano_objs_mask];
        auto raw_delta_eta_sc = get_delta_eta_sc(nano_objects)[nano_objs_mask];

        // will run if a trigger is found and matched
        auto nano_objects_pt_mask = raw_pt >= pt_min;
        bool has_trigger_match = static_cast<unsigned int>(VecOps::Sum(nano_objects_pt_mask)) >= min_number_of_matches;
        if (has_trigger_match)
        {
            // reverse ArgSort on pT
            auto sort_pt_indices = Argsort(raw_pt[nano_objects_pt_mask],
                                           [](double x, double y) -> bool
                                           {
                                               return x > y;
                                           });
            auto sorted_pt = Take(raw_pt[nano_objects_pt_mask], sort_pt_indices);
            auto sorted_eta = Take(raw_eta[nano_objects_pt_mask], sort_pt_indices);
            auto sorted_delta_eta_sc = Take(raw_delta_eta_sc[nano_objects_pt_mask], sort_pt_indices);

            // SingleMuon Paths
            if (id == PDG::Muon::Id and min_number_of_matches == 1)
            {
                auto _matched_nano_object_pt = sorted_pt.at(0);
                auto _matched_nano_object_eta = sorted_eta.at(0);

                trigger_sf_nominal = //
                    trigger_sf_correctors({CorrectionHelpers::get_year_for_muon_sf(year),
                                           fabs(_matched_nano_object_eta),
                                           _matched_nano_object_pt,
                                           "sf"});
                trigger_sf_up = //
                    trigger_sf_correctors({CorrectionHelpers::get_year_for_muon_sf(year),
                                           fabs(_matched_nano_object_eta),
                                           _matched_nano_object_pt,
                                           "systup"});
                trigger_sf_down = //
                    trigger_sf_correctors({CorrectionHelpers::get_year_for_muon_sf(year),
                                           fabs(_matched_nano_object_eta),
                                           _matched_nano_object_pt,
                                           "systdown"});
            }
            // SingleElectron Paths
            else if (id == PDG::Electron::Id and min_number_of_matches == 1)
            {
                auto _matched_nano_object_pt = sorted_pt.at(0);
                auto _matched_nano_object_eta_sc = sorted_eta.at(0) + sorted_delta_eta_sc.at(0);

                trigger_sf_nominal = //
                    trigger_sf_correctors(_matched_nano_object_eta_sc, _matched_nano_object_pt, "nominal");
                trigger_sf_up = //
                    trigger_sf_correctors(_matched_nano_object_eta_sc, _matched_nano_object_pt, "up");
                trigger_sf_down = //
                    trigger_sf_correctors(_matched_nano_object_eta_sc, _matched_nano_object_pt, "down");
            }
            else
            {
                throw std::runtime_error(
                    fmt::format("ERROR: Could not define Trigger SF. There is no trigger scale factor corrector "
                                "defined for path: {}\n",
                                hlt_path));
            }

            // fmt::print("SF: {} - {} - {}\n", trigger_sf_nominal, trigger_sf_up, trigger_sf_down);
        }

        // if (hlt_path == "SingleElectronLowPt")
        // {
        //     fmt::print("----------------------------- DEBUG ----------------------\n");
        //     fmt::print("HLT Path: {}\n", hlt_path);
        //     fmt::print("check_trigger_bit: {}\n", Trigger::check_trigger_bit(trgobjs.filterBits, hlt_path,
        //     year)); fmt::print("trgobj_mask: {}\n", trgobj_mask);
        //
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

        // const auto [has_trigger_match, matched_index] =
        //     Trigger::trigger_matcher(trgobjs.pt[trgobj_mask],                                       //
        //                              trgobjs.eta[trgobj_mask],                                      //
        //                              trgobjs.phi[trgobj_mask],                                      //
        //                              get_pt(nano_objects)[nano_objs_mask && nano_objects_pt_mask],  //
        //                              get_eta(nano_objects)[nano_objs_mask && nano_objects_pt_mask], //
        //                              get_phi(nano_objects)[nano_objs_mask && nano_objects_pt_mask], //
        //                              max_deltar_r);

        return std::make_tuple(has_trigger_match, trigger_sf_nominal, trigger_sf_up, trigger_sf_down);
    }
};

inline auto make_trgobj_matcher(Year year, bool is_data) -> std::map<std::string_view, TrgObjMatcher>
{
    std::map<std::string_view, TrgObjMatcher> matchers;
    for (auto &&hlt_path : Trigger::ActivatedHLTPath)
    {
        double _max_delta_r = 0.;
        unsigned int _min_number_of_matches = std::numeric_limits<unsigned int>::max();
        double _pt_min = std::numeric_limits<double>::max();
        unsigned int _id = std::numeric_limits<int>::max();

        if (hlt_path.find("Muon") != std::string::npos)
        {
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
                throw std::runtime_error(
                    fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).",
                                std::to_string(year)));
            }
            _id = PDG::Muon::Id;
            _max_delta_r = 0.1;
            _min_number_of_matches = 1;
        }
        else if (hlt_path.find("Electron") != std::string::npos)
        {
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
                throw std::runtime_error(
                    fmt::format("Year ({}) not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).",
                                std::to_string(year)));
            }
            _id = PDG::Electron::Id;
            _max_delta_r = 0.3;
            _min_number_of_matches = 1;
        }

        matchers.emplace(std::string_view(hlt_path),
                         TrgObjMatcher(hlt_path, _max_delta_r, _pt_min, _min_number_of_matches, _id, year, is_data));
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

    std::string as_string() const
    {
        return trigger_bits.to_string();
    }
};

#endif /*MUSIC_TRIGGER*/
