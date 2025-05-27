
#include <format>
#include <fstream>
#include <limits>
#include <optional>
#include <ranges>

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

#define DEFINE_IF_NOT_AVAILABLE(column, type)                                                                          \
    if (not(std::ranges::find(colNames, #column) != colNames.end()))                                                   \
    {                                                                                                                  \
        df = df.Define(#column,                                                                                        \
                       []() -> RVec<type>                                                                              \
                       {                                                                                               \
                           return {};                                                                                  \
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
    constexpr std::array<double, 12> pt_bins = {std::numeric_limits<double>::lowest(),
                                                20.,
                                                30.,
                                                50.,
                                                70.,
                                                100.,
                                                140.,
                                                200.,
                                                300.,
                                                600.,
                                                1000.,
                                                std::numeric_limits<double>::max()};
    auto btag_efficiency_light_num =
        TH2D(std::format("[{}]_light_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    auto btag_efficiency_light_den =
        TH2D(std::format("[{}]_light_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    auto btag_efficiency_c_num =
        TH2D(std::format("[{}]_c_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    auto btag_efficiency_c_den =
        TH2D(std::format("[{}]_c_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    auto btag_efficiency_b_num =
        TH2D(std::format("[{}]_b_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    auto btag_efficiency_b_den =
        TH2D(std::format("[{}]_b_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);

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

    DEFINE_IF_NOT_AVAILABLE(LHEPart_pt, float)
    DEFINE_IF_NOT_AVAILABLE(LHEPart_eta, float)
    DEFINE_IF_NOT_AVAILABLE(LHEPart_phi, float)
    DEFINE_IF_NOT_AVAILABLE(LHEPart_mass, float)
    DEFINE_IF_NOT_AVAILABLE(LHEPart_incomingpz, float)
    DEFINE_IF_NOT_AVAILABLE(LHEPart_pdgId, int)
    DEFINE_IF_NOT_AVAILABLE(LHEPart_status, int)

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
            float fixedGridRhoFastjetAll,       //
            RVec<float> GenJet_pt,              //
            RVec<float> GenJet_eta,             //
            RVec<float> GenJet_phi,             //
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

            auto weight = genWeight / event_weights.sum_weights * x_section * luminosity * filter_eff * k_factor;

            auto [nominal_jets,
                  nominal_bjets,
                  has_vetoed_jet,
                  nominal_selected_jet_indexes,
                  nominal_selected_bjet_indexes,
                  btag_weight] =
                ObjectFactories::make_jets(
                    Jet_pt,                                                      //
                    Jet_eta,                                                     //
                    Jet_phi,                                                     //
                    Jet_mass,                                                    //
                    Jet_jetId,                                                   //
                    Jet_btagDeepFlavB,                                           //
                    Jet_rawFactor,                                               //
                    Jet_area,                                                    //
                    Jet_chEmEF,                                                  //
                    Jet_puId,                                                    //
                    Jet_hadronFlavour,                                           //
                    Muon_eta,                                                    //
                    Muon_phi,                                                    //
                    Muon_isPFcand,                                               //
                    Jet_genJetIdx,                                               //
                    fixedGridRhoFastjetAll,                                      //
                    jet_corrections,                                             //
                    btag_sf_light,                                               //
                    btag_sf_bc,                                                  //
                    NanoAODGenInfo::GenJets(GenJet_pt,                           //
                                            GenJet_eta,                          //
                                            GenJet_phi),                         //
                    jet_veto_map,                                                //
                    BTagEffMaps(process_group, "", BTagEffMaps::IsDummy::Dummy), // no need to pass the btag eff maps
                    false,                                                       //
                    year,                                                        //
                    Shifts::Variations::Nominal);

            if (has_vetoed_jet)
            {
                return;
            }

            for (std::size_t i = 0; i < nominal_bjets.size(); i++)
            {
                switch (Jet_hadronFlavour.at(nominal_selected_bjet_indexes.at(i)))
                {
                case HadronFlavor::LIGHT:
                    btag_efficiency_light_num.Fill(
                        nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    btag_efficiency_light_den.Fill(
                        nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    break;
                case HadronFlavor::C:
                    btag_efficiency_c_num.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    btag_efficiency_c_den.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    break;
                case HadronFlavor::B:
                    btag_efficiency_b_num.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    btag_efficiency_b_den.Fill(nominal_bjets.p4[i].pt(), std::fabs(nominal_bjets.p4[i].eta()), weight);
                    break;
                default:
                    throw std::runtime_error(std::format("Invalid hadron flavor: {}", hadron_flavor));
                }
            }
            for (std::size_t i = 0; i < nominal_jets.size(); i++)
            {
                switch (Jet_hadronFlavour.at(nominal_selected_jet_indexes.at(i)))
                {
                case HadronFlavor::LIGHT:
                    btag_efficiency_light_den.Fill(
                        nominal_jets.p4[i].pt(), std::fabs(nominal_jets.p4[i].eta()), weight);
                    break;
                case HadronFlavor::C:
                    btag_efficiency_c_den.Fill(nominal_jets.p4[i].pt(), std::fabs(nominal_jets.p4[i].eta()), weight);
                    break;
                case HadronFlavor::B:
                    btag_efficiency_b_den.Fill(nominal_jets.p4[i].pt(), std::fabs(nominal_jets.p4[i].eta()), weight);
                    break;
                default:
                    throw std::runtime_error(std::format("Invalid hadron flavor: {}", hadron_flavor));
                }
            }
        },
        {genWeight_var_name,
         "LHEPart_pt",               //
         "LHEPart_eta",              //
         "LHEPart_phi",              //
         "LHEPart_mass",             //
         "LHEPart_incomingpz",       //
         "LHEPart_pdgId",            //
         "LHEPart_status",           //
         "GenPart_pt",               //
         "GenPart_eta",              //
         "GenPart_phi",              //
         "GenPart_mass",             //
         "GenPart_genPartIdxMother", //
         "GenPart_pdgId",            //
         "GenPart_status",           //
         "GenPart_statusFlags",      //
         "Jet_pt",                   //
         "Jet_eta",                  //
         "Jet_phi",                  //
         "Jet_mass",                 //
         "Jet_jetId",                //
         "Jet_btagDeepFlavB",        //
         "Jet_rawFactor",            //
         "Jet_area",                 //
         "Jet_chEmEF",               //
         "Jet_puId",                 //
         "Muon_eta",                 //
         "Muon_phi",                 //
         "Muon_isPFcand",            //
         "Jet_genJetIdx",            //
         "fixedGridRhoFastjetAll",   //
         "GenJet_pt",                //
         "GenJet_eta",               //
         "GenJet_phi",               //
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
