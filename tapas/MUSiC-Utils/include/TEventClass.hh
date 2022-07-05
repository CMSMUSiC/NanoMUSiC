#ifndef TEventClass_hh
#define TEventClass_hh
/*

TEventClass Class:

This class should provide the general functionality for a single event class
e.g. 1mu_2e_0Gam_1MET_4Jet

For each process (ttbar, Wmunu, ...) this class should automatically create
all the TH1's

*/

#include "TObject.h"
#include "TNamed.h"
#include "MConfig.hh"
#include <iostream>
#include "TBrowser.h"
#include "TFolder.h"
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include <map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>

class TEventClass : public TNamed {

public:
   struct PDFResult {
      float mean;
      float up;
      float down;
   };

   // default constructor needed for ROOT
   TEventClass() {};
   TEventClass( const std::string &EventType,
                const std::string &EventClassType,
                const std::string runhash,
                bool const data,
                double const cme,
                std::map< std::string , int> countmap,
                const bool analyzedBjets,
                std::map< std::string, std::vector< double > > distTypeBins,
                const bool analyzedCharge,
                const int numCharge,
                const bool isInclusive,
                // Minimum value for each distType from event topology [distType][minValue]
                std::map< std::string ,  double > distTypMins,
                unsigned int m_numPDFvariations,
                // Minimum value for each distType from config files [distType][minValue]
                std::map< std::string ,  double > distTypeMinsRequire,
                // list of ystematics names that will be filled
                double lumi,
                std::set< std::string > systNames = std::set< std::string >(),
                std::map< std::string, std::string > ECItemShortlist = std::map< std::string, std::string >(),
                double const bin_size_min = 10.0
                );
   // Copy Constructor
   void copyValues( const TEventClass& rhs, bool empty );
   TEventClass( const TEventClass &orig, bool empty );
   TEventClass( const TEventClass &orig );

   TEventClass& operator=( const TEventClass &rhs );

   ~TEventClass();

   void InitializeHistos( std::string const &process );
   void InitializePDFHistos( const std::string &process, const std::vector< float > &PDFweights );
   void InitializeResolutions( std::string const &process );

   // Fill the actual histograms.
   void Fill( std::string const &process,
              std::map< std::string, double> values,
              std::map< std::string, std::pair< double, double > > resolution_value,
              double weight=1.,
              std::map< std::string, double> systWeights =std::map< std::string, double>(),
              std::vector< float > const &PDFweights = std::vector< float >()
               );

   // Fill function for differential systematics
   void FillDifferentialSystematic( std::string const &process,
              std::map< std::string, double> values,
              double const weight,
              std::vector< float > const &PDFweights, // just used to initialize class if necessary
              std::string systName
               );

   // Scale event class to x-section and lumi:
   void scaleLumi (double targetLumi );
   void scaleAllCrossSections( double factor );
   void changeCrossSection( std::string process, double cross_section );
   void scale( double totalXsec );

   // returns the list of processes which have been filled into this SPECIFIC eventclass
   // i.e. which keys are used in the _SumPtHistos map
   std::set<std::string> ProcessList();

   // returns the list of ALL processes which have been filled in general (from (_genInclEventClasses["Empty"])->ProcessList() )

   std::set< std::string > getProcessList() {
       std::set< std::string > outset = std::set< std::string >();
       for( auto& procIter : allProcHistoMap ) outset.insert( procIter.first ) ;
       return outset;
    }
   std::set< std::string > getGlobalProcessList() { return m_globalProcessList; }

   // sets the list of ALL processes contributing in general
   void setGlobalProcessList( const std::set< std::string > &processList) { m_globalProcessList = processList; }
   void addToGlobalProcessList( const std::string proc ) { m_globalProcessList.insert( proc ); }

   void addToProcessGroupMap( std::string process, std::string processGroup ){
      auto foundEntry = processGroupMap.find( process );

      if( foundEntry == processGroupMap.end() ){
         processGroupMap.emplace( process, processGroup );
      }else{
         if( foundEntry->second !=  processGroup ){
            std::stringstream err;
            err << "In TEventClass::addToProcessGroupMap" << std::endl;
            err << "Trying to fill same process with different process groups" << std::endl;
            err << "process: "<< process << std::endl;
            err << "Existing process group: "<< process << std::endl;
            err << "Added process group: "<< process << std::endl;
            throw Tools::value_error( err.str() );
         }
      }
   }
   // returns the process group for a given process.
   // The process name is returned if no group was found
   std::string getProcessGroup( std::string process ){
      auto requestedGroup = processGroupMap.find( process );
      if(requestedGroup  != processGroupMap.end() ){
         return requestedGroup->second;
      }
      return process;
   }

   bool hasSystematic( std::string &systName ){
      return ( std::find( m_systNames.begin(), m_systNames.end(), systName ) != m_systNames.end() );
   }
   void addSystematic( std:: string systName ) { m_systNames.insert( systName ); };
   // getter functions for allProcHistoMap, SystematicsProcHistoMap and PDFvariedallProcHistoMap ( containers for hists in class ) Sytemati
    std::map< std::string, std::map<std::string, TH1F* > > getAllProcHistoMap(){
        return allProcHistoMap;
    }
    std::map< std::string, std::map<std::string, std::map< std::string, TH1F* > > > getSystematicsProcHistoMap(){
        return SystematicsProcHistoMap;
    }

    std::map< std::string, std::map<std::string, std::map< std::string, TH1F* > > > getPDFvariedallProcHistoMap(){
        return PDFvariedallProcHistoMap;
    }

    std::map< std::string, std::map<std::string, TH2F* > > getResolutionsMap(){
        return m_resolutionsMap;
    }

   inline bool &getPDFInit() { return m_pdfInit; }
   std::map<std::string, std::map< std::string, TH1F* > > getPDFWeightsMap( const std::string &process );
   void setPDFWeights( const std::string &process, const std::map< std::string, std::map< std::string, TH1F* > > &pdfDistMap);

   // check if fill function has been called at least one for this ec
   bool isFilled(){ return m_is_filled; };

   //---------------Return a pointer to the histo of a certain process------------------------------
   TH1F* getHistoPointer(const std::string& process, const std::string& distType);
   TH1F* getHistoPointerUnweighted(const std::string& process, const std::string& distType);
   TH1F* getPDFHistoPointer(const std::string& process,
      const std::string distType,
      const std::string histname);
   TH1F* getSystHistoPointer(const std::string& process,
      const std::string distType,
      const std::string systName);
   TH2F* getResolutionsHistoPointer( std::string const &process, std::string distType );

   void addHisto(const std::string& process,
                 std::string distType,
                 TH1F* histo,
                 std::map< std::string, std::map<std::string,  TH1F* > > &histoMap) {
      histoMap[process].emplace(distType, histo);
   };
   void addHisto(const std::string& process, std::string distType, TH1F* histo) {
      addHisto( process, distType, histo, allProcHistoMap);
   };
   void addHistoUnweighted(const std::string& process, std::string distType, TH1F* histo) {
      addHisto( process, distType, histo, allProcHistoMapUnweighted);
   };
   void addHisto(const std::string& process, std::string distType, std::string systName ,TH1F* histo) {
      SystematicsProcHistoMap[process][distType].emplace(systName, histo);
   };

   void addResolutionHistogram( std::string const &process, std::string distType, TH2F* const histo ) {
      m_resolutionsMap[ process ][distType] =  histo ;
   }

   void replaceHisto(const std::string& process, std::string distType, TH1F* histo) {
      allProcHistoMap[process][distType] = histo;
   };

   inline std::string getClassType() { return m_eventClassType; };
   std::set< std::string > getSystematicNames();
   inline std::string getRunHash() { return m_runHash; };
   inline std::string getType() { return m_eventType; };
   inline bool isInclusive() { return m_isInclusive; };
   inline bool isData() { return m_data; };
   inline double getCME() { return m_cme; }
   // Get map with counts for all ECItem types in this class
   inline std::map< std::string, int> getCountMap( ) { return m_countmap; };
   // get Number of enties for one kind of ECitem type "e.g." e
   inline int getnumECItem( std::string ECItemName) { return m_countmap.at( ECItemName ); };
   inline int getTotalNumECItems( ) {
      int count = 0;
      for( auto countpair : m_countmap )count += countpair.second;
      return count;
   };
   //returns true if the class was generate with active b-jet analysis
   bool analyzedBjets() { return m_analyzedBjets; }
   //true if class was generated with active charge analysis
   bool analyzedCharges() { return m_analyzedCharge; }
   //returns absolute sum of lepton charge or -1 in case charges were off
   int getNumCharge() { return m_analyzedCharge ? m_numCharge : -1; }

   inline double getLumi() { return m_lumi; };
   void setLumi( double targetLumi);
   inline double getScaleFactor( const std::string &process ) { return m_scaleFactor[ process ]; }
   inline void setScaleFactor( const std::string &process, double factor ) { m_scaleFactor[ process ] = factor; }
   inline double getCrossSection( const std::string &process ) { return m_crossSection[ process ]; }
   inline void setCrossSection( const std::string &process, double xsec ) { m_crossSection[ process ] = xsec; }

      //~ int getNumOfDistributions(const std::string process);
   unsigned int getNumOfDistributions(const std::string process, std::map< std::string, std::map<std::string, std::map< std::string, TH1F* > > > &procMap);
   unsigned int getNumOfDistributions(const std::string process, std::map< std::string, std::map<std::string,  TH1F* > > &procMap);
   unsigned int getNumOfDistributions(const std::string process);
   unsigned int getNumOfPDFHistos( const std::string &process );
   unsigned int getNumOfSystHistos( const std::string &process );
   unsigned int getNumOfResolutionHistos( std::string const &process ) const;
   // watch out since some EC have no histos due to double-counting removal
   // (especially in Gen where all e-mu combinations are removed from MuonStream)

   void calculatePDFUncertainty();
   // method which needs to be called once the EventClass is completetly finished
   // takes the PDFvariedallProcHistoMap an creates 2 additional histograms per process & distribution (sumPT, invMass, MET)
   // indicating the upper and lower uncertainties
   // Once the EventClasses are finally merged, we don't need the individual PDF
   // histograms any more, so they don't have to be stored in the final root
   // file. (Also used in the destructor).
   void dropPDFHistograms();

      //Makes TEventClass browsable in root
   Bool_t IsFolder() const { return kTRUE; }
   void Browse( TBrowser *b );
   //builds the event class name string dependend on the EventClass items
   static std::string calculateEventClass( const std::map< std::string, int > countmap,
             const std::map< std::string, int > shortmap = std::map< std::string, int >(),
             std::function<bool( std::string, std::string ) > orderFunction = [](std::string a, std::string b) { return (a <= b); } );


   //return the number of analyzed events for the requested process, without any weighting
   unsigned int getEventCount( const std::string& process="" ) const;
   //return the sum of weight for all analyzed events for the requested process, weighted to lumi and cross section
   double getTotalEvents( const std::string& process="" ) const;
   //return the sum of weight for all analyzed events for the requested process, not weighted
   double getTotalEventsUnweighted( const std::string& process="" ) const;

   //set the event number information for the provided process
   void setEventCount( const std::string &process, const unsigned int &num ) { eventCount[ process ] = num; }
   void setTotalEvents( const std::string &process, const double &num ) { totalEvents[ process ] = num; }
   void setTotalEventsUnweighted( const std::string &process, const double &num ) { totalEventsUnweighted[ process ] = num; }
   void setTotalEventsUnweighted( const std::map< std::string, double > &nums ) { totalEventsUnweighted = nums; }

   void addEventClass( TEventClass *ECtoBeAdded, std::set< std::string > &processesToMerge, bool data );

   std::vector< std::string> getDistTypes(){
      std::vector< std::string> outvec;
      for( auto distPair : m_distTypeBinInfo){
         outvec.push_back( distPair.first );
      }
      return outvec;
   }
   double getDistTypeMin( std::string distType ) const {

       return m_distTypeMins.at(distType ) >= m_distTypeMinsRequire.at(distType ) ? m_distTypeMins.at(distType ) : m_distTypeMinsRequire.at(distType );
    }

   // Get the smallest possible Sumpt, Minv, MET in this EventClass as defined
   // by the EventClass topology (number of objects and pt cuts).
   double getDistTypeMinTopo( std::string distType ) const {
       auto minval = m_distTypeMins.find(distType);
        if( minval !=  m_distTypeMins.end() ) {
            return minval->second;
        }else{
           std::stringstream errorMessage;
           errorMessage << "No Topo min value for distType " << distType << ").";
           errorMessage << std::endl;
           throw std::runtime_error( errorMessage.str() );
        }
       return 0;
    }

   // Get the smallest required Sumpt, Minv, MET in this EventClass as required
   // by the user in the classification.
   double getDistTypeMinRequire( std::string distType ) const {
       auto minval = m_distTypeMinsRequire.find(distType);
        if( minval !=  m_distTypeMinsRequire.end() ) {
            return minval->second;
        }else{
           std::stringstream errorMessage;
           errorMessage << "No reuired min value for distType " << distType << ").";
           errorMessage << std::endl;
           throw std::runtime_error( errorMessage.str() );
        }
       return 0;
    }

   bool isEmpty() const { return m_isEmpty; }

   //~ TH1F const &getBinHisto( std::string const dist ) const;
   TH1F* getBinHisto( std::string distType ) const{
      return m_distTypeBinsHisto.at(distType);
   }

   // "Meaningful" reimplementation of ROOT TH1 functions for TEventClass.
   // Atm. especially used by ECReader.
   int findBin( std::string histname, double const xvalue ) const {
      return getBinHisto( histname )->FindFixBin( xvalue );
   }
   double getBinCenter( std::string histname , int const bin ) const {
      return getBinHisto( histname )->GetBinCenter( bin );
   }

   double getBinLowEdge( std::string histname, int const bin ) const {
      return getBinHisto( histname )->GetBinLowEdge( bin );
   }

   int getNbins( std::string histname ) const {
      return getBinHisto( histname )->GetNbinsX();
   }

   // returns the minimal bin width, also used for plotting/normalization
   double getMinBinWidth() const {
      return m_bin_size_min;
   }

   //and for all processes at the same time
   void setEventCount( const std::map< std::string, unsigned int > &nums ) { eventCount = nums; }
   void setTotalEvents( const std::map< std::string, double > &nums ) { totalEvents = nums; }

   // Add the results for a (pseudo experiment) MUSiC scan
   void addScanResult( std::string distribution,
                                   double nData,
                                   double nMC,
                                   double totalUncert,
                                   double lowerEdge,
                                   double width,
                                   double compareScore,
                                   bool pseudoScan,
                                   const std::vector< double >& dicedData=std::vector< double >() ){

      if( not pseudoScan ){
         // using [] instead of .at() makes std::map call the default
         // value_type constructor (if the distribution has never been
         // mentioned before)
         scanResults_nData[ distribution ].push_back( nData );
         scanResults_nMC[ distribution ].push_back( nMC );
         scanResults_totalUncert[ distribution ].push_back( totalUncert );
         scanResults_lowerEdges[ distribution ].push_back( lowerEdge );
         scanResults_Widths[ distribution ].push_back( width );
         scanResults_CompareScores[ distribution ].push_back( compareScore );
         scanResults_dicedData[ distribution ].push_back( dicedData );
      }else{
         scanResults_pseudo_nData[ distribution ].push_back( nData );
         scanResults_pseudo_nMC[ distribution ].push_back( nMC );
         scanResults_pseudo_totalUncert[ distribution ].push_back( totalUncert );
         scanResults_pseudo_lowerEdges[ distribution ].push_back( lowerEdge );
         scanResults_pseudo_Widths[ distribution ].push_back( width );
         scanResults_pseudo_CompareScores[ distribution ].push_back( compareScore );
      }
   }

   void clearSignalScanResults() {
      scanResults_nData.clear();
      scanResults_nMC.clear();
      scanResults_totalUncert.clear();
      scanResults_lowerEdges.clear();
      scanResults_Widths.clear();
      scanResults_CompareScores.clear();
      scanResults_dicedData.clear();
   }

   void clearPseudoScanResults() {
      scanResults_pseudo_nData.clear();
      scanResults_pseudo_nMC.clear();
      scanResults_pseudo_totalUncert.clear();
      scanResults_pseudo_lowerEdges.clear();
      scanResults_pseudo_Widths.clear();
      scanResults_pseudo_CompareScores.clear();
   }

   bool hasDataScan( std::string distribution ) const {
       return getNsignalRounds( distribution ) > 0;
   }

   // get the number of pseudo experiments
   int getNpseudoExp( std::string distribution ) const {
      if ( scanResults_pseudo_CompareScores.find( distribution )
              != scanResults_pseudo_CompareScores.end() ){
          return scanResults_pseudo_CompareScores.at( distribution ).size();
      } else {
          return 0;
      }
   };

   // get the number of dicing rounds for signal
   int getNsignalRounds( std::string distribution ) const {
      if ( scanResults_CompareScores.find( distribution )
              != scanResults_CompareScores.end() ){
          return scanResults_CompareScores.at( distribution ).size();
      } else {
          return 0;
      }
   };

   // get number of data events in the RoI
   double getNDataEntry( std::string distribution, int iSignal = 0 ) const {
      return scanResults_nData.at( distribution ).at( iSignal );
   };
   // get number of data events in the RoI in the i-th (pseudo) experiment (iterator starts with 0)
   double getNDataPseudoEntry( std::string distribution, int iPseudo = 0) const {
      return scanResults_pseudo_nData.at( distribution ).at( iPseudo );
   };

   // get number of MC events in the RoI
   double getNMCEntry( std::string distribution, int iSignal = 0  ) const {
      return scanResults_nMC.at( distribution ).at(iSignal);
   };

   // get number of MC events in the RoI in the i-th (pseudo) experiment (iterator starts with 0)
   double getNMCPseudoEntry( std::string distribution, int iPseudo = 0) const {
      return scanResults_pseudo_nMC.at( distribution ).at( iPseudo );
   };

   // get total uncertanty in the RoI
   double getTotalUncertEntry( std::string distribution, int iSignal = 0 ) const {
      return scanResults_totalUncert.at( distribution ).at(iSignal);
   };

   // get total uncertanty  in the RoI in the i-th (pseudo) experiment (iterator starts with 0)
   double getTotalUncertPseudoEntry( std::string distribution, int iPseudo = 0 ) const {
      return scanResults_pseudo_totalUncert.at( distribution ).at( iPseudo );
   };

   // get lower edge for the RoI
   double getLowerEdgeEntry( std::string distribution, int iSignal = 0 ) const {
      return scanResults_lowerEdges.at( distribution ).at(iSignal);
   };
   // get lower edge for the RoI in the i-th (pseudo) experiment (iterator starts with 0)
   double getLowerEdgePseudoEntry( std::string distribution, int iPseudo = 0) const {
      return scanResults_pseudo_lowerEdges.at( distribution ).at( iPseudo );
   };

   // get width for the RoI
   double getWidthEntry( std::string distribution, int iSignal = 0  ) const {
      return scanResults_Widths.at( distribution ).at(iSignal);
   };

   // get width for the RoI in the i-th (pseudo) experiment (iterator starts with 0, not needed for dataScan)
   double getWidthPseudoEntry( std::string distribution, int iPseudo = 0 ) const {
      return scanResults_pseudo_Widths.at( distribution ).at( iPseudo );
   };
   // get width for the RoI
   double getCompareEntry( std::string distribution, int iSignal = 0 ) const {
      return scanResults_CompareScores.at( distribution ).at(iSignal);
   };

   // get width for the Roi in the i-th pseudo experiment (iterator starts with 0)
   double getComparePseudoEntry( std::string distribution, int iPseudo = 0) const {
      return scanResults_pseudo_CompareScores.at( distribution ).at( iPseudo );
   };

   TH1F getDicedDataHisto( std::string distribution, int iSignal = 0) const {
      // Explicitely copy reference histogram
      TH1F hist( *m_distTypeBinsHisto.at( distribution ) );

      // Generate new name
      const std::string hist_name = GetName() + std::string("_diced_data_") + std::to_string( iSignal );
      hist.SetName( hist_name.c_str() );

      const std::vector< double >& v = scanResults_dicedData.at( distribution ).at( iSignal );
      for ( size_t i = 0; i < v.size(); ++i ) {
         hist.SetBinContent( i+1, v[i] );
      }

      // Will (hopefully) use std::move on return (instead of copying)
      return hist;
   }

   // get and set changelog
   void addChangeLogEntry( std::string change )
   {
      m_changelog.emplace( std::time( nullptr ), std::make_pair( m_eventType + m_eventClassType + ": " + change, getProcessList() ) );
   }
   std::map< std::time_t, std::pair< std::string, std::set< std::string > > > getChangeLog()
   {
      return m_changelog;
   }
   void mergeChangeLogs ( std::map< std::time_t, std::pair< std::string, std::set< std::string > > > changeLogToMerge )
   {
      for( auto& logEntry : changeLogToMerge )
      {
         m_changelog.insert( logEntry );
      }
   }


private:

   // DataMembers:
   // containing EventClassType e.g. 1mu_1e_1gam_2et_1met
   std::string m_eventClassType;
   // containing EventType Gen/Rec
   std::string m_eventType;
   // Run hash to identify run conditions used to create this entry
   std::string m_runHash;
   // Switch for inclusive/exclusive class.
   bool m_isInclusive;
   // Data or MC EventClass?
   bool m_data;
   // Store center of mass energy.
   double m_cme;

   // if the histos are scaled to lumi, store it, else lumi = -1
   double m_lumi; // lumi in pb^-1

   // map of times -> pairs< message, set of processes >
   std::map< std::time_t, std::pair< std::string, std::set< std::string > > > m_changelog;

   // For better handling the EventClass does not only know its Type/Label as string
   // e.g. 1e_5jet_1met but also the number of particles as integer!
   //
   std::map< std::string , int> m_countmap;

   // Store the minimum possible Sumpt and MET (min. Minv is always 0) in this
   // EventClass, defined from trigger and selection pt cuts for all objects
   // contributing to this EventClass.
   std::map< std::string ,  double > m_distTypeMins;
   // Store the user required minimum values. These can be smaller or larger
   // than the topological minimum values.
   std::map< std::string ,  double > m_distTypeMinsRequire;

   // Whether event class was created with charges or not.
   bool m_analyzedCharge;
   // Absolute charge sum of leptons.
   int m_numCharge;

   // Store if the class uses b-Jets.
   bool m_analyzedBjets;

   // Add a map with distribution names and corresponding bin vectors
   std::map< std::string , std::vector < double > > m_distTypeBinInfo;
   std::set<std::string> m_systNames;
   unsigned int m_numPDFvariations;
   std::string m_histnamePDFup;

   // Store the smallest possible bin width in the dynamic binning.
   double m_bin_size_min;

   // These are dummy histograms that are initialised with dynamic binning.
   // They are used to retrieve information about the binning values, bin edges
   // etc.
   std::map< std::string,  TH1F* > m_distTypeBinsHisto;

   //following maps are not neccessarily only for this class
   //number of analyzed events, not weigthed at all
   std::map< std::string, unsigned int > eventCount;
   //sum of event weights, weighted to lumi and cross section
   std::map< std::string, double > totalEvents;
   //sum of event weights, not weighted
   std::map< std::string, double > totalEventsUnweighted;


   // This map stores a map of maps. The first layer contains one map
   // per process. Each map contains a map with one hist per distType
   // e.g. allProcHistoMap[process][distType]
   std::map< std::string, std::map<std::string,  TH1F* > > allProcHistoMap;
   // A copy of the original allProcHistoMap with unweighted events.
   std::map< std::string, std::map<std::string,  TH1F* > > allProcHistoMapUnweighted;

   // ProcHistoMaps for systematics and PDF
   // Each process map contains a vector of distributions
   // with histograms identified by their syst name or pdfweight number as a string
   std::map< std::string, std::map<std::string, std::map< std::string, TH1F* > > > SystematicsProcHistoMap;

   bool m_pdfInit;
   std::map< std::string, std::map<std::string, std::map< std::string, TH1F* > > > PDFvariedallProcHistoMap;

   // Store resultion histograms for the different processes separately.
    // in maps per distType
   //~ std::map< std::string, std::vector< TH2F* > > m_resolutionsMap;
   std::map< std::string, std::map< std::string, TH2F* > > m_resolutionsMap;

   // store the process group for each process in a map
   std::map< std::string, std::string > processGroupMap;

   // This map stores store alternative short names for ECItem names
   // if such a list was given in the constructor
   std::map< std::string, std::string > m_ECItemShortlist;

      //save the list of ALL processes which have been filled in general (from (_genInclEventClasses["Empty"])->ProcessList() )
   std::set< std::string > m_globalProcessList;
   // save scale factor and cross section (TConfig is too unsafe!!)
   std::map<std::string, double> m_scaleFactor;
   std::map< std::string, double > m_crossSection;

   // A TEventClass is only empty when it's been created via the Copy Constructor.
   // This is needed in ECMerger.py.
   bool m_isEmpty;
   // meber varibale to store if class is filled
   bool m_is_filled;
   // safety so hists are not saled twice
   bool m_scaledToXSec;
   // Generic implementations of getHistoPointer for different map types
   TH1F* getHistoPointer(const std::string& process,
                         const std::string distType,
                         const std::string histname,
                         std::map< std::string, std::map<std::string, std::map< std::string, TH1F* > > > &procMap) {
      if (procMap.find(process) == procMap.end() ) return 0;
      if (procMap[process].find(distType) == procMap[process].end() ) return 0;
      if (procMap[process][distType].find(histname) == procMap[process][distType].end() ) return 0;
      return procMap[process][distType][histname];
   }

   TH1F* getHistoPointer(const std::string& process,
      const std::string distType,
      std::map< std::string, std::map<std::string, TH1F* > > &procMap) {
      if (procMap.find(process) == procMap.end() ) return 0;
      if ( procMap[ process ].find( distType ) == procMap[ process ].end() ) return 0;
      return procMap[process][distType];
   }
   std::vector<double> convertHistoVector( const std::string distType, TH1F* hist );
   void checkLumi( TEventClass *firstEC, TEventClass *secondEC );
   void checkCrossSection( TEventClass *firstEC, TEventClass *secondEC, const std::string &process );
   void rescalePDFHistograms( const std::string &process, const double &oldWeight, const double &newWeight );

   void scaleAllHistograms( double relativeFactor );
   void scaleAllHistogramsForProcess( std::string process, double factor, bool is_absolute=false );
   template< typename T > static void scaleHistoMap(std::map< std::string, T > &map, double relativeFactor );
   static void scaleHistoMap(TH1* histogram, double relativeFactor );


   void mergeExistingProcess( std::string process,TEventClass *ECtoBeAdded, bool data );
   void addPDFtoEC( std::string process, TEventClass *ECtoBeAdded, bool data  );
   void mergeNewProcess( std::string process, TEventClass *ECtoBeAdded, bool data );
   void applyReweighting( std::vector< TH1F* > const &hists,
                          std::string const &process,
                          std::string distType
                          );
   void histNotFound(std::string classname, std::string process, std::string histname);
   float MasterFormula( std::vector< float > &weights ) const;

   TEventClass::PDFResult QuadSumCTEQ( std::vector< float >::const_iterator pdf_first,
                                       std::vector< float >::const_iterator pdf_last,
                                       std::vector< float >::const_iterator pdf_alpha
                                       ) const;

   TEventClass::PDFResult QuadSumMSTW( std::vector< float >::const_iterator pdf_first,
                                       std::vector< float >::const_iterator pdf_last,
                                       std::vector< float >::const_iterator pdf_alpha
                                       ) const;

   TEventClass::PDFResult CalcMean( std::vector< float >::const_iterator pdf_first,
                                    std::vector< float >::const_iterator pdf_last
                                    ) const;

   TEventClass::PDFResult CalcPairQuad( std::vector< float >::const_iterator first,
                                        std::vector< float >::const_iterator last
                                        ) const;


   //renames an element in a map
   //ATTENTION: If newKey is already in the map, it will be overwritten!
   template< class mapType, class keyType > bool renameInMap( mapType &aMap, const keyType &oldKey, const keyType &newKey ){
      //first check that the element to rename is actually in the map
      if( aMap.count( oldKey ) == 0 ){
         return false;
      }
      //swap the old and the new element, implicitly constructing the new element. the old element contains the empty new object afterwards, but as it will be discarded anyway...
      std::swap( aMap[ newKey ], aMap[ oldKey ] );
      //delete the old element
      size_t erased = aMap.erase( oldKey );
      //in a TEventClass, we should never have more than one process with the same name!
      if( erased == 0 || erased > 1 ){
         std::cout << "TEventClass::renameInMap: How odd, we removed no or more than 1 element from the map!" << std::endl;
         return false;
      }
      return true;
   }

   // results from ( pseudo ) scans for each distribution
   std::map< std::string, std::vector< double > > scanResults_nData;
   std::map< std::string, std::vector< double > > scanResults_pseudo_nData;

   std::map< std::string, std::vector< double > > scanResults_nMC;
   std::map< std::string, std::vector< double > > scanResults_pseudo_nMC;

   std::map< std::string, std::vector< double > > scanResults_totalUncert;
   std::map< std::string, std::vector< double > > scanResults_pseudo_totalUncert;

   // lower edges and widths for RoIs in pseudo experiments
   std::map< std::string, std::vector< double > > scanResults_lowerEdges;
   std::map< std::string, std::vector< double > > scanResults_pseudo_lowerEdges;

   std::map< std::string, std::vector< double > > scanResults_Widths;
   std::map< std::string, std::vector< double > > scanResults_pseudo_Widths;

   // compareScores (e.g. p-values) from pseudo experiments
   std::map< std::string, std::vector< double > > scanResults_CompareScores;
   std::map< std::string, std::vector< double > > scanResults_pseudo_CompareScores;

   // diced data from signal experiments
   std::map< std::string, std::vector< std::vector< double > > > scanResults_dicedData;

// Some thoughts on PDF Reweighting:
// Need for each distribution 40 additional temporary(?) histograms
//
// Possible implementation:
//
//  o add std::map<std::string, std::vector<TH1F*> > PDFvariedallProcHistoMap;
//  o filling: in fill method by handing over a vector of weights
//  o create two uncertainty histograms (an upper uncertainty hist and a lower uncert.hist)
//  o When do we have all information to create these histograms?
//    aka when do we calculate the histograms
//    are there any things to take care of? I.e. low statistics bins?
//  o ordering? SumpT1, InvMass1, MET1, SumPT2, InvMass2, MET2, ...
//    seems natural due to memory layout


    template< typename B, typename T >
    void AddToFolder( B* parent, const char* name,
        std::map<std::string, T >& map ) const;

    template< typename B, typename T >
    void AddToFolder( B* parent, const char* name,
        std::vector< T > objects ) const;

    template< typename B >
    void AddToFolder( B* parent, const char* name, TObject* object ) const;

   // Definition for ROOT
   ClassDef( TEventClass, 10 );
};


#endif // EventClassBase_hh
