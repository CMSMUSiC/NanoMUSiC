#include "Systematics.hh"
#include <vector>
#include "Tools/PXL/Sort.hh"

// ROOT includes
#include "TRandom3.h"
#include "TFile.h"

//--------------------Constructor-----------------------------------------------------------------

Systematics::Systematics(const Tools::MConfig &cfg, unsigned int const debug) : m_activeSystematics({}),
                                                                                // read uncertainties from config
                                                                                m_full(cfg.GetItem<bool>("General.Syst.fullview")),
                                                                                m_emptyShift(cfg.GetItem<bool>("General.Syst.emptyShift")),
                                                                                m_useJetEnergyResCorrection(cfg.GetItem<bool>("Jet.Resolutions.Corr.use")),
                                                                                m_ratioEleBarrel(cfg.GetItem<double>("Ele.Syst.Scale.Barrel")),
                                                                                m_ratioEleEndcap(cfg.GetItem<double>("Ele.Syst.Scale.Endcap")),
                                                                                m_ratioEleHEEP(cfg.GetItem<double>("Ele.Syst.Scale.HEEP")),
                                                                                m_ele_switchpt(cfg.GetItem<double>("Ele.ID.switchpt")),
                                                                                m_muoScaleMapFile(cfg.GetItem<std::string>("Muon.Syst.ScaleMap")),
                                                                                m_muoScaleType(cfg.GetItem<std::string>("Muon.Syst.ScaleType")),
                                                                                m_resMuo(cfg.GetItem<double>("Muon.Syst.Res")),
                                                                                m_ratioTau(cfg.GetItem<double>("Tau.Syst.Scale")),
                                                                                // read lepton names from config
                                                                                m_TauType(cfg.GetItem<std::string>("Tau.Type.Rec")),
                                                                                m_JetType(cfg.GetItem<std::string>("Jet.Type.Rec")),
                                                                                m_METType(cfg.GetItem<std::string>("MET.Type.Rec")),
                                                                                m_ratioGammaBarrel(cfg.GetItem<double>("Gamma.Syst.Scale.Barrel")),
                                                                                m_ratioGammaEndcap(cfg.GetItem<double>("Gamma.Syst.Scale.Endcap")),
                                                                                // To access the JEC uncertainties from file.
                                                                                m_jec_correction_set(correction::CorrectionSet::from_file(Tools::AbsolutePath(cfg.GetItem<std::string>("Jet.JSONFile")))),
                                                                                m_fat_jec_correction_set(correction::CorrectionSet::from_file(Tools::AbsolutePath(cfg.GetItem<std::string>("FatJet.JSONFile")))),
                                                                                m_jetRes(cfg, "Jet"),
                                                                                m_fatjetRes(cfg, "FatJet"),
                                                                                m_use_bJets(cfg.GetItem<bool>("Jet.BJets.use")),
                                                                                m_bJet_algo(cfg.GetItem<std::string>("Jet.BJets.Algo")),
                                                                                m_bJet_discriminatorThreshold(cfg.GetItem<float>("Jet.BJets.Discr.min")),
                                                                                m_bJets_sfMethod(cfg.GetItem<std::string>("Jet.BJets.SF.Method")), // which method of scale factor to apply
                                                                                // Method name tags should be constructed following the sheme:
                                                                                // ParticleType_ShiftType e.g. Ele_Scale
                                                                                // std::map<std::string, std::function<void()>>
                                                                                systFuncMap(
                                                                                    {{"Muon_Scale", std::bind(&Systematics::shiftObjAndMET, this, "Muon", "Scale")},
                                                                                     {"Muon_Resolution", std::bind(&Systematics::shiftObjAndMET, this, "Muon", "Resolution")},
                                                                                     {"Ele_Scale", std::bind(&Systematics::shiftObjAndMET, this, "Ele", "Scale")},
                                                                                     {"Ele_Resolution", std::bind(&Systematics::shiftObjAndMET, this, "Ele", "Resolution")},
                                                                                     {"Tau_Scale", std::bind(&Systematics::shiftObjAndMET, this, "Tau", "Scale")},
                                                                                     {"Gamma_Scale", std::bind(&Systematics::shiftObjAndMET, this, "Gamma", "Scale")},
                                                                                     {"Jet_Resolution", std::bind(&Systematics::shiftObjAndMET, this, "Jet", "Resolution")},
                                                                                     {"Jet_Scale", std::bind(&Systematics::shiftObjAndMET, this, "Jet", "Scale")},
                                                                                     {"Jet_BJetsSF2A", std::bind(&Systematics::shiftBJetsSF2A, this, "Jet", "BJetsSF2A")},
                                                                                     {"FatJet_Resolution", std::bind(&Systematics::shiftObjAndMET, this, "FatJet", "Resolution")},
                                                                                     {"FatJet_Scale", std::bind(&Systematics::shiftObjAndMET, this, "FatJet", "Scale")},
                                                                                     {"MET_Scale", std::bind(&Systematics::shiftMETUnclustered, this, "Scale")}}),
                                                                                m_particleUseMap(Tools::getConfigParticleMap(cfg, "use", true)),
                                                                                m_particleIDTypeMap(Tools::getConfigParticleMap(cfg, "ID.Type", std::string("dummy"))),
                                                                                m_particleRecoNameMap(getConfigParticleReverseMap(cfg, "Type.Rec", std::string("dummy"))),
                                                                                m_gen_rec_map(cfg),
                                                                                m_debug(debug),
                                                                                m_particleMap(std::map<std::string, std::vector<pxl::Particle *>>()),
                                                                                m_jet_rho_label(cfg.GetItem<std::string>("Jet.Rho.Label"))
{
   auto year = cfg.GetItem<std::string>("year");

   // build JEC evaluators
   // Jet
   std::map<std::string, std::string> jet_JEC_key = {
       {"2016APV", "Summer20UL16APV_V7_MC_" + cfg.GetItem<std::string>("Jet.Error.JESType") + "_AK4PFchs"},
       {"2016", "Summer20UL16_V7_MC_" + cfg.GetItem<std::string>("Jet.Error.JESType") + "_AK4PFchs"},
       {"2017", "Summer19UL17_V5_MC_" + cfg.GetItem<std::string>("Jet.Error.JESType") + "_AK4PFchs"},
       {"2018", "Summer19UL18_V5_MC_" + cfg.GetItem<std::string>("Jet.Error.JESType") + "_AK4PFchs"},
   };

   m_jecUnc = m_jec_correction_set->at(jet_JEC_key[year]);

   // FatJet
   std::map<std::string, std::string> fat_jet_JEC_key = {
       {"2016APV", "Summer20UL16APV_V7_MC_" + cfg.GetItem<std::string>("FatJet.Error.JESType") + "_AK8PFPuppi"},
       {"2016", "Summer20UL16_V7_MC_" + cfg.GetItem<std::string>("FatJet.Error.JESType") + "_AK8PFPuppi"},
       {"2017", "Summer19UL17_V5_MC_" + cfg.GetItem<std::string>("FatJet.Error.JESType") + "_AK8PFPuppi"},
       {"2018", "Summer19UL18_V5_MC_" + cfg.GetItem<std::string>("FatJet.Error.JESType") + "_AK8PFPuppi"},
   };

   m_fatjecUnc = m_fat_jec_correction_set->at(fat_jet_JEC_key[year]);

   rand = new TRandom3();

   std::vector<std::string> availableFunctions;
   for (auto entry : systFuncMap)
      availableFunctions.push_back(entry.first);
   // read in which systematics should be evaluated
   // loop all considered objects (Muo, Ele ...)
   for (auto &part_name_use : m_particleUseMap)
   {
      if (!part_name_use.second)
         continue;
      // add all selected systematic types
      auto systTypes = Tools::splitString<std::string>(
          cfg.GetItem<std::string>(part_name_use.first + ".Syst.Types"),
          true);
      for (const std::string &systType : systTypes)
      {
         std::string funcKey = part_name_use.first + "_" + systType;
         bool isAvailable = (std::find(availableFunctions.begin(),
                                       availableFunctions.end(),
                                       funcKey) != availableFunctions.end());
         if (isAvailable)
         {
            SystematicsInfo *thisSystematic =
                new SystematicsInfo(part_name_use.first, systType, funcKey);
            m_activeSystematics.push_back(thisSystematic);
         }
         else
         {
            std::cout << "Systematic type " << systType
                      << " is not available for " << part_name_use.first << std::endl;
            exit(1);
         }
      }
   }
   // load muon scale map if muons are used
   if (m_particleUseMap["Muon"])
   {
      m_muon_scale_ratio_hist = static_cast<TH2D *>(TFile(m_muoScaleMapFile.c_str()).Get("h2_muo_scale"));
   }
}

//--------------------Destructor------------------------------------------------------------------

Systematics::~Systematics()
{
   delete rand;
}

//------------
// public------
//------------

void Systematics::createShiftedViews()
{
   for (auto syst : m_activeSystematics)
   {
      m_activeSystematic = syst;
      // Method name tags should be constructed following the sheme:
      // ParticleType_ShiftType e.g. Ele_Scale
      // std::map<std::string, std::function<void()>>
      if (syst->m_isDifferential)
         systFuncMap[syst->m_funcKey]();
   }
}

// get all particles from the event and put them into vectors for later use
void Systematics::init(pxl::Event *event)
{
   m_event = event;
   m_eventView = m_event->getObjectOwner().findObject<pxl::EventView>("Rec");
   if (m_eventView == 0)
   {
      std::cout << "No EventView 'Rec' found in event:" << m_event->toString() << std::endl;
      return;
   }

   // clear shifted EventViews from last event
   for (auto syst : m_activeSystematics)
   {
      syst->eventViewPointers.clear();
   }

   // get all particles
   std::vector<pxl::Particle *> AllParticles;
   m_eventView->getObjectsOfType<pxl::Particle>(AllParticles);
   pxl::sortParticles(AllParticles);
   std::string persistent_id;
   // create a empty particle map
   std::map<std::string, std::vector<pxl::Particle *>> particleMap;
   for (auto &part_name_use : m_particleUseMap)
   {
      if (part_name_use.second)
         particleMap[part_name_use.first] = std::vector<pxl::Particle *>();
   }
   for (auto &part : AllParticles)
   {
      // particles which should not be shifted have IDpassed = false if
      // ID.Tag option is used or are already removed otherwise
      std::string partName = part->getName();
      // check if particle is used
      if (m_particleUseMap[m_particleRecoNameMap[partName]])
      {
         if (!(part->getUserRecord("systIDpassed").toBool()))
            continue;
         // mark both entries with same persistent ID ( standard ID changes when cloning views)
         persistent_id = Tools::random_string(16);
         part->setUserRecord("persistent_id", persistent_id);
         particleMap[m_particleRecoNameMap[partName]].push_back(part);
      }
   }
   // clear and replace member variable m_particleMap
   // push them into the corresponding vectors
   m_particleMap.clear();
   m_particleMap = particleMap;

   // clear gen  lists from previous event
   UnclusteredEnUp = nullptr;
   UnclusteredEnDown = nullptr;

   // the met shifts are stored in the gen met, becase they are only in MC
   // but they are reco objects
   m_GenEvtView = m_event->getObjectOwner().findObject<pxl::EventView>("Gen");
   std::vector<pxl::Particle *> GenParticles;
   m_GenEvtView->getObjectsOfType<pxl::Particle>(GenParticles);
   for (auto &part : GenParticles)
   {
      std::string Name = part->getName();
      // Only fill the collection if we want to use the particle!
      // copy already shifted MET from Event:
      if (Name == m_METType + "uncert_10")
         UnclusteredEnUp = part;
      else if (Name == m_METType + "uncert_11")
         UnclusteredEnDown = part;
      persistent_id = Tools::random_string(16);
      part->setUserRecord("persistent_id", persistent_id);
   }

   return;
}

std::list<std::string> Systematics::getAllSystNames() const
{
   std::list<std::string> result;
   for (auto syst : m_activeSystematics)
   {
      std::string partName = syst->m_particleType;
      if (partName == "MET")
      {
         partName = m_METType;
      }
      std::string prefix = partName + std::string("_syst") + syst->m_sysType;
      std::cout << prefix << std::endl;
      result.push_back(prefix + "Up");
      result.push_back(prefix + "Down");
   }
   return result;
}

//------------
// private-----
//------------

void Systematics::shiftMETUnclustered(const std::string &shiftType)
{

   // from MUSiCSkimmer_miniAOD.cc:
   // Get systmetShifts:
   // enum   METUncertainty {
   // JetEnUp =0, JetEnDown =1, JetResUp =2, JetResDown =3,
   // MuonEnUp =4, MuonEnDown =5, ElectronEnUp =6, ElectronEnDown =7,
   // TauEnUp =8, TauEnDown =9, UnclusteredEnUp =10, UnclusteredEnDown =11,
   // METUncertaintySize =12
   //}

   std::string find = m_METType + "uncert_";
   std::string prefix = m_METType + "_syst" + shiftType;
   pxl::EventView *evup = nullptr;
   pxl::EventView *evdown = nullptr;
   pxl::Particle *part = nullptr;

   createEventView(prefix, "Up", evup);
   createEventView(prefix, "Down", evdown);
   // create previously copied uncert MET

   if (m_particleMap["MET"].size() == 0)
      return;
   pxl::Particle *baseMET = m_particleMap["MET"][0];
   pxl::Particle *matched_unshifted_met = nullptr;
   if (UnclusteredEnUp != nullptr)
   {
      if (m_full)
         matched_unshifted_met = matchParticle(evup, baseMET);
      part = evup->getObjectOwner().create<pxl::Particle>(baseMET);
      auto vec = UnclusteredEnUp->getVector();
      part->setVector(vec);
      if (m_full)
         evup->removeObject(matched_unshifted_met);
   }
   if (UnclusteredEnDown != nullptr)
   {
      if (m_full)
         matched_unshifted_met = matchParticle(evdown, baseMET);
      part = evdown->getObjectOwner().create<pxl::Particle>(baseMET);
      auto vec = UnclusteredEnDown->getVector();
      part->setVector(vec);
      if (m_full)
         evdown->removeObject(matched_unshifted_met);
   }

   return;
}

void Systematics::removeParticleFromView(pxl::EventView *ev, const pxl::Particle *obj) const
{
   auto matched_particle = matchParticle(ev, obj);
   if (matched_particle)
      ev->removeObject(matched_particle);
   else
   {
      std::string message = "Systematics::removeParticleFromView: No matching particle found!";
      throw std::runtime_error(message);
   }
}

void Systematics::shiftBJetsSF2AUpDown(pxl::EventView *eview,
                                       pxl::Particle *obj,
                                       const std::string &suffix)
{
   bool isBJet = false;
   double bjetscalefactor = 1.0;
   double fraction = 0.0;
   double bjet_sf2a_randnum = 0.0;
   double bjet_sf2a_MCEfficiency = -1.0;
   double bjetdiscrim = 0.0;
   // for up variation
   pxl::Particle *jet = eview->getObjectOwner().create<pxl::Particle>(obj);
   bjetdiscrim = jet->getUserRecord(m_bJet_algo);
   isBJet = (bjetdiscrim > m_bJet_discriminatorThreshold);
   bjetscalefactor = jet->getUserRecord("bjet_sf2a_sf_" + suffix);
   bjet_sf2a_MCEfficiency = jet->getUserRecord("bjet_sf2a_MCEfficiency");
   bjet_sf2a_randnum = jet->getUserRecord("bjet_sf2a_randnum");
   jet->setUserRecord("isBjet", isBJet);
   if (bjetscalefactor < 1.0)
   {
      fraction = 1.0 - bjetscalefactor;
      if (isBJet and bjet_sf2a_randnum < fraction)
      {
         jet->setUserRecord("isBjet", "false");
      }
   }
   else
   {
      fraction = (1.0 - bjetscalefactor) / (1.0 - 1.0 / bjet_sf2a_MCEfficiency);
      if (not isBJet and bjet_sf2a_randnum < fraction)
      {
         jet->setUserRecord("isBjet", "true");
      }
   }

   // if full event view was created remove original particle
   if (m_full)
   {
      auto matched_particle = matchParticle(eview, jet);
      if (matched_particle)
         eview->removeObject(matched_particle);
      else
      {
         std::string message = "Systematics::shiftParticle: No matching particle found!";
         throw std::runtime_error(message);
      }
   }
}

// For bJet BTag Systematics of Type 2A
void Systematics::shiftBJetsSF2A(std::string const &partName,
                                 std::string const &shiftType)
{
   /*
    * For BTag Scale factor of Type 2A
    */
   // Do nothing if BJets are not used or SF2A not set
   // if ( (not m_use_bJets) or (m_bJets_sfMethod != "2A") ) return;
   if (not m_use_bJets)
      return;

   if (m_bJets_sfMethod != "2A")
   {
      throw std::runtime_error("BJet SF Method != 2A but systematic shift called for 2A");
   }

   // Do nothing if no object in event
   if (not m_emptyShift and m_particleMap[partName].size() == 0)
      return;

   std::string prefix = partName + std::string("_syst") + shiftType;
   pxl::EventView *evup = 0;
   pxl::EventView *evdown = 0;

   createEventView(prefix, "Up", evup);
   createEventView(prefix, "Down", evdown);

   // loop over jets and change the BTag status
   for (auto &obj : m_particleMap[partName])
   {
      shiftBJetsSF2AUpDown(evup, obj, "up");
      shiftBJetsSF2AUpDown(evdown, obj, "do");
   }
}

std::pair<double, double> Systematics::getObjScaleShift(pxl::Particle *obj,
                                                        std::string const &partName)
{

   //////////
   // Muons
   /////////
   if (partName == "Muon")
   {
      // Apply momentum scale uncertainty as additive curvature bias kappa with
      // uncertainty sigma_kappa
      // q/pT' = q/pT + (kappa +- sigma_kappa) -> pT/pT' = 1 + (kappa +- sigma_kappa) * pT / q
      // Reference and explanation:  https://indico.cern.ch/event/601852/
      double shift = m_muon_scale_ratio_hist->GetBinContent(
          m_muon_scale_ratio_hist->FindBin(obj->getEta(), obj->getPhi()));
      double uncertainty = m_muon_scale_ratio_hist->GetBinError(
          m_muon_scale_ratio_hist->FindBin(obj->getEta(), obj->getPhi()));

      // Central value correction + gaussian uncertainty
      if (std::abs(obj->getEta()) < 1.2)
      {
         shift = 0.0;
         uncertainty = 0.025;
      }
      double uncert = rand->Gaus(shift, uncertainty) * obj->getPt() / 1000. / obj->getCharge();
      if (m_muoScaleType == "symmetric")
      {
         const auto ratio = 1 + uncert;
         return std::make_pair(1. / ratio, 1. / ratio);
      }
      else if (m_muoScaleType == "asymmetric")
      {
         const auto ratio = 1 + uncert;
         return std::make_pair(1., 1. / ratio);
      }
      else if (m_muoScaleType == "smeared")
      {
         return std::make_pair(1. / (1 - std::min(uncert, 0.)), 1. / (1 + uncert));
      }
      else
      {
         std::stringstream err;
         err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
         err << "Unknown muon syst type " << m_muoScaleType << std::endl;
         throw Tools::value_error(err.str());
      }
   }
   //////////////
   // Electrons
   /////////////
   bool no_region = false;
   if (partName == "Ele")
   {
      if (obj->getPt() > m_ele_switchpt)
      {
         return std::make_pair(1. + m_ratioEleHEEP, 1. - m_ratioEleHEEP);
      }
      if (obj->getUserRecord("isBarrel").toBool())
      {
         return std::make_pair(1. + m_ratioEleBarrel, 1. - m_ratioEleBarrel);
      }
      else if (obj->getUserRecord("isEndcap").toBool())
      {
         return std::make_pair(1. + m_ratioEleEndcap, 1. - m_ratioEleEndcap);
      }
      else
      {
         no_region = true;
      }
   }
   /////////
   // Taus
   ////////
   if (partName == "Tau")
   {
      return std::make_pair(1. + m_ratioTau, 1. - m_ratioTau);
   }

   ///////////
   // Gammas
   //////////
   if (partName == "Gamma")
   {
      if (obj->getUserRecord("isBarrel").toBool())
      {
         return std::make_pair(1. + m_ratioGammaBarrel, 1. - m_ratioGammaBarrel);
      }
      else if (obj->getUserRecord("isEndcap").toBool())
      {
         return std::make_pair(1. + m_ratioGammaEndcap, 1. - m_ratioGammaEndcap);
      }
      else
      {
         no_region = true;
      }
   }

   //////////
   // Jets
   /////////
   if (partName == "Jet")
   {
      double ratio = m_jecUnc->evaluate({obj->getEta(), obj->getPt()});
      return std::make_pair(1. + ratio, 1. - ratio);
   }

   if (partName == "FatJet")
   {
      double ratio = m_fatjecUnc->evaluate({obj->getEta(), obj->getPt()});
      return std::make_pair(1. + ratio, 1. - ratio);
   }

   // Final check for Gammas and eles
   if (no_region)
   {
      throw std::runtime_error(std::string("Systematics.cc:") + partName + std::string("must be in either endcap or barrel and have an id"));
   }

   throw std::runtime_error(std::string("Systematics.cc:  No scale shift value found for: ") + partName);
}

std::pair<double, double> Systematics::getObjResolutionShift(pxl::Particle *obj, std::string const &partName)
{
   if (partName == "Muon")
   {
      // Apply Gaussian smearing to transverse momentum
      // Reference and explanation: https://indico.cern.ch/event/601852/
      // Determine value of smearing based on muon pT
      double smearing = 0.0;
      if (obj->getPt() < 200.)
      {
         smearing = 0.003;
      }
      else if (obj->getPt() < 500.)
      {
         smearing = 0.005;
      }
      else
      {
         smearing = 0.01;
      }
      // Double smearing for muons in the endcaps
      if (std::abs(obj->getEta()) > 1.2)
         smearing *= 2;
      // Return Gaussian smearing
      double ratio = rand->Gaus(0, smearing);
      return std::make_pair(1 + ratio, 1.);
   }

   if (partName == "Ele")
   {
      // Apply Gaussian smearing to transverse momentum
      double smearing = 0.012;
      // Double smearing for electron in the endcaps
      if (std::abs(obj->getEta()) > 1.5)
         smearing *= 2;
      // Return Gaussian smearing
      double ratio = rand->Gaus(0, smearing);
      return std::make_pair(1 + ratio, 1.);
   }

   if (partName == "Jet")
   {
      double ratio_up = 1.;
      double ratio_down = 1.;
      if (m_useJetEnergyResCorrection)
      {
         pxl::EventView *RecEvtView = m_event->getObjectOwner().findObject<pxl::EventView>("Rec");
         if (obj->getSoftRelations().hasName("priv-gen-rec"))
         {
            pxl::Particle *genPart = dynamic_cast<pxl::Particle *>(
                obj->getSoftRelations().getFirst(m_GenEvtView->getObjectOwner(), "priv-gen-rec"));
            ratio_up = m_jetRes.getJetResolutionCorrFactor(obj,
                                                           genPart,
                                                           RecEvtView->getUserRecord(m_jet_rho_label),
                                                           m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                           1);
            ratio_down = m_jetRes.getJetResolutionCorrFactor(obj,
                                                             genPart,
                                                             RecEvtView->getUserRecord(m_jet_rho_label),
                                                             m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                             -1);
         }
         else
         {
            ratio_up = m_jetRes.getJetResolutionCorrFactor(obj,
                                                           0,
                                                           RecEvtView->getUserRecord(m_jet_rho_label),
                                                           m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                           1);
            ratio_down = m_jetRes.getJetResolutionCorrFactor(obj,
                                                             0,
                                                             RecEvtView->getUserRecord(m_jet_rho_label),
                                                             m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                             -1);
         }
      }
      return std::make_pair(ratio_up, ratio_down);
   }

   if (partName == "FatJet")
   {
      double ratio_up = 1.;
      double ratio_down = 1.;
      if (m_useJetEnergyResCorrection)
      {
         pxl::EventView *RecEvtView = m_event->getObjectOwner().findObject<pxl::EventView>("Rec");
         if (obj->getSoftRelations().hasName("priv-gen-rec"))
         {
            pxl::Particle *genPart = dynamic_cast<pxl::Particle *>(
                obj->getSoftRelations().getFirst(m_GenEvtView->getObjectOwner(), "priv-gen-rec"));
            ratio_up = m_fatjetRes.getJetResolutionCorrFactor(obj,
                                                              genPart,
                                                              RecEvtView->getUserRecord(m_jet_rho_label),
                                                              m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                              1);
            ratio_down = m_fatjetRes.getJetResolutionCorrFactor(obj,
                                                                genPart,
                                                                RecEvtView->getUserRecord(m_jet_rho_label),
                                                                m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                                -1);
         }
         else
         {
            ratio_up = m_fatjetRes.getJetResolutionCorrFactor(obj,
                                                              0,
                                                              RecEvtView->getUserRecord(m_jet_rho_label),
                                                              m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                              1);
            ratio_down = m_fatjetRes.getJetResolutionCorrFactor(obj,
                                                                0,
                                                                RecEvtView->getUserRecord(m_jet_rho_label),
                                                                m_GenEvtView->getUserRecord("NumVerticesPU").toDouble(),
                                                                -1);
         }
      }
      return std::make_pair(ratio_up, ratio_down);
   }
   return std::make_pair(1., 1.);
}

std::pair<double, double> Systematics::getObjShiftValue(pxl::Particle *obj,
                                                        std::string const &partName,
                                                        std::string const &shiftType)
{
   if (shiftType == "Scale")
   {
      return getObjScaleShift(obj, partName);
   }
   else if (shiftType == "Resolution")
   {
      return getObjResolutionShift(obj, partName);
   }
   return std::make_pair(1., 1.);
}

void Systematics::shiftObjAndMET(std::string const &partName,
                                 std::string const &shiftType)
{
   /*
    * Generic implementation of systematic handling for scale uncerts.
    * ratio is interpreted for whole range if no ratio_endcap passed.
    */
   // Do nothing if no object in event
   if (not m_emptyShift and m_particleMap[partName].size() == 0)
      return;

   double dPx_up = 0;
   double dPy_up = 0;
   double dPx_down = 0;
   double dPy_down = 0;
   std::string prefix = partName + std::string("_syst") + shiftType;
   pxl::EventView *evup = 0;
   pxl::EventView *evdown = 0;

   createEventView(prefix, "Up", evup);
   createEventView(prefix, "Down", evdown);

   // add shifted particles to these EventViews
   for (auto &obj : m_particleMap[partName])
   {
      // "isBarrel" and "isEndcap" only set for reconstructed data skimmed by MUSiCSkimmer_miniAOD.cc
      std::pair<double, double> ratio_pair = getObjShiftValue(obj, partName, shiftType);
      shiftParticle(evup, obj, ratio_pair.first, dPx_up, dPy_up);
      shiftParticle(evdown, obj, ratio_pair.second, dPx_down, dPy_down);
   }

   METUp = nullptr;
   METDown = nullptr;

   // return if no MET object is present
   if (!m_particleMap["MET"].size())
   {
      return;
   }
   initMET(evup, METUp);
   initMET(evdown, METDown);

   shiftMET(dPx_up, dPy_up, METUp);
   shiftMET(dPx_down, dPy_down, METDown);

   return;
}

void Systematics::createEventView(const std::string &prefix, const std::string &suffix, pxl::EventView *&ev)
{
   bool success;

   // create new EventViews inside the event using deep copy and drop all objects
   // this preserves user record for the event view itself.
   // Shifted objects will be added in further systematics computation
   ev = m_event->getObjectOwner().create<pxl::EventView>(m_eventView);
   if (not m_full)
      ev->clearObjects();
   if (not ev)
      throw std::runtime_error("Systematics.cc: creating an event view failed!");
   success = m_event->getObjectOwner().setIndexEntry(prefix + suffix, ev);
   ev->setName(prefix + suffix);
   m_activeSystematic->eventViewPointers.push_back(ev);
   if (!success)
   {
      std::string message = "Systematics.cc: setIndex for event view" + prefix + suffix + " failed!";
      throw std::runtime_error(message);
   }
   return;
}

pxl::Particle *Systematics::matchParticle(const pxl::EventView *ev, const pxl::Particle *part) const
{
   std::vector<pxl::Particle *> AllParticlessShifted;
   ev->getObjectsOfType<pxl::Particle>(AllParticlessShifted);
   for (auto &partShifted : AllParticlessShifted)
   {
      if (part->hasUserRecord("persistent_id") &&
          partShifted->hasUserRecord("persistent_id") &&
          partShifted->getUserRecord("persistent_id") == part->getUserRecord("persistent_id"))
         return partShifted;
   }
   return nullptr;
}

void Systematics::initMET(pxl::EventView *ev, pxl::Particle *&metpart)
{
   // create new MET
   pxl::Particle *shiftedMET;
   for (auto &met : m_particleMap["MET"])
   {
      // if full event view was created remove original particle
      if (m_full)
      {
         removeParticleFromView(ev, met);
      }
      shiftedMET = ev->getObjectOwner().create<pxl::Particle>(met);
      metpart = shiftedMET;
   }
   return;
}

void Systematics::shiftParticle(pxl::EventView *eview, pxl::Particle *const part, double const &ratio, double &dPx, double &dPy)
{
   // create a copy of the original particle
   pxl::Particle *shiftedParticle = eview->getObjectOwner().create<pxl::Particle>(part);
   // add the shifted part up
   dPx += shiftedParticle->getPx() * (ratio - 1);
   dPy += shiftedParticle->getPy() * (ratio - 1);
   // WARNING change the particle content for the particle in the new event view
   shiftedParticle->setP4(ratio * shiftedParticle->getPx(),
                          ratio * shiftedParticle->getPy(),
                          ratio * shiftedParticle->getPz(),
                          ratio * shiftedParticle->getE());

   // if full event view was created remove original particle
   if (m_full)
   {
      auto matched_particle = matchParticle(eview, part);
      if (matched_particle)
         eview->removeObject(matched_particle);
      else
      {
         std::string message = "Systematics::shiftParticle: No matching particle found!";
         throw std::runtime_error(message);
      }
   }
   return;
}

// change according MET
void Systematics::shiftMET(double const dPx,
                           double const dPy,
                           pxl::Particle *&shiftedMET)
{

   double Px, Py, E;

   Px = shiftedMET->getPx() - dPx;
   Py = shiftedMET->getPy() - dPy;
   E = std::sqrt(Px * Px + Py * Py);
   shiftedMET->setP4(Px, Py, 0., E);

   return;
}
