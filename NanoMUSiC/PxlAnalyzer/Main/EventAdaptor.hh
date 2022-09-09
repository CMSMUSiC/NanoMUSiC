#ifndef EVENTADAPTOR
#define EVENTADAPTOR

#include "Main/GenRecNameMap.hh"
#include "Main/JetResolution.hh"

#include "TF1.h"

namespace pxl
{
class EventView;
class Particle;
} // namespace pxl

namespace Tools
{
class MConfig;
}

class EventAdaptor
{
  public:
    typedef std::vector<pxl::Particle *> pxlParticles;
    EventAdaptor(Tools::MConfig const &cfg, unsigned int const debug = 1);
    ~EventAdaptor()
    {
    }

    void initEvent(pxl::EventView const *RecEvtView);
    void applyCocktailMuons() const;
    // void applyHEEPElectrons() const;
    void applyPUPPIFatJets() const;

    void applyJETMETSmearing(pxl::EventView const *GenEvtView, pxl::EventView const *RecEvtView,
                             std::string const &linkName);
    void applyFatJETMETSmearing(pxl::EventView const *GenEvtView, pxl::EventView const *RecEvtView,
                                std::string const &linkName);
    static void adaptDoubleEleTrigger(const int run, pxl::EventView *trigger_view);

  private:
    void adaptMuon(pxl::Particle *muon) const;
    // void adaptEle( pxl::Particle *ele ) const;
    void adaptFatJet(pxl::Particle *fatjet) const;
    float getPUPPIweight(const float puppipt, const float puppieta) const;
    unsigned int const m_debug;

    GenRecNameMap const m_gen_rec_map;

    JetResolution m_jet_res;
    JetResolution m_fatjet_res;

    pxl::Particle *m_met;
    std::vector<pxl::Particle *> m_muo_list;
    std::vector<pxl::Particle *> m_ele_list;
    std::vector<pxl::Particle *> m_fatjet_list;

    bool const m_muo_useCocktail;
    double const m_muo_highptid_ptRelativeError_max; // maximum relative error on pt
    double m_ele_switchpt;                           // energy to swith between PF and HEEP
    bool const m_jet_res_corr_use;
    std::string const m_fatjet_mass_corr_file;

    TF1 puppisd_corrGEN;
    TF1 puppisd_corrRECO_cen;
    TF1 puppisd_corrRECO_for;
    std::string const m_ele_RecName;
    std::string const m_muo_RecName;
    std::string const m_jet_RecName;
    std::string const m_fatjet_RecName;
    std::string const m_met_RecName;

    std::string const m_jet_rho_label;

    void applyJetMetSmearing(pxl::EventView const *GenEvtView, pxl::EventView const *RecEvtView,
                             std::string const &linkName, std::string const &recName);
};

#endif /*EVENTADAPTOR*/
