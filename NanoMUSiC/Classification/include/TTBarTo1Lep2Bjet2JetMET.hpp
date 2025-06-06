#ifndef TTbarToLep
#define TTbarToLep

#include "Shifts.hpp"
#include "ObjectFactories/music_objects.hpp"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <array>
#include <memory>
#include <pybind11/detail/common.h>

using namespace ROOT;

class TTBarTo1Lep2Bjet2JetMET
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
    std::string analysis_name;

    std::array<TH1F, total_variations> h_invariant_mass_jet0_jet1;
    std::array<TH1F, total_variations> h_transverse_mass_lep_MET;
    std::array<TH1F, total_variations> h_ht_had_lep;

    TTBarTo1Lep2Bjet2JetMET() = default;

    TTBarTo1Lep2Bjet2JetMET(enum Leptons lepton,
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
    auto merge_inplace(const TTBarTo1Lep2Bjet2JetMET &other) -> void;
};

#endif // !TTbarToLep
