#include "ZToLepLepX.hpp"
#include "Histograms.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <string_view>

ZToLepLepX::ZToLepLepX(const std::string &output_path)
    : output_file(std::unique_ptr<TFile>(TFile::Open(output_path.c_str(), "RECREATE")))
{
}

auto ZToLepLepX::fill(Math::PtEtaPhiMVector lepton_1,
                      Math::PtEtaPhiMVector lepton_2,
                      std::optional<Math::PtEtaPhiMVector> bjet,
                      std::optional<Math::PtEtaPhiMVector> jet,
                      std::optional<float> met,
                      float weight) -> void
{
    h_invariant_mass.Fill((lepton_1 + lepton_2).mass(), weight);
}

auto ZToLepLepX::dump_outputs() -> void
{
    fmt::print("Saving outputs to: {}\n", output_file->GetPath());
    output_file->cd();
    h_invariant_mass.Write();
    output_file->Close();
}