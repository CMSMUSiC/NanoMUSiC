#ifndef GAMMAPLUSJETS
#define GAMMAPLUSJETS

#include "Histograms.hpp"
#include "Math/Vector4D.h"
#include <TFile.h>
#include <TH1F.h>
#include <memory>
#include <optional>
#include <string_view>

using namespace ROOT;
using namespace ROOT::Math;

class GammaPlusJet
{
  private:
  public:
    const std::string output_path;

    TH1F h_gamma_pt;
    TH1F h_gamma_eta;
    TH1F h_gamma_phi;

    double min_bin_width;
    std::map<std::string, int> countMap;
    std::string shift;

    GammaPlusJet() = default;

    GammaPlusJet(const std::string &_analysis_name,
                 const std::string &_output_path,
                 const std::map<std::string, int> &_countMap,
                 bool dummy,
                 const std::string _shift,
                 const std::string &_sample,
                 const std::string &_year,
                 const std::string &_process_group,
                 const std::string &_xs_order);

    auto fill(Math::PtEtaPhiMVector gamma, float weight = 1.) -> void;

    auto dump_outputs(std::unique_ptr<TFile> &output_file) -> void;
};

#endif