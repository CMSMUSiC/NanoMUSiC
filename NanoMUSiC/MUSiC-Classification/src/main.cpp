#include "Classification.hpp"

#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "Outputs.hpp"
#include "ROOT/RVec.hxx"
#include "RtypesCore.h"
#include "TFile.h"
#include "TH1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "fmt/core.h"
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

auto main(int argc, char *argv[]) -> int
{
    // silence LHAPDF
    LHAPDF::setVerbosity(0);

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
    const std::string filter_eff_str = cmdl({"-x", "--filter_eff"}).str();
    const std::string k_factor_str = cmdl({"-x", "--k_factor"}).str();
    const std::string luminosity_str = cmdl({"-x", "--luminosity"}).str();
    const std::string process_order = cmdl({"-x", "--process_order"}).str();
    const std::string process_group = cmdl({"-x", "--process_group"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();

    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or x_section_str == "" or
        filter_eff_str == "" or k_factor_str == "" or luminosity_str == "" or process_order == "" or
        process_group == "")
    {
        fmt::print("Usage: Classification [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: cross-section.\n");
        fmt::print("          -x|--filter_eff: Generator level filter efficiency.\n");
        fmt::print("          -x|--k_factor: K-Factor.\n");
        fmt::print("          -x|--luminosity: Int. luminosity.\n");
        fmt::print("          -x|--process_order: Process order.\n");
        fmt::print("          -x|--process_group: Process group.\n");
        fmt::print("          -i|--input: Path to a txt with input files (one per line).\n");

        exit(-1);
    }
    const double x_section = std::stod(x_section_str);
    const double filter_eff = std::stod(filter_eff_str);
    const double k_factor = std::stod(k_factor_str);
    const double luminosity = std::stod(luminosity_str);

    // create tree reader and add values and arrays
    auto input_chain = TChain("nano_music");
    for (auto &&file : load_input_files(input_file))
    {
        fmt::print("-- Adding file: {}", file);
        input_chain.Add(file.c_str());
    }
    auto tree_reader = TTreeReader(&input_chain);

    ADD_VALUE_READER(pass_low_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_low_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_jet_ht_trigger, bool);
    ADD_VALUE_READER(pass_jet_pt_trigger, bool);

    ADD_VALUE_READER(gen_weight, float);
    ADD_VALUE_READER(Pileup_nTrueInt, float);

    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);

    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);
    ADD_ARRAY_READER(Electron_deltaEtaSC, float);
    ADD_ARRAY_READER(Electron_cutBased, Int_t);
    ADD_ARRAY_READER(Electron_cutBased_HEEP, bool);

    ADD_ARRAY_READER(Photon_pt, float);
    ADD_ARRAY_READER(Photon_eta, float);
    ADD_ARRAY_READER(Photon_phi, float);
    ADD_ARRAY_READER(Photon_isScEtaEB, bool);
    ADD_ARRAY_READER(Photon_isScEtaEE, bool);
    ADD_ARRAY_READER(Photon_cutBased, Int_t);
    ADD_ARRAY_READER(Photon_pixelSeed, bool);

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

    // corrections
    auto jet_corrections = JetCorrector(get_runyear(year), get_era_from_process_name(process, is_data), is_data);

    auto pu_corrector =
        correction::CorrectionSet::from_file(
            "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2018_UL/puWeights.json.gz")
            ->at("Collisions18_UltraLegacy_goldenJSON");

    const auto cutflow_file =
        std::unique_ptr<TFile>(TFile::Open(fmt::format("{}/cutflow_{}_{}.root", output_path, process, year).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    // const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") + 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    int numPDFs = 101; // ?????????????
    auto class_factory = ClassFactory(is_data,
                                      x_section,
                                      filter_eff,
                                      k_factor,
                                      luminosity,
                                      numPDFs,
                                      process,
                                      process_order,
                                      process_group,
                                      fmt::format("{}/{}", output_path, process), // no ".root" is needed
                                      "dummyhash",
                                      1);

    auto trigger_filter = [&process, &is_data](bool pass_low_pt_muon_trigger,
                                               bool pass_high_pt_muon_trigger,
                                               bool pass_low_pt_electron_trigger,
                                               bool pass_high_pt_electron_trigger) -> bool
    {
        if (is_data)
        {
            // Electron/Photon dataset
            if (process.find("EGamma") != std::string::npos      //
                or process.find("Electron") != std::string::npos //
                or process.find("Photon") != std::string::npos)
            {
                return not(pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) and
                       (pass_low_pt_electron_trigger or pass_high_pt_electron_trigger);
            }

            // Muon dataset
            return (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger) and
                   not(pass_low_pt_electron_trigger or pass_high_pt_electron_trigger);
        }

        // MC
        return (pass_low_pt_muon_trigger or pass_high_pt_muon_trigger or pass_low_pt_electron_trigger or
                pass_high_pt_electron_trigger);
    };

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

        (void)event; // remove the "unused variable" warning during compilation

        // if (event > 10)
        // {
        //     break;
        // }

        // get effective event weight
        auto weight = 1.f;
        auto pu_weight = 1.f;

        if (not(is_data))
        {
            pu_weight = pu_corrector->evaluate({unwrap(Pileup_nTrueInt), "nominal"});

            weight = unwrap(gen_weight) * pu_weight * generator_filter / no_cuts / generator_filter * x_section *
                     filter_eff * k_factor * luminosity;
        }

        // Trigger
        // pass_low_pt_muon_trigger
        // pass_high_pt_muon_trigger
        // pass_low_pt_electron_trigger
        // pass_high_pt_electron_trigger
        // pass_jet_ht_trigger
        // pass_jet_pt_trigger
        bool is_good_trigger = trigger_filter(unwrap(pass_low_pt_muon_trigger),
                                              unwrap(pass_high_pt_muon_trigger),
                                              unwrap(pass_low_pt_electron_trigger),
                                              unwrap(pass_high_pt_electron_trigger));

        if (is_good_trigger)
        {
            float met_px = unwrap(MET_pt) * std::cos(unwrap(MET_phi));
            float met_py = unwrap(MET_pt) * std::sin(unwrap(MET_phi));
            // build good objects
            auto muons = ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                     unwrap(Muon_eta),            //
                                                     unwrap(Muon_phi),            //
                                                     unwrap(Muon_tightId),        //
                                                     unwrap(Muon_highPtId),       //
                                                     unwrap(Muon_pfRelIso04_all), //
                                                     unwrap(Muon_tkRelIso),       //
                                                     unwrap(Muon_tunepRelPt),
                                                     year);

            auto electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),  //
                                                             unwrap(Electron_eta), //
                                                             unwrap(Electron_phi), //
                                                             unwrap(Electron_deltaEtaSC),
                                                             unwrap(Electron_cutBased), //
                                                             unwrap(Electron_cutBased_HEEP),
                                                             year);

            auto photons = ObjectFactories::make_photons(unwrap(Photon_pt),        //
                                                         unwrap(Photon_eta),       //
                                                         unwrap(Photon_phi),       //
                                                         unwrap(Photon_isScEtaEB), //
                                                         unwrap(Photon_isScEtaEE), //
                                                         unwrap(Photon_cutBased),  //
                                                         unwrap(Photon_pixelSeed), //
                                                         year);

            auto [jets, bjets] = ObjectFactories::make_jets(unwrap(Jet_pt),                           //
                                                            unwrap(Jet_eta),                          //
                                                            unwrap(Jet_phi),                          //
                                                            unwrap(Jet_mass),                         //
                                                            unwrap(Jet_jetId),                        //
                                                            unwrap(Jet_btagDeepFlavB),                //
                                                            unwrap(Jet_rawFactor),                    //
                                                            unwrap(Jet_area),                         //
                                                            unwrap(Jet_genJetIdx),                    //
                                                            unwrap(fixedGridRhoFastjetAll),           //
                                                            jet_corrections,                          //
                                                            NanoObjects::GenJets(unwrap(GenJet_pt),   //
                                                                                 unwrap(GenJet_eta),  //
                                                                                 unwrap(GenJet_phi)), //
                                                            year);

            auto met = ObjectFactories::make_met(met_px, met_py, year);
            // class_factory.analyseEvent();
        }
        // process monitoring
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

    fmt::print("\n[MUSiC Classification] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);
    class_factory.endJob();

    fmt::print("\n[MUSiC Classification] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}