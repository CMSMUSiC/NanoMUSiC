#ifndef TAU_EFFICIENCY
#define TAU_EFFICIENCY

#include "Histograms.hpp"
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"
#include "TEfficiency.h"
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <memory>
#include <optional>
#include <string_view>

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

class Tau_efficiency
{
  private:
  public:
    std::string output_path;

    TH1F h_leptons_all;
    TH1F h_leptons_matched;
    TH1F h_efficiency;

    double min_bin_width;
    std::map<std::string, int> countMap;
    bool is_Z_mass_validation;
    std::string shift;

    Tau_efficiency() = default;

    Tau_efficiency(const std::string &_analysis_name,
             const std::string &_output_path,
             const std::map<std::string, int> &_countMap,
             bool _is_Z_mass_validation,
             const std::string _shift,
             const std::string &_sample,
             const std::string &_year,
             const std::string &_process_group,
             const std::string &_xs_order);

    auto fill_eff(const RVec<Math::PtEtaPhiMVector> &taus_p4,
              float weight,
              const RVec<bool> &fakeness) -> void;

    auto save_histo(TH1 histo) -> void;
    auto save_histo(TH2 histo) -> void;

    auto dump_outputs_eff() -> void;
};

#endif // !TAU_EFFICIENCY