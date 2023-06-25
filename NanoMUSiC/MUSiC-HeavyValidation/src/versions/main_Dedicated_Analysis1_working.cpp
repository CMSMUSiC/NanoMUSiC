//////////////////////////////////////////////////////////////
///////////       RECREATION OF DEDICATED ANALYSIS ///////////
///////////           without systematics          ///////////
//////////////////////////////////////////////////////////////
// analysis inspired by CMS AN-19-073

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

    JetClass2* validation_class;
    validation_class = new JetClass2(
        fmt::format("{}/{}_{}_{}_{}.root", output_path, "2widejet", "nominal", process, year), "2J+0BJ+0MET");

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
        float weight = 1.f;

        if (not(is_data))
        {
            float const_weights = 1.f;
            std::map<std::string, float> pu_weight;
            pu_weight["nominal"] = pu_corrector->evaluate({unwrap(Pileup_nTrueInt), "nominal"});

            // calculate event weight
            weight = const_weights * unwrap(gen_weight) * pu_weight["nominal"] * generator_filter /
                                     no_cuts / generator_filter * effective_x_section;
            // weight = const_weights * gen_weight * pu_weight * xsection * filter_eff * k_factor * luminosity /
            // no_cuts | python calculates: effective_x_section = xsection * filter_eff * k_factor * luminosity
            // (is the weighting formula of the MUSiC AN p.9)
        }

        // JET TRIGGER
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
        if (not(nelectron == 0 and nmuon == 0 and nphoton == 0)) // veto all leptons, photons
        {
            continue;                                            // veto is condition is not satisfied
        }
        //*/

        // Difference in object reconstruction:
        // They use pT > 30 and |eta| < 2.5 and NormalJetID
        // We use pT > 50 and |eta| < 2.4 and TightJetID

        // They do not perform b-tagging
        // We for now use only jets and ignore bjets

        // Displayed classname is 0MET however they simply dont care about met and do not explicitly veto it
        // We are also not vetiong MET the classname is just to use the already implemented classnames and plotting tools

        // DIJET REQUIREMENT
        if (not(njet >= 2))
        {
            continue;
        }

        // JET MERGING TO WIDE JETS
        auto jetseeds = ROOT::VecOps::Take(jets, 2); // jet seeds (first two leading jets)
        auto widejets = ROOT::VecOps::Take(jets, 2); // wide jets (there the not-leading jets might be added)
        if(njet > 2) // try to merge when there are more than two jets
        {
            auto not_leading = ROOT::VecOps::Take(jets, -(jets.size() - 2)); // get all other subleading jets
            for(size_t i = 0; i < not_leading.size(); i++)
            {
                auto cur_jet = not_leading.at(i);
                for (size_t j = 0; j < jetseeds.size(); j++)
                {
                    // try to merge non-leading jets to the seeds
                    // priotize the leading seed
                    if(std::abs(Math::VectorUtil::DeltaR(jetseeds.at(j), cur_jet)) < 1.1)
                    {
                        widejets.at(j) += cur_jet;
                        continue; // do not try to merge the same jet twice
                    }
                }
            }
        }

        // DELTA ETA FILTER FOR WIDE JETS
        // Only use the SR (signal region) of their analysis
        float delta_eta = std::abs(widejets.at(0).eta() - widejets.at(1).eta());
        if (not(delta_eta < 1.1))
        {
            continue;
        }

        ///*// optional: WIDE JET MASS CUT
        // dijet mass cut > 1530 GeV 
        auto widedijetsum = Math::PtEtaPhiMVector(0, 0, 0, 0);
        for(size_t i = 0; i < widejets.size(); i++)
        {
            widedijetsum += widejets.at(i);
        }
        if(not(widedijetsum.mass() > 1530))
        {
            continue;
        }
        //*/


        // FILL PLOTS
        met = RVec<Math::PtEtaPhiMVector>{};
        // dont fill met since we are only interested in the jets and the classname does not allow met
        validation_class->fill(widejets, bjets, nelectron, nmuon, met, weight); // fill histograms

        // std::cout << "Finished event classification." << std::endl;
    }

    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);
    // SAVE OUTPUTS
    // save the validation example classes
    validation_class->dump_outputs();

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}