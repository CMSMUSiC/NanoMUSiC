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

// macro to update class map
auto update_class_map(std::map<std::string, float> &classes, const std::string c_name, const float weight) -> void
{
    std::map<std::string, float>::iterator it = classes.find(c_name);
    if (it != classes.end()) // if in map, update count
    {
        it->second += weight;
    }
    else // it not in map, create new pair
    {
        classes.insert({c_name, weight});
    }
}

// string parser (separate string to set of substrings)
auto parsestring_set(const std::string &input, const char &separator) -> std::set<std::string>
{
    std::set<std::string> output;
    int startIndex = 0;
    int endIndex = 0;
    for (int i = 0; i <= (int)input.size(); i++)
    {
        if (input[i] == separator || i == (int)input.size())
        {
            endIndex = i;
            std::string temp;
            temp.append(input, startIndex, endIndex - startIndex);
            output.insert(temp); // store different classnames in the set
            startIndex = endIndex + 1;
        }
    }
    return output;
}
auto parsestring_vec(const std::string &input, const char &separator) -> std::vector<std::string>
{
    std::vector<std::string> output;
    int startIndex = 0;
    int endIndex = 0;
    for (int i = 0; i <= (int)input.size(); i++)
    {
        if (input[i] == separator || i == (int)input.size())
        {
            endIndex = i;
            std::string temp;
            temp.append(input, startIndex, endIndex - startIndex);
            output.push_back(temp); // store different classnames in the set
            startIndex = endIndex + 1;
        }
    }
    return output;
}

// main function
auto main(int argc, char *argv[]) -> int
{
    std::cout << "Start validation code." << std::endl;

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
    const std::string effective_x_section_str = cmdl({"-x", "--xsection"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();
    const std::string trigger_argument = cmdl({"-trg", "--trigger"}).str();
    const std::string tv_argument = cmdl({"-tv", "--tovalidate"}).str();

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
        fmt::print("          -i|--input: Path to a txt with input files (one per line).\n");
        fmt::print("        -trg|--trigger: Specify trigger and lower limits, e.g. HT1600/PT600.\n");
        fmt::print(
            "         -tv|--tovalidate: Names of the classes that should be plotted. Seperate "
            "Classnames by comma without spaces. Class name format is 'xJ+yBJ'/'xJ+yBJ+nJ'/'xJ+yBJ+X' for "
            "exclusive/jet-inclusive/all-inclusive class. Use class name 'COUNTS' to also create class inhabitation "
            "file (event counts per class).\n");

        exit(-1);
    }
    const double effective_x_section = std::stod(effective_x_section_str);

    std::cout << "Processing sample " << process << "..." << std::endl;

    // create tree reader and add values and arrays
    TChain input_chain("nano_music");

    for (auto &&file : load_input_files(input_file))
    {
        input_chain.Add(file.c_str());
    }
    auto tree_reader = TTreeReader(&input_chain);

    ADD_VALUE_READER(pass_low_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_muon_trigger, bool);
    ADD_VALUE_READER(pass_low_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_high_pt_electron_trigger, bool);
    ADD_VALUE_READER(pass_jet_ht_trigger, bool);
    ADD_VALUE_READER(pass_jet_pt_trigger, bool);

    ADD_VALUE_READER(nMuon, unsigned int);
    ADD_VALUE_READER(Pileup_nTrueInt, float);

    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);

    ADD_VALUE_READER(nElectron, unsigned int);
    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);

    // ADD_ARRAY_READER(Photon_pt, float);
    // ADD_ARRAY_READER(Photon_eta, float);
    // ADD_ARRAY_READER(Photon_phi, float);

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

    ADD_ARRAY_READER(MET_pt, float);
    ADD_ARRAY_READER(MET_phi, float);

    ADD_VALUE_READER(gen_weight, float);

    /*
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
    */

    // event classes map: {classname: counts}
    std::map<std::string, float> classes;

    /*
    // build lepton analysis
    auto z_to_mu_mu_x =
        ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map);

    auto z_to_mu_mu_x_Z_mass = ZToLepLepX(
        fmt::format("{}/z_to_mu_mu_x_Z_mass_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map, true);

    // build dijet analysis
    auto dijets = Dijets(fmt::format("{}/dijets_{}_{}.root", output_path, process, year), dijets_count_map);
    */

    // build jetclass analysis
    // jet classification instances: saved in a map {classname: pointer to jet class validation instance}
    std::map<std::string, JetClass2 *> validation_classes;
    // format: "xJ+yBJ"/"xJ+yBJ+X"/"xJ+yBJ+nJ" for exclusive/all-inclusive/jet-inclusive class containing x jets and y
    // bjets is currently extracted from the -tv argument: -tv argument string: "classname1,classname2..." has to be
    // separated
    auto to_validate = parsestring_set(tv_argument, ',');
    bool plotclasses = false;                            // at least one class validation should be executed
    bool countclasses = false;                           // the class counts should be calculated
    if (to_validate.find("COUNTS") != to_validate.end()) // check whether class counts should be calculated
    {
        countclasses = true;
        to_validate.erase("COUNTS"); // erase element because it is not a class name
    }
    if (to_validate.size() >= 1)     // check whether a class validation should be executed
    {
        plotclasses = true;
    }
    // or the classes to validate can be hardcoded:
    // to_validate = {"1J+0BJ", "2J+0BJ"};
    // create the classification instances
    if (plotclasses) // only if classes should be plotted
    {
        for (const auto &c_name : to_validate)
        {
            validation_classes.insert(std::make_pair(
                c_name, new JetClass2(fmt::format("{}/{}_{}_{}.root", output_path, c_name, process, year), c_name)));
            // file format: classname_process/sample_year.root
        }
    }

    // read in jet triggers
    // is currently extracted from the -trg argument:
    // -tv argument string: "HT1600,PT600" has to be separated
    std::vector<bool> trigger_flags{false, false};   // contains {PT on/off, HT on/off }
    std::vector<float> trigger_thresholds{0.f, 0.f}; // contains {PT thres, HT thres}
    auto trigger_strings = parsestring_set(trigger_argument, ',');
    for (const auto &trigger_string : trigger_strings)
    {
        auto trigger_parameters = parsestring_vec(trigger_string, 'T'); // vector that contains {"P"/"H", "number"}
        if (trigger_parameters.at(0) == "P")
        {
            trigger_flags.at(0) = true;
            trigger_thresholds.at(0) = std::stof(trigger_parameters.at(1));
        }
        else if (trigger_parameters.at(0) == "H")
        {
            trigger_flags.at(1) = true;
            trigger_thresholds.at(1) = std::stof(trigger_parameters.at(1));
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

    // std::cout << "Start event loop." << std::endl;
    //  launch event loop for Data or MC
    for (auto &&event : tree_reader)
    {
        (void)event; // remove the "unused variable" warning during compilation

        /* EVENT BREAK IF NECESSARY
        if (event > 10000)
        {
            break;
        }
        */
        // std::cout << "****************\nEvent No. " << event << std::endl;

        // get effective event weight
        auto weight = 1.f;
        auto const_weights = 1.f;
        auto pu_weight = 1.f;

        if (not(is_data))
        {
            pu_weight = pu_corrector->evaluate({unwrap(Pileup_nTrueInt), "nominal"});
            weight = const_weights * unwrap(gen_weight) * pu_weight * generator_filter / no_cuts / generator_filter *
                     effective_x_section;
        }

        // TRIGGER
        // Muons
        // bool is_good_trigger = unwrap(pass_low_pt_muon_trigger) and (unwrap(pass_high_pt_muon_trigger)) and
        //                        (unwrap(pass_low_pt_electron_trigger)) and (unwrap(pass_high_pt_electron_trigger));
        // bool is_good_trigger = unwrap(pass_low_pt_muon_trigger) or unwrap(pass_high_pt_muon_trigger);

        // Jets (JET TRIGGER)
        bool is_good_trigger = false;
        if (trigger_flags.at(0)) // PT trigger
        {
            is_good_trigger = is_good_trigger || unwrap(pass_jet_pt_trigger);
        }
        if (trigger_flags.at(1)) // HT trigger
        {
            is_good_trigger = is_good_trigger || unwrap(pass_jet_ht_trigger);
        }
        if (not(is_good_trigger))
        {
            continue; // skip if no trigger fired
        }
        // std::cout << "Passed trigger fire check." << std::endl;

        /*
        // muons
        auto muons = make_muons(unwrap(Muon_pt),
                                unwrap(Muon_eta),
                                unwrap(Muon_phi),
                                unwrap(Muon_tightId),
                                unwrap(Muon_highPtId),
                                unwrap(Muon_pfRelIso04_all),
                                unwrap(Muon_tkRelIso),
                                unwrap(Muon_tunepRelPt));
        */

        /* OLD MUON VALIDATION
        // MuMu + X
        if (muons.size() >= 2)
        {
            auto muon_1 = muons.at(0);
            auto muon_2 = muons.at(1);

            // wide mass range
            z_to_mu_mu_x.fill(muon_1, muon_2, 0, std::nullopt, 0, std::nullopt, std::nullopt, weight);

            // Z mass range
            if (PDG::Z::Mass - 20. < (muon_1 + muon_2).mass() and (muon_1 + muon_2).mass() < PDG::Z::Mass + 20.)
            {
                z_to_mu_mu_x_Z_mass.fill(muon_1, muon_2, 0, std::nullopt, 0, std::nullopt, std::nullopt, weight);
            }
        }
        */

        // Jets
        auto gen_jets = NanoObjects::GenJets(unwrap(GenJet_pt),  //
                                             unwrap(GenJet_eta), //
                                             unwrap(GenJet_phi));

        auto jets = make_jets(unwrap(Jet_pt),                 //
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
                              gen_jets,                       //
                              year,
                              false);

        auto bjets = make_jets(unwrap(Jet_pt),                 //
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
                               gen_jets,                       //
                               year,
                               true);

        // Type counts
        unsigned int nelectron = unwrap(nElectron);
        unsigned int nmuon = unwrap(nMuon);
        unsigned int njet = jets.size();
        unsigned int nbjet = bjets.size();

        // met
        auto met_pt = unwrap(MET_pt);
        auto get_met = [](RVec<float> met_pt) -> std::optional<float>
        {
            if (met_pt.size() == 0)
            {
                return std::nullopt;
            }
            return met_pt[0];
        };
        auto met = get_met(met_pt);

        // std::cout << "Generated objects." << std::endl;

        ///* LEPTON VETO
        if (not(nelectron == 0 and nmuon == 0))
        {
            continue; // veto any e,mu final states
        }
        // std::cout << "Passed e/mu veto." << std::endl;
        //*/

        // CHECK TRIGGER THRESHOLDS
        if (trigger_flags.at(0))                     // PT trigger
        {
            std::vector<float> leading_pt{0.f, 0.f}; // check threshold for pt of leading jet/bjet
            // because jet or bjet could have higher pt, use the following approach
            if (njet >= 1)
            {
                leading_pt.at(0) = jets.at(0).pt();
            }
            if (nbjet >= 1)
            {
                leading_pt.at(1) = bjets.at(0).pt();
            }
            if (not(leading_pt.at(0) >= trigger_thresholds.at(0) or leading_pt.at(1) >= trigger_thresholds.at(0)))
            {
                continue; // skip this event
            }
        }
        if (trigger_flags.at(1)) // HT trigger
        {
            float sum_pt = 0;    // check sum pt threshold
            for (size_t i = 0; i < njet; i++)
            {
                sum_pt += jets.at(i).pt();
            }
            for (size_t i = 0; i < nbjet; i++)
            {
                sum_pt += bjets.at(i).pt();
            }
            if (not(sum_pt >= trigger_thresholds.at(0)))
            {
                continue; // skip this event
            }
        }
        // std::cout << "Passed trigger threshold checks." << std::endl;

        // JET CLASS VALIDATION
        std::set<std::string> eventclass =
            {}; // set that includes all classes the current sample is a member of (for plotting)
        // find all classes for the event
        // event class inhabitation is counted in the map classes {classname: counts}
        // all event classes for current event are stored in the set eventclass {classnames}
        // std::cout << "Start event class loop." << std::endl;
        for (int c_njet = (int)njet; c_njet >= 0; c_njet--)
        {
            for (int c_nbjet = (int)nbjet; c_nbjet >= 0; c_nbjet--)
            {
                if ((c_njet == 0 and c_nbjet == 0) == false) // skip 0 jet class
                {
                    // std::cout << "Next loop iteration." << std::endl;
                    if (c_njet == (int)njet and c_nbjet == (int)nbjet) // exclusive class
                    {
                        // std::cout << "excl" << std::endl;
                        std::string c_name = fmt::format("{}J+{}BJ", c_njet, c_nbjet);
                        eventclass.insert(c_name); // log class name for this event (for plotting)
                        if (countclasses)          // update class count map if classes should be counted
                        {
                            update_class_map(classes, c_name, weight);
                        }
                    }
                    if (c_njet <= (int)njet and c_nbjet == (int)nbjet) // jet-inclusive (nJet) class
                    {
                        // std::cout << "jet-incl" << std::endl;
                        std::string c_name = fmt::format("{}J+{}BJ+nJ", c_njet, c_nbjet);
                        eventclass.insert(c_name); // log class name for this event (for plotting)
                        if (countclasses)          // update class count map if classes should be counted
                        {
                            update_class_map(classes, c_name, weight);
                        }
                    }
                    if (c_njet <= (int)njet and
                        c_nbjet <= (int)nbjet) // all-inclusive (+X) class, the jet-inclusive case is included here
                    {
                        // std::cout << "all-incl" << std::endl;
                        std::string c_name = fmt::format("{}J+{}BJ+X", c_njet, c_nbjet);
                        eventclass.insert(c_name); // log class name for this event (for plotting)
                        if (countclasses)          // update class count map if classes should be counted
                        {
                            update_class_map(classes, c_name, weight);
                        }
                    }
                    // note: inclusive classes can be X = 0 or nJet = 0, so the exclusive classes are
                    // included in the inclusive classes
                }
            }
        }
        // validate plotting with >= 2jet
        if (njet + nbjet >= 2)
        {
        }
        // fill histograms for all event classes that should be validated and that the current event is
        // a member of
        if (plotclasses)
        {
            for (const auto &c_name : eventclass)
            {
                for (const auto &c_name_toval : to_validate)
                {
                    if (c_name == c_name_toval)
                    {
                        // fill the event in the class
                        validation_classes[c_name]->fill(jets, bjets, nelectron, nmuon, met, weight);
                    }
                }
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
    // save classes and counts in toml file
    if (countclasses)
    {
        classfile.open(fmt::format("{}/classes_{}_{}.toml", output_path, process, year).c_str());
        classfile << "[counts]\n";
        for (auto &[c_name, c_count] : classes)
        {
            if (c_count < 0) // if negative weights dominate, set count to 0
            {
                c_count = 0;
            }
            classfile << "\"" << c_name << "\" = " << c_count << "\n";
        }
        classfile.close();
    }
    // save the validation example classes
    if (plotclasses)
    {
        for (const auto &c_name : to_validate)
        {
            validation_classes[c_name]->dump_outputs();
        }
    }

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}