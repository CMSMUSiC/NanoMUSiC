#ifndef JETCORRECTOR_HPP
#define JETCORRECTOR_HPP

#include <string>

// correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include <correction.h>

#include "TRandom3.h"

#include "Configs.hpp"

class JetCorrector
{
  private:
    const Year year;
    const std::string &era;
    const bool is_data;
    TRandom3 rand;
    correction::Correction::Ref pt_resolution_correction_ref;
    correction::Correction::Ref scale_factor_correction_ref;
    correction::Correction::Ref scale_correction_ref;
    correction::Correction::Ref scale_uncertainty_correction_ref;
    constexpr static double min_correction_factor = 1E-9;

  public:
    JetCorrector(const Year &_year, const std::string &_era, const bool _is_data);
    auto get_resolution(float pt, float eta, float rho) const -> float;
    auto get_resolution_scale_factor(float eta, const std::string &variation = "Nominal") const -> float;
    auto get_resolution_correction(float pt,
                                   float eta,
                                   float phi,
                                   float rho,
                                   int genjet_idx,
                                   float gen_jet_pt,
                                   float gen_jet_eta,
                                   float gen_jet_phi,
                                   const std::string &variation = "Nominal") -> float;
    auto get_scale_correction(float raw_pt,
                              float eta,
                              float phi,
                              float raw_factor,
                              float rho,
                              float area,
                              const std::string &variation = "Nominal") -> float;
};

#endif // !JETCORRECTOR_HPP
