#include <string>
#include "JetTypeWriter.hh"
#include "ParticleSplittingFunctions.hh"

// Constructor
JetTypeWriter::JetTypeWriter ( const Tools::MConfig &cfg ) :
   m_data(           cfg.GetItem< bool >( "General.RunOnData" ) ),
   m_use_bJets( cfg.GetItem< bool >( "Jet.BJets.use" ) ),
   m_bJet_algo( cfg.GetItem< std::string >( "Jet.BJets.Algo" ) ),
   m_bJet_algo1( cfg.GetItem< std::string >( "Jet.BJets.Algo1" ) ),
   m_bJet_algo2( cfg.GetItem< std::string >( "Jet.BJets.Algo2" ) ),
   m_bJet_algoname( cfg.GetItem< std::string >( "Jet.BJets.AlgoName" ) ),
   m_bJet_discriminatorWP( cfg.GetItem< std::string >( "Jet.BJets.Discr.WP" ) ),
   m_bJet_discriminatorThreshold( cfg.GetItem< float >( "Jet.BJets.Discr.min" ) ),
   //m_bJet_ptThreshhold( cfg.GetItem< float >( "Jet.BJets.pt.min" ) ),
   //m_bJet_etaThreshhold( cfg.GetItem< float >( "Jet.BJets.eta.max" ) ),
   m_bJets_sfMethod( cfg.GetItem< std::string >( "Jet.BJets.SF.Method" ) ),// which method of scale factor to apply
   m_bJet_scale_factor_file_name( cfg.GetItem< std::string >( "Jet.BJets.SF.file", "" ) ),//sarFIXME if write ScaleFactor full, will it be taken by the ObjectSelector
   m_bJet_efficiency_file_name( cfg.GetItem< std::string >( "Jet.BJets.Efficiency.file", "" ) ),
   m_bJet_efficiency_file_type( cfg.GetItem< std::string >( "Jet.BJets.Efficiency.type", "HadflavNjet" ) ),
   m_bJet_efficiency_directory_name( cfg.GetItem< std::string >( "Jet.BJets.Efficiency.dir", "" ) ),
   m_bJet_efficiency_hist_name( cfg.GetItem< std::string >( "Jet.BJets.Efficiency.hist", "" ) ),
   m_use_wJets( cfg.GetItem< bool >( "FatJet.WJets.use" ) ),
   m_wJet_mass_algo( cfg.GetItem< std::string >( "FatJet.WJets.Algo" ) ),
   m_wJet_tau1( cfg.GetItem< std::string >( "FatJet.WJets.tau1" ) ),
   m_wJet_tau2( cfg.GetItem< std::string >( "FatJet.WJets.tau2" ) ),
   m_wJet_mass_min( cfg.GetItem< float >( "FatJet.WJets.mass.min" ) ),
   m_wJet_mass_max( cfg.GetItem< float >( "FatJet.WJets.mass.max" ) ),
   m_wJet_tau_threshold( cfg.GetItem< float >( "FatJet.WJets.tau.max" ) ),
   m_calib( BTagCalibration( m_bJet_algoname, Tools::ExpandPath( m_bJet_scale_factor_file_name ) ) ),
   m_rand( TRandom3() )
{

   // check if all info for initilaization is available
   if( m_use_bJets && !m_data && !m_bJets_sfMethod.empty()){
      if( m_bJet_discriminatorWP.empty() ){
         std::stringstream err;
         err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
         err << "No workingpoint name specified for scale factor for BTag"  << std::endl;
         throw Tools::value_error( err.str() );
      }
      if( m_bJet_algo.empty() ){
         std::stringstream err;
         err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
         err << "No algo name specified for scale factor for BTag" << std::endl;
         throw Tools::value_error( err.str() );
      }
      if( m_bJet_scale_factor_file_name.empty() ){
         std::stringstream err;
         err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
         err << "No filename name specified for scale factor for BTag"  << std::endl;
         throw Tools::value_error( err.str() );
      }
      if( m_bJet_efficiency_file_name.empty() ){
         std::stringstream err;
         err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
         err << "No filename name specified for efficiency for BTag" << std::endl;
         throw Tools::value_error( err.str() );
      }
      if( m_bJet_efficiency_hist_name.empty() ){
         std::stringstream err;
         err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
         err << "No histogram name specified for efficiency for BTag" << std::endl;
         throw Tools::value_error( err.str() );
      }

      TFile fin_bJet_efficiency( m_bJet_efficiency_file_name.c_str(), "OPEN" );
      if ( !fin_bJet_efficiency.IsOpen() )
      {
       throw Tools::file_not_found( m_bJet_efficiency_file_name );
      }

      //auto* tmp =  ( TH2F* ) fin_bJet_efficiency.Get(  m_bJet_efficiency_hist_name.c_str() );
      auto* tmp =  ( TH2F* ) fin_bJet_efficiency.Get(  (m_bJet_efficiency_directory_name + "/" + m_bJet_efficiency_hist_name).c_str() );
      m_bJet_efficiency_hist = TH2F( *tmp ); // Calls copy constructor
      delete tmp;

      m_reader = BTagCalibrationReader(getOperatingPointEnum(m_bJet_discriminatorWP), "central", {"up", "down"});
      m_reader.BTagCalibrationReader::load(m_calib,            // calibration instance
                                          BTagEntry::FLAV_B,  // btag flavour
                                          "comb");            // measurement type
      m_reader.load(m_calib, BTagEntry::FLAV_C, "comb");       // for FLAV_C
      m_reader.load(m_calib, BTagEntry::FLAV_UDSG, "incl"); // for FLAV_UDSG
   }
}

void JetTypeWriter::writeJetTypes( std::vector< pxl::Particle* >& jets ) const {
   // Loop over jets
   for( auto& jet : jets )
   {
      // Check if criterion is fulfilled
      if ( m_use_bJets && jet->getName() == "AK4" )
      {
         if ( passBJetCriterion( jet ) )
            jet->setUserRecord( "isBjet", "true" );
         else
            jet->setUserRecord( "isBjet", "false" );
      }
      if ( m_use_wJets && jet->getName() == "AK8" )
      {
         if ( passWJetCriterion( jet ) )
            jet->setUserRecord( "isWjet", "true" );
         else
            jet->setUserRecord( "isWjet", "false" );
      }
   }
}

void JetTypeWriter::setBTagScaleFactor( std::vector< pxl::Particle* >& jets, int seed ) {

   if ( m_use_bJets && !m_bJets_sfMethod.empty() ){
      int nJetsAK4 = 1;
      nJetsAK4 = countNJetsType( jets, "AK4" );
      if (m_bJets_sfMethod == "2A") m_rand.SetSeed(seed);

      // Loop over jets
      for( auto& jet : jets )
      {
         // Check if criterion is fulfilled
         if ( m_use_bJets && jet->getName() == "AK4" )
         {
            if (m_bJets_sfMethod == "1A") {
               setBTagScaleFactor1A( jet, nJetsAK4 );
            } else if (m_bJets_sfMethod == "2A") {
               double bjet_sf2a_randnum = m_rand.Rndm();
               setBTagScaleFactor2A( jet, nJetsAK4, bjet_sf2a_randnum );
            } else {
               std::stringstream err;
               err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
               err << "The BTag scale factor method is not defined " << std::endl;
               throw Tools::value_error( err.str() );
            }
         }
      }
   }
}

bool JetTypeWriter::passBJetCriterion( pxl::Particle* jet ) const {
  float rec_discriminator;
  float discr0 ,discr1, discr2;

  if(m_bJet_algoname.find("Deep") < 0){
    rec_discriminator = jet->getUserRecord( m_bJet_algo );
    return rec_discriminator > m_bJet_discriminatorThreshold;
  } 
  else if(m_bJet_algoname.find("Deep") >= 0){ 
    if(m_bJet_algoname.find("CSV") >= 0){
      discr0 = jet->getUserRecord( m_bJet_algo);
      discr1 =  jet->getUserRecord( m_bJet_algo1);
      rec_discriminator = discr0+discr1;
      return rec_discriminator > m_bJet_discriminatorThreshold;
    }
    else if(m_bJet_algoname.find("Jet") >= 0){
      discr0 = jet->getUserRecord( m_bJet_algo);
      discr1 =jet->getUserRecord( m_bJet_algo1);
      discr2 =jet->getUserRecord( m_bJet_algo2);
      rec_discriminator= discr0+discr1+discr2;
      return rec_discriminator > m_bJet_discriminatorThreshold;
    }
    else{
      std::stringstream err;
      err << " BJet Algo name contains Deep. But it is nor DeepCSV nor DeepJet. Please change into the jet.cff config file " << std::endl;
      throw Tools::value_error( err.str() );
    }  
  }
  else{
      std::stringstream err;
      err << " BJet Algo name is nor CVS nor DeepCSV/DeepJet. Please change into the jet.cff config file " << std::endl;
      throw Tools::value_error( err.str() );
  }
}
bool JetTypeWriter::passWJetCriterion( pxl::Particle* jet ) const {
   // Choice of Algo should be added here, once more than one algorithm is available
   float wmass = jet->getUserRecord( m_wJet_mass_algo );
   float tau1 = jet->getUserRecord( m_wJet_tau1 );
   float tau2 = jet->getUserRecord( m_wJet_tau2 );
   return wmass > m_wJet_mass_min && wmass < m_wJet_mass_max && tau2 / tau1 < m_wJet_tau_threshold;
}

int JetTypeWriter::countNJetsType(std::vector< pxl::Particle* >& jets, const std::string& jetType  ) const {
   int nJets = 0;

   for( auto& jet : jets )
      {
         // Check if criterion is fulfilled
         if ( jet->getName() == jetType )
            nJets++;
      }
   return nJets;
}


void JetTypeWriter::setBTagScaleFactor1A( pxl::Particle* jet , int nJets ) const {
   std::pair< double, double > btagscalefactor_up_do =  getBTagScaleFactorError( jet );

   double bjetscalefactor = getBTagScaleFactor( jet );

   jet->setUserRecord( "btag_scale_factor", bjetscalefactor );
   jet->setUserRecord( "btag_scale_factor_up", btagscalefactor_up_do.first );
   jet->setUserRecord( "btag_scale_factor_down", btagscalefactor_up_do.second );
   jet->setUserRecord( "btag_mc_efficiency", getBTagMCEfficiency( jet, nJets) );
}

void JetTypeWriter::setBTagScaleFactor2A(  pxl::Particle* jet , int nJets , double bjet_sf2a_randnum) const {

   bool isBJet = passBJetCriterion( jet );
   double bjetscalefactor = getBTagScaleFactor( jet );
   std::pair< double, double > bTagScaleFactor_up_do =  getBTagScaleFactorError( jet );
   double fraction = 0.0;

   double jetMCEfficiency = getBTagMCEfficiency( jet, nJets);

   if ( bjetscalefactor < 1.0 ){
      fraction = 1.0 - bjetscalefactor;
      if(isBJet && bjet_sf2a_randnum < fraction)
         jet->setUserRecord( "isBjet", "false" );
   } else {
      fraction =  (1.0 - bjetscalefactor ) / ( 1.0 - 1.0 / jetMCEfficiency );
      if(!isBJet && bjet_sf2a_randnum < fraction)
         jet->setUserRecord( "isBjet", "true" );
   }

   jet->setUserRecord( "bjet_sf2a_sf", bjetscalefactor );
   jet->setUserRecord( "bjet_sf2a_sf_up", bTagScaleFactor_up_do.first);
   jet->setUserRecord( "bjet_sf2a_sf_do", bTagScaleFactor_up_do.second);
   jet->setUserRecord( "bjet_sf2a_MCEfficiency", jetMCEfficiency );
   jet->setUserRecord( "bjet_sf2a_randnum", bjet_sf2a_randnum );

}

//For B Tag SFs being read from the .csv file
double JetTypeWriter::getBTagScaleFactor( const pxl::Particle *object ) const {
   double eta = object->getEta();
   double pt = object->getPt();
   int hadronFlavour = object->getUserRecord("hadronFlavour").toInt32();

   double bTagScaleFactor = m_reader.eval_auto_bounds("central",
                                                      getJetFlavourEnum(hadronFlavour),
                                                      eta,
                                                      pt);
   return bTagScaleFactor;
}

//For B Tag SFs being read from the .csv file
std::pair<double,double> JetTypeWriter::getBTagScaleFactorError( const pxl::Particle *object ) const {
   double eta = object->getEta();
   double pt = object->getPt();
   int hadronFlavour = object->getUserRecord("hadronFlavour").toInt32();

   double bTagScaleFactor_up = 1.0;
   double bTagScaleFactor_do = 1.0;

   bTagScaleFactor_up = m_reader.eval_auto_bounds("up",
                                                  getJetFlavourEnum(hadronFlavour),
                                                  eta,
                                                  pt);

   bTagScaleFactor_do = m_reader.eval_auto_bounds("down",
                                                  getJetFlavourEnum(hadronFlavour),
                                                  eta,
                                                  pt);

   return std::make_pair( bTagScaleFactor_up, bTagScaleFactor_do );
}

double JetTypeWriter::getBTagMCEfficiency( const pxl::Particle *object , int njet) const {
   if( m_bJet_efficiency_file_type == "HadflavNjet") return getBTagMCEfficiencyHadflavNjet(object, njet);
   else if( m_bJet_efficiency_file_type == "HadflavPt") return getBTagMCEfficiencyHadflavPt(object);
   std::cout << "WARNING: Unknown type "<< m_bJet_efficiency_file_type <<" used for BJet MC Efficiency" << std::endl;
   return 0.5;
}

double JetTypeWriter::getBTagMCEfficiencyHadflavNjet( const pxl::Particle *object , int numjet) const {
   int hadronFlavour = object->getUserRecord("hadronFlavour").toInt32();
   int njet = numjet;
   return getBTagMCEfficiencyFromHist(hadronFlavour, njet);
}

double JetTypeWriter::getBTagMCEfficiencyHadflavPt( const pxl::Particle *object ) const {
   int hadronFlavour = object->getUserRecord("hadronFlavour").toInt32();
   double pt = object->getPt();
   return getBTagMCEfficiencyFromHist(hadronFlavour, pt);
}

double JetTypeWriter::getBTagMCEfficiencyFromHist(const int& x, const int& y) const
{
   enum Range {UNDER, OK, OVER};
   const auto check_range = [] (const TAxis* axis, const int& value)
   {
      if (value < axis->GetBinLowEdge(0))
         return Range::UNDER;
      else if (value > axis->GetBinUpEdge(axis->GetNbins()))
         return Range::OVER;
      else
         return Range::OK;
   };
   const auto range_y = check_range(m_bJet_efficiency_hist.GetYaxis(), y);
   if (check_range(m_bJet_efficiency_hist.GetXaxis(), x) != Range::OK or range_y == Range::UNDER)
   {
      std::stringstream err;
      err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
      err << "Out of range of histogram " << m_bJet_efficiency_hist_name << " for BTag scale factor  "  << std::endl;
      throw Tools::value_error( err.str() );
   }
   const int bin_number = m_bJet_efficiency_hist.FindFixBin(x, y);
   if (range_y == Range::OVER)
   {
      int binx, biny, binz;
      m_bJet_efficiency_hist.GetBinXYZ(bin_number, binx, biny, binz);
      biny--;
      return m_bJet_efficiency_hist.GetBinContent(binx, biny);
   }
   return m_bJet_efficiency_hist.GetBinContent(bin_number);
}

BTagEntry::JetFlavor JetTypeWriter::getJetFlavourEnum(const int& hadronflavour) const
{
   switch (hadronflavour)
   {
      case 0: return BTagEntry::FLAV_UDSG;
      case 4: return BTagEntry::FLAV_C;
      case 5: return BTagEntry::FLAV_B;
      default:
         std::stringstream err;
         err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
         err << "Unexpected hadronflavour: " << hadronflavour;
         throw Tools::value_error(err.str());
   }
}

BTagEntry::OperatingPoint JetTypeWriter::getOperatingPointEnum(const std::string& wp) const
{
   if (wp == "Loose")
      return BTagEntry::OP_LOOSE;
   else if (wp == "Medium")
      return BTagEntry::OP_MEDIUM;
   else if (wp == "Tight")
      return BTagEntry::OP_TIGHT;
   else
   {
      std::stringstream err;
      err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
      err << "Could not find matching working point for the BTag Scale Factors " <<  wp << " ; The available options are: Loose, Medium or Tight. " << std::endl;
      throw Tools::value_error( err.str() );
   }
}

// get the b tagging weight for an event
double JetTypeWriter::getBTagWeight( const std::map< std::string, std::vector< pxl::Particle* > >& part_map,
                                     JetTypeWriter::BTagVariation type,
                                     const std::vector< std::string >& names )
{
   auto part_map_copy = std::map< std::string, std::vector< pxl::Particle* > >( part_map );
   Splitting::merge( part_map_copy, names, "Jet" );
   std::string user_record_name = "";
   switch ( type )
   {
      case CENTRAL: user_record_name = "btag_scale_factor"; break;
      case UP:      user_record_name = "btag_scale_factor_up"; break;
      case DOWN:    user_record_name = "btag_scale_factor_down"; break;
   }
   double weight_mc = 1;
   double weight_data = 1;
   for ( auto& part : part_map_copy["Jet"] )
   {
      if ( part->getUserRecord( "isBjet" ).toBool() )
      {
         weight_mc *= part->getUserRecord( "btag_mc_efficiency" ).toDouble();
         weight_data *= part->getUserRecord( user_record_name ).toDouble() * part->getUserRecord( "btag_mc_efficiency" ).toDouble();
      }
      else
      {
         weight_mc *= 1 - part->getUserRecord( "btag_mc_efficiency" ).toDouble();
         weight_data *= 1 - part->getUserRecord( user_record_name ).toDouble() * part->getUserRecord( "btag_mc_efficiency" ).toDouble();
      }
   }
   return weight_data / weight_mc;
}
