//////////////////////////////////////////////////////////////
///////////       JET CLASS VALIDATION CODE        ///////////
///////////           with systematics!            ///////////
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

// for isnan
#include <math.h>

// update class count
inline auto update_class(std::set<std::string> &eventclass,
                         std::map<std::string, float> &classes,
                         std::map<std::string, float> &classes_stat,
                         const std::string &c_name,
                         const bool countclasses,
                         float &weight) -> void
{
    eventclass.insert(c_name); // log class name for this event (for plotting)
    if (countclasses)          // update class count map if classes should be counted
    {
        // create class counter if not there
        if (classes.find(c_name) == classes.end()) // it not in map, create new entry with counts 0
        {
            classes.insert({c_name, 0.f});
            classes_stat.insert({c_name, 0.f});
        }
        // add weight to class
        classes[c_name] += weight;               // add weight to class count
        classes_stat[c_name] += weight * weight; // add weight^2 to syst_err^2
    }
}

auto printpt(RVec<Math::PtEtaPhiMVector> vector) -> std::string
{
    std::string output = "{";
    for (size_t i = 0; i < vector.size(); i++)
    {
        output.append(fmt::format("{},\n", vector.at(i).pt()));
    }
    output.append("},");
    return output;
}

auto printeta(RVec<Math::PtEtaPhiMVector> vector) -> std::string
{
    std::string output = "{";
    for (size_t i = 0; i < vector.size(); i++)
    {
        output.append(fmt::format("{},\n", vector.at(i).eta()));
    }
    output.append("},");
    return output;
}

auto printphi(RVec<Math::PtEtaPhiMVector> vector) -> std::string
{
    std::string output = "{";
    for (size_t i = 0; i < vector.size(); i++)
    {
        output.append(fmt::format("{},\n", vector.at(i).phi()));
    }
    output.append("},");
    return output;
}

auto printvec(RVec<float> vector) -> std::string
{
    std::string output = "{";
    for (size_t i = 0; i < vector.size(); i++)
    {
        output.append(fmt::format("{},\n", vector.at(i)));
    }
    output.append("},");
    return output;
}

// string parser (separate string to set/vector of substrings)
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

// string utils
auto starts_with(const std::string &str, const std::string &prefix) -> bool
{
    return str.compare(0, prefix.length(), prefix) == 0;
}

// trigger filter check
inline auto trigger_filter(const std::string &process,
                           bool is_data,
                           bool pass_low_pt_muon_trigger,
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

// jet trigger filter check
inline auto jets_trigger_filter(bool pass_jet_ht_trigger, bool pass_jet_pt_trigger) -> bool
{
    return pass_jet_ht_trigger or pass_jet_pt_trigger;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// main function
auto main(int argc, char *argv[]) -> int
{
    bool debugprint = false; // print debug messages flag
    if (debugprint)
    {
        std::cout << "Start validation code." << std::endl;
    }

    // set SumW2 as default
    TH1::SetDefaultSumw2(true);

    // command line options, parse arguments
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
    const std::string process_order = cmdl({"-po", "--process_order"}).str();
    const std::string process_group = cmdl({"-pg", "--process_group"}).str();
    const std::string input_file = cmdl({"-i", "--input"}).str();
    const std::string trigger_argument = cmdl({"-trg", "--trigger"}).str();
    const std::string tv_argument = cmdl({"-tv", "--tovalidate"}).str();
    const std::string shift = cmdl({"-s", "--shift"}).str();
    if (show_help or process == "" or year == "" or output_path == "" or input_file == "" or x_section_str == "" or
        filter_eff_str == "" or k_factor_str == "" or luminosity_str == "" or process_order == "" or
        process_group == "" or shift == "" or trigger_argument == "" or tv_argument == "")
    {
        fmt::print("Usage: heavy_validation [OPTIONS]\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -p|--process: Process (aka sample).\n");
        fmt::print("          -y|--year: Year.\n");
        fmt::print("          -d|--is_data: Is data ?\n");
        fmt::print("          -o|--output: Output path.\n");
        fmt::print("          -x|--xsection: Cross-section.\n");
        fmt::print("          -f|--filter_eff: Filter efficiency.\n");
        fmt::print("          -k|--k_factor: K-Factor.\n");
        fmt::print("          -l|--luminosity: Integrated luminosity.\n");
        fmt::print("         -po|--process_order: Process order.\n");
        fmt::print("         -pg|--process_group: Process group.\n");
        fmt::print("        -trg|--trigger: Specify trigger and lower limits, e.g. HT1600/PT600.\n");
        fmt::print(
            "         -tv|--tovalidate: Names of the classes that should be plotted. Seperate "
            "Classnames by comma without spaces. Class name format is 'xJ+yBJ+zMET[+XJ]' for "
            "exclusive [_] / jet- and bjet-inclusive [+XJ] classes (with z = 0, 1). "
            "Use class name 'COUNTS' to also create class inhabitation file (event counts per class).\n");
        fmt::print("          -i|--input: Path to a txt with input files (one per line).\n");
        fmt::print("          -s|--shift: Shift being applied (systematics).\n");

        exit(-1);
    }
    // arguments for event info
    const std::string event_argument = cmdl({"-evt", "--event"}).str();
    const std::string lumisection_argument = cmdl({"-lsec", "--lumisection"}).str();
    const std::string run_argument = cmdl({"-run", "--runnumber"}).str();
    if (event_argument == "" or lumisection_argument == "" or run_argument == "")
    {
        throw std::runtime_error("No event, lumisection or runnumber was given although required.");
    }
    const unsigned long event_tofind = std::stoul(event_argument);
    const unsigned long lumisection_tofind = std::stoul(lumisection_argument);
    const unsigned long run_tofind = std::stoul(run_argument);

    // read in constants from argument (convert to float)
    const float x_section = std::stof(x_section_str);
    const float filter_eff = std::stof(filter_eff_str);
    const float k_factor = std::stof(k_factor_str);
    const float luminosity = std::stof(luminosity_str);

    if (debugprint)
    {
        std::cout << "-> Processing sample " << process << "..." << std::endl;
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
    ADD_VALUE_READER(L1PreFiringWeight_Up, float);
    ADD_VALUE_READER(L1PreFiringWeight_Dn, float);
    ADD_VALUE_READER(L1PreFiringWeight_Nom, float);

    ADD_ARRAY_READER(_LHEPdfWeight, float);
    ADD_VALUE_READER(Generator_scalePDF, float);
    ADD_VALUE_READER(Generator_x1, float);
    ADD_VALUE_READER(Generator_x2, float);
    ADD_VALUE_READER(Generator_id1, int);
    ADD_VALUE_READER(Generator_id2, int);
    //  ADD_VALUE_READER(_LHEWeight_originalXWGTUP, double); // fix error with the current skimmed dataset

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

    ADD_VALUE_READER(run, UInt_t);
    ADD_VALUE_READER(luminosityBlock, UInt_t);
    ADD_VALUE_READER(event, ULong64_t);

    // build jetclass analysis
    if (debugprint)
    {
        std::cout << "Read in class information." << std::endl;
    }

    // systematics prefixes (systnames): Nominal or Syst_Up/Down stored in Shifts class
    const auto shifts = Shifts(is_data);
    if (not(shifts.is_valid(shift)))
    {
        throw std::invalid_argument(fmt::format("ERROR: Could not validate shift: {}.", shift));
    }

    // create the classification instances
    if (debugprint)
    {
        std::cout << "Initialize the custom JetClass validation instances and files." << std::endl;
    }
    // classes event info map
    std::map<std::string, std::set<std::map<std::string, unsigned long>>> classes_events;

    // quality control to order argument
    const std::set<std::string> allowed_orders{"LO", "NLO", "NNLO", "N3LO"};
    if (!is_data and (allowed_orders.find(process_order) == allowed_orders.end()))
    {
        throw std::runtime_error(fmt::format("Invalid order given: {}", process_order));
    }
    // definition of cross section uncertainties
    std::map<std::string, float> x_sec_uncertainty{
        {"LO", 0.5},
        {"NLO", 0},
        {"NNLO", 0},
        {"N3LO", 0},
    };

    // file where the class counts are stored
    std::ofstream classfile;
    // file where the class event info are stored
    std::ofstream classeventfile;

    // read in demanded jet triggers and thresholds from argument
    if (debugprint)
    {
        std::cout << "Read in trigger information." << std::endl;
    }

    // build corrections
    if (debugprint)
    {
        std::cout << "Build corrections." << std::endl;
    }
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
    if (debugprint)
    {
        std::cout << "Build LHAPDF." << std::endl;
    }
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

    // read cutflow
    if (debugprint)
    {
        std::cout << "Read cutflow histogram." << std::endl;
    }
    const auto cutflow_file =
        std::unique_ptr<TFile>(TFile::Open(fmt::format("{}/cutflow_{}_{}.root", output_path, process, year).c_str()));
    const auto cutflow_histo = cutflow_file->Get<TH1F>("cutflow");
    // const auto total_generator_weight = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorWeight") + 1);
    const auto no_cuts = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("NoCuts") + 1);
    const auto generator_filter = cutflow_histo->GetBinContent(Outputs::Cuts.index_of("GeneratorFilter") + 1);

    // Status message to trace the process
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "[ MUSiC Heavy Validation (C++ process) ]" << std::endl;
    std::cout << "Process/Sample: " << process << std::endl;
    std::cout << "Year: " << year << std::endl;
    std::cout << "Output path: " << output_path << std::endl;
    std::cout << "Input file: " << input_file << std::endl;
    std::cout << "Cross section: " << x_section_str << std::endl;
    std::cout << "Filter Efficiency: " << filter_eff_str << std::endl;
    std::cout << "K Factor: " << k_factor_str << std::endl;
    std::cout << "Luminosity: " << luminosity_str << std::endl;
    std::cout << "Process order: " << process_order << std::endl;
    std::cout << "Process group: " << process_group << std::endl;
    std::cout << "Shift: " << shift << std::endl;
    std::cout << "Trigger argument: " << trigger_argument << std::endl;
    std::cout << "Classes argument: " << tv_argument << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////         EVENT LOOP ////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (debugprint)
    {
        std::cout << "Start event loop." << std::endl;
    }
    //  launch event loop for Data or MC
    for (auto &&event_tree : tree_reader)
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

        (void)event_tree; // remove the "unused variable" warning during compilation

        if (debugprint)
        {
            std::cout << std::endl;
        }

        /* EVENT BREAK IF NECESSARY
        if (event_tree > 10000)
        {
            throw std::runtime_error("finished after given event break");
            break;
        }
        */
        // std::cout << "****************\nEvent No. " << event << std::endl;

        // JET TRIGGER HT
        bool is_good_trigger = false;
        is_good_trigger = is_good_trigger || unwrap(pass_jet_ht_trigger);
        if (not(is_good_trigger))
        {
            continue; // skip if no trigger fired
        }
        if (debugprint)
        {
            std::cout << "Passed trigger fire check." << std::endl;
        }

        // BUILD GOOD OBJECTS (selection level objects)

        auto run_current = (unsigned long)unwrap(run);
        auto luminosityBlockcurrent = (unsigned long)unwrap(luminosityBlock);
        auto event_current = (unsigned long)unwrap(event);

        auto Muon_pt_unwrapped = unwrap(Muon_pt);
        auto Electron_pt_unwrapped = unwrap(Electron_pt);
        auto Photon_pt_unwrapped = unwrap(Photon_pt);
        auto Jet_pt_unwrapped = unwrap(Jet_pt);

        if (run_tofind == run_current and lumisection_tofind == luminosityBlockcurrent and
            event_tofind == event_current)
        {
            classeventfile.open(fmt::format("{}/eventinfo_{}_{}_{}.toml", output_path, shift, process, year).c_str());
            classeventfile << "[event_info]\n";
            classeventfile << "\"run\" = " << run_tofind << "\n";
            classeventfile << "\"lumi\" = " << lumisection_tofind << "\n";
            classeventfile << "\"event\" = " << event_tofind << "\n";

            classeventfile << "\n\n[pf_electrons_pt]\n";
            classeventfile << printvec(Electron_pt_unwrapped);
            classeventfile << "\n[pf_photons_pt]\n";
            classeventfile << printvec(Photon_pt_unwrapped);
            classeventfile << "\n[pf_muons_pt]\n";
            classeventfile << printvec(Muon_pt_unwrapped);
            classeventfile << "\n[pf_jets_pt]\n";
            classeventfile << printvec(Jet_pt_unwrapped);
        }

        auto muons = ObjectFactories::make_muons(Muon_pt_unwrapped,           //
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
                                                 "Nominal");

        auto electrons = ObjectFactories::make_electrons(Electron_pt_unwrapped,          //
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
                                                         "Nominal");

        auto photons = ObjectFactories::make_photons(Photon_pt_unwrapped,        //
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
                                                     "Nominal");

        auto [jets, bjets] = ObjectFactories::make_jets(Jet_pt_unwrapped,               //
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
                                                        shift);

        if (run_tofind == run_current and lumisection_tofind == luminosityBlockcurrent and
            event_tofind == event_current)
        {
            classeventfile << "\n\n[uncleared_electrons_pt]\n";
            classeventfile << printpt(electrons.p4);
            classeventfile << "\n[uncleared_photons_pt]\n";
            classeventfile << printpt(photons.p4);
            classeventfile << "\n[uncleared_muons_pt]\n";
            classeventfile << printpt(muons.p4);
            classeventfile << "\n[uncleared_jets_pt]\n";
            classeventfile << printpt(jets.p4);
        }

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
                                             shift);

        if (run_tofind == run_current and lumisection_tofind == luminosityBlockcurrent and
            event_tofind == event_current)
        {
            classeventfile << "\n\n[cleared_electrons_pt]\n";
            classeventfile << printpt(electrons.p4);
            classeventfile << "\n[cleared_photons_pt]\n";
            classeventfile << printpt(photons.p4);
            classeventfile << "\n[cleared_muons_pt]\n";
            classeventfile << printpt(muons.p4);
            classeventfile << "\n[cleared_jets_pt]\n";
            classeventfile << printpt(jets.p4);
            classeventfile << "\n[cleared_jets_eta]\n";
            classeventfile << printeta(jets.p4);
            classeventfile << "\n[cleared_jets_phi]\n";
            classeventfile << printphi(jets.p4);
            classeventfile.close();
            exit(0);
        }
    }

    fmt::print("\n[MUSiC Validation] Saving outputs ({} - {} - {}) ...\n", output_path, process, year);

    fmt::print("\n[MUSiC Validation] Done ...\n");
    PrintProcessInfo();

    return EXIT_SUCCESS;
}