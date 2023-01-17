#include "ObjectSelector.hpp"

#include <cmath>
#include <utility>
#include <vector>

// Pure Abstract base class for all object selectors in PXLanalzyer

// Constructor
ObjectSelector::ObjectSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap,
                               const std::string &object_name, bool endcap_eta_cuts, std::string alternative_eta_label)
    : oldNameMap(globalOldNameMap), m_object_name(object_name),
      m_alternative_eta_label(std::move(alternative_eta_label)), m_object_endcap_eta_cuts(endcap_eta_cuts),
      m_object_barrel_only(cfg.GetItem<bool>(object_name + ".barrel.only", false)),
      m_object_endcap_only(cfg.GetItem<bool>(object_name + ".endcap.only", false)),
      m_object_pt_min(cfg.GetItem<double>(object_name + ".pt.min")), m_object_eta_barrel_max(9999.),
      m_object_eta_endcap_min(-1.), m_object_eta_endcap_max(99999.),

      m_object_syst_pt_min(cfg.GetItem<double>(object_name + ".Syst.pt.min")), m_object_syst_eta_barrel_max(9999.),
      m_object_syst_eta_endcap_min(-1.), m_object_syst_eta_endcap_max(99999.),

      m_scale_factors(std::vector<ScaleFactor>())

{

    if (m_object_endcap_eta_cuts)
    {
        m_object_eta_barrel_max = cfg.GetItem<double>(object_name + ".eta.Barrel.max");
        m_object_eta_endcap_min = cfg.GetItem<double>(object_name + ".eta.Endcap.min");
        m_object_eta_endcap_max = cfg.GetItem<double>(object_name + ".eta.Endcap.max");

        m_object_syst_eta_barrel_max = cfg.GetItem<double>(object_name + ".Syst.eta.Barrel.max");
        m_object_syst_eta_endcap_min = cfg.GetItem<double>(object_name + ".Syst.eta.Endcap.min");
        m_object_syst_eta_endcap_max = cfg.GetItem<double>(object_name + ".Syst.eta.Endcap.max");
    }
    else
    {
        m_object_eta_barrel_max = cfg.GetItem<double>(object_name + ".eta.max");
        m_object_syst_eta_barrel_max = cfg.GetItem<double>(object_name + ".Syst.eta.max");
    }
    std::vector<std::string> sf_names =
        Tools::splitString<std::string>(cfg.GetItem<std::string>(object_name + ".ScaleFactors"), true);
    // in configs: ID.Tight, ID.Medium, ID.Loose,... same name as for ID.Type
    for (const auto &sf_name : sf_names)
    {
        m_scale_factors.push_back(ScaleFactor(object_name, sf_name, cfg));
    }
}

// Destructor
ObjectSelector::~ObjectSelector()
{
}

// Pass kinematics function checks for pt and eta cuts
bool ObjectSelector::passKinematics(pxl::Particle *object, const bool isSyst) const
{

    // check if a alternative label for eta was given
    // e.g. supercluster position for ele
    double abseta = fabs(object->getEta());
    if (!m_alternative_eta_label.empty())
    {
        abseta = fabs(object->getUserRecord(m_alternative_eta_label).toDouble());
    }

    double pt_min = m_object_pt_min;
    double eta_barrel_max = m_object_eta_barrel_max;
    double eta_endcap_min = m_object_eta_endcap_min;
    double eta_endcap_max = m_object_eta_endcap_max;
    if (isSyst)
    {
        eta_barrel_max = m_object_syst_eta_barrel_max;
        eta_endcap_min = m_object_syst_eta_endcap_min;
        eta_endcap_max = m_object_syst_eta_endcap_max;
    }
    bool isBarrel = false;
    bool isEndcap = false;
    if (abseta < eta_barrel_max)
        isBarrel = true;
    if (m_object_endcap_eta_cuts && !isBarrel && abseta > eta_endcap_min && abseta < eta_endcap_max)
        isEndcap = true;

    object->setUserRecord("isBarrel", isBarrel);
    object->setUserRecord("isEndcap", isEndcap);

    if (isBarrel and m_object_endcap_only)
        return false;
    if (isEndcap and m_object_barrel_only)
        return false;

    if (m_object_endcap_eta_cuts && !isBarrel && !isEndcap)
        return false;
    if (!m_object_endcap_eta_cuts && !isBarrel)
        return false;

    if (object->getPt() < pt_min)
        return false;

    return true;
}

void ObjectSelector::setScaleFactors(pxl::Particle *object)
{
    double total_scale_factor = 1.;
    double total_scale_factor_error = 0.;
    double total_scale_factor_error_syst = 0.;
    std::string userRecordName = "";
    std::string used_id = "";
    if (object->hasUserRecord("usedID"))
    {
        used_id = object->getUserRecord("usedID").asString();
    }
    else
    {
        return;
    }
    for (auto &sf : m_scale_factors)
    {
        // Special cases for combined IDs (identified by . after ID in name)
        if ((sf.getName().find("ID.") != std::string::npos) && // check if this is an combined ID related SF
            (used_id.find("None") == std::string::npos) &&     // check if this object matches no ID in combined ID
            (sf.getName() != ("ID." + used_id)))
        { // check if this SF does not match the ID for the current object
            continue;
        }
        if (sf.getName().find("Iso.") != std::string::npos && // check if this is an combined ID related SF
            used_id.find("None") == std::string::npos &&      // check if this object matches no ID in combined ID
            sf.getName() != ("Iso." + used_id))
        { // check if this SF does not match the ID for the current object
            continue;
        }
        double sf_temp = sf.getScaleFactor(object);
        double sf_temp_error = sf.getScaleFactorError(object);
        double sf_temp_error_syst = sf.getSystematic();
        userRecordName = "scale_factor_" + m_object_name + "_" + sf.getName();
        object->setUserRecord(userRecordName, sf_temp);
        total_scale_factor *= sf_temp;
        total_scale_factor_error = std::sqrt(std::pow(total_scale_factor_error, 2) + std::pow(sf_temp_error, 2));
        total_scale_factor_error_syst += sf_temp_error_syst * sf_temp_error_syst;
        object->setUserRecord(userRecordName + "_error", sf_temp_error);
    }
    object->setUserRecord("scale_factor", total_scale_factor);
    object->setUserRecord("scale_factor_error", total_scale_factor_error);
    object->setUserRecord("scale_factor_error_syst", total_scale_factor_error_syst);
}
