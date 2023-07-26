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
    // read in constants from argument (convert to float)
    const double x_section = std::stod(x_section_str);
    const double filter_eff = std::stod(filter_eff_str);
    const double k_factor = std::stod(k_factor_str);
    const double luminosity = std::stod(luminosity_str);

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

    // build jetclass analysis
    if (debugprint)
    {
        std::cout << "Read in class information." << std::endl;
    }
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
    if (to_validate.size() >= 1) // check whether a class validation (create histograms for class) should be executed
    {
        plotclasses = true;
    }
    // ==> classname convention: xJ+yBJ+zMET(+XJ)

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
    // jet classification instances: saved in a map {classname: pointer to jet class validation instance}
    // nl: nominal, up: systematic up, dn: systematic down
    std::map<std::string, JetClass2 *> validation_classes;
    // stores validation classes: {classname: jetclass instance pointer}
    // fill maps
    if (plotclasses) // only if classes should be plotted
    {
        for (const auto &c_name : to_validate)
        {
            validation_classes[c_name] =
                new JetClass2(fmt::format("{}/{}_{}_{}_{}.root", output_path, c_name, shift, process, year), c_name);
            // file format: classname_systname/shift_samplename_year.root (classname includes shift/systname after
            // underscore)
        }
    }

    // classes event counts map: {classname: counts}
    std::map<std::string, double> classes;
    // classes event counts systematic errors map: {classname: squared syst error}
    std::map<std::string, double> classes_stat;

    // quality control to order argument
    const std::set<std::string> allowed_orders{"LO", "NLO", "NNLO", "N3LO"};
    if (!is_data and (allowed_orders.find(process_order) == allowed_orders.end()))
    {
        throw std::runtime_error(fmt::format("Invalid order given: {}", process_order));
    }
    // definition of cross section uncertainties
    std::map<std::string, double> x_sec_uncertainty{
        {"LO", 0.5},
        {"NLO", 0.0},
        {"NNLO", 0.0},
        {"N3LO", 0.0},
    };

    // file where the class counts are stored
    std::ofstream classfile;

    // read in demanded jet triggers and thresholds from argument
    if (debugprint)
    {
        std::cout << "Read in trigger information." << std::endl;
    }
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

        if (debugprint)
        {
            std::cout << std::endl;
        }

        /* EVENT BREAK IF NECESSARY
        if (event > 10000)
        {
            //throw std::runtime_error("finished after given event break");
            break;
        }
        */
        // std::cout << "****************\nEvent No. " << event << std::endl;

        // JET TRIGGER
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
        if (debugprint)
        {
            std::cout << "Passed trigger fire check." << std::endl;
        }

        // BUILD GOOD OBJECTS (selection level objects)
        // muons
        // build good objects
        // dont correct systematics for muons, electrons, photons since they are only used to veto
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
                                                 "Nominal");

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
                                                         "Nominal");

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
                                                     "Nominal");

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
                                                        shift);

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

        // Generate 4-vectors (only for relevant objects)
        auto jets_4vec = jets.p4;
        auto bjets_4vec = bjets.p4;
        auto met_4vec = met.p4;

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
        // if (not((nelectron >= 1 or nmuon >= 1) and nphoton == 0))
        {
            continue; // veto is condition is not satisfied
        }
        if (debugprint)
        {
            std::cout << "Passed object veto." << std::endl;
        }
        //*/

        // CHECK TRIGGER THRESHOLDS
        if (trigger_flags.at(0))                     // PT trigger
        {
            std::vector<float> leading_pt{0.f, 0.f}; // check threshold for pt of leading jet/bjet
            // because jet or bjet could have higher pt, use the following approach
            if (njet >= 1)
            {
                leading_pt.at(0) = jets_4vec.at(0).pt();
            }
            if (nbjet >= 1)
            {
                leading_pt.at(1) = bjets_4vec.at(0).pt();
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
                sum_pt += jets_4vec.at(i).pt();
            }
            for (size_t i = 0; i < nbjet; i++)
            {
                sum_pt += bjets_4vec.at(i).pt();
            }
            if (not(sum_pt >= trigger_thresholds.at(1)))
            {
                continue; // skip this event
            }
        }
        if (debugprint)
        {
            std::cout << "Passed trigger threshold checks.\nCalculate event weight." << std::endl;
        }

        // CALCULATE EVENT WEIGHT
        // Here goes the real analysis...
        // get effective event weight
        double weight = 1.;

        if (not(is_data))
        {
            auto pu_weight = pu_corrector->evaluate({unwrap(Pileup_nTrueInt), Shifts::get_pu_variation(shift)});

            auto prefiring_weight = Shifts::get_prefiring_weight( //
                unwrap(L1PreFiringWeight_Nom, 1.),                //
                unwrap(L1PreFiringWeight_Up, 1.),                 //
                unwrap(L1PreFiringWeight_Dn, 1.),                 //
                shift);

            auto scaled_luminosity = Shifts::scale_luminosity(luminosity, shift);

            auto pdf_as_weight = Shifts::get_pdf_alpha_s_weights(shift,
                                                                 lha_indexes,
                                                                 default_pdf_sets,           //
                                                                 unwrap(_LHEPdfWeight),      //
                                                                 unwrap(Generator_scalePDF), //
                                                                 unwrap(Generator_x1),       //
                                                                 unwrap(Generator_x2),       //
                                                                 unwrap(Generator_id1),      //
                                                                 unwrap(Generator_id2));
            // unwrap(_LHEWeight_originalXWGTUP, 1.f),);

            // xSecOrder uncertainty for LO
            double scalingfactor_xsec = 1.0;
            if (shift == "xSecOrder_Up")
            {
                scalingfactor_xsec = (1.0 + x_sec_uncertainty[process_order]);
            }
            else if (shift == "xSecOrder_Down")
            {
                scalingfactor_xsec = (1.0 - x_sec_uncertainty[process_order]);
            }

            weight = unwrap(gen_weight, 1.) //
                     * pu_weight            //
                     * prefiring_weight     //
                     * x_section            //
                     * scalingfactor_xsec   //
                     * filter_eff           //
                     * k_factor             //
                     * scaled_luminosity    //
                     * pdf_as_weight        //
                     / no_cuts;

            /*/
            // bug search: check for nan
            if (not (isnormal(weight)))
            {
                throw std::runtime_error(fmt::format("not normal weight: {}, {}", weight, pdf_as_weight));
            }
            */
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////    ADD SOME SPECIFIC PROCESSING                                      /////////////////////////
        ////////////////                                                                      /////////////////////////
        ////////////////    Here special object selection and processing can be implemented   /////////////////////////
        ////////////////    Of the systematics is already taken care of                       /////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////    Specific processing name:   //////      ???????????????           /////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /*
        // NAME: MUSIC WIDE JETS

        // configuration of parameters
        const float seed_threshold = 150.0;        // select seeds // determined best value: 250.0
        const float delta_r_threshold = 1.1;       // merge non seeds // fixed
        const float delta_eta_max_threshold = 1.8; // delta eta max cut // determined best value: 1.4

        // SELECT SEED JETS
        auto seed_jets = RVec<Math::PtEtaPhiMVector>{};   // jet seeds
        auto noseed_jets = RVec<Math::PtEtaPhiMVector>{}; // jets that are no seeds
        for (size_t i = 0; i < jets_4vec.size(); i++)
        {
            if (jets_4vec.at(i).pt() > seed_threshold) // select as seed
            {
                seed_jets.push_back(jets_4vec.at(i));
            }
            else // don't select as seed
            {
                noseed_jets.push_back(jets_4vec.at(i));
            }
        }

        // JET MERGING TO WIDE JETS
        auto widejets = seed_jets;                           // wide jets
        if (noseed_jets.size() > 0 and seed_jets.size() > 0) // only try to merge if seed and noseed jets were found
        {
            for (size_t i = 0; i < noseed_jets.size(); i++)  // try to merge for each noseed jet
            {
                auto cur_jet = noseed_jets.at(i);
                auto all_delta_r = RVec<float>{};
                for (size_t j = 0; j < seed_jets.size(); j++) // try to merge to every seed jet
                {
                    // calculate distances to wide jets
                    all_delta_r.push_back(std::abs(Math::VectorUtil::DeltaR(seed_jets.at(j), cur_jet)));
                }
                // sort after shortest distance
                auto sortidx_delta_r = VecOps::Argsort(all_delta_r, // sort, smallest element first
                                                       [](auto p1, auto p2) -> bool
                                                       {
                                                           return p1 < p2;
                                                       });
                // std::cout << "WJ MERGING: DeltaR=" << all_delta_r << ", DeltaRMin=" <<
                // all_delta_r.at(sortidx_delta_r.at(0)) << ", Merge=" << (all_delta_r.at(sortidx_delta_r.at(0)) < 1.1)
                // << ", MergeIdx=" << sortidx_delta_r.at(0) << std::endl;
                if (all_delta_r.at(sortidx_delta_r.at(0)) < delta_r_threshold) // check if smallest deltar is < 1.1
                {
                    widejets.at(sortidx_delta_r.at(0)) += cur_jet; // if so, add the current jet to the closest widejet
                }
            }
        }

        // DO DELTA ETA MAX CUT
        if (widejets.size() > 1) // only try delta eta max cut if at least two wide jets exist
        {
            auto all_widejet_delta_eta = RVec<float>{};
            for (size_t i = 0; i < widejets.size(); i++)     // try to merge to every seed jet
            {
                for (size_t j = 0; j < widejets.size(); j++) // try to merge to every seed jet
                {
                    // calculate delta eta between all wide jets
                    if (i < j) // exclude double calculations and exclude two same jets
                    {
                        all_widejet_delta_eta.push_back(std::abs(widejets.at(i).eta() - widejets.at(j).eta()));
                    }
                }
            }
            auto sortidx_delta_eta = VecOps::Argsort(all_widejet_delta_eta, // sort, largest element first
                                                     [](auto p1, auto p2) -> bool
                                                     {
                                                         return p1 > p2;
                                                     });
            // std::cout << "WJ DEL ETA: DeltaEta=" << all_widejet_delta_eta << ", DeltaEtaMax=" <<
            // all_widejet_delta_eta.at(sortidx_delta_eta.at(0)) << ", Accept=" <<
            // (all_widejet_delta_eta.at(sortidx_delta_eta.at(0)) < 1.8) << std::endl;
            if (not(all_widejet_delta_eta.at(sortidx_delta_eta.at(0)) <
                    delta_eta_max_threshold)) // check if largest delta eta is below threshold
            {
                continue;                     // if not, reject event
            }
        }

        // REORDER WIDEJETS AFTER PT
        const auto wjets_reordering_mask = VecOps::Argsort(widejets,
                                                           [](auto wjet_1, auto wjet_2) -> bool
                                                           {
                                                               return wjet_1.pt() > wjet_2.pt();
                                                           });
        auto widejets_sorted = VecOps::Take(widejets, wjets_reordering_mask);

        // refer to wide jets as jets (formal change for plotting and classification)
        njet = widejets_sorted.size();
        jets_4vec = widejets_sorted;

        // effectively all other jets that are not merged are rejected
        */
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // JET CLASS VALIDATION AND CLASS COUNTING
        std::set<std::string> eventclass =
            {}; // set that includes all classes the current sample is a member of (for plotting)
        // find all classes for the event
        // event class inhabitation is counted in the map classes {classname: counts}
        // all event classes for current event are stored in the set eventclass {classnames}
        if (debugprint)
        {
            std::cout << "ACCEPTED EVENT.\nStart event class loop." << std::endl;
        }
        for (int c_njet = (int)njet; c_njet >= 0; c_njet--)
        {
            for (int c_nbjet = (int)nbjet; c_nbjet >= 0; c_nbjet--)
            {
                if ((c_njet == 0 and c_nbjet == 0) == false) // skip 0 jet class
                {
                    // std::cout << "Next loop iteration." << std::endl;
                    if (c_njet == (int)njet and c_nbjet == (int)nbjet) // exclusive class
                    {
                        // differentiate met in classname
                        std::string c_name = fmt::format("{}J+{}BJ+0MET", c_njet, c_nbjet);
                        if (is_met)
                        {
                            c_name = fmt::format("{}J+{}BJ+1MET", c_njet, c_nbjet);
                        }
                        eventclass.insert(c_name);                     // log class name
                    }
                    if (c_njet <= (int)njet and c_nbjet <= (int)nbjet) // jet- and bjet-inclusive class (+XJ)
                    {
                        // differentiate met in classname
                        std::string c_name = fmt::format("{}J+{}BJ+0MET+XJ", c_njet, c_nbjet);
                        if (is_met)
                        {
                            c_name = fmt::format("{}J+{}BJ+1MET+XJ", c_njet, c_nbjet);
                        }
                        eventclass.insert(c_name); // log class name
                    }
                    // note: inclusive classes can be XJ = 0, so the exclusive classes are
                    // included in the inclusive classes
                }
            }
        }

        // fill histograms and weight sum
        for (const auto &c_name : eventclass)
        {
            if (plotclasses) // fill histograms
            {
                for (const auto &c_name_toval : to_validate)
                {
                    if (c_name == c_name_toval) // check whether one of the classes to plot is in this event
                    {
                        validation_classes[c_name]->fill(jets_4vec, bjets_4vec, nelectron, nmuon, met_4vec, weight);
                    }
                }
            }
            if (countclasses) // fill global weight sum for each class
            {
                if (classes.find(c_name) == classes.end() and
                    classes_stat.find(c_name) == classes_stat.end()) // it not in map, create new entry with counts 0
                {
                    classes.insert({c_name, 0.f});
                    classes_stat.insert({c_name, 0.f});
                }
                classes[c_name] += weight;               // add weight to class count
                classes_stat[c_name] += weight * weight; // add weight^2 to syst_err^2
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
    // ==> class counts are stored in file classes_systname/shift_process_year.toml (so for each syst in a separate
    // file) the class counts for this systematic are stored with the key [counts] the stat error is included in each
    // file with key [stat]
    if (countclasses)
    {
        classfile.open(fmt::format("{}/classes_{}_{}_{}.toml", output_path, shift, process, year).c_str());
        // fill nominal and systematics
        classfile << "\n\n["
                  << "counts"
                  << "]\n";
        for (auto &[c_name, c_count] : classes)
        {
            /* // dont do this for now
            if (c_count < 0) // if negative weights dominate, set count to 0
            {
                c_count = 0;
            }
            */
            classfile << "\"" << c_name << "\" = " << std::to_string(c_count) << "\n";
        }
        // fill stat err
        classfile << "\n\n["
                  << "stat"
                  << "]\n";
        for (auto &[c_name, c_stat] : classes_stat)
        {
            classfile << "\"" << c_name << "\" = " << std::to_string(c_stat) << "\n";
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