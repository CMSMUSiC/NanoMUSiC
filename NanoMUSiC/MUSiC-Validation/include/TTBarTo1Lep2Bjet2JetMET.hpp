#ifndef TTbarToLep
#define TTbarToLep

#include "Histograms.hpp"
#include "Math/Vector4D.h"
#include <TFile.h>
#include <TH1F.h>
#include <memory>
#include <optional>
#include <string_view>

using namespace ROOT;
using namespace ROOT::Math;

class TTBarTo1Lep2Bjet2JetMET
{
  private:
  public:
    const std::string output_path;

    TH1F h_invariant_mass_jet0_jet1;
    TH1F h_transverse_mass_lep_MET;
    TH1F h_ht_had_lep;

    double min_bin_width;
    std::map<std::string, int> countMap;
    std::string shift;

    TTBarTo1Lep2Bjet2JetMET() = default;

    TTBarTo1Lep2Bjet2JetMET(const std::string &_analysis_name,
                            const std::string &_output_path,
                            const std::map<std::string, int> &_countMap,
                            const std::string _shift,
                            const std::string &_sample,
                            const std::string &_year,
                            const std::string &_process_group,
                            const std::string &_xs_order);

    auto fill(Math::PtEtaPhiMVector lep,
              Math::PtEtaPhiMVector jet0,
              Math::PtEtaPhiMVector jet1,
              Math::PtEtaPhiMVector bjet0,
              Math::PtEtaPhiMVector bjet1,
              Math::PtEtaPhiMVector met,
              float weight) -> void;

    auto dump_outputs() -> void;
};

#endif // !TTbarToLep