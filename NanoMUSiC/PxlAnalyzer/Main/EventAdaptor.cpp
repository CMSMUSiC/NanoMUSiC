#include "EventAdaptor.hpp"

#include "TLorentzVector.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

#include "Tools/MConfig.hpp"
// PXL
#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include "Tools/PXL/Sort.hpp"

#include "TFile.h"
#include "TLorentzVector.h"

EventAdaptor::EventAdaptor(Tools::MConfig const &cfg, unsigned int const debug)
    : m_debug(debug),
      m_gen_rec_map(cfg),
      m_jet_res(cfg, "Jet"),
      m_fatjet_res(cfg, "FatJet"),

      m_met(0),
      m_muo_list(std::vector<pxl::Particle *>()),
      m_ele_list(std::vector<pxl::Particle *>()),
      m_fatjet_list(std::vector<pxl::Particle *>()),

      m_muo_useCocktail(cfg.GetItem<bool>("Muon.UseCocktail")),
      m_muo_highptid_ptRelativeError_max(cfg.GetItem<double>("Muon.HighPtID.PtRelativeError.max")),
      m_ele_switchpt(cfg.GetItem<double>("Ele.ID.switchpt")),
      m_jet_res_corr_use(cfg.GetItem<bool>("Jet.Resolutions.Corr.use")),
      m_fatjet_mass_corr_file(cfg.GetItem<std::string>("FatJet.PUPPI.SDMassCorrection")),

      puppisd_corrGEN(),
      puppisd_corrRECO_cen(),
      puppisd_corrRECO_for(),

      // Cache the particle names.
      m_ele_RecName(m_gen_rec_map.get("Ele").RecName),
      m_muo_RecName(m_gen_rec_map.get("Muon").RecName),
      m_jet_RecName(m_gen_rec_map.get("Jet").RecName),
      m_fatjet_RecName(m_gen_rec_map.get("FatJet").RecName),
      m_met_RecName(m_gen_rec_map.get("MET").RecName),

      m_jet_rho_label(cfg.GetItem<std::string>("Jet.Rho.Label"))
{
    TFile file(m_fatjet_mass_corr_file.c_str(), "READ");
    puppisd_corrGEN = TF1(*((TF1 *)file.Get("puppiJECcorr_gen")));
    puppisd_corrRECO_cen = TF1(*((TF1 *)file.Get("puppiJECcorr_reco_0eta1v3")));
    puppisd_corrRECO_for = TF1(*((TF1 *)file.Get("puppiJECcorr_reco_1v3eta2v5")));
}

// fill particle list for particles which may be addpated this avoids
// repeated loops over particles in later lists
void EventAdaptor::initEvent(pxl::EventView const *RecEvtView)
{

    // clear lists from last event
    m_ele_list.clear();
    m_muo_list.clear();
    m_fatjet_list.clear();
    m_met = 0;
    // Get all particles in this event.
    pxlParticles allparticles;
    RecEvtView->getObjectsOfType<pxl::Particle>(allparticles);
    pxl::sortParticles(allparticles);

    // Find MET object first
    for (auto &part : allparticles)
    {
        if (part->getName() == m_met_RecName)
        {
            m_met = part;
        }
        if (part->getName() == m_muo_RecName)
        {
            m_muo_list.push_back(part);
        }
        if (part->getName() == m_ele_RecName)
        {
            m_ele_list.push_back(part);
        }
        if (part->getName() == m_fatjet_RecName)
        {
            m_fatjet_list.push_back(part);
        }
    }
}

// Works on data as well as MC.
void EventAdaptor::applyCocktailMuons() const
{
    // Do we even want to use cocktail muons?
    if (not m_muo_useCocktail)
    {
        if (m_debug > 0)
        {
            std::stringstream warn;
            warn << "[WARNING] (EventAdaptor): " << std::endl;
            warn << "Using 'applyCocktailMuons(...)', but config file says: ";
            warn << "'Muon.UseCocktail = " << m_muo_useCocktail << "'";
            warn << std::endl << std::endl;

            std::cerr << warn.str();
        }
    }
    for (auto &muon : m_muo_list)
        adaptMuon(muon);
}

// This function changes the Muon quantities from "normal" to "cocktail" and also adapts MET accordingly
// (Only doing this for the four momentum of the muon atm. To be extended in the future.)
void EventAdaptor::adaptMuon(pxl::Particle *muon) const
{
    // We are only interested in muons here.
    if (muon->getUserRecord("tightCharge") == 2)
    {
        muon->setUserRecord("validCocktail", true);
        // Change muon 4-vector
        auto cocktail = pxl::Particle();
        cocktail.setPtEtaPhiM((muon->getPt()) * (muon->getUserRecord("tunepRelPt").asFloat()),
                              muon->getEta(),
                              muon->getPhi(),
                              muon->getMass());

        // PAT px,py is substracted from cocktail px to take the sign flip for the
        // MET correction w.r.t. to the muon into account
        double const mass = muon->getMass();
        double const pxCocktail = cocktail.getPx();
        double const dpx = pxCocktail - muon->getPx();
        double const pyCocktail = cocktail.getPy();
        double const dpy = pyCocktail - muon->getPy();
        double const pzCocktail = cocktail.getPz();

        double const E =
            std::sqrt(mass * mass + pxCocktail * pxCocktail + pyCocktail * pyCocktail + pzCocktail * pzCocktail);

        muon->setP4(pxCocktail, pyCocktail, pzCocktail, E);

        // Adapt MET
        if (m_met)
        {
            double met_px = m_met->getPx() - dpx;
            double met_py = m_met->getPy() - dpy;
            double met_e = std::sqrt(met_px * met_px + met_py * met_py + m_met->getPz() * m_met->getPz());
            m_met->setP4(met_px, met_py, m_met->getPz(), met_e);
        }
    }
}

// it is not possible to do it anymore, using NanoAOD.
// This function changes the Muon quantities from "normal" (PF) to "HEEP" which is largely driven by
// super cluster information from the ECAL
void EventAdaptor::applyHEEPElectrons() const
{
    for (auto &ele : m_ele_list)
    {
        if (ele->getPt() > m_ele_switchpt)
            adaptEle(ele);
    }
}

// It is not possible to do it anymore, using NanoAOD: Electron SC information is no available.
void EventAdaptor::adaptEle(pxl::Particle *ele) const
{
    // Get Et and Eta from super cluster
    double const SCEt = ele->getUserRecord("SCEt");
    double const SCEta = ele->getUserRecord("SCeta");

    // check if SCeta has eta outside acceptance and show warning
    if (fabs(SCEta) > 3.)
    {
        std::cerr << "[WARNING] Electron with SCEta " << SCEta << " (>3/<-3) detected" << std::endl;
    }

    auto temp_lor = pxl::Particle();
    temp_lor.setPtEtaPhiM(SCEt, SCEta, ele->getPhi(), 0.);

    // PAT px,py is substracted from cocktail px to take the sign flip for the
    // MET correction w.r.t. to the ele into account
    double const dpx = temp_lor.getPx() - ele->getPx();
    double const dpy = temp_lor.getPy() - ele->getPy();

    // change ele 4-vector
    ele->setP4(temp_lor.getPx(), temp_lor.getPy(), temp_lor.getPz(), temp_lor.getE());

    // Adapt MET
    if (m_met)
    {
        double met_px = m_met->getPx() - dpx;
        double met_py = m_met->getPy() - dpy;
        double met_e = std::sqrt(met_px * met_px + met_py * met_py + m_met->getPz() * m_met->getPz());
        m_met->setP4(met_px, met_py, m_met->getPz(), met_e);
    }
}

// This function changes the AK8CHS jet kinematics to the matched PUPPI jet
void EventAdaptor::applyPUPPIFatJets() const
{
    for (auto &fatjet : m_fatjet_list)
    {
        if (fatjet->hasUserRecord("puppi_pt") and fatjet->getUserRecord("puppi_pt").toDouble() > 0)
            adaptFatJet(fatjet);
    }
}

// function to get puppi soft drop mass correction
float EventAdaptor::getPUPPIweight(const float puppipt, const float puppieta) const
{

    float genCorr = 1.;
    float recoCorr = 1.;
    float totalWeight = 1.;

    genCorr = puppisd_corrGEN.Eval(puppipt);
    if (fabs(puppieta) <= 1.3)
    {
        recoCorr = puppisd_corrRECO_cen.Eval(puppipt);
    }
    else
    {
        recoCorr = puppisd_corrRECO_for.Eval(puppipt);
    }

    totalWeight = genCorr * recoCorr;

    return totalWeight;
}

void EventAdaptor::adaptFatJet(pxl::Particle *fatjet) const
{
    TLorentzVector temp_lor = TLorentzVector();
    temp_lor.SetPtEtaPhiM(fatjet->getUserRecord("puppi_pt"),
                          fatjet->getUserRecord("puppi_eta"),
                          fatjet->getUserRecord("puppi_phi"),
                          fatjet->getUserRecord("puppi_mass"));
    // adapt met if puppi met is not used
    if (m_met_RecName != "slimmedMETsPuppi" and m_met)
    {
        double const dpx = temp_lor.Px() - fatjet->getPx();
        double const dpy = temp_lor.Py() - fatjet->getPy();
        double const met_px = m_met->getPx() - dpx;
        double const met_py = m_met->getPy() - dpy;
        double const met_e = std::sqrt(met_px * met_px + met_py * met_py + m_met->getPz() * m_met->getPz());
        m_met->setP4(met_px, met_py, m_met->getPz(), met_e);
    }
    fatjet->setP4(temp_lor.Px(), temp_lor.Py(), temp_lor.Pz(), temp_lor.E());
    fatjet->setUserRecord("puppi_softDropMassCorrected",
                          fatjet->getUserRecord("puppi_softDropMass").toDouble() *
                              getPUPPIweight(fatjet->getUserRecord("puppi_pt"), fatjet->getUserRecord("puppi_eta")));
}

// Wrapper to smear AK8
void EventAdaptor::applyFatJETMETSmearing(pxl::EventView const *GenEvtView,
                                          pxl::EventView const *RecEvtView,
                                          std::string const &linkName)
{
    applyJetMetSmearing(GenEvtView, RecEvtView, linkName, m_fatjet_RecName);
}

// Wrapper to smear AK4
void EventAdaptor::applyJETMETSmearing(pxl::EventView const *GenEvtView,
                                       pxl::EventView const *RecEvtView,
                                       std::string const &linkName)
{
    applyJetMetSmearing(GenEvtView, RecEvtView, linkName, m_jet_RecName);
}

// Obviously, this can only be done on MC!
void EventAdaptor::applyJetMetSmearing(pxl::EventView const *GenEvtView,
                                       pxl::EventView const *RecEvtView,
                                       std::string const &linkName,
                                       std::string const &recName)
{
    if (not m_jet_res_corr_use)
    {
        if (m_debug > 0)
        {
            std::stringstream warn;
            warn << "[WARNING] (EventAdaptor): " << std::endl;
            warn << "Using 'applyJETMETSmearing(...)', but config file says: ";
            warn << "'Jet.Resolutions.Corr.use = " << m_jet_res_corr_use << "'";
            warn << std::endl << std::endl;

            std::cerr << warn.str();
        }
    }
    // Get all Rec jets with the specified name in the config file.
    pxlParticles recJets;
    pxl::ParticlePtEtaNameCriterion const critJet(m_jet_RecName);
    pxl::ParticleFilter particleFilter;
    particleFilter.apply(RecEvtView->getObjectOwner(), recJets, critJet);

    if (m_debug > 2)
    {
        std::cerr << "[DEBUG] (EventAdaptor): RecEvtView before:" << std::endl;
        RecEvtView->print(1, std::cerr);
    }

    if (recJets.size() == 0)
    {
        if (m_debug > 1)
        {
            std::cerr << "[INFO] (EventAdaptor): " << std::endl;
            std::cerr << "No jets in this event, no changes applied!" << std::endl;
        }
        return;
    }

    // Sum up corrections for MET coming from jets.
    double fullCorrPx = 0.0;
    double fullCorrPy = 0.0;

    // Get gen jets
    // get all particles
    std::vector<pxl::Particle *> allparticles;
    std::vector<pxl::Particle *> gen_jets;
    GenEvtView->getObjectsOfType<pxl::Particle>(allparticles);
    pxl::sortParticles(allparticles);

    for (const auto part : allparticles)
    {
        if (part->getName() == recName)
        {
            gen_jets.push_back(part);
        }
    }

    pxlParticles::const_iterator part = recJets.begin();
    for (; part != recJets.end(); ++part)
    {
        pxl::Particle *recJet = *part;

        double jetPtCorrFactor = 1;
        if (recName == m_jet_RecName)
        {
            // Match gen jet
            const auto matchedJet =
                m_jet_res.matchGenJet(recJet, gen_jets, 0.4, GenEvtView->getUserRecord("NumVerticesPU").toDouble());
            jetPtCorrFactor =
                m_jet_res.getJetResolutionCorrFactor(recJet,
                                                     matchedJet,
                                                     RecEvtView->getUserRecord(m_jet_rho_label),
                                                     GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                     0);
        }
        else if (recName == m_fatjet_RecName)
        {
            const auto matchedJet =
                m_fatjet_res.matchGenJet(recJet, gen_jets, 0.8, GenEvtView->getUserRecord("NumVerticesPU").toDouble());
            jetPtCorrFactor =
                m_fatjet_res.getJetResolutionCorrFactor(recJet,
                                                        matchedJet,
                                                        RecEvtView->getUserRecord(m_jet_rho_label),
                                                        GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                        0);
        }
        else
            std::cerr << "[WARNING] (EventAdaptor): Unknown RecName '" << recName << "' used. No smearing."
                      << std::endl;

        double const mass = recJet->getMass();
        double const pt = recJet->getPt();
        double const eta = recJet->getEta();
        double const phi = recJet->getPhi();

        // In case the matched jet pt is too far off, we get jetPtCorrFactor = 0.
        // In case we have no matched jet, the Gaussian can (randomly) give you
        // jetPtCorrFactor < 0.
        // In both cases we remove the Rec jet from the event and correct the MET
        // by the full[!] jet pt.
        if (jetPtCorrFactor <= 0.0)
        {
            fullCorrPx -= pt * std::cos(phi);
            fullCorrPy -= pt * std::sin(phi);

            recJet->owner()->remove(recJet);
        }
        else
        {
            fullCorrPx += pt * (jetPtCorrFactor - 1) * std::cos(phi);
            fullCorrPy += pt * (jetPtCorrFactor - 1) * std::sin(phi);

            // Changing the jet in the event!
            TLorentzVector tmp_four_vec = TLorentzVector();
            tmp_four_vec.SetPtEtaPhiM(pt * jetPtCorrFactor, eta, phi, mass);
            recJet->setUserRecord("jetPtJERFactor", jetPtCorrFactor);
            recJet->setP4(tmp_four_vec.Px(), tmp_four_vec.Py(), tmp_four_vec.Pz(), tmp_four_vec.E());
        }
    }

    if (m_debug > 1)
    {
        std::cerr << "[INFO] (EventAdaptor): Cumulated jet corrections:" << std::endl;
        std::cerr << "px = " << fullCorrPx << std::endl;
        std::cerr << "py = " << fullCorrPy << std::endl;
    }

    // Don't have to correct MET, if there were no jets left in the event.
    if (recJets.size() > 0)
    {
        // Get all Rec METs with the specified name in the config file.
        pxlParticles recMETs;
        pxl::ParticlePtEtaNameCriterion const critMET(m_met_RecName);
        particleFilter.apply(RecEvtView->getObjectOwner(), recMETs, critMET);

        // Reuse iterator from above!
        for (part = recMETs.begin(); part != recMETs.end(); ++part)
        {
            // NOTE: The MET components and the sum of the corrections (fullCorr)
            // are both signed numbers. The minus sign in the following equations
            // takes all four combinations into account.
            double const newPx = (*part)->getPx() - fullCorrPx;
            double const newPy = (*part)->getPy() - fullCorrPy;

            // Changing the MET in the event!
            (*part)->setP4(newPx, newPy, 0.0, std::sqrt(newPx * newPx + newPy * newPy));
        }
    }

    if (m_debug > 2)
    {
        std::cerr << "[DEBUG] (EventAdaptor): RecEvtView after:" << std::endl;
        RecEvtView->print(1, std::cerr);
    }
}

// Adapt Double Ele Trigger as the standard "MW" Trigger was prescaled for a run period in 2016
void EventAdaptor::adaptDoubleEleTrigger(const int run, pxl::EventView *trigger_view)
{
    // Hard coded as this is supposed to be a fix for a single issue only which should not occur again
    if (run >= 276453 and run <= 278822)
    {
        const std::string new_trigger = "HLT_HLT_DoubleEle33_CaloIdL_MW_v";
        const std::string old_trigger = "HLT_HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_v";

        pxl::UserRecords allTriggerRecords = trigger_view->getUserRecords();
        for (pxl::UserRecords::const_iterator trigger = allTriggerRecords.begin(); trigger != allTriggerRecords.end();
             ++trigger)
        {
            if (trigger->first.find(old_trigger) != std::string::npos)
            {
                trigger_view->setUserRecord(new_trigger, 1);
                break;
            }
        }
    }
}
