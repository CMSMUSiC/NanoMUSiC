#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

namespace Tools
{
class MConfig;
}

// correction lib
#include "correction.h"

class ReWeighter
{
  public:
    ReWeighter(const Tools::MConfig &cutconfig, int i);
    ~ReWeighter()
    {
    }

    void ReWeightEvent(pxl::Event *event);
    void init();
    static void adaptConfig(Tools::MConfig &config, const std::string &filename);

  private:
    // std::string dataname;
    // std::string mcname;
    // std::string datahistname;
    // std::string mchistname;
    int m_syst;
    std::unique_ptr<correction::CorrectionSet> m_pu_correction_set;
    correction::Correction::Ref m_pu_correction;
    std::string m_year = "dummy";
    std::string m_json_pu_file;
    const bool m_useGenWeights;
    const bool m_useREcoVertices;
    const bool m_usePileUpReWeighting;
    const bool m_quietMode;
    bool m_init;
    int m_max_gen_vertices;
};
