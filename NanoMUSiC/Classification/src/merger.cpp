#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "fmt/format.h"

#include "TFile.h"
#include "TH1F.h"
#include "TKey.h"

using namespace ROOT;

auto merger(const std::vector<std::string> &input_files, const std::string &output_file) -> void
{
    if (input_files.size() == 1)
    {
        std::unique_ptr<TFile> root_file(TFile::Open(input_files[0].c_str()));
        if (not(root_file->Cp(output_file.c_str(), false)))
        {
            fmt::print(
                stderr, "ERROR: Could not copy source file ({}) to destination ({}).", input_files[0], output_file);
            std::exit(EXIT_FAILURE);
        }

        return;
    }
    auto histos = std::unordered_map<std::string, std::unique_ptr<TH1F>>();
    for (auto &&file_path : input_files)
    {
        std::unique_ptr<TFile> root_file(TFile::Open(file_path.c_str()));

        TIter keyList(root_file->GetListOfKeys());
        TKey *key;
        while ((key = (TKey *)keyList()))
        {
            auto full_name = std::string(key->GetName());

            if (histos.find(full_name) == histos.end())
            {
                histos.insert({full_name, std::unique_ptr<TH1F>(static_cast<TH1F *>(key->ReadObj()))});
            }
            else
            {
                histos[full_name]->Add(static_cast<TH1F *>(key->ReadObj()));
            }
        }
    }

    std::unique_ptr<TFile> output_root_file(TFile::Open(output_file.c_str(), "RECREATE", "", 0, 0));

    for (auto &&[name, histo] : histos)
    {
        output_root_file->WriteObject(histo.get(), name.c_str());
    }
}

auto main(int argc, char *argv[]) -> int
{
    TH1::AddDirectory(false);
    TDirectory::AddDirectory(false);

    if (argc < 3)
    {
        fmt::print(stderr, "ERROR: Could not merge files.\nUsage: {} <output> <input1> <input2> ...\n", argv[0]);
        std::exit(EXIT_FAILURE);
    }

    std::string output_file = argv[1];

    std::vector<std::string> inputs_files = {};
    for (int i = 2; i < argc; i++)
    {
        inputs_files.push_back(argv[i]);
    }

    merger(inputs_files, output_file);
    fmt::print("Done: {}\n", output_file);

    return EXIT_SUCCESS;
}