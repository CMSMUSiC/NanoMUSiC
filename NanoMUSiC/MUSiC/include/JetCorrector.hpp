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

#include "Configs.hpp"

class JetCorrector
{
  private:
    const Year year;
    const bool is_data;
    correction::Correction::Ref pt_resolution_correction_ref;
    correction::Correction::Ref scale_factor_correction_ref;

  public:
    JetCorrector(const Year &_year,
                 const bool _is_data,
                 const std::string &_correction_file,
                 const std::string &_correction_key);
    auto get_resolution_correction(float pt, float eta, float rho) -> float;
    auto get_resolution(float pt, float eta, float rho) const -> float;
    auto get_resolution_scale_factor(float eta, const std::string &variation = "Nominal") const -> float;
};

#endif // !JETCORRECTOR_HPP
