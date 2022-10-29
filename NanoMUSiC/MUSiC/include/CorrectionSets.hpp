
// add correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include "correction.h"

enum class CorrectionTypes
{
    PU,
};

class Corrector
{
  public:
    const CorrectionTypes correction_type;
    const std::string year;
    const bool isData;
    correction::Correction::Ref correction_ref;

    const std::map<std::pair<CorrectionTypes, std::string>, std::pair<std::string, std::string>> correction_keys = {
        // {{TYPE, YEAR}, {JSON_FILE, CORRECTION_KEY}},
        {{CorrectionTypes::PU, "2016APV"},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016preVFP_UL/puWeights.json.gz",
          "Collisions16_UltraLegacy_goldenJSON"}},
        {{CorrectionTypes::PU, "2016"},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2016postVFP_UL/puWeights.json.gz",
          "Collisions16_UltraLegacy_goldenJSON"}},
        {{CorrectionTypes::PU, "2017"},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2017_UL/puWeights.json.gz",
          "Collisions17_UltraLegacy_goldenJSON"}},
        {{CorrectionTypes::PU, "2018"},
         {"/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/LUM/2018_UL/puWeights.json.gz",
          "Collisions18_UltraLegacy_goldenJSON"}},
    };

    Corrector(const CorrectionTypes _correction_type, const std::string _year, bool _isData)
        : correction_type(_correction_type), year(_year), isData(_isData)
    {
        if (!isData)
        {
            auto [json_file, key] = correction_keys.at({correction_type, year});
            correction_ref = correction::CorrectionSet::from_file(json_file)->at(key);
        }
    }
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    double operator()(T &&values)
    {
        if (!isData)
        {
            return correction_ref->evaluate(std::forward<T>(values));
        }
        return 1.;
    }
};