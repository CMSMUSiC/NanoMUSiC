#include "ec_scanner.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

auto make_shifts() -> std::pair<std::unordered_map<std::string, std::vector<double>>, unsigned int>
{
    std::unordered_map<std::string, std::vector<double>> shifts;
    unsigned int n_shifts = 1000;
    std::vector<double> a(n_shifts, 0);
    srand(time(0));
    std::generate(a.begin(), a.end(), rand);
    std::vector<double> b(n_shifts, 0);
    srand(time(0));
    std::generate(b.begin(), b.end(), rand);

    return {{{"a", a}, {"b", b}}, n_shifts};
}

int main(int argc, char *argv[])
{
    TH1::AddDirectory(false);
    TDirectory::AddDirectory(false);
    gROOT->SetBatch(true);
    ROOT::EnableThreadSafety();

    // argument parsing
    auto args = arg_parse(argc, argv);

    // main loop
    for (auto &&year : args.years)
    {
        auto input_files = get_source_files(args.histograms_dir, year, 11);
        if (input_files.size() == 0)
        {
            fmt::print(stderr, "ERROR: Input file list is empty.\n");
            exit(EXIT_FAILURE);
        }

        fmt::print("Creating EC Collection ...\n");
        auto ec_collection = NanoEventClassCollection(input_files, args.classes);

        fmt::print("Build scanner ...\n");
        auto scanner = Scanner();

        fmt::print("Building Distributions ...\n");
        auto [shifts, n_shifts] = make_shifts();

        auto distributions = distribution_factory(ec_collection, false, false);
        for (auto dist : distributions)
        {
            auto scan_results = scanner.scan(dist, shifts, n_shifts);
            fmt::print("{}\n", ScanResult::header());
            for (std::size_t i = 1; i < scan_results.size(); i++)
            {
                if (i <= 6 or i >= scan_results.size() - 5)
                {
                    fmt::print("{}\n", scan_results[i].to_string());
                }
                if (i == 7)
                {
                    fmt::print("[...]\n");
                }
            }
            fmt::print("\n");
        }

        fmt::print("Done.\n");
    }

    //
    return EXIT_SUCCESS;
}
