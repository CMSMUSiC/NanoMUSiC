
#include <format>
#include <fstream>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>

#include "ROOT/RDataFrame.hxx"
#include "TH2.h"

#include "json.hpp"
using json = nlohmann::json;

#include "Configs.hpp"
#include "CorrectionLibUtils.hpp"
#include "GeneratorFilters.hpp"
#include "JetCorrector.hpp"
#include "NanoAODGenInfo.hpp"
#include "ObjectFactories/make_jets.hpp"

// Sorry Felipe from the future...
// This is the quickiest way to get this madaflipa ACliC to
// work the way it is expected from a decent compiler...
#include "GeneratorFilters.cpp"
#include "JetCorrector.cpp"
#include "NanoAODGenInfo.cpp"

inline auto met_filters(bool Flag_goodVertices,
                        bool Flag_globalSuperTightHalo2016Filter,
                        bool Flag_HBHENoiseFilter,
                        bool Flag_HBHENoiseIsoFilter,
                        bool Flag_EcalDeadCellTriggerPrimitiveFilter,
                        bool Flag_BadPFMuonFilter,
                        bool Flag_BadPFMuonDzFilter,
                        bool Flag_eeBadScFilter,
                        bool Flag_hfNoisyHitsFilter,
                        bool Flag_ecalBadCalibFilter,
                        const std::string &year) -> bool
{
    auto _year = get_runyear(year);

    if (_year == Year::Run2016APV or _year == Year::Run2016)
    {
        return Flag_goodVertices and Flag_globalSuperTightHalo2016Filter and Flag_HBHENoiseFilter and
               Flag_HBHENoiseIsoFilter and Flag_EcalDeadCellTriggerPrimitiveFilter and Flag_BadPFMuonFilter and
               Flag_BadPFMuonDzFilter and Flag_eeBadScFilter and Flag_hfNoisyHitsFilter;
    }

    if (_year == Year::Run2017 or _year == Year::Run2018)
    {
        return Flag_goodVertices and Flag_globalSuperTightHalo2016Filter and Flag_HBHENoiseFilter and
               Flag_HBHENoiseIsoFilter and Flag_EcalDeadCellTriggerPrimitiveFilter and Flag_BadPFMuonFilter and
               Flag_BadPFMuonDzFilter and Flag_hfNoisyHitsFilter and Flag_eeBadScFilter and Flag_ecalBadCalibFilter;
    }

    throw std::runtime_error(
        std::format("Could not define MET filters bits. The requested year ({}) is invalid.", year));
};

auto get_era_from_process_name(const std::string &process, bool is_data) -> std::string
{
    if (is_data)
    {
        if (not(process.empty()))
        {
            return process.substr(process.length() - 1);
        }
        throw std::runtime_error(fmt::format("ERROR: Could not get era from process name ({}).\n", process));
    }
    return "_";
}

struct EventWeights
{
    double sum_weights;
    double total_events;
    bool should_use_LHEWeight;
};

#define DEFINE_ARRAY_IF_NOT_AVAILABLE(column, type)                                                                    \
    if (not(std::ranges::find(colNames, #column) != colNames.end()))                                                   \
    {                                                                                                                  \
        df = df.Define(#column,                                                                                        \
                       []() -> RVec<type>                                                                              \
                       {                                                                                               \
                           return {};                                                                                  \
                       },                                                                                              \
                       {});                                                                                            \
    }
#define DEFINE_VALUE_IF_NOT_AVAILABLE(column, type)                                                                    \
    if (not(std::ranges::find(colNames, #column) != colNames.end()))                                                   \
    {                                                                                                                  \
        df = df.Define(#column,                                                                                        \
                       []() -> type                                                                                    \
                       {                                                                                               \
                           return type();                                                                              \
                       },                                                                                              \
                       {});                                                                                            \
    }

#define DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(column, type, default_val)                                          \
    if (not(std::ranges::find(colNames, #column) != colNames.end()))                                                   \
    {                                                                                                                  \
        df = df.Define(#column,                                                                                        \
                       []() -> type                                                                                    \
                       {                                                                                               \
                           return default_val;                                                                         \
                       },                                                                                              \
                       {});                                                                                            \
    }

auto compute_btag_efficiency(const std::string &sample,
                             const std::string &process_group,
                             const std::string &generator_filter,
                             const std::string &input_file,
                             const std::string &year,
                             const std::string &sum_weights_json_filepath,
                             const double x_section,
                             const double luminosity,
                             const double filter_eff,
                             const double k_factor) -> void
{
    // create btag efficiency histograms
    constexpr std::array<double, 12> pt_bins = {0., 20., 30., 50., 70., 100., 140., 200., 300., 600., 1000., 7000.};
    auto btag_efficiency_light_num =
        TH2D(std::format("[{}]_light_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 1, 0., 3.);
    auto btag_efficiency_light_den =
        TH2D(std::format("[{}]_light_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 1, 0., 3.);
    auto btag_efficiency_c_num =
        TH2D(std::format("[{}]_c_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 1, 0., 3.);
    auto btag_efficiency_c_den =
        TH2D(std::format("[{}]_c_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 1, 0., 3.);
    auto btag_efficiency_b_num =
        TH2D(std::format("[{}]_b_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 1, 0., 3.);
    auto btag_efficiency_b_den =
        TH2D(std::format("[{}]_b_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 1, 0., 3.);

    btag_efficiency_light_num.Sumw2();
    btag_efficiency_light_den.Sumw2();
    btag_efficiency_c_num.Sumw2();
    btag_efficiency_c_den.Sumw2();
    btag_efficiency_b_num.Sumw2();
    btag_efficiency_b_den.Sumw2();

    // get sum of weights
    auto sum_weights_json_file = std::ifstream(sum_weights_json_filepath);
    if (!sum_weights_json_file.is_open())
    {
        throw std::runtime_error(
            std::format("ERROR: Could not open sum of weights JSON file. {}\n", sum_weights_json_filepath));
    }
    json sum_weights_json = json::parse(sum_weights_json_file);

    ROOT::RDataFrame df_src("Events", input_file);
    auto df = df_src.Filter(
        []() -> bool
        {
            return true;
        },
        {});

    auto colNames = df.GetColumnNames();

    DEFINE_ARRAY_IF_NOT_AVAILABLE(LHEPart_pt, float)
    DEFINE_ARRAY_IF_NOT_AVAILABLE(LHEPart_eta, float)
    DEFINE_ARRAY_IF_NOT_AVAILABLE(LHEPart_phi, float)
    DEFINE_ARRAY_IF_NOT_AVAILABLE(LHEPart_mass, float)
    DEFINE_ARRAY_IF_NOT_AVAILABLE(LHEPart_incomingpz, float)
    DEFINE_ARRAY_IF_NOT_AVAILABLE(LHEPart_pdgId, int)
    DEFINE_ARRAY_IF_NOT_AVAILABLE(LHEPart_status, int)

    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_goodVertices, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_globalSuperTightHalo2016Filter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_HBHENoiseFilter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_HBHENoiseIsoFilter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_EcalDeadCellTriggerPrimitiveFilter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_BadPFMuonFilter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_BadPFMuonDzFilter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_eeBadScFilter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_hfNoisyHitsFilter, bool, true)
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(Flag_ecalBadCalibFilter, bool, true)

    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Mu50, bool, false)                                               //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_TkMu50, bool, false)                                             //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_TkMu100, bool, false)                                            //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_OldMu100, bool, false)                                           //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ, bool, false)                  //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ, bool, false)                //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ, bool, false)                    //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL, bool, false)                     //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL, bool, false)                   //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL, bool, false)                       //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8, bool, false)              //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8, bool, false)            //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Photon175, bool, false)                                          //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Photon200, bool, false)                                          //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Ele115_CaloIdVT_GsfTrkIdT, bool, false)                          //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW, bool, false)                  //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleEle33_CaloIdL_MW, bool, false)                             //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleEle25_CaloIdL_MW, bool, false)                             //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90, bool, false) //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90, bool, false) //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass95, bool, false) //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_VLooseIsoPFTau120_Trk50_eta2p1, bool, false)                     //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_VLooseIsoPFTau140_Trk50_eta2p1, bool, false)                     //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(
        HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1, bool, false)                                       //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg, bool, false)               //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg, bool, false)       //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg, bool, false) //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(
        HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg, bool, false)                                   //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg, bool, false)     //
    DEFINE_VALUE_WITH_DEFAULT_IF_NOT_AVAILABLE(HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg, bool, false) //

    auto has_genWeight = false;
    auto has_LHEWeight_originalXWGTUP = false;
    for (auto &&colName : colNames)
    {
        if (colName == "genWeight")
        {
            has_genWeight = true;
        };
        if (colName == "LHEWeight_originalXWGTUP")
        {
            has_LHEWeight_originalXWGTUP = true;
        };
    }

    const auto event_weights = [&sum_weights_json, &sample, &year]() -> EventWeights
    {
        double sum_genWeight = sum_weights_json[sample][year]["sum_genWeight"];
        double sum_LHEWeight_originalXWGTUP = sum_weights_json[sample][year]["sum_LHEWeight"];
        long long raw_events = sum_weights_json[sample][year]["raw_events"];
        int _has_genWeight = sum_weights_json[sample][year]["has_genWeight"];
        bool has_genWeight = static_cast<bool>(_has_genWeight);
        int _has_LHEWeight_originalXWGTUP = sum_weights_json[sample][year]["has_LHEWeight_originalXWGTUP"];
        bool has_LHEWeight_originalXWGTUP = static_cast<bool>(_has_LHEWeight_originalXWGTUP);

        bool should_use_LHEWeight = false;
        if (has_genWeight and has_LHEWeight_originalXWGTUP)
        {
            if (sum_genWeight / static_cast<double>(raw_events) == 1.)
            {
                should_use_LHEWeight = true;
            }
        }

        if (has_genWeight and not(has_LHEWeight_originalXWGTUP))
        {
            should_use_LHEWeight = false;
        }

        if (not(has_genWeight) and has_LHEWeight_originalXWGTUP)
        {
            should_use_LHEWeight = true;
        }

        if (not(has_genWeight) and not(has_LHEWeight_originalXWGTUP))
        {
            throw std::runtime_error(
                std::format("ERROR: Could not assing sum of weights. This sample ({} - {}) has not genWeight or "
                            "LHEWeight_originalXWGTUP.",
                            sample,
                            year));
        }

        if (should_use_LHEWeight)
        {
            return EventWeights{.sum_weights = sum_LHEWeight_originalXWGTUP,
                                .total_events = static_cast<double>(raw_events),
                                .should_use_LHEWeight = should_use_LHEWeight};
        }
        else
        {
            return EventWeights{.sum_weights = sum_genWeight,
                                .total_events = static_cast<double>(raw_events),
                                .should_use_LHEWeight = should_use_LHEWeight};
        }
        return EventWeights{.sum_weights = 1., .total_events = 1., .should_use_LHEWeight = false};
    }();

    std::string genWeight_var_name = "genWeight";
    if (event_weights.should_use_LHEWeight)
    {
        genWeight_var_name = "LHEWeight_originalXWGTUP";
    }

    auto jet_corrections = JetCorrector(get_runyear(year), get_era_from_process_name(sample, false), false);
    auto correctionlib_utils = CorrectionLibUtils();
    auto jet_veto_map = correctionlib_utils.make_correctionlib_ref("JetVetoMap", year);

    auto btag_sf_bc = correctionlib_utils.make_correctionlib_ref("BTagSFbc", year);
    auto btag_sf_light = correctionlib_utils.make_correctionlib_ref("BTagSFlight", year);

    // constexpr unsigned int MAX_EVENTS = 1;
    constexpr unsigned int MAX_EVENTS = std::numeric_limits<unsigned int>::max();
    unsigned int event_counter = 0;

    df.Foreach(
        [&](float genWeight,                    //
            RVec<float> LHEPart_pt,             //
            RVec<float> LHEPart_eta,            //
            RVec<float> LHEPart_phi,            //
            RVec<float> LHEPart_mass,           //
            RVec<float> LHEPart_incomingpz,     //
            RVec<int> LHEPart_pdgId,            //
            RVec<int> LHEPart_status,           //
            RVec<float> GenPart_pt,             //
            RVec<float> GenPart_eta,            //
            RVec<float> GenPart_phi,            //
            RVec<float> GenPart_mass,           //
            RVec<int> GenPart_genPartIdxMother, //
            RVec<int> GenPart_pdgId,            //
            RVec<int> GenPart_status,           //
            RVec<int> GenPart_statusFlags,      //
            RVec<float> Jet_pt,                 //
            RVec<float> Jet_eta,                //
            RVec<float> Jet_phi,                //
            RVec<float> Jet_mass,               //
            RVec<int> Jet_jetId,                //
            RVec<float> Jet_btagDeepFlavB,      //
            RVec<float> Jet_rawFactor,          //
            RVec<float> Jet_area,               //
            RVec<float> Jet_chEmEF,             //
            RVec<int> Jet_puId,                 //
            RVec<float> Muon_eta,               //
            RVec<float> Muon_phi,               //
            RVec<bool> Muon_isPFcand,           //
            RVec<int> Jet_genJetIdx,            //
            RVec<int> Tau_jetIdx,               //
            RVec<int> Electron_jetIdx,          //
            RVec<int> Muon_jetIdx,              //
            float fixedGridRhoFastjetAll,       //
            RVec<float> GenJet_pt,              //
            RVec<float> GenJet_eta,             //
            RVec<float> GenJet_phi,             //
            int PV_npvsGood,
            bool Flag_goodVertices,                                         //
            bool Flag_globalSuperTightHalo2016Filter,                       //
            bool Flag_HBHENoiseFilter,                                      //
            bool Flag_HBHENoiseIsoFilter,                                   //
            bool Flag_EcalDeadCellTriggerPrimitiveFilter,                   //
            bool Flag_BadPFMuonFilter,                                      //
            bool Flag_BadPFMuonDzFilter,                                    //
            bool Flag_eeBadScFilter,                                        //
            bool Flag_hfNoisyHitsFilter,                                    //
            bool Flag_ecalBadCalibFilter,                                   //
            bool HLT_Mu50,                                                  //
            bool HLT_TkMu50,                                                //
            bool HLT_TkMu100,                                               //
            bool HLT_OldMu100,                                              //
            bool HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ,                     //
            bool HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ,                   //
            bool HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ,                       //
            bool HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL,                        //
            bool HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL,                      //
            bool HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL,                          //
            bool HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8,                 //
            bool HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8,               //
            bool HLT_Photon175,                                             //
            bool HLT_Photon200,                                             //
            bool HLT_Ele115_CaloIdVT_GsfTrkIdT,                             //
            bool HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW,                     //
            bool HLT_DoubleEle33_CaloIdL_MW,                                //
            bool HLT_DoubleEle25_CaloIdL_MW,                                //
            bool HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90,    //
            bool HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90,    //
            bool HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass95,    //
            bool HLT_VLooseIsoPFTau120_Trk50_eta2p1,                        //
            bool HLT_VLooseIsoPFTau140_Trk50_eta2p1,                        //
            bool HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1, //
            bool HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg,                //
            bool HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg,        //
            bool HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg,  //
            bool HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg, //
            bool HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg,          //
            bool HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg,      //
            RVec<int> Jet_hadronFlavour)
        {
            if (event_counter >= MAX_EVENTS)
            {
                return;
            }
            event_counter++;

            // GENERATOR FILTER
            if (not(GeneratorFilters::pass_generator_filter(generator_filter,
                                                            year,
                                                            LHEPart_pt,
                                                            LHEPart_eta,
                                                            LHEPart_phi,
                                                            LHEPart_mass,
                                                            LHEPart_incomingpz,
                                                            LHEPart_pdgId,
                                                            LHEPart_status,
                                                            GenPart_pt,
                                                            GenPart_eta,
                                                            GenPart_phi,
                                                            GenPart_mass,
                                                            GenPart_genPartIdxMother,
                                                            GenPart_pdgId,
                                                            GenPart_status,
                                                            GenPart_statusFlags)))
            {
                // printf("%s",
                // std::format("INFO: Event filtered out: {} - {} - {}\n", sample, process_group, year).c_str());

                return;
            }

            if (not(PV_npvsGood > 0))
            {
                return;
            }

            if (not(met_filters(Flag_goodVertices,
                                Flag_globalSuperTightHalo2016Filter,
                                Flag_HBHENoiseFilter,
                                Flag_HBHENoiseIsoFilter,
                                Flag_EcalDeadCellTriggerPrimitiveFilter,
                                Flag_BadPFMuonFilter,
                                Flag_BadPFMuonDzFilter,
                                Flag_eeBadScFilter,
                                Flag_hfNoisyHitsFilter,
                                Flag_ecalBadCalibFilter,
                                year)))
            {
                return;
            }

            // auto pass_low_pt_muon_trigger = [&](const std::string &year) -> bool
            // {
            //     auto _year = get_runyear(year);
            //     if (_year == Year::Run2016APV)
            //     {
            //         return HLT_IsoMu24 or HLT_IsoTkMu24;
            //     }

            //     if (_year == Year::Run2016)
            //     {
            //         return HLT_IsoMu24 or HLT_IsoTkMu24;
            //     }

            //     if (_year == Year::Run2017)
            //     {
            //         return HLT_IsoMu27;
            //     }

            //     if (_year == Year::Run2018)
            //     {
            //         return HLT_IsoMu24;
            //     }

            //     throw std::runtime_error( fmt::format("Could not define trigger bits. The requested year ({}) is
            //     invalid.", year) );
            // };

            auto pass_high_pt_muon_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);
                if (_year == Year::Run2016APV)
                {
                    return HLT_Mu50 or HLT_TkMu50;
                }

                if (_year == Year::Run2016)
                {
                    return HLT_Mu50 or HLT_TkMu50;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_Mu50 or HLT_TkMu100 or HLT_OldMu100;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_Mu50 or HLT_TkMu100 or HLT_OldMu100;
                }

                throw std::runtime_error(
                    std::format("Could not define trigger bits. The requested year ({}) is invalid.", year));
            };

            auto pass_double_muon_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);
                std::vector<std::string> double_muon_triggers = {};
                if (_year == Year::Run2016APV or _year == Year::Run2016)
                {
                    return HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ or HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ or
                           HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ or HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL or
                           HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL or HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8 or HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8;
                }

                throw std::runtime_error(
                    fmt::format("Could not define trigger bits. The requested year ({}) is invalid.", year));
            };

            // auto pass_low_pt_electron_trigger = [&](const std::string &year) -> bool
            // {
            //     auto _year = get_runyear(year);
            //     if (_year == Year::Run2016APV)
            //     {
            //         return HLT_Ele27_WPTight_Gsf;
            //     }

            //     if (_year == Year::Run2016)
            //     {
            //         return HLT_Ele27_WPTight_Gsf;
            //     }

            //     if (_year == Year::Run2017)
            //     {
            //         return HLT_Ele35_WPTight_Gsf;
            //     }

            //     if (_year == Year::Run2018)
            //     {
            //         return HLT_Ele32_WPTight_Gsf;
            //     }

            //     throw std::runtime_error( fmt::format("Could not define trigger bits. The requested year ({}) is
            //     invalid.", year) );
            // };

            auto pass_high_pt_electron_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);
                if (_year == Year::Run2016APV)
                {
                    return HLT_Photon175 or HLT_Ele115_CaloIdVT_GsfTrkIdT;
                    // return unwrap_or(HLT_Ele115_CaloIdVT_GsfTrkIdT;
                }

                if (_year == Year::Run2016)
                {
                    return HLT_Photon175 or HLT_Ele115_CaloIdVT_GsfTrkIdT;
                    // return unwrap_or(HLT_Ele115_CaloIdVT_GsfTrkIdT;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_Photon200 or HLT_Ele115_CaloIdVT_GsfTrkIdT;
                    // return unwrap_or(HLT_Ele115_CaloIdVT_GsfTrkIdT;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_Photon200 or HLT_Ele115_CaloIdVT_GsfTrkIdT;
                    // return unwrap_or(HLT_Ele115_CaloIdVT_GsfTrkIdT;
                }

                throw std::runtime_error(
                    fmt::format("Could not define trigger bits. The requested year ({}) is invalid.", year));
            };

            auto pass_double_electron_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);

                if (_year == Year::Run2016APV or _year == Year::Run2016)
                {
                    return HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW or HLT_DoubleEle33_CaloIdL_MW;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_DoubleEle33_CaloIdL_MW or HLT_DoubleEle25_CaloIdL_MW;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_DoubleEle25_CaloIdL_MW;
                }

                throw std::runtime_error(
                    fmt::format("Could not define trigger bits. The requested year ({}) is invalid.", year));
            };

            auto pass_photon_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);
                if (_year == Year::Run2016APV or _year == Year::Run2016)
                {
                    return HLT_Photon175;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_Photon200;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_Photon200;
                }

                throw std::runtime_error(
                    fmt::format("Could not define trigger bits. The requested year ({}) is invalid.", year));
            };

            // Source:
            // https://cmshltinfo.app.cern.ch/summary#state=ff9f9cf5-fecc-49de-8e42-9de74bb45ed6&session_state=bc67d35b-00fe-4c35-b107-24a35b8e9fb5&code=d14fac8b-5b46-49d2-8889-0adfe5f52145.bc67d35b-00fe-4c35-b107-24a35b8e9fb5.1363e04b-e180-4d83-92b3-3aca653d1d8d
            auto pass_double_photon_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);
                if (_year == Year::Run2016APV or _year == Year::Run2016)
                {
                    return HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90 or
                           HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass95;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90 or
                           HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass95;
                }

                throw std::runtime_error(
                    fmt::format("Could not define trigger bits. The requested year ({}) is invalid.", year));
            };

            auto pass_high_pt_tau_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);
                std::vector<std::string> high_pt_tau_triggers = {};
                if (_year == Year::Run2016APV or _year == Year::Run2016)
                {
                    return HLT_VLooseIsoPFTau120_Trk50_eta2p1 or HLT_VLooseIsoPFTau140_Trk50_eta2p1;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1;
                }

                throw std::runtime_error(
                    fmt::format("Could not define trigger bits. The requested year ({}) is invalid.", year));
            };

            auto pass_double_tau_trigger = [&](const std::string &year) -> bool
            {
                auto _year = get_runyear(year);
                std::vector<std::string> double_tau_triggers = {};

                if (_year == Year::Run2016APV or _year == Year::Run2016)
                {
                    return HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg or
                           HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg;
                }

                if (_year == Year::Run2017)
                {
                    return HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg or
                           HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg or
                           HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg;
                }

                if (_year == Year::Run2018)
                {
                    return HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg or
                           HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg or
                           HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg or
                           HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg;
                }

                throw std::runtime_error(
                    std::format("Could not define double tau trigger bits. The requested year ({}) is invalid.", year));
            };

            // Trigger
            auto is_good_trigger = std::set<bool>({false /*pass_low_pt_muon_trigger(year)*/,
                                                   pass_high_pt_muon_trigger(year),
                                                   pass_double_muon_trigger(year),
                                                   false /*pass_low_pt_electron_trigger(year)*/,
                                                   pass_high_pt_electron_trigger(year),
                                                   pass_double_electron_trigger(year),
                                                   pass_high_pt_tau_trigger(year),
                                                   pass_double_tau_trigger(year),
                                                   pass_photon_trigger(year),
                                                   pass_double_photon_trigger(year)})
                                       .contains(true);
            if (not(is_good_trigger))
            {
                return;
            }

            auto weight = genWeight / event_weights.sum_weights * x_section * luminosity * filter_eff * k_factor;

            // Check for NaNs
            if (std::isnan(weight) or std::isinf(weight))
            {
                throw std::runtime_error(std::format("NaN or INF weight found for weight!\n {} - {} - {} - {}",
                                                     sample,
                                                     process_group,
                                                     input_file,
                                                     year));
            }

            auto [nominal_jets,
                  nominal_bjets,
                  has_vetoed_jet,
                  nominal_selected_jet_indexes,
                  nominal_selected_bjet_indexes] =
                ObjectFactories::make_jets(
                    Jet_pt,                              //
                    Jet_eta,                             //
                    Jet_phi,                             //
                    Jet_mass,                            //
                    Jet_jetId,                           //
                    Jet_btagDeepFlavB,                   //
                    Jet_rawFactor,                       //
                    Jet_area,                            //
                    Jet_chEmEF,                          //
                    Jet_puId,                            //
                    Jet_hadronFlavour,                   //
                    Muon_eta,                            //
                    Muon_phi,                            //
                    Muon_isPFcand,                       //
                    Jet_genJetIdx,                       //
                    Tau_jetIdx,                          //
                    Electron_jetIdx,                     //
                    Muon_jetIdx,                         //
                    fixedGridRhoFastjetAll,              //
                    jet_corrections,                     //
                    btag_sf_light,                       //
                    btag_sf_bc,                          //
                    NanoAODGenInfo::GenJets(GenJet_pt,   //
                                            GenJet_eta,  //
                                            GenJet_phi), //
                    jet_veto_map,                        //
                    BTagEffMaps(
                        process_group, "", false, BTagEffMaps::IsDummy::Dummy), // no need to pass the btag eff maps
                    false,                                                      //
                    year,                                                       //
                    Shifts::Variations::Nominal);

            if (has_vetoed_jet)
            {
                return;
            }

            for (std::size_t i = 0; i < nominal_bjets.size(); i++)
            {
                switch (Jet_hadronFlavour.at(nominal_selected_bjet_indexes.at(i)))
                {
                case BTagEffMaps::HadronFlavor::LIGHT:
                    btag_efficiency_light_num.Fill(
                        nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    btag_efficiency_light_den.Fill(
                        nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    break;
                case BTagEffMaps::HadronFlavor::C:
                    btag_efficiency_c_num.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    btag_efficiency_c_den.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    break;
                case BTagEffMaps::HadronFlavor::B:
                    btag_efficiency_b_num.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    btag_efficiency_b_den.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    break;
                default:
                    throw std::runtime_error(std::format("Invalid hadron flavor: {}",
                                                         Jet_hadronFlavour.at(nominal_selected_bjet_indexes.at(i))));
                }
            }
            for (std::size_t i = 0; i < nominal_jets.size(); i++)
            {
                switch (Jet_hadronFlavour.at(nominal_selected_jet_indexes.at(i)))
                {
                case BTagEffMaps::HadronFlavor::LIGHT:
                    btag_efficiency_light_den.Fill(
                        nominal_jets.p4[i].pt(), std::fabs(nominal_jets.p4[i].eta()), weight);
                    break;
                case BTagEffMaps::HadronFlavor::C:
                    btag_efficiency_c_den.Fill(nominal_jets.p4[i].pt(), std::fabs(nominal_jets.p4[i].eta()), weight);
                    break;
                case BTagEffMaps::HadronFlavor::B:
                    btag_efficiency_b_den.Fill(nominal_jets.p4[i].pt(), std::fabs(nominal_jets.p4[i].eta()), weight);
                    break;
                default:
                    throw std::runtime_error(std::format("Invalid hadron flavor: {}",
                                                         Jet_hadronFlavour.at(nominal_selected_jet_indexes.at(i))));
                }
            }
        },
        {genWeight_var_name,
         "LHEPart_pt",                                                //
         "LHEPart_eta",                                               //
         "LHEPart_phi",                                               //
         "LHEPart_mass",                                              //
         "LHEPart_incomingpz",                                        //
         "LHEPart_pdgId",                                             //
         "LHEPart_status",                                            //
         "GenPart_pt",                                                //
         "GenPart_eta",                                               //
         "GenPart_phi",                                               //
         "GenPart_mass",                                              //
         "GenPart_genPartIdxMother",                                  //
         "GenPart_pdgId",                                             //
         "GenPart_status",                                            //
         "GenPart_statusFlags",                                       //
         "Jet_pt",                                                    //
         "Jet_eta",                                                   //
         "Jet_phi",                                                   //
         "Jet_mass",                                                  //
         "Jet_jetId",                                                 //
         "Jet_btagDeepFlavB",                                         //
         "Jet_rawFactor",                                             //
         "Jet_area",                                                  //
         "Jet_chEmEF",                                                //
         "Jet_puId",                                                  //
         "Muon_eta",                                                  //
         "Muon_phi",                                                  //
         "Muon_isPFcand",                                             //
         "Jet_genJetIdx",                                             //
         "Tau_jetIdx",                                                //
         "Electron_jetIdx",                                           //
         "Muon_jetIdx",                                               //
         "fixedGridRhoFastjetAll",                                    //
         "GenJet_pt",                                                 //
         "GenJet_eta",                                                //
         "GenJet_phi",                                                //
         "PV_npvsGood",                                               //
         "Flag_goodVertices",                                         //
         "Flag_globalSuperTightHalo2016Filter",                       //
         "Flag_HBHENoiseFilter",                                      //
         "Flag_HBHENoiseIsoFilter",                                   //
         "Flag_EcalDeadCellTriggerPrimitiveFilter",                   //
         "Flag_BadPFMuonFilter",                                      //
         "Flag_BadPFMuonDzFilter",                                    //
         "Flag_eeBadScFilter",                                        //
         "Flag_hfNoisyHitsFilter",                                    //
         "Flag_ecalBadCalibFilter",                                   //
         "HLT_Mu50",                                                  //
         "HLT_TkMu50",                                                //
         "HLT_TkMu100",                                               //
         "HLT_OldMu100",                                              //
         "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ",                     //
         "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ",                   //
         "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ",                       //
         "HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL",                        //
         "HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL",                      //
         "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL",                          //
         "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8",                 //
         "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",               //
         "HLT_Photon175",                                             //
         "HLT_Photon200",                                             //
         "HLT_Ele115_CaloIdVT_GsfTrkIdT",                             //
         "HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW",                     //
         "HLT_DoubleEle33_CaloIdL_MW",                                //
         "HLT_DoubleEle25_CaloIdL_MW",                                //
         "HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90",    //
         "HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90",    //
         "HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass95",    //
         "HLT_VLooseIsoPFTau120_Trk50_eta2p1",                        //
         "HLT_VLooseIsoPFTau140_Trk50_eta2p1",                        //
         "HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1", //
         "HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg",                //
         "HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg",        //
         "HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg",  //
         "HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg", //
         "HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg",          //
         "HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg",      //
         "Jet_hadronFlavour"});

    // save btag efficiency histograms
    std::unique_ptr<TFile> btag_eff_maps_file(TFile::Open(
        std::format("btag_eff_maps_buffer/{}_{}.root", process_group, std::hash<std::string>{}(input_file)).c_str(),
        "RECREATE"));
    btag_efficiency_light_num.Write();
    btag_efficiency_light_den.Write();
    btag_efficiency_c_num.Write();
    btag_efficiency_c_den.Write();
    btag_efficiency_b_num.Write();
    btag_efficiency_b_den.Write();
}
