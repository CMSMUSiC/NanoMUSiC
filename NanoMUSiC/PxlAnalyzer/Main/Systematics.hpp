#ifndef Systematics_hh
#define Systematics_hh

#include "GenRecNameMap.hpp"
#include "JetResolution.hpp"
#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include "SystematicsInfo.hpp"
#include "Tools.hpp"
#include "Tools/MConfig.hpp"
#include <functional>
#include <iostream>
#include <string>

#include "TH2D.h"

// correction lib
#include "correction.h"

class TRandom3;

class Systematics
{
  public:
    Systematics(const Tools::MConfig &cfg, unsigned int const debug);
    ~Systematics();

    std::vector<SystematicsInfo *> m_activeSystematics;
    void init(pxl::Event *event);
    void createShiftedViews();
    std::list<std::string> getAllSystNames() const;

  private:
    // variables
    bool m_full;
    bool m_emptyShift;
    bool m_useJetEnergyResCorrection;
    // Electron variables
    double const m_ratioEleBarrel, m_ratioEleEndcap, m_ratioEleHEEP;
    double const m_ele_switchpt;
    // Muon variables
    const std::string m_muoScaleMapFile;
    const std::string m_muoScaleType;
    TH2D *m_muon_scale_ratio_hist;
    const double m_resMuo;
    // Tau variables
    const double m_ratioTau;
    // Type identifiers
    std::string const m_TauType, m_JetType, m_METType;
    // Gamma variable
    const double m_ratioGammaBarrel, m_ratioGammaEndcap;
    // To access the JEC uncertainties from file.
    // New recipe:
    // https://twiki.cern.ch/twiki/bin/view/CMS/JECUncertaintySources?rev=19#Code_example
    std::string const m_jecType;
    // JetCorrectorParameters const m_jecPara;
    // JetCorrectorParameters const m_fatjecPara;
    // JetCorrectionUncertainty m_jecUnc;
    // JetCorrectionUncertainty m_fatjecUnc;
    std::unique_ptr<correction::CorrectionSet> m_jec_correction_set;
    std::unique_ptr<correction::CorrectionSet> m_fat_jec_correction_set;
    correction::Correction::Ref m_jecUnc;
    correction::Correction::Ref m_fatjecUnc;
    JetResolution m_jetRes;
    JetResolution m_fatjetRes;

    // For bJets
    bool m_use_bJets;
    std::string const m_bJet_algo;             // What criterion do we check?
    float const m_bJet_discriminatorThreshold; // What threshold is used?
    std::string const m_bJets_sfMethod;

    // method map for function calls by string
    // Method name tags should be constructed following the sheme:
    // ParticleType_ShiftType e.g. Ele_Scale
    std::map<std::string, std::function<void()>> systFuncMap;

    // Map with .use option for each particle type
    std::map<std::string, bool> m_particleUseMap;
    // map with ID.Type option for each particle type
    std::map<std::string, std::string> m_particleIDTypeMap;
    // map with Type.Rec option for each particle type
    std::map<std::string, std::string> m_particleRecoNameMap;
    GenRecNameMap const m_gen_rec_map;

    unsigned int const m_debug;

    pxl::Event *m_event;
    pxl::EventView *m_eventView;
    std::map<std::string, std::vector<pxl::Particle *>> m_particleMap;
    pxl::Particle *METUp;
    pxl::Particle *METDown;
    pxl::Particle *UnclusteredEnUp;
    pxl::Particle *UnclusteredEnDown;

    std::string const m_jet_rho_label;

    pxl::EventView *m_GenEvtView;

    TRandom3 *rand;
    //~ std::vector< std::reference_wrapper<SystematicsInfo> > m_activeSystematics;
    // methods
    SystematicsInfo *m_activeSystematic;
    void shiftMETUnclustered(const std::string &shiftType);
    void shiftBJetsSF2AUpDown(pxl::EventView *eview, pxl::Particle *obj, const std::string &suffix);
    void shiftBJetsSF2A(std::string const &partName, std::string const &shiftType);
    pxl::Particle *matchParticle(const pxl::EventView *ev, const pxl::Particle *part) const;
    void removeParticleFromView(pxl::EventView *ev, const pxl::Particle *obj) const;
    std::pair<double, double> getObjScaleShift(pxl::Particle *obj, std::string const &partName);
    std::pair<double, double> getObjResolutionShift(pxl::Particle *obj, std::string const &partName);
    void shiftObjAndMET(std::string const &partName, std::string const &shiftType);
    std::pair<double, double> getObjShiftValue(pxl::Particle *obj, std::string const &partName,
                                               std::string const &shiftType);
    void createEventViews(std::string prefix, pxl::EventView **evup, pxl::EventView **evdown);
    void createEventView(const std::string &prefix, const std::string &suffix, pxl::EventView *&ev);
    void initMET(pxl::EventView *ev, pxl::Particle *&metpart);
    void shiftParticle(pxl::EventView *eview, pxl::Particle *const part, double const &ratio, double &dPx, double &dPy);

    void shiftMET(double const dPx, double const dPy, pxl::Particle *&shiftedMET);
};
#endif /*Systematics_hh*/
