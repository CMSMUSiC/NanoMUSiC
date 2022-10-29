#ifndef ObjectSelector_hh
#define ObjectSelector_hh

#include "MConfig.hpp"
#include "OldNameMapper.hpp"
#include "ScaleFactor.hpp"

#include <vector>

class ObjectSelector
{

  public:
    ObjectSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap, const std::string &onjectName,
                   bool endcap_eta_cuts, std::string alternative_eta_label = "");
    virtual ~ObjectSelector();
    // generic virtual function for selection
    virtual int passObjectSelection(pxl::Particle *object, double const objectRho, const std::string &idType,
                                    const bool isSyst // use alternative kinematic cuts for syst
    ) const = 0;

    bool passKinematics(pxl::Particle *object, const bool isSyst = false) const;
    virtual void setScaleFactors(pxl::Particle *object);
    OldNameMapper *oldNameMap;

  protected:
    std::string m_object_name;

  private:
    std::string m_alternative_eta_label;

    bool m_object_endcap_eta_cuts;
    bool m_object_barrel_only;
    bool m_object_endcap_only;
    double m_object_pt_min;
    double m_object_eta_barrel_max;
    double m_object_eta_endcap_min;
    double m_object_eta_endcap_max;
    // alternative variables for syst
    double m_object_syst_pt_min;
    double m_object_syst_eta_barrel_max;
    double m_object_syst_eta_endcap_min;
    double m_object_syst_eta_endcap_max;

  protected:
    std::vector<ScaleFactor> m_scale_factors;
};
#endif
