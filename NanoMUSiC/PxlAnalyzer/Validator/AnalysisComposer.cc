#include "Validator/AnalysisComposer.hh"

// some standard libraries
#include <exception>
#include <sstream>
#include <stdexcept>

#include <iostream>
#include <csignal>
#include <iomanip>

// Tools for configs
#include "Tools/Tools.hh"
// Pxl Libraries
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

// include the validator
#include "Validator/specialAna.hh"
#include "boost/program_options.hpp"
using std::string;
namespace po = boost::program_options;
AnalysisComposer::AnalysisComposer( ) :
   m_analysisName("Validation"),
   runOnData( false )
{}

po::options_description AnalysisComposer::getCmdArguments( ){
    // You may add your analysis specific options here.
    // you should stor them as member variables of the AnalysisComposer
    // in order to access them later in addForkObjects or endAnalysis
   po::options_description myoptions("Analysis options");
    return myoptions;
}

void AnalysisComposer::endAnalysis(){
    // You may add code here which should be called after the complete
    // analysis has finished. Place e.g. merging of different ForkObject
    // results here.
}

pxl::AnalysisFork AnalysisComposer::addForkObjects ( const Tools::MConfig &config,
                                        string outputDirectory,
                                        pdf::PDFInfo const &pdfInfo,
                                        EventSelector &selector,
                                        Systematics &syst,
                                        const bool debug){
    // This is the function where you need to initalize your Analysis.
    // Create one or several implementations of pxl::AnalysisProcess and
    //add it / them to the fork which is returned by this function.
    pxl::AnalysisFork fork;
    fork.setName( m_analysisName );
    // add validation to fork
    specialAna *ana = 0;
    ana = new specialAna( config );
    fork.insertObject( ana , "Validator" );
    return fork;
}
