#include "Validation.hpp"

#include "Configs.hpp"
#include "Math/Vector4Dfwd.h"
#include "Outputs.hpp"
#include "ROOT/RVec.hxx"
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
auto update_class_map(std::map<std::string, unsigned int> &classes, const std::string c_name, const float weight)
    -> void
{
    // new version
    std::map<std::string, unsigned int>::iterator it = classes.find(c_name);
    if (it != classes.end())
    {
        it->second += weight;
    }
    else
    {
        classes.insert({c_name, weight});
    }
}

// main function
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
    const std::string effective_x_section_str = cmdl({"-x", "--xsection"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();
    const std::string mode = cmdl({"-m", "--mode"}).str();
    const std::string minpt_argument = cmdl({"-pt", "--minpt"}).str();
    const std::string tv_argument = cmdl({"-tv", "--tovalidate"}).str();

    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or
        effective_x_section_str == "" or mode == "")
    {
        fmt::print("Usage: validation [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: Effective cross-section (xsection * lumi).\n");
        fmt::print("          -i|--input: Path to a txt withg input files (one per line).\n");
        fmt::print(
            "          -m|--mode: Validation mode ('Jet'/'J' [Old jet validation] or 'JetClass'/'JC' [New Jet "
            "classification validation] or 'Lepton'/'L' [Legacy lepton validation]).\n");
        fmt::print("         -pt|--minpt: In 'Jet' mode: Minimum pt for jets to validate.\n");
        fmt::print(
            "         -tv|--tovalidate: In 'JetClass' mode: Names of the classes that should be plotted. Seperate "
            "Classnames by comma without spaces. Class name format is 'xJ+yBJ'/'xJ+yBJ+nJ'/'xJ+yBJ+X' for "
            "exclusive/jet-inclusive/all-inclusive class. Use class name 'COUNTS' to also create class inhabitation "
            "file (event counts per class).\n");

        exit(-1);
    }
    if (mode != "J" and mode != "Jet" and mode != "JC" and mode != "JetClass" and mode != "L" and mode != "Lepton")
    {
        throw std::runtime_error("Valid arguments for -m/--mode are only J,JC,L (Jet, JetClass, Lepton).");
    }
    if (minpt_argument == "" and (mode == "J" or mode == "Jet"))
    {
        throw std::runtime_error("In jet mode, the minimum pt has to specified (with '-pt'/'--minpt').");
    }
    if (tv_argument == "" and (mode == "JC" or mode == "JetClass"))
    {
        throw std::runtime_error(
            "In jet classification mode, the classes to plot have to be specified (with '-tv'/'--tovalidate').");
    }
    const double effective_x_section = std::stod(effective_x_section_str);

    std::cout << "Processing sample " << process << "..." << std::endl;

    // create tree reader and add values and arrays
    TChain input_chain("nano_music");
    ;
    for (auto &&file : load_input_files(input_file))
    {
        input_chain.Add(file.c_str());
    }
    auto tree_reader = TTreeReader(&input_chain);

    ADD_VALUE_READER(run, unsigned int);
    ADD_VALUE_READER(event_number, unsigned long);

    ADD_VALUE_READER(kTotalWeights, unsigned int);
    ADD_ARRAY_READER(weights_nominal, float);
    ADD_ARRAY_READER(weights_up, float);
    ADD_ARRAY_READER(weights_down, float);

    ADD_VALUE_READER(nMuon, unsigned int);
    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);

    ADD_VALUE_READER(nElectron, unsigned int);
    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);

    // ADD_VALUE_READER(nPhoton, unsigned int);
    // ADD_ARRAY_READER(Photon_pt, float);
    // ADD_ARRAY_READER(Photon_eta, float);
    // ADD_ARRAY_READER(Photon_phi, float);

    ADD_VALUE_READER(nBJet, unsigned int);
    ADD_ARRAY_READER(BJet_pt, float);
    ADD_ARRAY_READER(BJet_eta, float);
    ADD_ARRAY_READER(BJet_phi, float);
    ADD_ARRAY_READER(BJet_mass, float);

    ADD_VALUE_READER(nJet, unsigned int);
    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);
    ADD_ARRAY_READER(Jet_mass, float);

    ADD_VALUE_READER(nMET, unsigned int);
    ADD_ARRAY_READER(MET_pt, float);

    // count maps
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
    const std::map<int, std::map<std::string, int>> jet_count_map = {{0,
                                                                      {{"Ele", 0},
                                                                       {"EleEE", 0},
                                                                       {"EleEB", 0},
                                                                       {"Muon", 0},
                                                                       {"Gamma", 0},
                                                                       {"GammaEB", 0},
                                                                       {"GammaEE", 0},
                                                                       {"Tau", 0},
                                                                       {"Jet", 0},
                                                                       {"bJet", 0},
                                                                       {"MET", 0}}},
                                                                     {1,
                                                                      {{"Ele", 0},
                                                                       {"EleEE", 0},
                                                                       {"EleEB", 0},
                                                                       {"Muon", 0},
                                                                       {"Gamma", 0},
                                                                       {"GammaEB", 0},
                                                                       {"GammaEE", 0},
                                                                       {"Tau", 0},
                                                                       {"Jet", 1},
                                                                       {"bJet", 0},
                                                                       {"MET", 0}}},
                                                                     {2,
                                                                      {{"Ele", 0},
                                                                       {"EleEE", 0},
                                                                       {"EleEB", 0},
                                                                       {"Muon", 0},
                                                                       {"Gamma", 0},
                                                                       {"GammaEB", 0},
                                                                       {"GammaEE", 0},
                                                                       {"Tau", 0},
                                                                       {"Jet", 2},
                                                                       {"bJet", 0},
                                                                       {"MET", 0}}}};

    // event classes map: {classname: counts}
    std::map<std::string, unsigned int> classes;

    ///////////////////////////////// create instances /////////////////////////////////
    auto z_to_mu_mu_x =
        ZToLepLepX(fmt::format("{}/z_to_mu_mu_x_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map);

    auto z_to_ele_ele_x =
        ZToLepLepX(fmt::format("{}/z_to_ele_ele_x_{}_{}.root", output_path, process, year), z_to_ele_ele_x_count_map);

    auto z_to_mu_mu_x_Z_mass = ZToLepLepX(
        fmt::format("{}/z_to_mu_mu_x_Z_mass_{}_{}.root", output_path, process, year), z_to_mu_mu_x_count_map, true);

    auto z_to_ele_ele_x_Z_mass = ZToLepLepX(
        fmt::format("{}/z_to_ele_ele_x_Z_mass_{}_{}.root", output_path, process, year), z_to_ele_ele_x_count_map, true);

    auto jet_val = JetVal(fmt::format("{}/jet_val_{}_{}.root", output_path, process, year), jet_count_map);

    // jet classification instances: saved in a map {classname: pointer to jet class validation instance}
    std::map<std::string, JetClass *> validation_classes;

    /////////////// CHOOSE CLASSES TO VALIDATE ///////////////
    // format: "xJ+yBJ"/"xJ+yBJ+X"/"xJ+yBJ+nJ" for exclusive/all-inclusive/jet-inclusive class containing x jets and y
    // bjets
    std::set<std::string> to_validate;
    // is currently extracted from the -tv argument:
    // -tv argument string: "classname1,classname2..." has to be separated
    int startIndex = 0;
    int endIndex = 0;
    const char separator = ',';
    for (int i = 0; i <= (int)tv_argument.size(); i++)
    {
        if (tv_argument[i] == separator || i == (int)tv_argument.size())
        {
            endIndex = i;
            std::string temp;
            temp.append(tv_argument, startIndex, endIndex - startIndex);
            to_validate.insert(temp); // store different classnames in the set
            startIndex = endIndex + 1;
        }
    }
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
    //////////////////////////////////////////////////////////

    // create the classification instances
    if (plotclasses) // only if classes should be plotted
    {
        for (const auto &c_name : to_validate)
        {
            validation_classes.insert(std::make_pair(
                c_name, new JetClass(fmt::format("{}/{}_{}_{}.root", output_path, c_name, process, year), c_name)));
            // file format: classname_process/sample_year.root
        }
    }

    ///////////////////////////////// import skimmed files /////////////////////////////////
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

    // launch event loop for Data or MC
    // const auto total_number_of_events = tree_reader.GetEntries();
    if (tree_reader.GetEntries(true) >= 1) // check that tree is not empty
    {
        ///////////////////////////////// process all events in a loop /////////////////////////////////
        for (auto &&event : tree_reader)
        {
            (void)event; // remove the unused warning during compilation

            // check if the event has already been processed (only for Data)
            bool already_processed = false;
            auto this_run = unwrap(run);
            auto this_event_number = unwrap(event_number);
            if (is_data)
            {
                if (processed_data_events.count(this_run) > 0)
                {
                    if (processed_data_events[this_run].count(this_event_number) > 0)
                    {
                        already_processed = true;
                    }
                    else
                    {
                        processed_data_events[this_run].insert(this_event_number);
                    }
                }
                else
                {
                    processed_data_events.insert({this_run, std::unordered_set<unsigned long>({this_event_number})});
                }
            }

            if (not(already_processed))
            {
                // get effective event weight
                auto weight = 1.f;
                if (not(is_data))
                {
                    weight =
                        Outputs::get_event_weight(unwrap(weights_nominal), unwrap(weights_up), unwrap(weights_down)) *
                        generator_filter / no_cuts / generator_filter * effective_x_section;

                    // fmt::print("------------------\n");
                    // fmt::print(
                    //     "event weight: {}\n",
                    //     Outputs::get_event_weight(unwrap(weights_nominal), unwrap(weights_up),
                    //     unwrap(weights_down)));
                    // fmt::print("weights_nominal: {}\n", weights_nominal);
                    // fmt::print("Eff xSec: {}\n", effective_x_section);
                    // fmt::print("Weight: {}\n", weight);
                    // fmt::print("total gen weight: {}\n", generator_filter);
                }

                ///////////////////////////////// order objects after highest pt /////////////////////////////////
                // reorder objects
                // muons
                const auto muon_reordering_mask = VecOps::Argsort(unwrap(Muon_pt),
                                                                  [](double x, double y) -> bool
                                                                  {
                                                                      return x > y;
                                                                  });

                const auto muon_pt = VecOps::Take(unwrap(Muon_pt), muon_reordering_mask);
                const auto muon_eta = VecOps::Take(unwrap(Muon_eta), muon_reordering_mask);
                const auto muon_phi = VecOps::Take(unwrap(Muon_phi), muon_reordering_mask);
                const auto nmuons = unwrap(nMuon);

                // electrons
                const auto electron_reordering_mask = VecOps::Argsort(unwrap(Electron_pt),
                                                                      [](double x, double y) -> bool
                                                                      {
                                                                          return x > y;
                                                                      });
                const auto electron_pt = VecOps::Take(unwrap(Electron_pt), electron_reordering_mask);
                const auto electron_eta = VecOps::Take(unwrap(Electron_eta), electron_reordering_mask);
                const auto electron_phi = VecOps::Take(unwrap(Electron_phi), electron_reordering_mask);
                const auto nelectrons = unwrap(nElectron);

                // // photons
                // const  auto photon_reordering_mask = VecOps::Argsort(unwrap(Photon_pt),
                //   [](double x, double y) -> bool
                //   {
                //       return x > y;
                //   });
                // const auto photon_pt = VecOps::Take(unwrap(Photon_pt),photon_reordering_mask);
                // const auto photon_eta = VecOps::Take(unwrap(Photon_eta),photon_reordering_mask);
                // const auto photon_phi = VecOps::Take(unwrap(Photon_phi),photon_reordering_mask);

                // bjets
                const auto bjet_reordering_mask = VecOps::Argsort(unwrap(BJet_pt),
                                                                  [](double x, double y) -> bool
                                                                  {
                                                                      return x > y;
                                                                  });
                const auto bjet_pt = VecOps::Take(unwrap(BJet_pt), bjet_reordering_mask);
                const auto bjet_eta = VecOps::Take(unwrap(BJet_eta), bjet_reordering_mask);
                const auto bjet_phi = VecOps::Take(unwrap(BJet_phi), bjet_reordering_mask);
                const auto bjet_mass = VecOps::Take(unwrap(BJet_mass), bjet_reordering_mask);

                // jets
                const auto jet_reordering_mask = VecOps::Argsort(unwrap(Jet_pt),
                                                                 [](double x, double y) -> bool
                                                                 {
                                                                     return x > y;
                                                                 });
                const auto jet_pt = VecOps::Take(unwrap(Jet_pt), jet_reordering_mask);
                const auto jet_eta = VecOps::Take(unwrap(Jet_eta), jet_reordering_mask);
                const auto jet_phi = VecOps::Take(unwrap(Jet_phi), jet_reordering_mask);
                const auto jet_mass = VecOps::Take(unwrap(Jet_mass), jet_reordering_mask);

                // MET
                auto met_pt = unwrap(MET_pt);
                auto get_leading = [](RVec<float> pt,
                                      RVec<float> eta,
                                      RVec<float> phi) -> std::pair<unsigned int, std::optional<Math::PtEtaPhiMVector>>
                {
                    if (pt.size() == 0)
                    {
                        return std::make_pair(0, std::nullopt);
                    }
                    return std::make_pair(pt.size(), Math::PtEtaPhiMVector(pt[0], eta[0], phi[0], 1E-9));
                };
                auto get_met = [](RVec<float> met_pt) -> std::optional<float>
                {
                    if (met_pt.size() == 0)
                    {
                        return std::nullopt;
                    }
                    return met_pt[0];
                };
                auto [nBJet, bjet] = get_leading(bjet_pt, bjet_eta, bjet_phi);
                auto [nJet, jet] = get_leading(jet_pt, jet_eta, jet_phi);
                auto met = get_met(met_pt);
                int njet = (int)nJet;
                int nbjet = (int)nBJet;

                ///////////////////////////////// calculate and fill histograms of validation files
                ////////////////////////////////////
                if (mode == "Lepton" or mode == "L") // fill lepton validation files (legacy code)
                {
                    // ---- MuMu + X
                    if (muon_pt.size() >= 2)
                    {
                        auto muon_1 = Math::PtEtaPhiMVector(muon_pt[0], muon_eta[0], muon_phi[0], PDG::Muon::Mass);
                        auto muon_2 = Math::PtEtaPhiMVector(muon_pt[1], muon_eta[1], muon_phi[1], PDG::Muon::Mass);

                        // wide mass range
                        z_to_mu_mu_x.fill(muon_1, muon_2, nBJet, bjet, nJet, jet, met, weight);

                        // Z mass range
                        if ((muon_1 + muon_2).mass() > PDG::Z::Mass - 20. and
                            (muon_1 + muon_2).mass() < PDG::Z::Mass + 20.)
                        {
                            z_to_mu_mu_x_Z_mass.fill(muon_1, muon_2, nBJet, bjet, nJet, jet, met, weight);
                        }
                    }

                    // ---- EleEle + X
                    if (electron_pt.size() >= 2)
                    {
                        auto electron_1 = Math::PtEtaPhiMVector(
                            electron_pt[0], electron_eta[0], electron_phi[0], PDG::Electron::Mass);
                        auto electron_2 = Math::PtEtaPhiMVector(
                            electron_pt[1], electron_eta[1], electron_phi[1], PDG::Electron::Mass);

                        // wide mass range
                        z_to_ele_ele_x.fill(electron_1, electron_2, nBJet, bjet, nJet, jet, met, weight);

                        // Z mass range
                        if ((electron_1 + electron_2).mass() > PDG::Z::Mass - 20. and
                            (electron_1 + electron_2).mass() < PDG::Z::Mass + 20.)
                        {
                            z_to_ele_ele_x_Z_mass.fill(electron_1, electron_2, nBJet, bjet, nJet, jet, met, weight);
                        }
                    }
                }
                // /*
                else if (mode == "JetClass" or mode == "JC") // fill jet classification validation files
                {
                    if (nelectrons == 0 and nmuons == 0)     // veto any e,mu final states
                    {
                        std::set<std::string> eventclass =
                            {}; // set that includes all classes the current sample is a member of (for plotting)
                        // find all classes for the event
                        // event class inhabitation is counted in the map classes {classname: counts}
                        // all event classes for current event are stored in the set eventclass {classnames}
                        for (int c_njet = njet; c_njet >= 0; c_njet--)
                        {
                            for (int c_nbjet = nbjet; c_nbjet >= 0; c_nbjet--)
                            {
                                if ((c_njet == 0 and c_nbjet == 0) == false) // skip 0 jet class
                                {
                                    if (c_njet == njet and c_nbjet == nbjet) // exclusive class
                                    {
                                        std::string c_name = fmt::format("{}J+{}BJ", c_njet, c_nbjet);
                                        eventclass.insert(c_name); // log class name for this event (for plotting)
                                        if (countclasses) // update class count map if classes should be counted
                                        {
                                            update_class_map(classes, c_name, weight);
                                        }
                                    }
                                    if (c_njet <= njet and c_nbjet == nbjet) // jet-inclusive (nJet) class
                                    {
                                        std::string c_name = fmt::format("{}J+{}BJ+nJ", c_njet, c_nbjet);
                                        eventclass.insert(c_name); // log class name for this event (for plotting)
                                        if (countclasses) // update class count map if classes should be counted
                                        {
                                            update_class_map(classes, c_name, weight);
                                        }
                                    }
                                    if (c_njet <= njet and
                                        c_nbjet <=
                                            nbjet) // all-inclusive (+X) class, the jet-inclusive case is included here
                                    {
                                        std::string c_name = fmt::format("{}J+{}BJ+X", c_njet, c_nbjet);
                                        eventclass.insert(c_name); // log class name for this event (for plotting)
                                        if (countclasses) // update class count map if classes should be counted
                                        {
                                            update_class_map(classes, c_name, weight);
                                        }
                                    }
                                    // note: inclusive classes can be X = 0 or nJet = 0, so the exclusive classes are
                                    // included in the inclusive classes
                                }
                            }
                        }
                        // validation for nJet/nBJet, probably not necessary
                        if (jet_pt.size() < nJet)
                        {
                            throw std::runtime_error(
                                fmt::format("ERROR: Jet vector (size {}) set can not be smaller as nJet (value {}).",
                                            jet_pt.size(),
                                            nJet)
                                    .c_str());
                        }
                        if (bjet_pt.size() < nBJet)
                        {
                            throw std::runtime_error(
                                fmt::format("ERROR: Bjet vector (size {}) set can not be smaller as nBJet (value {}).",
                                            bjet_pt.size(),
                                            nBJet)
                                    .c_str());
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
                                        validation_classes[c_name]->fill(jet_pt,
                                                                         jet_eta,
                                                                         jet_phi,
                                                                         jet_mass,
                                                                         bjet_pt,
                                                                         bjet_eta,
                                                                         bjet_phi,
                                                                         bjet_mass,
                                                                         nelectrons,
                                                                         nmuons,
                                                                         met,
                                                                         weight);
                                    }
                                }
                            }
                        }
                    }
                }
                //*/

                // /* BEFORE CLASS VALIDATION, simple other
                else if (mode == "Jet" or mode == "J")   // fill jet validation files
                {
                    if (nelectrons == 0 and nmuons == 0) // veto any e,mu final states
                    {
                        const float min_pt =
                            std::stof(minpt_argument); // minimum pt [GeV] for jets to be included in validation
                        // ---- 0Jet + X
                        // fill0(nBJet, nJet, nElectron, nMuon, met, weight)
                        jet_val.fill0(nBJet, nJet, nelectrons, nmuons, met, weight);
                        // ---- 1Jet + X
                        // fill1(jet_1, nElectron, nMuon, met, weight)
                        if (jet_pt.size() >= 1)
                        {
                            // create 4-vectors for the leading jet
                            auto jet_1 = Math::PtEtaPhiMVector(jet_pt[0], jet_eta[0], jet_phi[0], jet_mass[0]);
                            if (jet_1.pt() >= min_pt)
                            {
                                jet_val.fill1(jet_1, nelectrons, nmuons, met, weight);
                            }
                        }
                        // ---- 2Jet + X
                        // fill2(jet_1, jet_2, nElectron, nMuon, met, weight)
                        if (jet_pt.size() >= 2)
                        {
                            // create 4-vectors for the two leading jets
                            auto jet_1 = Math::PtEtaPhiMVector(jet_pt[0], jet_eta[0], jet_phi[0], jet_mass[0]);
                            auto jet_2 = Math::PtEtaPhiMVector(jet_pt[1], jet_eta[1], jet_phi[1], jet_mass[1]);

                            if (jet_1.pt() >= min_pt and jet_2.pt() >= min_pt)
                            {
                                // wide mass range
                                jet_val.fill2(jet_1, jet_2, nelectrons, nmuons, met, weight);
                                // Z mass range
                                if ((jet_1 + jet_2).mass() > PDG::Z::Mass - 20. and
                                    (jet_1 + jet_2).mass() < PDG::Z::Mass + 20.)
                                {
                                    jet_val.fill2z(jet_1, jet_2, weight);
                                }
                            }
                        }
                    }
                }
                // */

                // // process monitoring
                // if (event < 10 || (event < 100 && event % 10 == 0) || (event < 1000 && event % 100 == 0) ||
                //     (event < 10000 && event % 1000 == 0) || (event < 100000 && event % 10000 == 0) ||
                //     (event < 1000000 && event % 100000 == 0) || (event < 10000000 && event % 1000000 == 0) ||
                //     (event >= 10000000 && event % 10000000 == 0))
                // {
                //     fmt::print("\n\n[MUSiC Validation] Processed {} events.\n", event);
                //     PrintProcessInfo();
                // }
            }
        }
    }
    else
    {
        std::cout << "The sample " << process << " has an empty histogram!" << std::endl;
    }

    //////////////////// SAVE OUTPUTS ////////////////////
    // remove the files that were not filled but created since the constructor of all validation classes were called,
    // not nice but easy fix :D
    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);
    if (mode == "Lepton" or mode == "L")
    {
        z_to_mu_mu_x.dump_outputs();
        z_to_ele_ele_x.dump_outputs();
        z_to_mu_mu_x_Z_mass.dump_outputs();
        z_to_ele_ele_x_Z_mass.dump_outputs();
        std::system(fmt::format("rm -rf {}/jet_val_{}_{}.root", output_path, process, year).c_str());
        if (plotclasses)
        {
            for (const auto &c_name : to_validate)
            {
                std::system(fmt::format("rm -rf {}/{}_{}_{}.root", output_path, c_name, process, year).c_str());
            }
        }
    }
    else if (mode == "Jet" or mode == "J")
    {
        jet_val.dump_outputs();
        std::system(fmt::format("rm -rf {}/z_to_mu_mu_x_{}_{}.root", output_path, process, year).c_str());
        std::system(fmt::format("rm -rf {}/z_to_ele_ele_x_{}_{}.root", output_path, process, year).c_str());
        std::system(fmt::format("rm -rf {}/z_to_mu_mu_x_Z_mass_{}_{}.root", output_path, process, year).c_str());
        std::system(fmt::format("rm -rf {}/z_to_ele_ele_x_Z_mass_{}_{}.root", output_path, process, year).c_str());
        if (plotclasses)
        {
            for (const auto &c_name : to_validate)
            {
                std::system(fmt::format("rm -rf {}/{}_{}_{}.root", output_path, c_name, process, year).c_str());
            }
        }
    }
    else if (mode == "JetClass" or mode == "JC")
    {
        std::system(fmt::format("rm -rf {}/z_to_mu_mu_x_{}_{}.root", output_path, process, year).c_str());
        std::system(fmt::format("rm -rf {}/z_to_ele_ele_x_{}_{}.root", output_path, process, year).c_str());
        std::system(fmt::format("rm -rf {}/z_to_mu_mu_x_Z_mass_{}_{}.root", output_path, process, year).c_str());
        std::system(fmt::format("rm -rf {}/z_to_ele_ele_x_Z_mass_{}_{}.root", output_path, process, year).c_str());
        std::system(fmt::format("rm -rf {}/jet_val_{}_{}.root", output_path, process, year).c_str());
        // save classes and counts in toml file
        if (countclasses)
        {
            classfile.open(fmt::format("{}/classes_{}_{}.toml", output_path, process, year).c_str());
            classfile << "[counts]\n";
            for (const auto &[c_name, c_count] : classes)
            {
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
    }

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}