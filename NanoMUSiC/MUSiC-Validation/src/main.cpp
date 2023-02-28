#include "TH1.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "Validation.hpp"

auto main(int argc, char *argv[]) -> int
{
    fmt::print("\n\n\n\n\nMUSiC Validation\n\n");

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    const std::string config_file = cmdl({"-c", "--config"}).str();

    if (show_help or config_file == "")
    {
        fmt::print("Usage: validation [-h|--help] --config <config_file>\n");
        fmt::print("          -h|--help: Shows this message.\n");
        fmt::print("          -c|--config: Input file.\n");
        return (config_file == "" ? -1 : 0);
    }

    // task configuration
    const auto task_config = TOMLConfig::make_toml_config(config_file);
    const auto output_folder = task_config.get<std::string>("output");
    const auto is_data = task_config.get<bool>("is_data");
    const auto x_section_file(MUSiCTools::parse_and_expand_music_base(task_config.get<std::string>("x_section_file")));
    const auto process = task_config.get<std::string>("process");
    const auto input_files = task_config.get_vector<std::string>("input_files");

    fmt::print("********************* Task Configuration *********************\n");
    fmt::print("Process: {}\n", process);
    fmt::print("Output Folder: {}\n", output_folder);
    fmt::print("Is Data(?): {}\n", is_data);
    // fmt::print("Cross-sections file: {}\n", x_section_file);
    fmt::print("**************************************************************\n");

    fmt::print("Loading input files ...\n");
    auto chain = TChain("nano_music");
    for (auto &&file : input_files)
    {
        chain.Add(file.c_str());
    }

    // create tree reader and add values and arrays
    auto tree_reader = TTreeReader(&chain);

    ADD_VALUE_READER(nMuon, unsigned int);
    // auto nMuon = TTreeReaderValue<unsigned int>(tree_reader, "nMuon");
    ADD_ARRAY_READER(Muon_pt, float);

    fmt::print("Starting timer  ...\n");
    auto dTime1 = getCpuTime(); // Start Timer

    // launch event loop for Data or MC
    fmt::print("Launching event loop ...\n");

    auto h = TH1F("foo", "bar", 10, 0., 10.);
    for (auto &&event : tree_reader)
    {
        h.Fill(unwrap(nMuon));
        // std::cout << unwrap(Muon_pt) << std::endl;
        // process monitoring
        if (event < 10 || (event < 100 && event % 10 == 0) || (event < 1000 && event % 100 == 0) ||
            (event < 10000 && event % 1000 == 0) || (event >= 10000 && event % 10000 == 0))
        {
            // PrintProcessInfo();
        }
    }

    PrintProcessInfo();
    return 0;
}