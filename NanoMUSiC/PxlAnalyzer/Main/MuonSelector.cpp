#include "MuonSelector.hpp"

using namespace std;

//--------------------Constructor-----------------------------------------------------------------

MuonSelector::MuonSelector(const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap)
    : // initalize parent constructor
      ObjectSelector(cfg, globalOldNameMap, "Muon", false),
      // General
      m_muo_id_type(cfg.GetItem<std::string>("Muon.ID.Type", "TightID")),
      m_muo_ptSwitch(cfg.GetItem<double>("Muon.ID.PtSwitch", 200)), m_muo_pt_min(cfg.GetItem<double>("Muon.pt.min")),
      m_muo_eta_max(cfg.GetItem<double>("Muon.eta.max")), m_muo_invertIso(cfg.GetItem<bool>("Muon.InvertIsolation")),

      // Isolation
      m_muo_iso_type(cfg.GetItem<string>("Muon.Iso.Type")),
      m_muo_iso_puCorrection(cfg.GetItem<string>("Muon.Iso.PUCorrection")),
      m_muo_iso_max(cfg.GetItem<double>("Muon.Iso.max")), m_muo_iso_pf_max(cfg.GetItem<double>("Muon.Iso.PF.max")),
      m_muo_iso_tracker_max(cfg.GetItem<double>("Muon.Iso.Tracker.max")),

      // Effective area
      m_muo_EA(cfg, "Muon"),

      // Soft ID
      m_muo_softid_useBool(cfg.GetItem<bool>("Muon.SoftID.UseBool")),
      m_muo_softid_boolName(cfg.GetItem<string>("Muon.SoftID.BoolName")),
      m_muo_softid_isGoodMuon(cfg.GetItem<bool>("Muon.SoftID.IsGoodMuon")),
      m_muo_softid_trackerLayersWithMeas_min(cfg.GetItem<int>("Muon.SoftID.TrackerLayersWithMeas.min")),
      m_muo_softid_pixelLayersWithMeas_min(cfg.GetItem<int>("Muon.SoftID.PixelLayersWithMeas.min")),
      m_muo_softid_QualityInnerTrack(cfg.GetItem<bool>("Muon.SoftID.QualityInnerTrack")),
      m_muo_softid_dxy_max(cfg.GetItem<double>("Muon.SoftID.Dxy.max")),
      m_muo_softid_dz_max(cfg.GetItem<double>("Muon.SoftID.Dz.max")),

      // Loose ID
      m_muo_looseid_useBool(cfg.GetItem<bool>("Muon.LooseID.UseBool")),
      m_muo_looseid_boolName(cfg.GetItem<string>("Muon.LooseID.BoolName")),
      m_muo_looseid_isPFMuon(cfg.GetItem<bool>("Muon.LooseID.IsPFMuon")),
      m_muo_looseid_isGlobalMuon(cfg.GetItem<bool>("Muon.LooseID.IsGlobalMuon")),
      m_muo_looseid_isTrackerMuon(cfg.GetItem<bool>("Muon.LooseID.IsTrackerMuon")),

      // Medium ID
      m_muo_mediumid_useBool(cfg.GetItem<bool>("Muon.MediumID.UseBool")),
      m_muo_mediumid_boolName(cfg.GetItem<string>("Muon.MediumID.BoolName")),
      m_muo_mediumid_isLooseMuon(cfg.GetItem<bool>("Muon.MediumID.IsLooseMuon")),
      m_muo_mediumid_validFraction_min(cfg.GetItem<double>("Muon.MediumID.ValidFraction.min")),
      m_muo_mediumid_isGlobalMuon(cfg.GetItem<bool>("Muon.MediumID.IsGlobalMuon")),
      m_muo_mediumid_normalizedChi2_max(cfg.GetItem<double>("Muon.MediumID.NormalizedChi2.max")),
      m_muo_mediumid_chi2LocalPosition_max(cfg.GetItem<double>("Muon.MediumID.Chi2LocalPosition.max")),
      m_muo_mediumid_trkKink_max(cfg.GetItem<double>("Muon.MediumID.TrkKink.max")),
      m_muo_mediumid_segCompGlobal_min(cfg.GetItem<double>("Muon.MediumID.SegCompGlobal.min")),
      m_muo_mediumid_segCompTight_min(cfg.GetItem<double>("Muon.MediumID.SegCompTight.min")),

      // Tight ID
      m_muo_tightid_useBool(cfg.GetItem<bool>("Muon.TightID.UseBool")),
      m_muo_tightid_boolName(cfg.GetItem<string>("Muon.TightID.BoolName")),
      m_muo_tightid_isGlobalMuon(cfg.GetItem<bool>("Muon.TightID.IsGlobalMuon")),
      m_muo_tightid_isPFMuon(cfg.GetItem<bool>("Muon.TightID.IsPFMuon")),
      m_muo_tightid_normalizedChi2_max(cfg.GetItem<double>("Muon.TightID.NormalizedChi2.max")),
      m_muo_tightid_vHitsMuonSys_min(cfg.GetItem<int>("Muon.TightID.VHitsMuonSys.min")),
      m_muo_tightid_nMatchedStations_min(cfg.GetItem<int>("Muon.TightID.NMatchedStations.min")),
      m_muo_tightid_dxy_max(cfg.GetItem<double>("Muon.TightID.Dxy.max")),
      m_muo_tightid_dz_max(cfg.GetItem<double>("Muon.TightID.Dz.max")),
      m_muo_tightid_vHitsPixel_min(cfg.GetItem<int>("Muon.TightID.VHitsPixel.min")),
      m_muo_tightid_trackerLayersWithMeas_min(cfg.GetItem<int>("Muon.TightID.TrackerLayersWithMeas.min")),

      // High Pt ID
      m_muo_highptid_useBool(cfg.GetItem<bool>("Muon.HighPtID.UseBool")),
      m_muo_highptid_boolName(cfg.GetItem<string>("Muon.HighPtID.BoolName")),
      m_muo_highptid_isGlobalMuon(cfg.GetItem<bool>("Muon.HighPtID.IsGlobalMuon")),
      m_muo_highptid_ptRelativeError_max(cfg.GetItem<double>("Muon.HighPtID.PtRelativeError.max")),
      m_muo_highptid_nMatchedStations_min(cfg.GetItem<int>("Muon.HighPtID.NMatchedStations.min")),
      m_muo_highptid_vHitsMuonSys_min(cfg.GetItem<int>("Muon.HighPtID.VHitsMuonSys.min")),
      m_muo_highptid_vHitsPixel_min(cfg.GetItem<int>("Muon.HighPtID.VHitsPixel.min")),
      m_muo_highptid_trackerLayersWithMeas_min(cfg.GetItem<int>("Muon.HighPtID.TrackerLayersWithMeas.min")),
      m_muo_highptid_dxy_max(cfg.GetItem<double>("Muon.HighPtID.Dxy.max")),
      m_muo_highptid_dz_max(cfg.GetItem<double>("Muon.HighPtID.Dz.max")),

      // Tracker ID
      m_muo_trackerid_useBool(cfg.GetItem<bool>("Muon.TrackerID.UseBool")),
      m_muo_trackerid_boolName(cfg.GetItem<string>("Muon.TrackerID.BoolName")),
      m_muo_trackerid_isTrackerMuon(cfg.GetItem<bool>("Muon.TrackerID.IsTrackerMuon")),
      m_muo_trackerid_ptRelativeError_max(cfg.GetItem<double>("Muon.TrackerID.PtRelativeError.max")),
      m_muo_trackerid_nMatchedStations_min(cfg.GetItem<int>("Muon.TrackerID.NMatchedStations.min")),
      m_muo_trackerid_vHitsPixel_min(cfg.GetItem<int>("Muon.TrackerID.VHitsPixel.min")),
      m_muo_trackerid_trackerLayersWithMeas_min(cfg.GetItem<int>("Muon.TrackerID.TrackerLayersWithMeas.min")),
      m_muo_trackerid_dxy_max(cfg.GetItem<double>("Muon.TrackerID.Dxy.max")),
      m_muo_trackerid_dz_max(cfg.GetItem<double>("Muon.TrackerID.Dz.max"))
{
    m_useAlternative = false;
}
//--------------------Destructor-----------------------------------------------------------------

MuonSelector::~MuonSelector()
{
}

int MuonSelector::passObjectSelection(pxl::Particle *muon, double const muonRho, const std::string &idType,
                                      const bool isSyst // use alternative kinematic cuts for syst
) const
{

    try
    {
        return muonID(muon, muonRho, idType, isSyst);
    }
    catch (std::runtime_error &e)
    {
        std::cout << e.what() << '\n';
        std::cout << e.what() << '\n';
        m_useAlternative = true;
        m_alternativeUserVariables["DxyGTBS"] = "DxyBS";
        m_alternativeUserVariables["DzIT"] = "Dz";
        m_alternativeUserVariables["Dz"] = "DzBT";
        m_alternativeUserVariables["Dxy"] = "DxyBT";
        m_alternativeUserVariables["isGoodTMOneST"] = "TMOneStationTight";
        m_alternativeUserVariables["isGoodLastS"] = "lastStationTight";
        m_alternativeUserVariables["normalizedChi2"] = "NormChi2";

        return muonID(muon, muonRho, idType, isSyst);
    }

    return 0;
}

int MuonSelector::muonID(pxl::Particle *muon, double const rho, const std::string &idType, const bool isSyst) const
{
    bool passKin = ObjectSelector::passKinematics(muon, isSyst);
    bool passID = false;
    bool passIso = false;

    // the muon cuts are according to :
    // https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideMuonId?rev=49
    // status: 17.9.2014

    muon->setUserRecord("usedID", idType);
    // decide which ID should be performed
    if (idType == "CombinedID")
    {
        if (muon->getPt() < m_muo_ptSwitch)
        {
            passID = passTightID(muon);
            muon->setUserRecord("usedID", "TightID");
        }
        else
        {
            passID = passHighPtID(muon);
            muon->setUserRecord("usedID", "HighPtID");
        }
    }
    else if (idType == "HighPtID")
    {
        passID = passHighPtID(muon);
    }
    else if (idType == "TightID")
    {
        passID = passTightID(muon);
    }
    else if (idType == "MediumID")
    {
        passID = passMediumID(muon);
    }
    else if (idType == "LooseID")
    {
        passID = passLooseID(muon);
    }
    else if (idType == "SoftID")
    {
        passID = passSoftID(muon);
    }
    else if (idType == "TrackerID")
    {
        passID = passTrackerID(muon);
    }
    else if (idType == "None")
    {
        passID = true;
    }
    else
    {
        throw Tools::config_error("'Muon.ID.Type' must be one of these values: 'CombinedID', 'TightID', 'MediumID', "
                                  "'LooseID', 'SoftID', 'TrackerID' or 'None'. The value is '" +
                                  m_muo_id_type + "'");
        passID = false;
    }

    // decide which isolation to perform
    if (m_muo_iso_type == "PFIso")
    {
        passIso = passPFIso(muon);
    }
    else if (m_muo_iso_type == "MiniIso")
    {
        passIso = passMiniIso(muon);
    }
    else if (m_muo_iso_type == "TrackerIso")
    {
        passIso = passTrackerIso(muon);
    }
    else if (m_muo_iso_type == "None")
    {
        passIso = true;
        //  use tracker iso for highPt and PFIso for low Pt
    }
    else if (m_muo_iso_type == "CombinedIso")
    {
        if (muon->getPt() < m_muo_ptSwitch)
        {
            passIso = passPFIso(muon);
        }
        else
        {
            passIso = passTrackerIso(muon);
        }
    }
    else
    {
        throw Tools::config_error(
            "'Muon.Iso.Type' must be one of these values: 'PFIso', 'MiniIso', 'TrackerIso' or 'None'. The value is '" +
            m_muo_iso_type + "'");
        passIso = false;
    }

    // perform iso inversion if requested
    if (m_muo_invertIso)
        passIso = !passIso;

    muon->setUserRecord("IDpassed", false);
    muon->setUserRecord("ISOfailed", false);
    muon->setUserRecord("IDfailed", false);
    muon->setUserRecord("KINfailed", false);
    muon->setUserRecord("multipleFails", false);

    // return code depending on passing variables
    if (passKin && passID && passIso)
    {
        return 0;
    }
    else if (passKin && passID && !passIso)
    {
        muon->setUserRecord("ISOfailed", true);
        return 1;
    }
    else if (passKin && !passID && passIso)
    {
        muon->setUserRecord("IDfailed", true);
        return 2;
    }
    else if (!passKin && passID && passIso)
    {
        muon->setUserRecord("KINfailed", true);
        return 3;
    }
    muon->setUserRecord("multipleFails", true);
    return 4;
}

bool MuonSelector::passSoftID(pxl::Particle *muon) const
{
    // return built-in bool if requested
    if (m_muo_softid_useBool)
        return muon->getUserRecord(m_muo_softid_boolName).toBool();
    // do the cut based ID if we are not using the bool
    if (!(muon->getUserRecord("TrackerLayersWithMeas").toInt32() > m_muo_softid_trackerLayersWithMeas_min))
        return false;
    if (!(muon->getUserRecord("PixelLayersWithMeas").toInt32() > m_muo_softid_pixelLayersWithMeas_min))
        return false;
    if (!(muon->getUserRecord("QualityInnerTrack").toBool() == m_muo_softid_QualityInnerTrack))
        return false;
    if (!(fabs(muon->getUserRecord("DxyIT").toDouble()) < m_muo_softid_dxy_max))
        return false;
    if (!m_useAlternative)
    {
        if (!(fabs(muon->getUserRecord("DzIT").toDouble()) < m_muo_softid_dz_max))
            return false;
        if (!(muon->getUserRecord("isGoodTMOneST").toBool() == m_muo_softid_isGoodMuon))
            return false;
    }
    else
    {
        if (!(fabs(muon->getUserRecord(m_alternativeUserVariables["DzIT"]).toDouble()) < m_muo_softid_dz_max))
            return false;
        if (!(muon->getUserRecord(m_alternativeUserVariables["isGoodTMOneST"]).toBool() == m_muo_softid_isGoodMuon))
            return false;
    }

    return true;
}

bool MuonSelector::passLooseID(pxl::Particle *muon) const
{
    // return built-in bool if requested
    if (m_muo_looseid_useBool)
        return muon->getUserRecord(m_muo_looseid_boolName).toBool();
    // do the cut based ID if we are not using the bool
    if (!(muon->getUserRecord("isPFMuon").toBool() == m_muo_looseid_isPFMuon))
        return false;
    if (!((muon->getUserRecord("isGlobalMuon").toBool() == m_muo_looseid_isGlobalMuon) ||
          (muon->getUserRecord("isTrackerMuon").toBool() == m_muo_looseid_isTrackerMuon)))
        return false;
    return true;
}

bool MuonSelector::passMediumID(pxl::Particle *muon) const
{
    // return built-in bool if requested
    if (m_muo_mediumid_useBool)
        return muon->getUserRecord(m_muo_mediumid_boolName).toBool();
    // do the cut based ID if we are not using the bool
    if (!(muon->getUserRecord("isLooseMuon").toBool() == m_muo_mediumid_isLooseMuon))
        return false;
    if (!(muon->getUserRecord("validFraction").toDouble() > m_muo_mediumid_validFraction_min))
        return false;
    // there are two different set of cuts for the medium ID - at least one of them must be true
    // first set:
    // need to start with this set and return true since there is no isValidGlobalTrack variable and thus
    // muon->getUserRecord("normalizedChi2").toDouble() will fail if there is no global track
    // (therefore one has to return from the function if isGlobalMuon is false)
    if (muon->getUserRecord("SegComp").toDouble() > m_muo_mediumid_segCompTight_min)
        return true;
    // second set:
    if (!(muon->getUserRecord("isGlobalMuon").toBool() == m_muo_mediumid_isGlobalMuon))
        return false;
    if (!m_useAlternative)
    {
        if (muon->hasUserRecord("normalizedChi2") &&
            !(muon->getUserRecord("normalizedChi2").toDouble() < m_muo_mediumid_normalizedChi2_max))
            return false;
    }
    else
    {
        if (!(muon->getUserRecord(m_alternativeUserVariables["normalizedChi2"]).toDouble() <
              m_muo_mediumid_normalizedChi2_max))
            return false;
    }
    if (!(muon->getUserRecord("chi2LocalPosition").toDouble() < m_muo_mediumid_chi2LocalPosition_max))
        return false;
    if (!(muon->getUserRecord("trkKink").toDouble() < m_muo_mediumid_trkKink_max))
        return false;
    if (!(muon->getUserRecord("SegComp").toDouble() > m_muo_mediumid_segCompGlobal_min))
        return false;
    return true;
}

bool MuonSelector::passTightID(pxl::Particle *muon) const
{
    return muon->getUserRecord("tightId").toBool();
}

bool MuonSelector::passHighPtID(pxl::Particle *muon) const
{
    return muon->getUserRecord("highPtId").toBool();
}

bool MuonSelector::passTrackerID(pxl::Particle *muon) const
{
    return muon->getUserRecord("isTracker").toBool();
}

bool MuonSelector::passTrackerIso(pxl::Particle *muon) const
{
    // TkIso ID (1=TkIsoLoose, 2=TkIsoTight)
    return (muon->getUserRecord("tkIsoId").toInt32() == 1);
}

bool MuonSelector::passPFIso(pxl::Particle *muon) const
{
    // PFIso ID from miniAOD selector:
    // 1 = PFIsoVeryLoose
    // 2 = PFIsoLoose
    // 3 = PFIsoMedium
    // 4 = PFIsoTight  🔥🔥🔥
    // 5 = PFIsoVeryTight
    // 6 = PFIsoVeryVeryTight
    return (muon->getUserRecord("pfIsoId").toInt32() >= 4);
}

bool MuonSelector::passMiniIso(pxl::Particle *muon) const
{
    // Description: MiniIso ID from miniAOD selector
    // 1 = MiniIsoLoose
    // 2 = MiniIsoMedium
    // 3 = MiniIsoTight 🔥🔥🔥
    // 4 = MiniIsoVeryTight
    return (muon->getUserRecord("miniIsoId").toInt32() >= 3);
}
