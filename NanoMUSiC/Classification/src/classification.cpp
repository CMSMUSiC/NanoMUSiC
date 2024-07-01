#include "Classification.hpp"

#include "Configs.hpp"
#include "GeneratorFilters.hpp"
#include "RtypesCore.h"
#include "RunLumiFilter.hpp"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "ValidationContainer.hpp"
#include "ZToLepLepX.hpp"
#include "fmt/core.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fmt/format.h>
#include <memory>
#include <optional>
#include <stdlib.h>
#include <string>

auto print_debug(long long global_event_index, bool debug) -> void
{
    // process monitoring
    if (debug)
    {
        if ((global_event_index < 10) or                                         //
            (global_event_index < 100 and global_event_index % 10 == 0) or       //
            (global_event_index < 1000 and global_event_index % 100 == 0) or     //
            (global_event_index < 10000 and global_event_index % 1000 == 0) or   //
            (global_event_index < 100000 and global_event_index % 10000 == 0) or //
            (global_event_index >= 100000 and global_event_index % 100000 == 0)  //
        )
        {
            fmt::print("\n\nProcessed {} events ...\n", global_event_index + 1);
            PrintProcessInfo();
        }
    }
    else
    {
        if ((global_event_index > 100000 and global_event_index % 100000 == 0))
        {
            fmt::print("\n\nProcessed {} events ...\n", global_event_index);
            PrintProcessInfo();
        }
    }
}

auto pass_generator_filter(const std::string &generator_filter,
                           const std::string &year_str,
                           ROOT::RVec<float> LHEPart_pt,
                           ROOT::RVec<float> LHEPart_eta,
                           ROOT::RVec<float> LHEPart_phi,
                           ROOT::RVec<float> LHEPart_mass,
                           ROOT::RVec<float> LHEPart_incomingpz,
                           ROOT::RVec<int> LHEPart_pdgId,
                           ROOT::RVec<int> LHEPart_status,
                           ROOT::RVec<float> GenPart_pt,
                           ROOT::RVec<float> GenPart_eta,
                           ROOT::RVec<float> GenPart_phi,
                           ROOT::RVec<float> GenPart_mass,
                           ROOT::RVec<int> GenPart_genPartIdxMother,
                           ROOT::RVec<int> GenPart_pdgId,
                           ROOT::RVec<int> GenPart_status,
                           ROOT::RVec<int> GenPart_statusFlags) -> bool
{
    if (generator_filter != "")
    {
        auto gen_filter_func = GeneratorFilters::get_filter(generator_filter);
        const auto lhe_particles = NanoAODGenInfo::LHEParticles(
            LHEPart_pt, LHEPart_eta, LHEPart_phi, LHEPart_mass, LHEPart_incomingpz, LHEPart_pdgId, LHEPart_status);

        const auto gen_particles = NanoAODGenInfo::GenParticles(GenPart_pt,
                                                                GenPart_eta,
                                                                GenPart_phi,
                                                                GenPart_mass,
                                                                GenPart_genPartIdxMother,
                                                                GenPart_pdgId,
                                                                GenPart_status,
                                                                GenPart_statusFlags);
        auto year = get_runyear(year_str);
        debugger_t debugger = std::nullopt;
        return gen_filter_func(lhe_particles, gen_particles, year, debugger);
    }

    return true;
}

struct EventWeights
{
    double sum_weights;
    double sum_pass_filter;
    double total_events;
    double total_events_pass_filter;
    bool should_use_LHEWeight;
};

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

    fmt::print(stderr, "ERROR: Could not define MET filters bits. The requested year ({}) is invalid.", year);
    std::exit(EXIT_FAILURE);
};

auto classification(const std::string process,
                    const std::string year,
                    const bool is_data,
                    const double x_section,
                    const double filter_eff,
                    const double k_factor,
                    const double luminosity,
                    const std::string xs_order,
                    const std::string process_group,
                    const std::string sum_weights_json_filepath,
                    const std::string input_file,
                    const std::string &generator_filter,
                    // [EVENT_CLASS_NAME, [SHIFT, EVENT_CLASS_OBJECT] ]
                    EventClassContainer &event_classes,
                    ValidationContainer &validation_container,
                    std::optional<unsigned long> first_event,
                    std::optional<long> last_event,
                    const bool debug) -> void
{
    fmt::print("\n[MUSiC Classification] Starting ...\n");
    if (debug)
    {
        fmt::print("[MUSiC Classification] Will process file: {}\n", input_file);
    }

    if (not(first_event))
    {
        first_event = 0;
    }
    if (not(last_event))
    {
        last_event = -1;
    }

    const auto shifts = Shifts(is_data);

    // Run/Lumi filter
    auto golden_json = [](const std::string &year) -> std::string
    {
        auto _year = get_runyear(year);
        if (_year == Year::Run2016APV)
        {
            return std::string(RunConfig::Run2016APV.golden_json);
        }
        if (_year == Year::Run2016)
        {
            return std::string(RunConfig::Run2016.golden_json);
        }
        if (_year == Year::Run2017)
        {
            return std::string(RunConfig::Run2017.golden_json);
        }
        if (_year == Year::Run2018)
        {
            return std::string(RunConfig::Run2018.golden_json);
        }

        fmt::print(stderr, "ERROR: Could not find Golden JSON for the requested year ({}).", year);
        std::exit(EXIT_FAILURE);
    };
    auto run_lumi_filter = RunLumiFilter(golden_json(year));

    // corrections
    auto correctionlib_utils = CorrectionLibUtils();
    auto jet_corrections = JetCorrector(get_runyear(year), get_era_from_process_name(process, is_data), is_data);
    auto pu_corrector = correctionlib_utils.make_correctionlib_ref("PU", year);

    auto low_pt_muon_trigger_sf = correctionlib_utils.make_correctionlib_ref("SingleMuonLowPt", year);
    auto high_pt_muon_trigger_sf = correctionlib_utils.make_correctionlib_ref("SingleMuonHighPt", year);

    auto muon_sf_reco = correctionlib_utils.make_correctionlib_ref("MuonReco", year);
    auto muon_sf_id_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIdLowPt", year);
    auto muon_sf_id_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIdHighPt", year);
    auto muon_sf_iso_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoLowPt", year);
    auto muon_sf_iso_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoHighPt", year);

    auto electron_sf = correctionlib_utils.make_correctionlib_ref("ElectronSF", year);

    auto photon_sf = correctionlib_utils.make_correctionlib_ref("PhotonSF", year);
    auto pixel_veto_sf = correctionlib_utils.make_correctionlib_ref("PixelVetoSF", year);

    auto deep_tau_2017_v2_p1_vs_e = correctionlib_utils.make_correctionlib_ref("TauVSe", year);
    auto deep_tau_2017_v2_p1_vs_mu = correctionlib_utils.make_correctionlib_ref("TauVSmu", year);
    auto deep_tau_2017_v2_p1_vs_jet = correctionlib_utils.make_correctionlib_ref("TauVSjet", year);
    auto tau_energy_scale = correctionlib_utils.make_correctionlib_ref("TauEnergyScale", year);

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    // [ BEGIN ]  LHAPDF
    /////////////////////////////////////////////
    /////////////////////////////////////////////
    // initilize pdf sets for fallback cases ...
    // silence LHAPDF
    LHAPDF::setVerbosity(0);
    // NNPDF31_nnlo_as_0118_hessian
    int lha_id_fallback = 304400;
    int lha_size = 101;

    // Compute the PDF weight for this event using NNPDF31_nnlo_as_0118_hessian (304400) and divide the
    // new weight by the weight from the PDF the event was produced with.
    std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>, std::unique_ptr<LHAPDF::PDF>, std::unique_ptr<LHAPDF::PDF>>
        default_pdf_sets;
    std::get<0>(default_pdf_sets).reserve(101);
    for (int idx = lha_id_fallback; idx < (lha_id_fallback + lha_size); idx++)
    {
        std::get<0>(default_pdf_sets).push_back(std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(lha_id_fallback)));
    }

    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0120 (319500) and divide the new
    // weight by the weight from the PDF the event was produced with.
    std::get<1>(default_pdf_sets) = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(319500));

    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0116 (319300) and divide the new
    // weight by the weight from the PDF the event was produced with.
    std::get<2>(default_pdf_sets) = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(319300));

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    // [ END ] LHAPDF
    /////////////////////////////////////////////
    /////////////////////////////////////////////

    // get sum of weights
    auto sum_weights_json_file = std::ifstream(sum_weights_json_filepath);
    if (!sum_weights_json_file.is_open())
    {
        fmt::print(stderr, "ERROR: Coudl not open sum of weights JSON file. {}\n", sum_weights_json_filepath);
        std::exit(EXIT_FAILURE);
    }
    json sum_weights_json = json::parse(sum_weights_json_file);

    const auto event_weights = [&sum_weights_json, &process, &year, is_data]() -> EventWeights
    {
        if (not(is_data))
        {
            double sum_genWeight = sum_weights_json[process][year]["sum_genWeight"];
            double sum_genWeight_pass_filter = sum_weights_json[process][year]["sum_genWeight_pass_generator_filter"];
            double sum_LHEWeight_originalXWGTUP = sum_weights_json[process][year]["sum_LHEWeight"];
            double sum_LHEWeight_originalXWGTUP_pass_filter =
                sum_weights_json[process][year]["sum_LHEWeight_pass_generator_filter"];
            long long raw_events = sum_weights_json[process][year]["raw_events"];
            long long raw_events_pass_filters = sum_weights_json[process][year]["pass_generator_filter"];
            int _has_genWeight = sum_weights_json[process][year]["has_genWeight"];
            bool has_genWeight = static_cast<bool>(_has_genWeight);
            int _has_LHEWeight_originalXWGTUP = sum_weights_json[process][year]["has_LHEWeight_originalXWGTUP"];
            bool has_LHEWeight_originalXWGTUP = static_cast<bool>(_has_LHEWeight_originalXWGTUP);

            bool should_use_LHEWeight = false;
            if (has_genWeight and has_LHEWeight_originalXWGTUP)
            {
                if (sum_genWeight / static_cast<double>(raw_events) != 1.)
                {
                    should_use_LHEWeight = false;
                }
                else
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
                fmt::print(stderr,
                           "ERROR: Could not assing sum of weights. This sample ({} - {}) has not genWeight or "
                           "LHEWeight_originalXWGTUP.",
                           process,
                           year);
                std::exit(EXIT_FAILURE);
            }

            if (should_use_LHEWeight)
            {
                return EventWeights{.sum_weights = sum_LHEWeight_originalXWGTUP,
                                    .sum_pass_filter = sum_LHEWeight_originalXWGTUP_pass_filter,
                                    .total_events = static_cast<double>(raw_events),
                                    .total_events_pass_filter = static_cast<double>(raw_events_pass_filters),
                                    .should_use_LHEWeight = should_use_LHEWeight};
            }
            else
            {
                return EventWeights{.sum_weights = sum_genWeight,
                                    .sum_pass_filter = sum_genWeight_pass_filter,
                                    .total_events = static_cast<double>(raw_events),
                                    .total_events_pass_filter = static_cast<double>(raw_events_pass_filters),
                                    .should_use_LHEWeight = should_use_LHEWeight};
            }
        }
        return EventWeights{.sum_weights = 1.,
                            .sum_pass_filter = 1.,
                            .total_events = 1.,
                            .total_events_pass_filter = 1.,
                            .should_use_LHEWeight = false};
    }();

    auto input_root_file = std::unique_ptr<TFile>(TFile::Open(input_file.c_str()));
    auto input_ttree = input_root_file->Get<TTree>("Events");
    input_ttree->SetBranchStatus("*", false);
    auto tree_reader = TTreeReader(input_ttree);

    // check for the available pdf weights in the tree
    std::optional<std::pair<unsigned int, unsigned int>> lha_indexes =
        PDFAlphaSWeights::get_pdf_ids(tree_reader.GetTree());

    std::optional<std::unique_ptr<LHAPDF::PDF>> this_sample_pdf = std::nullopt;
    if (lha_indexes)
    {
        this_sample_pdf = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(std::get<0>(*lha_indexes)));
    }

    ADD_VALUE_READER(run, unsigned int);
    ADD_VALUE_READER(luminosityBlock, unsigned int);

    ADD_VALUE_READER(PV_npvsGood, int);

    // https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2
    ADD_VALUE_READER(Flag_goodVertices, bool);
    ADD_VALUE_READER(Flag_globalSuperTightHalo2016Filter, bool);
    ADD_VALUE_READER(Flag_HBHENoiseFilter, bool);
    ADD_VALUE_READER(Flag_HBHENoiseIsoFilter, bool);
    ADD_VALUE_READER(Flag_EcalDeadCellTriggerPrimitiveFilter, bool);
    ADD_VALUE_READER(Flag_BadPFMuonFilter, bool);
    ADD_VALUE_READER(Flag_BadPFMuonDzFilter, bool);
    ADD_VALUE_READER(Flag_eeBadScFilter, bool);
    ADD_VALUE_READER(Flag_hfNoisyHitsFilter, bool);
    ADD_VALUE_READER(Flag_ecalBadCalibFilter, bool);

    ADD_VALUE_READER(HLT_IsoMu24, bool);
    ADD_VALUE_READER(HLT_IsoTkMu24, bool);
    ADD_VALUE_READER(HLT_IsoMu27, bool);
    ADD_VALUE_READER(HLT_Mu50, bool);
    ADD_VALUE_READER(HLT_TkMu50, bool);
    ADD_VALUE_READER(HLT_TkMu100, bool);
    ADD_VALUE_READER(HLT_OldMu100, bool);
    ADD_VALUE_READER(HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ, bool);
    ADD_VALUE_READER(HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ, bool);
    ADD_VALUE_READER(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ, bool);
    ADD_VALUE_READER(HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL, bool);
    ADD_VALUE_READER(HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL, bool);
    ADD_VALUE_READER(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL, bool);
    ADD_VALUE_READER(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8, bool);
    ADD_VALUE_READER(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8, bool);
    ADD_VALUE_READER(HLT_Ele27_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Ele35_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Ele32_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Photon175, bool);
    ADD_VALUE_READER(HLT_Ele115_CaloIdVT_GsfTrkIdT, bool);
    ADD_VALUE_READER(HLT_Photon200, bool);
    ADD_VALUE_READER(HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW, bool);
    ADD_VALUE_READER(HLT_DoubleEle33_CaloIdL_MW, bool);
    ADD_VALUE_READER(HLT_DoubleEle25_CaloIdL_MW, bool);
    ADD_VALUE_READER(HLT_VLooseIsoPFTau120_Trk50_eta2p1, bool);
    ADD_VALUE_READER(HLT_VLooseIsoPFTau140_Trk50_eta2p1, bool);
    ADD_VALUE_READER(HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1, bool);
    ADD_VALUE_READER(HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg, bool);
    ADD_VALUE_READER(HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg, bool);
    ADD_VALUE_READER(HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg, bool);
    ADD_VALUE_READER(HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg, bool);
    ADD_VALUE_READER(HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg, bool);
    ADD_VALUE_READER(HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg, bool);
    ADD_VALUE_READER(HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90, bool);
    ADD_VALUE_READER(HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90, bool);

    ADD_VALUE_READER(genWeight, float);
    ADD_VALUE_READER(LHEWeight_originalXWGTUP, float);

    ADD_VALUE_READER(Pileup_nTrueInt, float);
    ADD_VALUE_READER(L1PreFiringWeight_Up, float);
    ADD_VALUE_READER(L1PreFiringWeight_Dn, float);
    ADD_VALUE_READER(L1PreFiringWeight_Nom, float);

    ADD_ARRAY_READER(LHEPdfWeight, float);
    ADD_VALUE_READER(Generator_scalePDF, float);
    ADD_VALUE_READER(Generator_x1, float);
    ADD_VALUE_READER(Generator_x2, float);
    ADD_VALUE_READER(Generator_id1, int);
    ADD_VALUE_READER(Generator_id2, int);

    ADD_ARRAY_READER(LHEScaleWeight, float);

    ADD_ARRAY_READER(LHEPart_pt, float);
    ADD_ARRAY_READER(LHEPart_eta, float);
    ADD_ARRAY_READER(LHEPart_phi, float);
    ADD_ARRAY_READER(LHEPart_mass, float);
    ADD_ARRAY_READER(LHEPart_incomingpz, float);
    ADD_ARRAY_READER(LHEPart_pdgId, int);
    ADD_ARRAY_READER(LHEPart_status, int);

    ADD_ARRAY_READER(GenPart_pt, float);
    ADD_ARRAY_READER(GenPart_eta, float);
    ADD_ARRAY_READER(GenPart_phi, float);
    ADD_ARRAY_READER(GenPart_mass, float);
    ADD_ARRAY_READER(GenPart_genPartIdxMother, int);
    ADD_ARRAY_READER(GenPart_pdgId, int);
    ADD_ARRAY_READER(GenPart_status, int);
    ADD_ARRAY_READER(GenPart_statusFlags, int);

    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);
    ADD_ARRAY_READER(Muon_highPurity, bool);
    ADD_ARRAY_READER(Muon_genPartIdx, int);

    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);
    ADD_ARRAY_READER(Electron_deltaEtaSC, float);
    ADD_ARRAY_READER(Electron_cutBased, int);
    ADD_ARRAY_READER(Electron_cutBased_HEEP, bool);
    ADD_ARRAY_READER(Electron_scEtOverPt, float);
    ADD_ARRAY_READER(Electron_dEscaleUp, float);
    ADD_ARRAY_READER(Electron_dEscaleDown, float);
    ADD_ARRAY_READER(Electron_dEsigmaUp, float);
    ADD_ARRAY_READER(Electron_dEsigmaDown, float);
    ADD_ARRAY_READER(Electron_genPartIdx, int);

    ADD_ARRAY_READER(Tau_pt, float);
    ADD_ARRAY_READER(Tau_eta, float);
    ADD_ARRAY_READER(Tau_phi, float);
    ADD_ARRAY_READER(Tau_dz, float);
    ADD_ARRAY_READER(Tau_mass, float);
    ADD_ARRAY_READER(Tau_idDeepTau2017v2p1VSe, UChar_t);
    ADD_ARRAY_READER(Tau_idDeepTau2017v2p1VSjet, UChar_t);
    ADD_ARRAY_READER(Tau_idDeepTau2017v2p1VSmu, UChar_t);
    ADD_ARRAY_READER(Tau_decayMode, int);
    ADD_ARRAY_READER(Tau_genPartIdx, int);
    ADD_ARRAY_READER(Tau_genPartFlav, UChar_t);

    ADD_ARRAY_READER(Photon_pt, float);
    ADD_ARRAY_READER(Photon_eta, float);
    ADD_ARRAY_READER(Photon_phi, float);
    ADD_ARRAY_READER(Photon_isScEtaEB, bool);
    ADD_ARRAY_READER(Photon_isScEtaEE, bool);
    ADD_ARRAY_READER(Photon_cutBased, Int_t);
    ADD_ARRAY_READER(Photon_pixelSeed, bool);
    ADD_ARRAY_READER(Photon_dEscaleUp, float);
    ADD_ARRAY_READER(Photon_dEscaleDown, float);
    ADD_ARRAY_READER(Photon_dEsigmaUp, float);
    ADD_ARRAY_READER(Photon_dEsigmaDown, float);
    ADD_ARRAY_READER(Photon_genPartIdx, int);

    ADD_VALUE_READER(fixedGridRhoFastjetAll, float);

    ADD_ARRAY_READER(GenJet_pt, float);
    ADD_ARRAY_READER(GenJet_eta, float);
    ADD_ARRAY_READER(GenJet_phi, float);

    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);
    ADD_ARRAY_READER(Jet_mass, float);
    ADD_ARRAY_READER(Jet_jetId, Int_t);
    ADD_ARRAY_READER(Jet_btagDeepFlavB, float);
    ADD_ARRAY_READER(Jet_rawFactor, float);
    ADD_ARRAY_READER(Jet_area, float);
    ADD_ARRAY_READER(Jet_genJetIdx, Int_t);

    ADD_VALUE_READER(RawMET_pt, float);
    ADD_VALUE_READER(RawMET_phi, float);
    ADD_VALUE_READER(MET_MetUnclustEnUpDeltaX, float);
    ADD_VALUE_READER(MET_MetUnclustEnUpDeltaY, float);

    //  launch event loop for Data or MC
    fmt::print("\n[MUSiC Classification] Starting event loop ...\n");
    long long global_event_index = 0;
    for (auto &&event : tree_reader)
    {
        print_debug(global_event_index, debug);
        if (event < static_cast<long long>(*first_event))
        {
            continue;
        }
        else if (event > static_cast<long long>(*last_event) and static_cast<long long>(*last_event) >= 0)
        {
            break;
        }
        global_event_index++;

        // check for chain readout quality
        // REFERENCE: https://root.cern.ch/doc/v608/classTTreeReader.html#a568e43c7d7d8b1f511bbbeb92c9094a8
        if (tree_reader.GetEntryStatus() != TTreeReader::EEntryStatus::kEntryValid)
        {
            fmt::print(stderr, "ERROR: Could not load TTree entry.");
            std::exit(EXIT_FAILURE);
        }

        if (not(pass_generator_filter(generator_filter,
                                      year,
                                      unwrap(LHEPart_pt),
                                      unwrap(LHEPart_eta),
                                      unwrap(LHEPart_phi),
                                      unwrap(LHEPart_mass),
                                      unwrap(LHEPart_incomingpz),
                                      unwrap(LHEPart_pdgId),
                                      unwrap(LHEPart_status),
                                      unwrap(GenPart_pt),
                                      unwrap(GenPart_eta),
                                      unwrap(GenPart_phi),
                                      unwrap(GenPart_mass),
                                      unwrap(GenPart_genPartIdxMother),
                                      unwrap(GenPart_pdgId),
                                      unwrap(GenPart_status),
                                      unwrap(GenPart_statusFlags))))
        {
            continue;
        }

        if (not(run_lumi_filter(unwrap(run), unwrap(luminosityBlock), is_data)))
        {
            continue;
        }

        if (not(unwrap_or(PV_npvsGood, 1) > 0))
        {
            continue;
        }

        if (not(met_filters(unwrap(Flag_goodVertices),
                            unwrap(Flag_globalSuperTightHalo2016Filter),
                            unwrap(Flag_HBHENoiseFilter),
                            unwrap(Flag_HBHENoiseIsoFilter),
                            unwrap(Flag_EcalDeadCellTriggerPrimitiveFilter),
                            unwrap(Flag_BadPFMuonFilter),
                            unwrap(Flag_BadPFMuonDzFilter),
                            unwrap(Flag_eeBadScFilter),
                            unwrap(Flag_hfNoisyHitsFilter),
                            unwrap(Flag_ecalBadCalibFilter),
                            year)))
        {
            continue;
        }

        auto pass_low_pt_muon_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            if (_year == Year::Run2016APV)
            {
                return unwrap(HLT_IsoMu24) or unwrap(HLT_IsoTkMu24);
            }

            if (_year == Year::Run2016)
            {
                return unwrap(HLT_IsoMu24) or unwrap(HLT_IsoTkMu24);
            }

            if (_year == Year::Run2017)
            {
                return unwrap(HLT_IsoMu27);
            }

            if (_year == Year::Run2018)
            {
                return unwrap(HLT_IsoMu24);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_high_pt_muon_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            if (_year == Year::Run2016APV)
            {
                return unwrap(HLT_Mu50) or unwrap(HLT_TkMu50);
            }

            if (_year == Year::Run2016)
            {
                return unwrap(HLT_Mu50) or unwrap(HLT_TkMu50);
            }

            if (_year == Year::Run2017)
            {
                return unwrap(HLT_Mu50) or unwrap(HLT_TkMu100) or unwrap(HLT_OldMu100);
            }

            if (_year == Year::Run2018)
            {
                return unwrap(HLT_Mu50) or unwrap(HLT_TkMu100) or unwrap(HLT_OldMu100);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_double_muon_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            std::vector<std::string> double_muon_triggers = {};
            if (_year == Year::Run2016APV or _year == Year::Run2016)
            {
                return unwrap(HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ) or
                       unwrap(HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ) or unwrap(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ) or
                       unwrap(HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL) or unwrap(HLT_TkMu17_TrkIsoVVL_TkMu8_TrkIsoVVL) or
                       unwrap(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL);
            }

            if (_year == Year::Run2017)
            {
                return unwrap(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8) or
                       unwrap(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8);
            }

            if (_year == Year::Run2018)
            {
                return unwrap(HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_low_pt_electron_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            if (_year == Year::Run2016APV)
            {
                return unwrap(HLT_Ele27_WPTight_Gsf);
            }

            if (_year == Year::Run2016)
            {
                return unwrap(HLT_Ele27_WPTight_Gsf);
            }

            if (_year == Year::Run2017)
            {
                return unwrap(HLT_Ele35_WPTight_Gsf);
            }

            if (_year == Year::Run2018)
            {
                return unwrap(HLT_Ele32_WPTight_Gsf);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_high_pt_electron_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            if (_year == Year::Run2016APV)
            {
                return unwrap(HLT_Photon175) or unwrap(HLT_Ele115_CaloIdVT_GsfTrkIdT);
            }

            if (_year == Year::Run2016)
            {
                return unwrap(HLT_Photon175) or unwrap(HLT_Ele115_CaloIdVT_GsfTrkIdT);
            }

            if (_year == Year::Run2017)
            {
                return unwrap(HLT_Photon200) or unwrap(HLT_Ele115_CaloIdVT_GsfTrkIdT);
            }
            if (_year == Year::Run2018)
            {
                return unwrap(HLT_Photon200) or unwrap(HLT_Ele115_CaloIdVT_GsfTrkIdT);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_double_electron_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);

            if (_year == Year::Run2016APV or _year == Year::Run2016)
            {
                return unwrap(HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW) or unwrap(HLT_DoubleEle33_CaloIdL_MW);
            }
            if (_year == Year::Run2017)
            {
                return unwrap(HLT_DoubleEle33_CaloIdL_MW) or unwrap(HLT_DoubleEle25_CaloIdL_MW);
            }
            if (_year == Year::Run2018)
            {
                return unwrap(HLT_DoubleEle25_CaloIdL_MW);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_photon_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            if (_year == Year::Run2016APV or _year == Year::Run2016)
            {
                return unwrap(HLT_Photon175);
            }
            if (_year == Year::Run2017)
            {
                return unwrap(HLT_Photon200);
            }

            if (_year == Year::Run2018)
            {
                return unwrap(HLT_Photon200);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        // Source:
        // https://cmshltinfo.app.cern.ch/summary#state=ff9f9cf5-fecc-49de-8e42-9de74bb45ed6&session_state=bc67d35b-00fe-4c35-b107-24a35b8e9fb5&code=d14fac8b-5b46-49d2-8889-0adfe5f52145.bc67d35b-00fe-4c35-b107-24a35b8e9fb5.1363e04b-e180-4d83-92b3-3aca653d1d8d
        auto pass_double_photon_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            if (_year == Year::Run2016APV or _year == Year::Run2016)
            {
                return unwrap(HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90);
            }
            if (_year == Year::Run2017)
            {
                return unwrap(HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90);
            }

            if (_year == Year::Run2018)
            {
                return unwrap(HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_high_pt_tau_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            std::vector<std::string> high_pt_tau_triggers = {};
            if (_year == Year::Run2016APV or _year == Year::Run2016)
            {
                return unwrap(HLT_VLooseIsoPFTau120_Trk50_eta2p1) or unwrap(HLT_VLooseIsoPFTau140_Trk50_eta2p1);
            }
            if (_year == Year::Run2017)
            {
                return unwrap(HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1);
            }

            if (_year == Year::Run2018)
            {
                return unwrap(HLT_MediumChargedIsoPFTau180HighPtRelaxedIso_Trk50_eta2p1);
            }

            fmt::print(stderr, "ERROR: Could not define trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        auto pass_double_tau_trigger = [&](const std::string &year) -> bool
        {
            auto _year = get_runyear(year);
            std::vector<std::string> double_tau_triggers = {};

            if (_year == Year::Run2016APV or _year == Year::Run2016)
            {
                return unwrap(HLT_DoubleMediumIsoPFTau35_Trk1_eta2p1_Reg) or
                       unwrap(HLT_DoubleMediumCombinedIsoPFTau35_Trk1_eta2p1_Reg);
            }
            if (_year == Year::Run2017)
            {
                return unwrap(HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg) or
                       unwrap(HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg) or
                       (HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg);
            }
            if (_year == Year::Run2018)
            {
                return unwrap(HLT_DoubleTightChargedIsoPFTau35_Trk1_TightID_eta2p1_Reg) or
                       unwrap(HLT_DoubleMediumChargedIsoPFTau40_Trk1_TightID_eta2p1_Reg) or
                       unwrap(HLT_DoubleTightChargedIsoPFTau40_Trk1_eta2p1_Reg) or
                       unwrap(HLT_DoubleMediumChargedIsoPFTauHPS35_Trk1_eta2p1_Reg);
            }

            fmt::print(
                stderr, "ERROR: Could not define double tau trigger bits. The requested year ({}) is invalid.", year);
            std::exit(EXIT_FAILURE);
        };

        // Trigger
        auto is_good_trigger = trigger_filter(process,           //
                                              is_data,           //
                                              get_runyear(year), //
                                              pass_low_pt_muon_trigger(year),
                                              pass_high_pt_muon_trigger(year),
                                              pass_double_muon_trigger(year),
                                              pass_low_pt_electron_trigger(year),
                                              pass_high_pt_electron_trigger(year),
                                              pass_double_electron_trigger(year),
                                              pass_high_pt_tau_trigger(year), //
                                              pass_double_tau_trigger(year),  //
                                              pass_photon_trigger(year),
                                              pass_double_photon_trigger(year));
        if (not(is_good_trigger))
        {
            continue;
        }

        // build Nominal objects
        auto nominal_muons = ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                         unwrap(Muon_eta),            //
                                                         unwrap(Muon_phi),            //
                                                         unwrap(Muon_tightId),        //
                                                         unwrap(Muon_highPtId),       //
                                                         unwrap(Muon_pfRelIso04_all), //
                                                         unwrap(Muon_tkRelIso),       //
                                                         unwrap(Muon_tunepRelPt),     //
                                                         unwrap(Muon_highPurity),     //
                                                         unwrap(Muon_genPartIdx),     //
                                                         muon_sf_reco,                //
                                                         muon_sf_id_low_pt,           //
                                                         muon_sf_id_high_pt,          //
                                                         muon_sf_iso_low_pt,          //
                                                         muon_sf_iso_high_pt,         //
                                                         is_data,                     //
                                                         year,                        //
                                                         Shifts::Variations::Nominal);

        auto nominal_electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),               //
                                                                 unwrap(Electron_eta),              //
                                                                 unwrap(Electron_phi),              //
                                                                 unwrap(Electron_deltaEtaSC),       //
                                                                 unwrap(Electron_cutBased),         //
                                                                 unwrap(Electron_cutBased_HEEP),    //
                                                                 unwrap(Electron_scEtOverPt, true), //
                                                                 unwrap(Electron_dEscaleUp),        //
                                                                 unwrap(Electron_dEscaleDown),      //
                                                                 unwrap(Electron_dEsigmaUp),        //
                                                                 unwrap(Electron_dEsigmaDown),      //
                                                                 unwrap(Electron_genPartIdx),       //
                                                                 electron_sf,                       //
                                                                 is_data,                           //
                                                                 year,                              //
                                                                 Shifts::Variations::Nominal);

        auto nominal_taus = ObjectFactories::make_taus(unwrap(Tau_pt),   //
                                                       unwrap(Tau_eta),  //
                                                       unwrap(Tau_phi),  //
                                                       unwrap(Tau_dz),   //
                                                       unwrap(Tau_mass), //
                                                       unwrap(Tau_genPartFlav),
                                                       unwrap(Tau_genPartIdx),             //
                                                       unwrap(Tau_decayMode),              //
                                                       unwrap(Tau_idDeepTau2017v2p1VSe),   //
                                                       unwrap(Tau_idDeepTau2017v2p1VSmu),  //
                                                       unwrap(Tau_idDeepTau2017v2p1VSjet), //
                                                       deep_tau_2017_v2_p1_vs_e,           //
                                                       deep_tau_2017_v2_p1_vs_mu,          //
                                                       deep_tau_2017_v2_p1_vs_jet,         //
                                                       tau_energy_scale,                   //
                                                       is_data,                            //
                                                       year,                               //
                                                       Shifts::Variations::Nominal);

        auto nominal_photons = ObjectFactories::make_photons(unwrap(Photon_pt),          //
                                                             unwrap(Photon_eta),         //
                                                             unwrap(Photon_phi),         //
                                                             unwrap(Photon_isScEtaEB),   //
                                                             unwrap(Photon_isScEtaEE),   //
                                                             unwrap(Photon_cutBased),    //
                                                             unwrap(Photon_pixelSeed),   //
                                                             unwrap(Photon_dEscaleUp),   //
                                                             unwrap(Photon_dEscaleDown), //
                                                             unwrap(Photon_dEsigmaUp),   //
                                                             unwrap(Photon_dEsigmaDown), //
                                                             unwrap(Photon_genPartIdx),  //
                                                             photon_sf,                  //
                                                             pixel_veto_sf,              //
                                                             is_data,                    //
                                                             year,                       //
                                                             Shifts::Variations::Nominal);

        auto [nominal_jets, nominal_bjets] = ObjectFactories::make_jets(unwrap(Jet_pt),                 //
                                                                        unwrap(Jet_eta),                //
                                                                        unwrap(Jet_phi),                //
                                                                        unwrap(Jet_mass),               //
                                                                        unwrap(Jet_jetId),              //
                                                                        unwrap(Jet_btagDeepFlavB),      //
                                                                        unwrap(Jet_rawFactor),          //
                                                                        unwrap(Jet_area),               //
                                                                        unwrap(Jet_genJetIdx),          //
                                                                        unwrap(fixedGridRhoFastjetAll), //
                                                                        jet_corrections,                //
                                                                        // btag_sf_Corrector,                    //
                                                                        NanoAODGenInfo::GenJets(unwrap(GenJet_pt),   //
                                                                                                unwrap(GenJet_eta),  //
                                                                                                unwrap(GenJet_phi)), //
                                                                        is_data,                                     //
                                                                        year,                                        //
                                                                        Shifts::Variations::Nominal);

        // check for trigger matching
        // is_good_trigger is garantied to be filled by the "if" statement above
        const auto trigger_matches = make_trigger_matches(
            *is_good_trigger, nominal_muons, nominal_electrons, nominal_taus, nominal_photons, get_runyear(year));

        bool has_trigger_match = false;
        for (auto &&[trigger_path, trigger_match] : trigger_matches)
        {
            if (trigger_match)
            {
                has_trigger_match = true;
                break;
            }
        }

        if (not(has_trigger_match))
        {
            continue;
        }

        for (auto &&diff_shift : shifts.get_differential_shifts())
        {
            auto muons = [&]() -> MUSiCObjects
            {
                if (starts_with(Shifts::variation_to_string(diff_shift), "Muon"))
                {
                    return ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                       unwrap(Muon_eta),            //
                                                       unwrap(Muon_phi),            //
                                                       unwrap(Muon_tightId),        //
                                                       unwrap(Muon_highPtId),       //
                                                       unwrap(Muon_pfRelIso04_all), //
                                                       unwrap(Muon_tkRelIso),       //
                                                       unwrap(Muon_tunepRelPt),     //
                                                       unwrap(Muon_highPurity),     //
                                                       unwrap(Muon_genPartIdx),     //
                                                       muon_sf_reco,                //
                                                       muon_sf_id_low_pt,           //
                                                       muon_sf_id_high_pt,          //
                                                       muon_sf_iso_low_pt,          //
                                                       muon_sf_iso_high_pt,         //
                                                       is_data,                     //
                                                       year,                        //
                                                       diff_shift);
                }

                return nominal_muons;
            }();

            auto electrons = [&]() -> MUSiCObjects
            {
                if (starts_with(Shifts::variation_to_string(diff_shift), "Electron"))
                {
                    return ObjectFactories::make_electrons(unwrap(Electron_pt),               //
                                                           unwrap(Electron_eta),              //
                                                           unwrap(Electron_phi),              //
                                                           unwrap(Electron_deltaEtaSC),       //
                                                           unwrap(Electron_cutBased),         //
                                                           unwrap(Electron_cutBased_HEEP),    //
                                                           unwrap(Electron_scEtOverPt, true), //
                                                           unwrap(Electron_dEscaleUp),        //
                                                           unwrap(Electron_dEscaleDown),      //
                                                           unwrap(Electron_dEsigmaUp),        //
                                                           unwrap(Electron_dEsigmaDown),      //
                                                           unwrap(Electron_genPartIdx),       //
                                                           electron_sf,                       //
                                                           is_data,                           //
                                                           year,                              //
                                                           diff_shift);
                }

                return nominal_electrons;
            }();

            auto taus = [&]() -> MUSiCObjects
            {
                if (starts_with(Shifts::variation_to_string(diff_shift), "Tau"))
                {
                    return ObjectFactories::make_taus(unwrap(Tau_pt),   //
                                                      unwrap(Tau_eta),  //
                                                      unwrap(Tau_phi),  //
                                                      unwrap(Tau_dz),   //
                                                      unwrap(Tau_mass), //
                                                      unwrap(Tau_genPartFlav),
                                                      unwrap(Tau_genPartIdx),             //
                                                      unwrap(Tau_decayMode),              //
                                                      unwrap(Tau_idDeepTau2017v2p1VSe),   //
                                                      unwrap(Tau_idDeepTau2017v2p1VSmu),  //
                                                      unwrap(Tau_idDeepTau2017v2p1VSjet), //
                                                      deep_tau_2017_v2_p1_vs_e,           //
                                                      deep_tau_2017_v2_p1_vs_mu,          //
                                                      deep_tau_2017_v2_p1_vs_jet,         //
                                                      tau_energy_scale,                   //
                                                      is_data,                            //
                                                      year,                               //
                                                      diff_shift);
                }

                return nominal_taus;
            }();

            auto photons = [&]() -> MUSiCObjects
            {
                if (starts_with(Shifts::variation_to_string(diff_shift), "Photon"))
                {
                    return ObjectFactories::make_photons(unwrap(Photon_pt),          //
                                                         unwrap(Photon_eta),         //
                                                         unwrap(Photon_phi),         //
                                                         unwrap(Photon_isScEtaEB),   //
                                                         unwrap(Photon_isScEtaEE),   //
                                                         unwrap(Photon_cutBased),    //
                                                         unwrap(Photon_pixelSeed),   //
                                                         unwrap(Photon_dEscaleUp),   //
                                                         unwrap(Photon_dEscaleDown), //
                                                         unwrap(Photon_dEsigmaUp),   //
                                                         unwrap(Photon_dEsigmaDown), //
                                                         unwrap(Photon_genPartIdx),  //
                                                         photon_sf,                  //
                                                         pixel_veto_sf,              //
                                                         is_data,                    //
                                                         year,                       //
                                                         diff_shift);
                }

                return nominal_photons;
            }();

            auto [jets, bjets] = [&]() -> std::pair<MUSiCObjects, MUSiCObjects>
            {
                if (starts_with(Shifts::variation_to_string(diff_shift), "Jet"))
                {
                    return ObjectFactories::make_jets(unwrap(Jet_pt),                 //
                                                      unwrap(Jet_eta),                //
                                                      unwrap(Jet_phi),                //
                                                      unwrap(Jet_mass),               //
                                                      unwrap(Jet_jetId),              //
                                                      unwrap(Jet_btagDeepFlavB),      //
                                                      unwrap(Jet_rawFactor),          //
                                                      unwrap(Jet_area),               //
                                                      unwrap(Jet_genJetIdx),          //
                                                      unwrap(fixedGridRhoFastjetAll), //
                                                      jet_corrections,                //
                                                      // btag_sf_Corrector,                    //
                                                      NanoAODGenInfo::GenJets(unwrap(GenJet_pt),   //
                                                                              unwrap(GenJet_eta),  //
                                                                              unwrap(GenJet_phi)), //
                                                      is_data,                                     //
                                                      year,                                        //
                                                      diff_shift);
                }

                return {nominal_jets, nominal_bjets};
            }();

            auto met = ObjectFactories::make_met(              //
                unwrap(RawMET_pt),                             //
                unwrap(RawMET_phi),                            //
                unwrap_or(MET_MetUnclustEnUpDeltaX, 0., true), //
                unwrap_or(MET_MetUnclustEnUpDeltaY, 0., true), //
                muons.get_delta_met_x(),                       //
                muons.get_delta_met_y(),                       //
                electrons.get_delta_met_x(),                   //
                electrons.get_delta_met_y(),                   //
                taus.get_delta_met_x(),                        //
                taus.get_delta_met_y(),                        //
                photons.get_delta_met_x(),                     //
                photons.get_delta_met_y(),                     //
                jets.get_delta_met_x(),                        //
                jets.get_delta_met_y(),                        //
                bjets.get_delta_met_x(),                       //
                bjets.get_delta_met_y(),                       //
                is_data,                                       //
                year,                                          //
                diff_shift);

            // clear objects
            auto electrons_idxs = electrons.indexes();
            auto electrons_clear_mask = electrons.clear_mask(muons, 0.4);
            if (not(std::equal(electrons_clear_mask.cbegin(), electrons_clear_mask.cend(), electrons_idxs.cbegin())))
            {
                electrons = electrons.take_as_copy(electrons_clear_mask);
            }

            auto taus_idxs = taus.indexes();
            auto taus_clear_mask = taus.clear_mask(electrons, 0.4);
            if (not(std::equal(taus_clear_mask.cbegin(), taus_clear_mask.cend(), taus_idxs.cbegin())))
            {
                taus = taus.take_as_copy(taus_clear_mask);
            }
            taus_clear_mask = taus.clear_mask(muons, 0.4);
            if (not(std::equal(taus_clear_mask.cbegin(), taus_clear_mask.cend(), taus_idxs.cbegin())))
            {
                taus = taus.take_as_copy(taus_clear_mask);
            }

            auto photons_idxs = photons.indexes();
            auto photons_clear_mask = photons.clear_mask(taus, 0.4);
            if (not(std::equal(photons_clear_mask.cbegin(), photons_clear_mask.cend(), photons_idxs.cbegin())))
            {
                photons = photons.take_as_copy(photons_clear_mask);
            }
            photons_clear_mask = photons.clear_mask(electrons, 0.4);
            if (not(std::equal(photons_clear_mask.cbegin(), photons_clear_mask.cend(), photons_idxs.cbegin())))
            {
                photons = photons.take_as_copy(photons_clear_mask);
            }
            photons_clear_mask = photons.clear_mask(muons, 0.4);
            if (not(std::equal(photons_clear_mask.cbegin(), photons_clear_mask.cend(), photons_idxs.cbegin())))
            {
                photons = photons.take_as_copy(photons_clear_mask);
            }

            auto jets_idxs = jets.indexes();
            auto bjets_idxs = bjets.indexes();
            auto jets_clear_mask = jets.clear_mask(photons, 0.5);
            if (not(std::equal(jets_clear_mask.cbegin(), jets_clear_mask.cend(), jets_idxs.cbegin())))
            {
                jets = jets.take_as_copy(jets_clear_mask);
            }
            auto bjets_clear_mask = bjets.clear_mask(photons, 0.5);
            if (not(std::equal(bjets_clear_mask.cbegin(), bjets_clear_mask.cend(), bjets_idxs.cbegin())))
            {
                bjets = bjets.take_as_copy(bjets_clear_mask);
            }
            jets_clear_mask = jets.clear_mask(taus, 0.5);
            if (not(std::equal(jets_clear_mask.cbegin(), jets_clear_mask.cend(), jets_idxs.cbegin())))
            {
                jets = jets.take_as_copy(jets_clear_mask);
            }
            bjets_clear_mask = bjets.clear_mask(taus, 0.5);
            if (not(std::equal(bjets_clear_mask.cbegin(), bjets_clear_mask.cend(), bjets_idxs.cbegin())))
            {
                bjets = bjets.take_as_copy(bjets_clear_mask);
            }
            jets_clear_mask = jets.clear_mask(electrons, 0.5);
            if (not(std::equal(jets_clear_mask.cbegin(), jets_clear_mask.cend(), jets_idxs.cbegin())))
            {
                jets = jets.take_as_copy(jets_clear_mask);
            }
            bjets_clear_mask = bjets.clear_mask(electrons, 0.5);
            if (not(std::equal(bjets_clear_mask.cbegin(), bjets_clear_mask.cend(), bjets_idxs.cbegin())))
            {
                bjets = bjets.take_as_copy(bjets_clear_mask);
            }
            jets_clear_mask = jets.clear_mask(muons, 0.5);
            if (not(std::equal(jets_clear_mask.cbegin(), jets_clear_mask.cend(), jets_idxs.cbegin())))
            {
                jets = jets.take_as_copy(jets_clear_mask);
            }
            bjets_clear_mask = bjets.clear_mask(muons, 0.5);
            if (not(std::equal(bjets_clear_mask.cbegin(), bjets_clear_mask.cend(), bjets_idxs.cbegin())))
            {
                bjets = bjets.take_as_copy(bjets_clear_mask);
            }

            auto get_effective_weight = [&](Shifts::Variations shift,
                                            std::size_t num_muon,
                                            std::size_t num_electron,
                                            std::size_t num_tau,
                                            std::size_t num_photon,
                                            std::size_t num_bjet,
                                            std::size_t num_jet,
                                            std::size_t num_met) -> double
            {
                double weight = 1.;
                if (not(is_data))
                {
                    // get trigger SF
                    auto trigger_sf = 1.;

                    auto pu_weight = pu_corrector->evaluate({unwrap(Pileup_nTrueInt), Shifts::get_pu_variation(shift)});

                    auto prefiring_weight = Shifts::get_prefiring_weight(unwrap_or(L1PreFiringWeight_Nom, 1.), //
                                                                         unwrap_or(L1PreFiringWeight_Up, 1.),  //
                                                                         unwrap_or(L1PreFiringWeight_Dn, 1.),
                                                                         shift);

                    auto pdf_as_weight =
                        get_pdf_alpha_s_weights(shift,
                                                lha_indexes,
                                                default_pdf_sets,           //
                                                unwrap(LHEPdfWeight),       //
                                                unwrap(Generator_scalePDF), //
                                                unwrap(Generator_x1),       //
                                                unwrap(Generator_x2),       //
                                                unwrap(Generator_id1),      //
                                                unwrap(Generator_id2),      //
                                                this_sample_pdf
                                                // unwrap_or(LHEWeight_originalXWGTUP,
                                                //                                                                1.f)
                        );
                    double mc_weight = [is_data, &genWeight, &LHEWeight_originalXWGTUP, &event_weights]() -> double
                    {
                        if (not(is_data))
                        {
                            if (event_weights.should_use_LHEWeight)
                            {
                                return unwrap(LHEWeight_originalXWGTUP);
                            }
                            return unwrap(genWeight);
                        }
                        return 1.;
                    }();

                    weight = mc_weight * pu_weight * prefiring_weight * trigger_sf / event_weights.sum_weights *
                             x_section * luminosity * filter_eff * k_factor * pdf_as_weight *
                             Shifts::get_reco_scale_factor(shift,
                                                           {num_muon, muons},
                                                           {num_electron, electrons},
                                                           {num_tau, taus},
                                                           {num_photon, photons},
                                                           {num_bjet, bjets},
                                                           {num_jet, jets},
                                                           {num_met, met}) *
                             Shifts::get_fakes_variation_weight(shift,
                                                                {num_muon, muons},
                                                                {num_electron, electrons},
                                                                {num_tau, taus},
                                                                {num_photon, photons},
                                                                {num_bjet, bjets},
                                                                {num_jet, jets}) *
                             Shifts::get_qcd_scale_weight(shift, unwrap(LHEScaleWeight));

                    // Check for NaNs
                    if (std::isnan(weight) or std::isinf(weight))
                    {
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr,
                                   "ERROR: NaN or INF weight found when "
                                   "processing shift: {}!\n",
                                   Shifts::variation_to_string(shift));
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        fmt::print(stderr, "##########################\n");
                        std::exit(EXIT_FAILURE);
                    }
                }

                return weight;
            };

            // Here goes the real analysis...
            auto temp_event_classes = TempEC::make_temp_event_classes(
                muons.size(), electrons.size(), taus.size(), photons.size(), bjets.size(), jets.size(), met.size());
            for (auto &temp_ec : temp_event_classes)
            {

                temp_ec.push(muons.p4, temp_ec.max_muon_idx);
                temp_ec.push(electrons.p4, temp_ec.max_electron_idx);
                temp_ec.push(taus.p4, temp_ec.max_tau_idx);
                temp_ec.push(photons.p4, temp_ec.max_photon_idx);
                temp_ec.push(bjets.p4, temp_ec.max_bjet_idx);
                temp_ec.push(jets.p4, temp_ec.max_jet_idx);
                temp_ec.push(met.p4, temp_ec.max_met_idx);

                auto classes_names = temp_ec.make_event_class_name();
                for (auto &&const_shift : shifts.get_constant_shifts(diff_shift))
                {
                    if (not(const_shift == Shifts::Variations::Nominal or diff_shift == Shifts::Variations::Nominal))
                    {
                        continue;
                    }

                    auto shift = Shifts::resolve_shifts(const_shift, diff_shift);

                    // fill event classes
                    for (auto &&class_name : {classes_names.exclusive_class_name,
                                              classes_names.inclusive_class_name,
                                              classes_names.jet_inclusive_class_name})
                    {
                        if (not(class_name))
                        {
                            continue;
                        }

                        // create event class, if does not exists
                        if (not(event_classes.has_ec(*class_name)))
                        {
                            event_classes.push(*class_name);
                        }

                        // fill class
                        event_classes.unsafe_ec(*class_name)
                            .push(temp_ec.get_sum_pt(),
                                  temp_ec.get_mass(),
                                  temp_ec.get_met(),
                                  get_effective_weight(shift,
                                                       temp_ec.num_muons,
                                                       temp_ec.num_electrons,
                                                       temp_ec.num_taus,
                                                       temp_ec.num_photons,
                                                       temp_ec.num_bjets,
                                                       temp_ec.num_jets,
                                                       temp_ec.num_met),
                                  shift);
                    }
                }
            }

            //////////////////////////////////////////////
            /// Validation analysis
            for (auto &&const_shift : shifts.get_constant_shifts(diff_shift))
            {
                if (not(const_shift == Shifts::Variations::Nominal or diff_shift == Shifts::Variations::Nominal))
                {
                    continue;
                }

                auto shift = Shifts::resolve_shifts(const_shift, diff_shift);

                validation_container.z_to_muon_muon_x.fill(
                    muons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift, std::min(static_cast<int>(muons.size()), 2), 0, 0, 0, 0, 0, met.size()),
                    shift);
                validation_container.z_to_muon_muon_x_z_mass.fill(
                    muons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(
                        shift, 0, std::min(static_cast<int>(electrons.size()), 2), 0, 0, 0, 0, met.size()),
                    shift);

                validation_container.z_to_electron_electron_x.fill(
                    electrons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(
                        shift, 0, std::min(static_cast<int>(electrons.size()), 2), 0, 0, 0, 0, met.size()),
                    shift);
                validation_container.z_to_electron_electron_x_z_mass.fill(
                    electrons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift, std::min(static_cast<int>(muons.size()), 2), 0, 0, 0, 0, 0, met.size()),
                    shift);

                validation_container.z_to_tau_tau_x.fill(
                    taus,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift, 0, 0, std::min(static_cast<int>(taus.size()), 2), 0, 0, 0, met.size()),
                    shift);
                validation_container.z_to_tau_tau_x_z_mass.fill(
                    taus,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift, 0, 0, std::min(static_cast<int>(taus.size()), 2), 0, 0, 0, met.size()),
                    shift);
                validation_container.w_to_muon_nutrino_x.fill(
                    muons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift, std::min(static_cast<int>(muons.size()), 1), 0, 0, 0, 0, 0, met.size()),
                    shift);
                validation_container.w_to_electron_nutrino_x.fill(
                    electrons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(
                        shift, 0, std::min(static_cast<int>(electrons.size()), 1), 0, 0, 0, 0, met.size()),
                    shift);
                validation_container.w_to_tau_nutrino_x.fill(
                    taus,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift, 0, 0, std::min(static_cast<int>(taus.size()), 1), 0, 0, 0, met.size()),
                    shift);
                validation_container.ttbar_to_1muon_2bjet_2jet_met.fill(
                    muons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift,
                                         std::min(static_cast<int>(muons.size()), 1),
                                         0,
                                         0,
                                         0,
                                         std::min(static_cast<int>(bjets.size()), 2),
                                         std::min(static_cast<int>(jets.size()), 2),
                                         met.size()),
                    shift);
                validation_container.ttbar_to_1electron_2bjet_2jet_met.fill(
                    electrons,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift,
                                         0,
                                         std::min(static_cast<int>(electrons.size()), 1),
                                         0,
                                         0,
                                         std::min(static_cast<int>(bjets.size()), 2),
                                         std::min(static_cast<int>(jets.size()), 2),
                                         met.size()),
                    shift);
                validation_container.ttbar_to_1tau_2bjet_2jet_met.fill(
                    taus,
                    bjets,
                    jets,
                    met,
                    get_effective_weight(shift,
                                         0,
                                         0,
                                         std::min(static_cast<int>(taus.size()), 1),
                                         0,
                                         std::min(static_cast<int>(bjets.size()), 2),
                                         std::min(static_cast<int>(jets.size()), 2),
                                         met.size()),
                    shift);
                validation_container.gamma_plus_jets.fill(
                    photons,
                    jets,
                    get_effective_weight(shift,
                                         0,
                                         0,
                                         0,
                                         std::min(static_cast<int>(photons.size()), 1),
                                         0,
                                         std::min(static_cast<int>(jets.size()), 1),
                                         met.size()),
                    shift);
            }
            //////////////////////////////////////////////
        }
    }

    fmt::print("\n[MUSiC Classification] Done ...\n");
    fmt::print("\n\nProcessed {} events ...\n", global_event_index);
    PrintProcessInfo();
}
