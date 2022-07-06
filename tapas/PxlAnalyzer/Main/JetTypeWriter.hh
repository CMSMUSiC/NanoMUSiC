#ifndef JetTypeWriter_hh
#define JetTypeWriter_hh
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Main/GenRecNameMap.hh"
#include "Tools/MConfig.hh"

#include <vector>
#include <string>
#include "TROOT.h"
#include "TFile.h"
#include "TH2F.h"
#include "TRandom3.h"

//for BTag SFs :
//https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// with CMSSW:
#include "CondFormats/BTauObjects/interface/BTagCalibration.h"
#include "CondTools/BTau/interface/BTagCalibrationReader.h"
// without CMSSW / standalone:
//#include "Main/BTagCalibrationStandalone.h"
//#include "Main/BTagCalibrationStandalone.cpp"

///////////////////////////////////////////////////////
// Class used to write tagging information into jets //
///////////////////////////////////////////////////////

class JetTypeWriter {

   public:
      JetTypeWriter ( const Tools::MConfig &cfg );

      // Main method to write jet tag info to all jets
      void writeJetTypes( std::vector< pxl::Particle* >& jets ) const;
      // For BTag Scale Factors
      void setBTagScaleFactor( std::vector< pxl::Particle* >& jets, int seed );

      // Get the b tagging weight for an event
      enum BTagVariation { DOWN, CENTRAL, UP };
      static double getBTagWeight( const std::map< std::string, std::vector< pxl::Particle* > >& part_map,
                                   JetTypeWriter::BTagVariation type,
                                   const std::vector< std::string >& names = { "Jet" } );
   private:
      // Helper method to test if jet passed B jet criterion
      bool passBJetCriterion( pxl::Particle *jet ) const;
      bool passWJetCriterion( pxl::Particle* jet ) const;
      // Helper method for BTag Scale Factors
      int countNJetsType(std::vector< pxl::Particle* >& jets, const std::string& jetType  ) const;
      void setBTagScaleFactor1A( pxl::Particle* jet , int nJets ) const;
      void setBTagScaleFactor2A( pxl::Particle* jet , int nJets , double bjet_sf2a_randnum) const;
      double getBTagScaleFactor( const pxl::Particle *object ) const;
      std::pair<double,double> getBTagScaleFactorError( const pxl::Particle *object ) const;
      double getBTagMCEfficiency( const pxl::Particle *object , int njet) const;
      double getBTagMCEfficiencyHadflavNjet( const pxl::Particle *object , int numjet) const;
      double getBTagMCEfficiencyHadflavPt( const pxl::Particle *object ) const;
      double getBTagMCEfficiencyFromHist(const int& x, const int& y) const;
      BTagEntry::JetFlavor getJetFlavourEnum(const int& hadronflavour) const;
      BTagEntry::OperatingPoint getOperatingPointEnum(const std::string& wp) const;

      bool const m_data;
      // bJet specific
      bool              m_use_bJets;
      std::string const m_bJet_algo;                     // What criterion do we check?
      std::string const m_bJet_algo1;
      std::string const m_bJet_algo2;
      std::string const m_bJet_algoname;                 // Name for the CVS file algorithm used
      std::string const m_bJet_discriminatorWP;          // Name for WorkingPoint according to the Calib Input
      float const       m_bJet_discriminatorThreshold;   // What threshold is used?
      //float const       m_bJet_ptThreshhold;
      //float const       m_bJet_etaThreshhold;
      std::string const m_bJets_sfMethod;
      std::string const m_bJet_scale_factor_file_name;   // Names fdor files, histograms etc to access the Btag SFs and efficiency
      std::string const m_bJet_efficiency_file_name;
      std::string const m_bJet_efficiency_file_type;
      std::string const m_bJet_efficiency_directory_name;
      std::string const m_bJet_efficiency_hist_name;

      bool m_use_wJets;
      std::string const m_wJet_mass_algo;
      std::string const m_wJet_tau1;
      std::string const m_wJet_tau2;
      float const m_wJet_mass_min;
      float const m_wJet_mass_max;
      float const m_wJet_tau_threshold;

      BTagCalibration m_calib;
      BTagCalibrationReader m_reader;

      // For BTag ScaleFactors
      TRandom3 m_rand;
      TH2F m_bJet_efficiency_hist;

};
#endif
