#ifndef WTOLEPNUX
#define WTOLEPNUX

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

class WToLepNuX
{
  private:
  public:
    std::string output_path;

    TH1F h_invariant_mass;
    TH1F h_sum_pt;
    TH1F h_met;
    TH1F h_lepton_1_pt;
    TH1F h_met_pt;
    TH1F h_lepton_1_eta;
    TH1F h_met_eta;
    TH1F h_lepton_1_phi;
    TH1F h_met_phi;
    //TH1F h_lepton_1_jet_1_dPhi;
    //TH1F h_lepton_1_jet_1_dR;
    //TH1F h_jet_multiplicity;
    //TH1F h_bjet_multiplicity;
    TH2F h_lepton_1_pt_eta;
    TH2F h_lepton_1_pt_phi;
    TH1F h_leptons_all;
    TH1F h_leptons_matched;

    double min_bin_width;
    std::map<std::string, int> countMap;
    bool is_Z_mass_validation;
    std::string shift;

    WToLepNuX() = default;

    WToLepNuX(const std::string &_analysis_name,
             const std::string &_output_path,
             const std::map<std::string, int> &_countMap,
             bool _is_Z_mass_validation,
             const std::string _shift,
             const std::string &_sample,
             const std::string &_year,
             const std::string &_process_group,
             const std::string &_xs_order);

    auto fill(const Math::PtEtaPhiMVector &lepton_1,
              const RVec<Math::PtEtaPhiMVector> &met,
              float weight) -> void;

    auto save_histo(TH1 histo) -> void;
    auto save_histo(TH2 histo) -> void;

    auto dump_outputs() -> void;
};

#endif // !WTOLEPNUX