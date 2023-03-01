#include "ZToLepLepX.hpp"
#include "Histograms.hpp"
#include <fmt/format.h>
#include <string_view>

ZToLepLepX::ZToLepLepX()
{
}

auto ZToLepLepX::fill(float lepton_1_pt,
                      float lepton_1_eta,
                      float lepton_1_phi,
                      float lepton_2_pt,
                      float lepton_2_eta,
                      float lepton_2_phi,
                      int n_bjets,
                      float bjet_1_pt,
                      float bjet_1_eta,
                      float bjet_1_phi,
                      float bjet_2_pt,
                      float bjet_2_eta,
                      float bjet_2_phi,
                      int n_jets,
                      float jet_1_pt,
                      float jet_1_eta,
                      float jet_1_phi,
                      float jet_2_pt,
                      float jet_2_eta,
                      float jet_2_phi,
                      float met,
                      float weight) -> void
{
}

auto ZToLepLepX::dump_outputs(const std::string_view &output_path) -> void
{
    fmt::print("Saving outputs to: {}\n", output_path);
}