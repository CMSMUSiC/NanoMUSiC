#ifndef ZTOLEPLEPX
#define ZTOLEPLEPX

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

class ZToLepLepX
{
  private:
  public:
    const std::string output_path;

    TH1F h_invariant_mass;
    TH1F h_sum_pt;
    TH1F h_met;
    TH1F h_lepton_1_pt;
    TH1F h_lepton_2_pt;
    TH1F h_lepton_1_eta;
    TH1F h_lepton_2_eta;
    TH1F h_lepton_1_phi;
    TH1F h_lepton_2_phi;
    TH1F h_lepton_1_jet_1_dPhi;
    TH1F h_lepton_1_jet_1_dR;
    TH1F h_jet_multiplicity;
    TH1F h_bjet_multiplicity;
    TH2F h_lepton_1_pt_eta;
    TH2F h_lepton_1_pt_phi;

    const double min_bin_width;
    const std::map<std::string, int> countMap;
    const bool is_Z_mass_validation;
    std::string shift;

    ZToLepLepX(const std::string &output_path,
               const std::map<std::string, int> &countMap,
               bool is_Z_mass_validation,
               std::string _shift,
               const std::string &_process,
               const std::string &_year);

    auto fill(const Math::PtEtaPhiMVector &lepton_1,
              const Math::PtEtaPhiMVector &lepton_2,
              const RVec<Math::PtEtaPhiMVector> &bjets,
              const RVec<Math::PtEtaPhiMVector> &jets,
              const RVec<Math::PtEtaPhiMVector> &met,
              float weight) -> void;

    auto save_histo(TH1 histo) -> void;
    auto save_histo(TH2 histo) -> void;

    auto dump_outputs() -> void;
};

#endif // !ZTOLEPLEPX