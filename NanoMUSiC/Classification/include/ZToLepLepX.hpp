#ifndef ZTOLEPLEPX
#define ZTOLEPLEPX

#include "Shifts.hpp"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <array>
#include <memory>
#include <pybind11/detail/common.h>

using namespace ROOT;

class ZToLepLepX
{
  private:
    static constexpr std::size_t total_variations = static_cast<std::size_t>(Shifts::Variations::kTotalVariations);

  public:
    enum class Leptons
    {
        MUONS,
        ELECTRONS,
        TAUS
    };

    Leptons lepton;
    bool around_to_Z_mass;
    std::string analysis_name;

    std::array<TH1F, total_variations> h_invariant_mass;
    std::array<TH1F, total_variations> h_met;
    std::array<TH1F, total_variations> h_sum_pt;
    std::array<TH1F, total_variations> h_lepton_1_pt;
    std::array<TH1F, total_variations> h_lepton_2_pt;
    std::array<TH1F, total_variations> h_lepton_1_eta;
    std::array<TH1F, total_variations> h_lepton_2_eta;
    std::array<TH1F, total_variations> h_lepton_1_phi;
    std::array<TH1F, total_variations> h_lepton_2_phi;
    std::array<TH1F, total_variations> h_lepton_1_jet_1_dPhi;
    std::array<TH1F, total_variations> h_lepton_1_jet_1_dR;
    std::array<TH1F, total_variations> h_jet_multiplicity;
    std::array<TH1F, total_variations> h_bjet_multiplicity;

    ZToLepLepX() = default;

    ZToLepLepX(enum Leptons lepton,
               bool around_to_Z_mass,
               const std::string &process_group,
               const std::string &xs_order,
               const std::string &sample,
               const std::string &year);

    auto fill(const MUSiCObjects &leptons,
              const MUSiCObjects &bjets,
              const MUSiCObjects &jets,
              const MUSiCObjects &met,
              double weight,
              Shifts::Variations shift) -> void;

    auto serialize_to_root(const std::unique_ptr<TFile> &output_file) -> void;
    auto merge_inplace(const ZToLepLepX &other) -> void;
};

#endif // !ZTOLEPLEPX
