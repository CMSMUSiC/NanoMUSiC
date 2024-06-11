#ifndef GAMMAPLUSJETS
#define GAMMAPLUSJETS

#include "Shifts.hpp"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <array>
#include <memory>
#include <pybind11/detail/common.h>

using namespace ROOT;

class GammaPlusJet
{
  private:
    static constexpr std::size_t total_variations = static_cast<std::size_t>(Shifts::Variations::kTotalVariations);

  public:
    std::string analysis_name;

    std::array<TH1F, total_variations> h_gamma_pt;
    std::array<TH1F, total_variations> h_gamma_eta;
    std::array<TH1F, total_variations> h_gamma_phi;

    GammaPlusJet() = default;

    GammaPlusJet(const std::string &process_group,
                 const std::string &xs_order,
                 const std::string &sample,
                 const std::string &year);

    auto fill(const MUSiCObjects &jets, const MUSiCObjects &photons, double weight, Shifts::Variations shift) -> void;

    auto serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void;
    auto merge_inplace(const GammaPlusJet &other) -> void;
};

#endif // !GAMMAPLUSJETS
