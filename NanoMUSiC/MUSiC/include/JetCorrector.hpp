#ifndef JETCORRECTOR_HPP
#define JETCORRECTOR_HPP

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

    static auto make_correction_ref(const Year &_year,
                                    const std::string &_correction_file,
                                    const std::string &_correction_key) -> correction::Correction::Ref;

  public:
    JetCorrector(const Year &_year,
                 const bool _is_data,
                 const std::string &_correction_file,
                 const std::string &_correction_key);
    auto get_correction(const float &pt) -> float;
    auto get_resolution(const float &pt) -> float;
};

#endif // !JETCORRECTOR_HPP
