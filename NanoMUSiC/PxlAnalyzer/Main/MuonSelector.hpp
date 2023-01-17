#ifndef MuonSelector_hh
#define MuonSelector_hh

/*

This class contains all the muon selections

*/

#include "EffectiveArea.hpp"
#include "ObjectSelector.hpp"
#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include "Tools/MConfig.hpp"
#include <map>
#include <string>

class MuonSelector : public ObjectSelector
{
  public:
    MuonSelector(const Tools::MConfig &config, OldNameMapper *globalOldNameMap);
    // Destruktor
    ~MuonSelector();
    int passMuon(pxl::Particle *muon, double const rho, const bool &isRec) const;
    int passObjectSelection(pxl::Particle *muon, double const muonRho, const std::string &idType,
                            const bool isSyst // use alternative kinematic cuts for syst
    ) const;

  private:
    // Methods
    int muonID(pxl::Particle *muon, double const rho, const std::string &idType, const bool isSyst) const;
    //~ bool passKinematics(pxl::Particle *muon ) const;
    bool passSoftID(pxl::Particle *muon) const;
    bool passLooseID(pxl::Particle *muon) const;
    bool passMediumID(pxl::Particle *muon) const;
    bool passTightID(pxl::Particle *muon) const;
    bool passHighPtID(pxl::Particle *muon) const;
    bool passTrackerID(pxl::Particle *muon) const;
    bool passPFIso(pxl::Particle *muon) const;
    bool passMiniIso(pxl::Particle *muon) const;
    bool passTrackerIso(pxl::Particle *muon) const;

    // Variables
    std::string const m_muo_id_type;
    double const m_muo_ptSwitch;
    double const m_muo_pt_min;
    double const m_muo_eta_max;
    bool const m_muo_invertIso;

    // Isolation
    std::string const m_muo_iso_type;
    std::string const m_muo_iso_puCorrection;
    double const m_muo_iso_max;
    double const m_muo_iso_pf_max;
    double const m_muo_iso_tracker_max;
    // Effective area
    EffectiveArea const m_muo_EA;

    // Soft ID variables
    const bool m_muo_softid_useBool;
    const std::string m_muo_softid_boolName;
    const bool m_muo_softid_isGoodMuon;
    const int m_muo_softid_trackerLayersWithMeas_min;
    const int m_muo_softid_pixelLayersWithMeas_min;
    const bool m_muo_softid_QualityInnerTrack;
    const double m_muo_softid_dxy_max;
    const double m_muo_softid_dz_max;

    // Loose ID variables
    const bool m_muo_looseid_useBool;
    const std::string m_muo_looseid_boolName;
    const bool m_muo_looseid_isPFMuon;
    const bool m_muo_looseid_isGlobalMuon;
    const bool m_muo_looseid_isTrackerMuon;

    // Medium ID variables
    const bool m_muo_mediumid_useBool;
    const std::string m_muo_mediumid_boolName;
    const bool m_muo_mediumid_isLooseMuon;
    const double m_muo_mediumid_validFraction_min;
    const bool m_muo_mediumid_isGlobalMuon;
    const double m_muo_mediumid_normalizedChi2_max;
    const double m_muo_mediumid_chi2LocalPosition_max;
    const double m_muo_mediumid_trkKink_max;
    const double m_muo_mediumid_segCompGlobal_min;
    const double m_muo_mediumid_segCompTight_min;

    // Tight ID variables
    const bool m_muo_tightid_useBool;
    const std::string m_muo_tightid_boolName;
    const bool m_muo_tightid_isGlobalMuon;
    const bool m_muo_tightid_isPFMuon;
    const double m_muo_tightid_normalizedChi2_max;
    const int m_muo_tightid_vHitsMuonSys_min;
    const int m_muo_tightid_nMatchedStations_min;
    const double m_muo_tightid_dxy_max;
    const double m_muo_tightid_dz_max;
    const int m_muo_tightid_vHitsPixel_min;
    const int m_muo_tightid_trackerLayersWithMeas_min;

    // High Pt ID variables
    const bool m_muo_highptid_useBool;
    const std::string m_muo_highptid_boolName;
    const bool m_muo_highptid_isGlobalMuon;
    const double m_muo_highptid_ptRelativeError_max;
    const int m_muo_highptid_nMatchedStations_min;
    const int m_muo_highptid_vHitsMuonSys_min;
    const int m_muo_highptid_vHitsPixel_min;
    const int m_muo_highptid_trackerLayersWithMeas_min;
    const double m_muo_highptid_dxy_max;
    const double m_muo_highptid_dz_max;

    // Tracker ID variables (for boosted typology)
    const bool m_muo_trackerid_useBool;
    const std::string m_muo_trackerid_boolName;
    const bool m_muo_trackerid_isTrackerMuon;
    const double m_muo_trackerid_ptRelativeError_max;
    const int m_muo_trackerid_nMatchedStations_min;
    const int m_muo_trackerid_vHitsPixel_min;
    const int m_muo_trackerid_trackerLayersWithMeas_min;
    const double m_muo_trackerid_dxy_max;
    const double m_muo_trackerid_dz_max;

    bool mutable m_useAlternative;
    std::map<std::string, std::string> mutable m_alternativeUserVariables;
};
#endif
