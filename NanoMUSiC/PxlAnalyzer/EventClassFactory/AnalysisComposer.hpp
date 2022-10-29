#ifndef ANALYSISCOMPOSER
#define ANALYSISCOMPOSER
#include "boost/program_options.hpp"

#include "Main/EventSelector.hpp"
#include "Main/PDFTool.hpp"
#include "Main/Systematics.hpp"
#include "Tools/Tools.hpp"

#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
namespace po = boost::program_options;
using std::string;
class AnalysisComposer
{

  public:
    AnalysisComposer();
    // Destructor
    ~AnalysisComposer()
    {
        ;
    };
    //~ po::options_description addCmdArguments( argstream &as );
    po::options_description getCmdArguments();
    pxl::AnalysisFork addForkObjects(const Tools::MConfig &config, string outputDirectory, EventSelector &selector,
                                     Systematics &syst, const bool debug);
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
