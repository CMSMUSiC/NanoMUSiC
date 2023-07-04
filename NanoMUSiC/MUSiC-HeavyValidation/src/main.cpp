#include "HeavyValidation.hpp"

#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "Outputs.hpp"
#include "ROOT/RVec.hxx"
#include "RtypesCore.h"
#include "TFile.h"
#include "TH1.h"
#include "TROOT.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "fmt/core.h"
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>

auto starts_with(const std::string &str, const std::string &prefix) -> bool
{
    return str.compare(0, prefix.length(), prefix) == 0;
}

inline auto trigger_filter(const std::string &process,
                           bool is_data,
                           bool pass_low_pt_muon_trigger,
                           bool pass_high_pt_muon_trigger,
                           bool pass_low_pt_electron_trigger,
                           bool pass_high_pt_electron_trigger) -> std::optional<std::map<std::string, bool>>
{
    std::optional<std::map<std::string, bool>> trigger_filter_res = std::nullopt;

    // Data
    if (is_data)
    {
        // Electron/Photon/EGamma dataset
        if (                                                 //
            process.find("EGamma") != std::string::npos      //
            or process.find("Electron") != std::string::npos //
            or process.find("Photon") != std::string::npos   //
        )
        {
            if (not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) and
                (pass_low_pt_electron_trigger or pass_high_pt_electron_trigger))
            {
                trigger_filter_res = {{"pass_low_pt_muon_trigger", false},
                                      {"pass_high_pt_muon_trigger", false},
                                      {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},
                                      {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}};
            }
        }

        // Muon dataset
        else if ((pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) and
                 not(pass_low_pt_electron_trigger or pass_high_pt_electron_trigger))
        {
            trigger_filter_res = {{"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},
                                  {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},
                                  {"pass_low_pt_electron_trigger", false},
                                  {"pass_high_pt_electron_trigger", false}};
        }

        return trigger_filter_res;
    }

    // MC
    if (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_low_pt_electron_trigger or
        pass_high_pt_electron_trigger)
    {
        trigger_filter_res = {{"pass_low_pt_muon_trigger", pass_low_pt_muon_trigger},
                              {"pass_high_pt_muon_trigger", pass_high_pt_muon_trigger},
                              {"pass_low_pt_electron_trigger", pass_low_pt_electron_trigger},
                              {"pass_high_pt_electron_trigger", pass_high_pt_electron_trigger}};
    }

    return trigger_filter_res;
};

inline auto jets_trigger_filter(bool pass_jet_ht_trigger, bool pass_jet_pt_trigger) -> bool
{
    return pass_jet_ht_trigger or pass_jet_pt_trigger;
};

auto main(int argc, char *argv[]) -> int
{

    // set SumW2 as default
    TH1::SetDefaultSumw2(true);

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    const std::string process = cmdl({"-p", "--process"}).str();
    const std::string year = cmdl({"-y", "--year"}).str();
    const bool is_data = cmdl[{"-d", "--is_data"}];
    const std::string output_path = cmdl({"-o", "--output"}).str();
    const std::string x_section_str = cmdl({"-x", "--xsection"}).str();
    const std::string filter_eff_str = cmdl({"-f", "--filter_eff"}).str();
    const std::string k_factor_str = cmdl({"-k", "--k_factor"}).str();
    const std::string luminosity_str = cmdl({"-l", "--luminosity"}).str();
    const std::string xs_order = cmdl({"-po", "--xs_order"}).str();
    const std::string process_group = cmdl({"-pg", "--process_group"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();
    const std::string shift = cmdl({"-s", "--shift"}).str();
    const bool debug = cmdl[{"--debug"}];

    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or x_section_str == "" or
        filter_eff_str == "" or k_factor_str == "" or luminosity_str == "" or xs_order == "" or process_group == "")
    {
        fmt::print("Usage: heavy_validation [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: cross-section.\n");
        fmt::print("          -f|--filter_eff: Generator level filter efficiency.\n");
        fmt::print("          -k|--k_factor: K-Factor.\n");
        fmt::print("          -l|--luminosity: Int. luminosity.\n");
        fmt::print("          -po|--xs_order: Process order.\n");
        fmt::print("          -pg|--process_group: Process group.\n");
        fmt::print("          -i|--input: Path to a txt with input files (one per line).\n");
        fmt::print("          --debug: Run in debug mode..\n");

        exit(-1);
    }

    const double x_section = std::stod(x_section_str);
    const double filter_eff = std::stod(filter_eff_str);
    const double k_factor = std::stod(k_factor_str);
    const double luminosity = std::stod(luminosity_str);

    // create tree reader and add values and arrays
    // TChain input_chain("nano_music");
    // for (auto &&file : load_input_files(input_file))
    // {
    //     input_chain.Add(file.c_str());
    // }

    auto f = std::unique_ptr<TFile>(TFile::Open(input_file.c_str()));
    auto input_chain = std::unique_ptr<TTree>(f->Get<TTree>("nano_music"));

    auto tree_reader = TTreeReader(input_chain.get());

    ADD_VALUE_READER(pass_low_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_low_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_jet_ht_trigger, bool);
    ADD_VALUE_READER(pass_jet_pt_trigger, bool);

    ADD_VALUE_READER(mc_weight, float);
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
    ADD_VALUE_READER(LHEWeight_originalXWGTUP, float);

    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);
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

    ADD_VALUE_READER(MET_pt, float);
    ADD_VALUE_READER(MET_phi, float);

    const auto shifts = Shifts(is_data);

    // corrections
    auto correctionlib_utils = CorrectionLibUtils();
    auto jet_corrections = JetCorrector(get_runyear(year), get_era_from_process_name(process, is_data), is_data);
    auto pu_corrector = correctionlib_utils.make_correctionlib_ref("PU", year);
    auto muon_sf_reco = correctionlib_utils.make_correctionlib_ref("MuonReco", year);
    auto muon_sf_id_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIdLowPt", year);
    auto muon_sf_id_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIdHighPt", year);
    auto muon_sf_iso_low_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoLowPt", year);
    auto muon_sf_iso_high_pt = correctionlib_utils.make_correctionlib_ref("MuonIsoHighPt", year);
    auto electron_sf = correctionlib_utils.make_correctionlib_ref("ElectronSF", year);
    auto photon_sf = correctionlib_utils.make_correctionlib_ref("PhotonSF", year);
    auto pixel_veto_sf = correctionlib_utils.make_correctionlib_ref("PixelVetoSF", year);

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

    // check for the available pdf weights in the tree
    std::optional<std::pair<unsigned int, unsigned int>> lha_indexes =
        PDFAlphaSWeights::get_pdf_ids(tree_reader.GetTree());

    /////////////////////////////////////////////
    /////////////////////////////////////////////
    // [ END ] LHAPDF
    /////////////////////////////////////////////
    /////////////////////////////////////////////

    const std::map<std::string, int> z_to_mu_mu_x_count_map = {{"Ele", 0},
                                                               {"EleEE", 0},
                                                               {"EleEB", 0},
                                                               {"Muon", 2},
                                                               {"Gamma", 0},
                                                               {"GammaEB", 0},
                                                               {"GammaEE", 0},
                                                               {"Tau", 0},
                                                               {"Jet", 0},
                                                               {"bJet", 0},
                                                               {"MET", 0}};

    const std::map<std::string, int> z_to_ele_ele_x_count_map = {{"Ele", 2},
                                                                 {"EleEE", 0},
                                                                 {"EleEB", 0},
                                                                 {"Muon", 0},
                                                                 {"Gamma", 0},
                                                                 {"GammaEB", 0},
                                                                 {"GammaEE", 0},
                                                                 {"Tau", 0},
                                                                 {"Jet", 0},
                                                                 {"bJet", 0},
                                                                 {"MET", 0}};

    const std::map<std::string, int> dijets_count_map = {{"Ele", 0},
                                                         {"EleEE", 0},
                                                         {"EleEB", 0},
                                                         {"Muon", 0},
                                                         {"Gamma", 0},
                                                         {"GammaEB", 0},
                                                         {"GammaEE", 0},
                                                         {"Tau", 0},
                                                         {"Jet", 2},
                                                         {"bJet", 0},
                                                         {"MET", 0}};

    // build validation factories
    // map each shift to one analysis factory
    std::unordered_map<std::string, ZToLepLepX> z_to_mu_mu_x;
    std::unordered_map<std::string, ZToLepLepX> z_to_mu_mu_x_Z_mass;
    std::unordered_map<std::string, ZToLepLepX> z_to_ele_ele_x;
    std::unordered_map<std::string, ZToLepLepX> z_to_ele_ele_x_Z_mass;

    for (auto &&shift : shifts.get_constant_shifts())
    {
        z_to_mu_mu_x.insert(
            {shift,
             ZToLepLepX("z_to_mu_mu_x",
                        get_output_file_path(
                            "z_to_mu_mu_x", output_path, process, year, process_group, xs_order, is_data, shift),
                        z_to_mu_mu_x_count_map,
                        false,
                        shift,
                        process,
                        year,
                        process_group,
                        xs_order)});

        z_to_mu_mu_x_Z_mass.insert(
            {shift,
             ZToLepLepX("z_to_mu_mu_x_Z_mass",
                        get_output_file_path(
                            "z_to_mu_mu_x_Z_mass", output_path, process, year, process_group, xs_order, is_data, shift),
                        z_to_mu_mu_x_count_map,
                        true,
                        shift,
                        process,
                        year,
                        process_group,
                        xs_order)});

        z_to_ele_ele_x.insert(
            {shift,
             ZToLepLepX("z_to_ele_ele_x",
                        get_output_file_path(
                            "z_to_ele_ele_x", output_path, process, year, process_group, xs_order, is_data, shift),
                        z_to_ele_ele_x_count_map,
                        false,
                        shift,
                        process,
                        year,
                        process_group,
                        xs_order)});

        z_to_ele_ele_x_Z_mass.insert(
            {shift,
             ZToLepLepX(
                 "z_to_ele_ele_x_Z_mass",
                 get_output_file_path(
                     "z_to_ele_ele_x_Z_mass", output_path, process, year, process_group, xs_order, is_data, shift),
                 z_to_ele_ele_x_count_map,
                 true,
                 shift,
                 process,
                 year,
                 process_group,
                 xs_order)});
    }

    for (auto &&shift : shifts.get_differential_shifts())
    {
        if (shift != "Nominal")
        {
            z_to_mu_mu_x.insert(
                {shift,
                 ZToLepLepX("z_to_mu_mu_x",
                            get_output_file_path(
                                "z_to_mu_mu_x", output_path, process, year, process_group, xs_order, is_data, shift),
                            z_to_mu_mu_x_count_map,
                            false,
                            shift,
                            process,
                            year,
                            process_group,
                            xs_order)});

            z_to_mu_mu_x_Z_mass.insert(
                {shift,
                 ZToLepLepX(
                     "z_to_mu_mu_x_Z_mass",
                     get_output_file_path(
                         "z_to_mu_mu_x_Z_mass", output_path, process, year, process_group, xs_order, is_data, shift),
                     z_to_mu_mu_x_count_map,
                     true,
                     shift,
                     process,
                     year,
                     process_group,
                     xs_order)});

            z_to_ele_ele_x.insert(
                {shift,
                 ZToLepLepX("z_to_ele_ele_x",
                            get_output_file_path(
                                "z_to_ele_ele_x", output_path, process, year, process_group, xs_order, is_data, shift),
                            z_to_ele_ele_x_count_map,
                            false,
                            shift,
                            process,
                            year,
                            process_group,
                            xs_order)});

            z_to_ele_ele_x_Z_mass.insert(
                {shift,
                 ZToLepLepX(
                     "z_to_ele_ele_x_Z_mass",
                     get_output_file_path(
                         "z_to_ele_ele_x_Z_mass", output_path, process, year, process_group, xs_order, is_data, shift),
                     z_to_ele_ele_x_count_map,
                     true,
                     shift,
                     process,
                     year,
                     process_group,
                     xs_order)});
        }
    }

    // build Dijets
    // auto dijets = Dijets(fmt::format("{}/dijets_{}_{}.root", output_path, process, year), dijets_count_map);

    auto cutflow_file = std::unique_ptr<TFile>(
        TFile::Open(fmt::format("{}/../../cutflow_{}_{}.root", output_path, process, year).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    // const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") +
    // 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    //  launch event loop for Data or MC
    for (auto &&event : tree_reader)
    {
        // check for chain readout quaility
        // REFERENCE: https://root.cern.ch/doc/v608/classTTreeReader.html#a568e43c7d7d8b1f511bbbeb92c9094a8
        auto reader_status = tree_reader.GetEntryStatus();
        if (reader_status == TTreeReader::EEntryStatus::kEntryChainFileError or
            reader_status == TTreeReader::EEntryStatus::kEntryNotLoaded or
            reader_status == TTreeReader::EEntryStatus::kEntryNoTree or
            reader_status == TTreeReader::EEntryStatus::kEntryNotFound or
            reader_status == TTreeReader::EEntryStatus::kEntryChainSetupError or
            reader_status == TTreeReader::EEntryStatus::kEntryDictionaryError)
        {
            throw std::runtime_error(fmt::format("ERROR: Could not load TChain entry."));
        }

        // remove the "unused variable" warning during compilation
        static_cast<void>(event);

         if (event > 30000)
         {
             break;
         }

        // Trigger
        //
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //
        // ATTENTION:The function "trigger_filter" should be modified
        // for the JetHT data set!!!!!!
        //
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //
        // pass_low_pt_muon_trigger
        // pass_high_pt_muon_trigger
        // pass_low_pt_electron_trigger
        // pass_high_pt_electron_trigger
        // pass_jet_ht_trigger
        // pass_jet_pt_trigger
        auto is_good_trigger = trigger_filter(process,
                                              is_data,
                                              unwrap(pass_low_pt_muon_trigger),
                                              unwrap(pass_high_pt_muon_trigger),
                                              unwrap(pass_low_pt_electron_trigger),
                                              unwrap(pass_high_pt_electron_trigger));

        // bool is_good_trigger = jets_trigger_filter(unwrap(pass_jet_ht_trigger), //
        //                                            unwrap(pass_jet_pt_trigger));

        if (not(is_good_trigger))
        {
            continue;
        }

        for (auto &&diff_shift : shifts.get_differential_shifts())
        {
            // build good objects
            // muons
            auto muons = ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                     unwrap(Muon_eta),            //
                                                     unwrap(Muon_phi),            //
                                                     unwrap(Muon_tightId),        //
                                                     unwrap(Muon_highPtId),       //
                                                     unwrap(Muon_pfRelIso04_all), //
                                                     unwrap(Muon_tkRelIso),       //
                                                     unwrap(Muon_tunepRelPt),     //
                                                     unwrap(Muon_genPartIdx),     //
                                                     muon_sf_reco,                //
                                                     muon_sf_id_low_pt,           //
                                                     muon_sf_id_high_pt,          //
                                                     muon_sf_iso_low_pt,          //
                                                     muon_sf_iso_high_pt,         //
                                                     is_data,                     //
                                                     year,                        //
                                                     diff_shift);

            auto electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),            //
                                                             unwrap(Electron_eta),           //
                                                             unwrap(Electron_phi),           //
                                                             unwrap(Electron_deltaEtaSC),    //
                                                             unwrap(Electron_cutBased),      //
                                                             unwrap(Electron_cutBased_HEEP), //
                                                             unwrap(Electron_scEtOverPt),    //
                                                             unwrap(Electron_dEscaleUp),     //
                                                             unwrap(Electron_dEscaleDown),   //
                                                             unwrap(Electron_dEsigmaUp),     //
                                                             unwrap(Electron_dEsigmaDown),   //
                                                             unwrap(Electron_genPartIdx),    //
                                                             electron_sf,                    //
                                                             is_data,                        //
                                                             year,                           //
                                                             diff_shift);

            auto photons = ObjectFactories::make_photons(unwrap(Photon_pt),          //
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

            auto [jets, bjets] = ObjectFactories::make_jets(unwrap(Jet_pt),                 //
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
                                                            NanoObjects::GenJets(unwrap(GenJet_pt),   //
                                                                                 unwrap(GenJet_eta),  //
                                                                                 unwrap(GenJet_phi)), //
                                                            is_data,                                  //
                                                            year,                                     //
                                                            diff_shift);

            // clear objects
            electrons.clear(muons, 0.4);
            photons.clear(electrons, 0.4);
            photons.clear(muons, 0.4);
            jets.clear(photons, 0.5);
            bjets.clear(photons, 0.5);
            jets.clear(electrons, 0.5);
            bjets.clear(electrons, 0.5);
            jets.clear(muons, 0.5);
            bjets.clear(muons, 0.5);

            auto met = ObjectFactories::make_met(unwrap(MET_pt),              //
                                                 unwrap(MET_phi),             //
                                                 muons.get_delta_met_x(),     //
                                                 muons.get_delta_met_y(),     //
                                                 electrons.get_delta_met_x(), //
                                                 electrons.get_delta_met_y(), //
                                                 photons.get_delta_met_x(),   //
                                                 photons.get_delta_met_y(),   //
                                                 jets.get_delta_met_x(),      //
                                                 jets.get_delta_met_y(),      //
                                                 bjets.get_delta_met_x(),     //
                                                 bjets.get_delta_met_y(),     //
                                                 is_data,                     //
                                                 year,                        //
                                                 diff_shift);

            // Here goes the real analysis...
            for (auto &&const_shift : shifts.get_constant_shifts(diff_shift))
            {
                if (const_shift == "Nominal")
                {
                    // get effective event weight
                    double weight = 1.;

                    auto shift = Shifts::resolve_shifts(const_shift, diff_shift);

                    if (not(is_data))
                    {
                        auto pu_weight =
                            pu_corrector->evaluate({unwrap(Pileup_nTrueInt), Shifts::get_pu_variation(shift)});

                        auto prefiring_weight = Shifts::get_prefiring_weight( //
                            unwrap(L1PreFiringWeight_Nom, 1.),                //
                            unwrap(L1PreFiringWeight_Up, 1.),                 //
                            unwrap(L1PreFiringWeight_Dn, 1.),                 //
                            shift);

                        auto scaled_luminosity = Shifts::scale_luminosity(luminosity, shift);

                        auto pdf_as_weight = Shifts::get_pdf_alpha_s_weights(shift,
                                                                             lha_indexes,
                                                                             default_pdf_sets,           //
                                                                             unwrap(LHEPdfWeight),       //
                                                                             unwrap(Generator_scalePDF), //
                                                                             unwrap(Generator_x1),       //
                                                                             unwrap(Generator_x2),       //
                                                                             unwrap(Generator_id1),      //
                                                                             unwrap(Generator_id2),      //
                                                                             unwrap(LHEWeight_originalXWGTUP, 1.f));

                        weight = unwrap(mc_weight, 1.)                                          //
                                 * pu_weight                                                    //
                                 * prefiring_weight                                             //
                                 * generator_filter                                             //
                                 / no_cuts                                                      //
                                 / generator_filter                                             //
                                 * x_section * Shifts::get_xsec_order_modifier(shift, xs_order) //
                                 * filter_eff                                                   //
                                 * k_factor                                                     //
                                 * scaled_luminosity                                            //
                                 * pdf_as_weight;
                    }

                    // MuMu + X
                    unsigned int n_muons = 2;
                    if (muons.size() >= n_muons)
                    {
                        auto muon_1 = muons.p4[0];
                        auto muon_2 = muons.p4[1];

                        // wide mass range
                        z_to_mu_mu_x[shift].fill(
                            muon_1,
                            muon_2,
                            bjets.p4,
                            jets.p4,
                            met.p4,
                            weight * Shifts::get_scale_factor(
                                         shift, n_muons, 0, 0, 0, 0, 0, muons, electrons, photons, bjets, jets, met));

                        // Z mass range
                        if (PDG::Z::Mass - 20. < (muon_1 + muon_2).mass() and
                            (muon_1 + muon_2).mass() < PDG::Z::Mass + 20.)
                        {
                            z_to_mu_mu_x_Z_mass[shift].fill(
                                muon_1,
                                muon_2,
                                bjets.p4,
                                jets.p4,
                                met.p4,
                                weight *
                                    Shifts::get_scale_factor(
                                        shift, n_muons, 0, 0, 0, 0, 0, muons, electrons, photons, bjets, jets, met));
                        }
                    }

                    // EleEle + X
                    unsigned int n_electrons = 2;
                    if (electrons.size() >= n_electrons)
                    {
                        auto electron_1 = electrons.p4[0];
                        auto electron_2 = electrons.p4[1];

                        // wide mass range
                        z_to_ele_ele_x[shift].fill(
                            electron_1,
                            electron_2,
                            bjets.p4,
                            jets.p4,
                            met.p4,
                            weight * Shifts::get_scale_factor(
                                         shift, n_muons, 0, 0, 0, 0, 0, muons, electrons, photons, bjets, jets, met));

                        // Z mass range
                        if (PDG::Z::Mass - 20. < (electron_1 + electron_2).mass() and
                            (electron_1 + electron_2).mass() < PDG::Z::Mass + 20.)
                        {
                            z_to_ele_ele_x_Z_mass[shift].fill(
                                electron_1,
                                electron_2,
                                bjets.p4,
                                jets.p4,
                                met.p4,
                                weight *
                                    Shifts::get_scale_factor(
                                        shift, n_muons, 0, 0, 0, 0, 0, muons, electrons, photons, bjets, jets, met));
                        }
                    }

                    // // Dijets
                    // if (jets.size() >= 2)
                    // {
                    //     auto jet_1 = jets.p4[0];
                    //     auto jet_2 = jets.p4[1];

                    //     if ((jet_1.pt() > 600.) and std::fabs(jet_1.eta() - jet_2.eta()) < 1.1)
                    //     {
                    //         dijets.fill(jet_1, jet_2, std::nullopt, weight);
                    //     }
                    // }
                }
            }

            // process monitoring
            if (debug)
            {
                if ((event < 10) or                           //
                    (event < 100 && event % 10 == 0) or       //
                    (event < 1000 && event % 100 == 0) or     //
                    (event < 10000 && event % 1000 == 0) or   //
                    (event < 100000 && event % 10000 == 0) or //
                    (event >= 100000 && event % 100000 == 0)  //
                )
                {
                    fmt::print("\n\nProcessed {} events ...\n", event);
                    PrintProcessInfo();
                }
            }
        }
    }

    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {} - {}) ...\n", output_path, process, year, shift);
    for (auto &&shift : shifts.get_constant_shifts())
    {
        z_to_mu_mu_x[shift].dump_outputs();
        z_to_mu_mu_x_Z_mass[shift].dump_outputs();
        z_to_ele_ele_x[shift].dump_outputs();
        z_to_ele_ele_x_Z_mass[shift].dump_outputs();
        // dijets.dump_outputs();
    }
    for (auto &&shift : shifts.get_differential_shifts())
    {
        if (shift != "Nominal")
        {
            z_to_mu_mu_x[shift].dump_outputs();
            z_to_mu_mu_x_Z_mass[shift].dump_outputs();
            z_to_ele_ele_x[shift].dump_outputs();
            z_to_ele_ele_x_Z_mass[shift].dump_outputs();
            // dijets.dump_outputs();
        }
    }

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}