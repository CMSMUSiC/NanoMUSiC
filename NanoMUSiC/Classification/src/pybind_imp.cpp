#include "Classification.hpp"
#include "Distribution.hpp"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include <cstdlib>
#include <pybind11/pytypes.h>
namespace py = pybind11;
using namespace pybind11::literals;

PYBIND11_MODULE(classification_imp, m)
{
    m.def("classification",
          &classification,
          "process"_a,
          "year"_a,
          "is_data"_a,
          "x_section"_a,
          "filter_eff"_a,
          "k_factor"_a,
          "luminosity"_a,
          "xs_order"_a,
          "process_group"_a,
          "sum_weights_json_filepath"_a,
          "input_file"_a,
          "generator_filter"_a,
          "event_classes"_a,
          "validation_container"_a,
          "first_event"_a = std::nullopt,
          "last_event"_a = std::nullopt,
          "debug"_a = false,
          "Entry point for classification code.");
    m.doc() = "python bindings for classification";

    py::class_<EventClassContainer>(m, "EventClassContainer")
        .def(py::init<>())
        .def_static(
            "save",
            [](EventClassContainer &cont, const std::string &object_name, const std::string &output_path) -> void
            {
                std::unique_ptr<TFile> output_file(TFile::Open(output_path.c_str(), "RECREATE"));
                output_file->WriteObject(&cont, object_name.c_str());
            },
            "event_classes_container"_a,
            "object_name"_a,
            "output_path"_a)
        .def_static(
            "merge_many",
            [](const std::string &object_name,
               const std::vector<std::string> &input_paths,
               const std::string &output_path) -> void
            {
                if (input_paths.size() == 0)
                {
                    return;
                }

                auto input_file = std::unique_ptr<TFile>(TFile::Open(input_paths[0].c_str()));
                auto buffer = input_file->Get<EventClassContainer>(object_name.c_str());
                if (not(buffer))
                {
                    fmt::print(stderr, "Could not find the requested EventClassContainer: {}.\n", object_name);
                    std::exit(EXIT_FAILURE);
                }

                for (std::size_t i = 1; i < input_paths.size(); i++)
                {
                    auto _input_file = std::unique_ptr<TFile>(TFile::Open(input_paths[i].c_str()));
                    auto ec = std::unique_ptr<EventClassContainer>(
                        _input_file->Get<EventClassContainer>(object_name.c_str()));
                    if (not(ec))
                    {
                        fmt::print(stderr, "Could not find the requested EventClassContainer: {}.\n", object_name);
                        std::exit(EXIT_FAILURE);
                    }
                    buffer->merge_inplace(std::move(ec));
                }

                // dump buffer to file
                std::unique_ptr<TFile> output_file(TFile::Open(output_path.c_str(), "RECREATE"));
                output_file->WriteObject(buffer, object_name.c_str());
            },
            "object_name"_a,
            "input_paths"_a,
            "output_path"_a)
        .def_static(
            "serialize_to_root",
            [](const std::string &input_file_path,
               const std::string &ouput_file_path,
               const std::string &process_name,
               const std::string &process_group,
               const std::string &xsec_order,
               const std::string &year,
               bool is_data) -> std::vector<std::string>
            {
                auto input_file = std::unique_ptr<TFile>(TFile::Open(input_file_path.c_str()));
                auto cont = input_file->Get<EventClassContainer>(fmt::format("{}_{}", process_name, year).c_str());
                if (not(cont))
                {
                    fmt::print(stderr,
                               "Could not find the requested EventClassContainer: {}.\n",
                               fmt::format("{}_{}", process_name, year));
                    std::exit(EXIT_FAILURE);
                }

                EventClassContainer::serialize_to_root(
                    *cont, ouput_file_path, process_name, process_group, xsec_order, year, is_data);

                std::vector<std::string> classes;
                classes.reserve(cont->classes.size());
                for (auto &&[class_name, _] : cont->classes)
                {
                    classes.push_back(class_name);
                }

                return classes;
            },
            "input_file_path"_a,
            "ouput_file_path"_a,
            "process_name"_a,
            "process_group"_a,
            "xsec_order"_a,
            "year"_a,
            "is_data"_a);

    py::class_<ValidationContainer>(m, "ValidationContainer")
        .def(py::init<const std::string &, const std::string &, const std::string &, const std::string &>(),
             "process_group"_a,
             "xs_order"_a,
             "process"_a,
             "year"_a)
        .def_static(
            "save",
            [](ValidationContainer &cont, const std::string &output_path) -> void
            {
                std::unique_ptr<TFile> output_file(TFile::Open(output_path.c_str(), "RECREATE"));
                output_file->WriteObject(&cont, "validation");
            },
            "validation_container"_a,
            "output_path"_a)
        .def_static(
            "merge_many",
            [](const std::vector<std::string> &input_paths, const std::string &output_path) -> void
            {
                if (input_paths.size() == 0)
                {
                    return;
                }

                auto input_file = std::unique_ptr<TFile>(TFile::Open(input_paths[0].c_str()));
                auto buffer = input_file->Get<ValidationContainer>("validation");
                if (not(buffer))
                {
                    fmt::print(stderr, "Could not find the requested ValidationContainer: {}.\n", "validation");
                    std::exit(EXIT_FAILURE);
                }

                for (std::size_t i = 1; i < input_paths.size(); i++)
                {
                    auto _input_file = std::unique_ptr<TFile>(TFile::Open(input_paths[i].c_str()));
                    auto val =
                        std::unique_ptr<ValidationContainer>(_input_file->Get<ValidationContainer>("validation"));
                    if (not(val))
                    {
                        fmt::print(stderr, "Could not find the requested ValidationContainer: {}.\n", "validation");
                        std::exit(EXIT_FAILURE);
                    }
                    buffer->merge_inplace(std::move(val));
                }

                // dump buffer to file
                std::unique_ptr<TFile> output_file(TFile::Open(output_path.c_str(), "RECREATE"));
                output_file->WriteObject(buffer, "validation");
            },
            "input_paths"_a,
            "output_path"_a)
        .def_static(
            "serialize_to_root",
            [](const std::string &input_file_path, const std::string &ouput_file_path) -> std::vector<std::string>
            {
                auto input_file = std::unique_ptr<TFile>(TFile::Open(input_file_path.c_str()));
                auto cont = input_file->Get<ValidationContainer>("validation");
                if (not(cont))
                {
                    fmt::print(stderr, "Could not find the requested ValidationContainer: {}.\n", "validation");
                    std::exit(EXIT_FAILURE);
                }

                return cont->serialize_to_root(ouput_file_path);
            },
            "input_file_path"_a,
            "ouput_file_path"_a);

    py::class_<Distribution>(m, "Distribution")
        .def_static("fold", Distribution::fold, "input_files"_a, "ouput_dir"_a, "analyses_to_fold"_a)
        .def_static("make_distributions",
                    Distribution::make_distributions,
                    "input_file"_a,
                    "ouput_dir"_a,
                    "analysis_to_build"_a,
                    "rescaling"_a);
}
