//////////////////////////////////////////////////////////////
///////////     JET CLASS 2D VALIDATION CODE        ///////////
///////////      2D angular plots, only stat err   ///////////
//////////////////////////////////////////////////////////////

#include "HeavyValidation.hpp"

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
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

// add zero counts for new classname
inline auto zero_class_map(std::map<std::string, std::map<std::string, float>> &classes,
                           std::map<std::string, float> &classes_stat,
                           const std::set<std::string> &systematics,
                           const std::string c_name) -> void
{
    std::map<std::string, std::map<std::string, float>>::iterator it = classes.find(c_name);
    if (it == classes.end()) // it not in map, create new entry with counts 0
    {
        std::map<std::string, float> emptyweights;
        for (const auto &_s_name : systematics)
        {
            emptyweights.insert({_s_name, 0.f});
        }
        classes.insert({c_name, emptyweights});
        classes_stat.insert({c_name, 0.f});
    }
}

inline auto update_class(std::set<std::string> &eventclass,
                         std::map<std::string, std::map<std::string, float>> &classes,
                         std::map<std::string, float> &classes_stat,
                         const std::string &c_name,
                         const std::set<std::string> &systematics,
                         const bool countclasses,
                         std::map<std::string, float> &weight) -> void
{
    eventclass.insert(c_name); // log class name for this event (for plotting)
    if (countclasses)          // update class count map if classes should be counted
    {
        zero_class_map(classes, classes_stat, systematics, c_name);
        for (const auto &s_name : systematics)
        {
            classes[c_name][s_name] += weight[s_name];                     // add weight to class count
            classes_stat[c_name] += weight["nominal"] * weight["nominal"]; // add weight^2 to syst_err^2
        }
    }
}

// main function
auto main(int argc, char *argv[]) -> int
{
    bool debugprint = false; // print debug messages flag
    if (debugprint)
    {
        std::cout << "Start validation code." << std::endl;
    }

    // silence LHAPDF
    LHAPDF::setVerbosity(0);

    // set SumW2 as default
    TH1::SetDefaultSumw2(true);

    // command line options, parse arguments
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    const std::string process = cmdl({"-p", "--process"}).str();
    const std::string year = cmdl({"-y", "--year"}).str();
    const bool is_data = cmdl[{"-d", "--is_data"}];
    const std::string output_path = cmdl({"-o", "--output"}).str();
    const std::string effective_x_section_str = cmdl({"-x", "--xsection"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();
    const std::string trigger_argument = cmdl({"-trg", "--trigger"}).str();
    const std::string tv_argument = cmdl({"-tv", "--tovalidate"}).str();
    const std::string order = cmdl({"-or", "--order"}).str();
    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or
        effective_x_section_str == "" or trigger_argument == "" or tv_argument == "")
    {
        fmt::print("Usage: validation [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: Effective cross-section (xsection * lumi).\n");
        fmt::print("         -or|--order: Order of MC (LO,...).\n");
        fmt::print("        -trg|--trigger: Specify trigger and lower limits, e.g. HT1600/PT600.\n");
        fmt::print(
            "         -tv|--tovalidate: Names of the classes that should be plotted. Seperate "
            "Classnames by comma without spaces. Class name format is 'xJ+yBJ+zMET[+XJ]' for "
            "exclusive [_] / jet- and bjet-inclusive [+XJ] classes (with z = 0, 1). "
            "Use class name 'COUNTS' to also create class inhabitation file (event counts per class).\n");

        exit(-1);
    }
    // read in effective cross section (calculated by python code)
    const double effective_x_section = std::stod(effective_x_section_str);

    if (debugprint)
    {
        std::cout << "Processing sample " << process << "..." << std::endl;
    }

    // load input files
    if (debugprint)
    {
        std::cout << "Create input chain." << std::endl;
    }
    TChain input_chain("nano_music");

    if (debugprint)
    {
        std::cout << "Load input files." << std::endl;
    }
    for (auto &&file : load_input_files(input_file))
    {
        input_chain.Add(file.c_str());
    }

    // value and array readers to read from skimmed files
    if (debugprint)
    {
        std::cout << "Add variable readers." << std::endl;
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

    // build jetclass analysis
    if (debugprint)
    {
        std::cout << "Read in class information." << std::endl;
    }
    // format: "xJ+yBJ"/"xJ+yBJ+X"/"xJ+yBJ+nJ" for exclusive/all-inclusive/jet-inclusive class containing x jets and y
    // bjets is currently extracted from the -tv argument: -tv argument string: "classname1,classname2..." has to be
    // separated
    std::vector<std::vector<int>> ptbins{
        {0, 100},
        {100, 200},
        {200, 300},
        {300, 400},
        {400, 500},
        {500, 600},
        {600, 700},
    };
    std::string class_prefix = "2J+0BJ+0MET_";
    std::set<std::string> to_validate;
    for (size_t i = 0; i < ptbins.size(); i++)
    {
        to_validate.insert(class_prefix + std::to_string(ptbins.at(i).at(0)) + "_" +
                           std::to_string(ptbins.at(i).at(1)));
    }

    // systematics prefixes (systnames):
    // nominal: nominal (no systematics applied)
    // up_name: constant systematics "name" applied up
    // down_name: constant systematics "name" applied down
    std::set<std::string> systematics{
        // holds all systematics, the specific systematics are merged into this set
        "nominal",
    };

    // create the classification instances
    if (debugprint)
    {
        std::cout << "Create class instances." << std::endl;
    }
    // jet classification instances: saved in a map {classname: pointer to jet class validation instance}
    // nl: nominal, up: systematic up, dn: systematic down
    std::map<std::string, std::map<std::string, JetClass2D *>> validation_classes;
    // stores validation classes: {classname: {systname: jetclass instance pointer}}
    // fill maps
    for (const auto &c_name : to_validate)
    {
        for (const auto &s_name : systematics)
        {
            validation_classes[c_name][s_name] =
                new JetClass2D(fmt::format("{}/{}_{}_{}_{}.root", output_path, c_name, s_name, process, year), c_name);
            // file format: classname_systname_samplename_year.root
        }
    }

    // build jet corrections
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

    // fmt::print("\n[MUSiC Validation] Creating set of processed events ...\n");
    // MAP[run_number : SET[event_number]]
    std::unordered_map<unsigned int, std::unordered_set<unsigned long>> processed_data_events;

    // file where the class counts are stored
    std::ofstream classfile;

    if (debugprint)
    {
        std::cout << "Start event loop." << std::endl;
    }
    //  launch event loop for Data or MC
    for (auto &&event : tree_reader)
    {
        (void)event; // remove the "unused variable" warning during compilation

        if (debugprint)
        {
            std::cout << std::endl;
        }

        /* EVENT BREAK IF NECESSARY
        if (event > 10000)
        {
            throw std::runtime_error("finished after given event break");
            break;
        }
        */
        // std::cout << "****************\nEvent No. " << event << std::endl;

        // calculate effective event weight

        // create weight set
        std::map<std::string, float> weight{{"nominal", 1.f}};

        if (not(is_data))
        {
            float const_weights = 1.f;
            std::map<std::string, float> pu_weight;
            pu_weight["nominal"] = pu_corrector->evaluate({unwrap(Pileup_nTrueInt), "nominal"});

            // calculate event weight
            weight["nominal"] = const_weights * unwrap(gen_weight) * pu_weight["nominal"] * generator_filter / no_cuts /
                                generator_filter * effective_x_section;
            // weight = const_weights * gen_weight * pu_weight * xsection * filter_eff * k_factor * luminosity /
            // no_cuts | python calculates: effective_x_section = xsection * filter_eff * k_factor * luminosity
            // (is the weighting formula of the MUSiC AN p.9)
        }

        // JET TRIGGER (HT1050)
        bool is_good_trigger = false;
        is_good_trigger = unwrap(pass_jet_ht_trigger);
        if (not(is_good_trigger))
        {
            continue; // skip if no trigger fired
        }
        if (debugprint)
        {
            std::cout << "Passed trigger fire check." << std::endl;
        }

        // Build good objects (selection level objects)
        // jets
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
        // muons
        auto muons = ObjectFactories::make_muons(unwrap(Muon_pt),             //
                                                 unwrap(Muon_eta),            //
                                                 unwrap(Muon_phi),            //
                                                 unwrap(Muon_tightId),        //
                                                 unwrap(Muon_highPtId),       //
                                                 unwrap(Muon_pfRelIso04_all), //
                                                 unwrap(Muon_tkRelIso),       //
                                                 unwrap(Muon_tunepRelPt),
                                                 year);
        // electrons
        auto electrons = ObjectFactories::make_electrons(unwrap(Electron_pt),       //
                                                         unwrap(Electron_eta),      //
                                                         unwrap(Electron_phi),      //
                                                         unwrap(Electron_deltaEtaSC),
                                                         unwrap(Electron_cutBased), //
                                                         unwrap(Electron_cutBased_HEEP),
                                                         year);
        // photons
        auto photons = ObjectFactories::make_photons(unwrap(Photon_pt),        //
                                                     unwrap(Photon_eta),       //
                                                     unwrap(Photon_phi),       //
                                                     unwrap(Photon_isScEtaEB), //
                                                     unwrap(Photon_isScEtaEE), //
                                                     unwrap(Photon_cutBased),  //
                                                     unwrap(Photon_pixelSeed), //
                                                     year);
        // met
        float met_px = unwrap(MET_pt) * std::cos(unwrap(MET_phi));
        float met_py = unwrap(MET_pt) * std::sin(unwrap(MET_phi));
        auto met = ObjectFactories::make_met(met_px, met_py, year);

        // Type counts
        unsigned int nelectron = electrons.size();
        unsigned int nmuon = muons.size();
        unsigned int njet = jets.size();
        unsigned int nbjet = bjets.size();
        unsigned int nphoton = photons.size();
        bool is_met = false; // set met flag
        if (met.size() >= 1)
        {
            is_met = true;
        }

        if (debugprint)
        {
            std::cout << "Generated objects." << std::endl;
        }

        ///* optional: LEPTON VETO or CONDITIONS
        // if (not(nelectron == 0 and nmuon == 0)) // veto all leptons
        // if (not(nelectron >= 1 or nmuon >= 1)) // at least one lepton
        // if (not(nelectron == 0 and nmuon == 0 and (not is_met) and nphoton == 0)) // veto all leptons, photons and
        // met
        if (not(nelectron == 0 and nmuon == 0 and nphoton == 0)) // veto all leptons, photons
        {
            continue;                                            // veto is condition is not satisfied
        }
        if (debugprint)
        {
            std::cout << "Passed object veto." << std::endl;
        }
        //*/

        // 2J+0BJ excl class only
        if (nbjet >= 1)
        {
            continue;
        }
        if (not(njet == 2))
        {
            continue;
        }
        if (is_met)
        {
            continue;
        }

        // HT CUT (for trigger efficiency)
        float sum_pt = 0.f;
        for (size_t i = 0; i < jets.size(); i++)
        {
            sum_pt += jets.at(i).pt();
        }
        if (sum_pt < 1600)
        {
            continue;
        }

        // JET CLASS VALIDATION
        if (debugprint)
        {
            std::cout << "ACCEPTED EVENT.\nStart event class loop." << std::endl;
        }

        // fill histograms for the respective ptbin
        for (size_t i = 0; i < ptbins.size(); i++)
        {
            if (jets.at(1).pt() >= ptbins.at(i).at(0) and jets.at(1).pt() < ptbins.at(i).at(1))
            {
                validation_classes[class_prefix + std::to_string(ptbins.at(i).at(0)) + "_" +
                                   std::to_string(ptbins.at(i).at(1))]["nominal"]
                    ->fill(jets, bjets, nelectron, nmuon, met, weight["nominal"]);
            }
        }

        // std::cout << "Finished event classification." << std::endl;
    }

    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);
    /*
    z_to_mu_mu_x.dump_outputs();
    z_to_mu_mu_x_Z_mass.dump_outputs();
    dijets.dump_outputs();
    */
    // SAVE JET CLASS VALIDATION
    // save the validation example classes
    for (const auto &c_name : to_validate)
    {
        validation_classes[c_name]["nominal"]->dump_outputs();
    }

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}