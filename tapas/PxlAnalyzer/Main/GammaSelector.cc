#include "GammaSelector.hh"

GammaSelector::GammaSelector( const Tools::MConfig &cfg, OldNameMapper *globalOldNameMap ):
   // Add parent constructor
   ObjectSelector( cfg, globalOldNameMap, "Gamma", true, "SCeta" ),
    // Gammas:
   m_gam_useConverted(             cfg.GetItem< bool   >( "Gamma.UseConverted" ) ),
   m_gam_useElectronVeto(          cfg.GetItem< bool   >( "Gamma.UseElectronVeto" ) ),
   m_gam_usePixelSeed(             cfg.GetItem< bool   >( "Gamma.UsePixelSeed" ) ),
   m_gam_rejectOutOfTime(          cfg.GetItem< bool   >( "Gamma.RejectOutOfTime" ) ),
   m_gam_corrFactor_max(           cfg.GetItem< double >( "Gamma.CorrFactor.max" ) ),

   m_gam_barrel_sigmaIetaIeta_min( cfg.GetItem< double >( "Gamma.Barrel.SigmaIetaIeta.min" ) ),
   m_gam_barrel_sigmaIetaIeta_max( cfg.GetItem< double >( "Gamma.Barrel.SigmaIetaIeta.max" ) ),
   m_gam_endcap_sigmaIetaIeta_max( cfg.GetItem< double >( "Gamma.Endcap.SigmaIetaIeta.max" ) ),

   m_gam_SpikeCleaning(                    cfg.GetItem< double >( "Gamma.SpikeCleaning.use" ) ),
   // CutBasedPhotonID2012:
   m_gam_EA( cfg , "Gamma" ),


   // ID:
//    m_gam_ID_use(                           cfg.GetItem< bool   >( "Gamma.ID.use" ) ),
   m_gam_ID_Type(                          cfg.GetItem< std::string >( "Gamma.ID.Type" ) ),
   m_idErrReported(                          false ),

   m_gam_cb_use_bool(                            cfg.GetItem< bool >( "Gamma.CB.usebool" ) ),
   m_gam_cb_boolname(                            cfg.GetItem< std::string >( "Gamma.CB.boolname" ) ),
   m_gam_barrel_HoEm_max(                        cfg.GetItem< double >( "Gamma.CB.Barrel.HoEm.max" ) ),
   m_gam_barrel_PFIsoChargedHadron_max(          cfg.GetItem< double >( "Gamma.CB.Barrel.PFIsoChargedHadron.max" ) ),
   m_gam_barrel_PFIsoNeutralHadron_offset(       cfg.GetItem< double >( "Gamma.CB.Barrel.PFIsoNeutralHadron.Offset" ) ),
   m_gam_barrel_PFIsoNeutralHadron_linscale(     cfg.GetItem< double >( "Gamma.CB.Barrel.PFIsoNeutralHadron.Linscale" ) ),
   m_gam_barrel_PFIsoNeutralHadron_quadscale(    cfg.GetItem< double >( "Gamma.CB.Barrel.PFIsoNeutralHadron.Quadscale" ) ),
   m_gam_barrel_PFIsoPhoton_offset(              cfg.GetItem< double >( "Gamma.CB.Barrel.PFIsoPhoton.Offset" ) ),
   m_gam_barrel_PFIsoPhoton_linscale(            cfg.GetItem< double >( "Gamma.CB.Barrel.PFIsoPhoton.Linscale" ) ),
   m_gam_endcap_HoEm_max(                        cfg.GetItem< double >( "Gamma.CB.Endcap.HoEm.max" ) ),
   m_gam_endcap_PFIsoChargedHadron_max(          cfg.GetItem< double >( "Gamma.CB.Endcap.PFIsoChargedHadron.max" ) ),
   m_gam_endcap_PFIsoNeutralHadron_offset(       cfg.GetItem< double >( "Gamma.CB.Endcap.PFIsoNeutralHadron.Offset" ) ),
   m_gam_endcap_PFIsoNeutralHadron_linscale(     cfg.GetItem< double >( "Gamma.CB.Endcap.PFIsoNeutralHadron.Linscale" ) ),
   m_gam_endcap_PFIsoNeutralHadron_quadscale(    cfg.GetItem< double >( "Gamma.CB.Endcap.PFIsoNeutralHadron.Quadscale" ) ),
   m_gam_endcap_PFIsoPhoton_offset(              cfg.GetItem< double >( "Gamma.CB.Endcap.PFIsoPhoton.Offset" ) ),
   m_gam_endcap_PFIsoPhoton_linscale(            cfg.GetItem< double >( "Gamma.CB.Endcap.PFIsoPhoton.Linscale" ) ),

   m_gam_mva_use_bool(                           cfg.GetItem< bool >( "Gamma.MVA.usebool" ) ),
   m_gam_mva_boolname(                            cfg.GetItem< std::string >( "Gamma.MVA.boolname" ) ),
   m_gam_barrel_mva_min(                         cfg.GetItem< double >( "Gamma.MVA.Barrel.min" ) ),
   m_gam_endcap_mva_min(                         cfg.GetItem< double >( "Gamma.MVA.Endcap.min" ) )


   {}

GammaSelector::~GammaSelector(){
}

int GammaSelector::passObjectSelection( pxl::Particle *gam,
                   double const gamRho,
                   const std::string& idType,
                   const bool isSyst // use alternative kinematic cuts for syst
                   ) const {
   bool passKin = ObjectSelector::passKinematics( gam, isSyst );
   bool passID=true;
   bool passIso=true;

//    double const gamPt = gam->getPt();
   // set in passKinematics
   bool barrel = gam->getUserRecord( "isBarrel" );
   bool endcap = gam->getUserRecord( "isEndcap" );



   //~ oldNameMap.getUserRecordName( const_cast<const pxl::Particle*>(gam), "Gamma", "sigma_iEta_iEta" ) );
   gam->setUserRecord( "usedID", m_gam_ID_Type );
   //cut on sigmaietaieta ("eta width") which is different for EB and EE
   //double const gam_sigma_ieta_ieta = gam->getUserRecord( "sigma_iEta_iEta" );
   double const gam_sigma_ieta_ieta = gam->getUserRecord(
   ObjectSelector::oldNameMap->getUserRecordName( gam , "Gamma", "sigma_iEta_iEta" ) );
   if ( m_gam_ID_Type == "CB" ) {
      if( m_gam_cb_use_bool ){
         passID = gam->getUserRecord( m_gam_cb_boolname ).asBool();
      }else{
         passID = passCBID( gam, gamRho, barrel, endcap );
      }
   } else if( m_gam_ID_Type == "MVA "){
      if(m_gam_cb_use_bool){
         passID = gam->getUserRecord( m_gam_mva_boolname ).asBool();
      }else{
         passID = passMVAID( gam, barrel, endcap );
      }
   } else if ( m_gam_ID_Type == "OVERRIDE" ) {
       passID= true;
   } else {
      throw Tools::config_error( "'Gamma.ID.Type' must be one of these values: 'CB', 'MVA' or 'OVERRIDE'. The value is '" + m_gam_ID_Type + "'" );
      passID = false;
    }

   // Special needs: Check your config!
   if( m_gam_rejectOutOfTime ) {
      if( gam->hasUserRecord("recoFlag") ) {
         if( gam->getUserRecord( "recoFlag" ).toUInt32() == 2 )
            passID = false;
      } else {
          // In case "recoFlag" is not set, we assume, it is *not* out of
           // time!
      }
   }

   if( m_gam_SpikeCleaning ){
      if( barrel ) {
         //Additional spike cleaning
         if( gam_sigma_ieta_ieta < m_gam_barrel_sigmaIetaIeta_min ) passID = false;
         if( gam_sigma_ieta_ieta > m_gam_barrel_sigmaIetaIeta_max ) passID = false;
      }

      if( endcap ) {
         if( gam_sigma_ieta_ieta > m_gam_endcap_sigmaIetaIeta_max ) passID = false;
      }
   }

   //do we care about converted photons?
   if( m_gam_useConverted and gam->getUserRecord( "Converted" ).asBool() ) passID = false;
   if( m_gam_useElectronVeto and not gam->getUserRecord( "passElectronVeto" ).asBool() ) passID = false;
   if( m_gam_usePixelSeed and gam->getUserRecord( "HasSeed" ).asBool() ) passID = false;

   if( m_gam_corrFactor_max > 0.0 ) {
      //too large correction factors are not good for photons
      if( gam->getE() / gam->getUserRecord( "rawEnergy" ).toDouble() > m_gam_corrFactor_max ) passID = false;
   }

   // return code depending on passing variables
   if      (passKin  && passID  && passIso)  return 0;
   else if (passKin  && passID  && !passIso) return 1;
   else if (passKin  && !passID && passIso)  return 2;
   else if (!passKin && passID  && passIso)  return 3;
   return 4;

}

bool GammaSelector::passCBID( pxl::Particle const *gam,
                                              double const gamRho,
                                              bool const barrel,
                                              bool const endcap
                                              ) const {
   double const gamPt  = gam->getPt();
   double const abseta = fabs( gam->getUserRecord("SCeta").toDouble() );

   double const neutralHadronEA = m_gam_EA.getEffectiveArea( abseta, EffectiveArea::neutralHadron );
   double const photonEA        = m_gam_EA.getEffectiveArea( abseta, EffectiveArea::photon );

   // set cuts
   double HoEm = m_gam_endcap_HoEm_max;
   double sigmaIetaIeta = m_gam_endcap_sigmaIetaIeta_max;
   double isoChHadr = m_gam_endcap_PFIsoChargedHadron_max;
   double isoNeuHadr = m_gam_endcap_PFIsoNeutralHadron_offset +
                       gamPt * m_gam_endcap_PFIsoNeutralHadron_linscale +
                       gamPt * gamPt * m_gam_endcap_PFIsoNeutralHadron_quadscale;
   double isoPhoton = m_gam_endcap_PFIsoPhoton_offset +
                      gamPt * m_gam_endcap_PFIsoPhoton_linscale;
   if ( barrel )
   {
      HoEm = m_gam_barrel_HoEm_max;
      sigmaIetaIeta = m_gam_barrel_sigmaIetaIeta_max;
      isoChHadr = m_gam_barrel_PFIsoChargedHadron_max;
      isoNeuHadr = m_gam_barrel_PFIsoNeutralHadron_offset +
                   gamPt * m_gam_barrel_PFIsoNeutralHadron_linscale +
                   gamPt * gamPt * m_gam_barrel_PFIsoNeutralHadron_quadscale;
      isoPhoton = m_gam_barrel_PFIsoPhoton_offset +
                  gamPt * m_gam_barrel_PFIsoPhoton_linscale;
   }

   //std::cout << "TestCB ID for gasmma pt = " << gamPt << std::endl;

   if ( HoEm < (gam->getUserRecord(
      ObjectSelector::oldNameMap->getUserRecordName( gam, "Gamma", "hadTowOverEm" ) ).toDouble() )) return false;
   if ( sigmaIetaIeta < gam->getUserRecord( "full5x5_sigma_iEta_iEta" ).toDouble()) return false;
   if ( isoChHadr < gam->getUserRecord( "chargedHadronIso" ).toDouble() ) return false;
   if ( isoNeuHadr < gam->getUserRecord( "neutralHadronIso" ).toDouble() -gamRho * neutralHadronEA) return false;
   if ( isoPhoton < gam->getUserRecord( "photonIso" ).toDouble() -gamRho * photonEA) return false;

   //std::cout << "PASSED" << std::endl;
   return true;
}

bool GammaSelector::passMVAID( pxl::Particle const *gam,
                                              bool const barrel,
                                              bool const endcap
                                              ) const {

   double mva_min = m_gam_endcap_mva_min;
   double mva_value = gam->getUserRecord( "MVA_wp90_value" ).toDouble();
   if(gam->hasUserRecord("MVA_wp90_category")){
      int cat = gam->getUserRecord("MVA_wp90_category");
      // categorization ints taken from:
      // 0 - barrel 1 - endcap
      //https://github.com/ikrav/cmssw/blob/egm_id_80X_v3_photons/RecoEgamma/PhotonIdentification/python/Identification/mvaPhotonID_Spring16_nonTrig_V1_cff.py
      if( cat == 0 )
         mva_min = m_gam_barrel_mva_min;
   }else{ // 2015 compability mode
      if ( barrel )
         mva_min = m_gam_barrel_mva_min;
   }
   if ( mva_min > mva_value ) return false;
   return true;
}
