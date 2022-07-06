#ifndef ANALYSISCOMPOSER
#define ANALYSISCOMPOSER
#include "boost/program_options.hpp"

#include "Main/EventSelector.hh"
#include "Main/PDFTool.hh"
#include "Main/Systematics.hh"
#include "Tools/Tools.hh"

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
namespace po = boost::program_options;
using std::string;
class AnalysisComposer {

public:
    AnalysisComposer();
    // Destructor
    ~AnalysisComposer() {;};
    //~ po::options_description addCmdArguments( argstream &as );
    po::options_description getCmdArguments( );
    pxl::AnalysisFork addForkObjects ( const Tools::MConfig &config,
                                       string outputDirectory,
                                       EventSelector &selector,
                                       Systematics &syst,
                                       const bool debug);
    void endAnalysis();

private:
    string m_analysisName;
    string m_outputDirectory;
    bool runOnData;
    // music variables
    std::string m_outfilename;
    string m_XSectionsFile;
    string m_RunHash;

};

#endif /*ANALYSISCOMPOSER*/
