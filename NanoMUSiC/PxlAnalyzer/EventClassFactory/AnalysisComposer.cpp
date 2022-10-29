#include "AnalysisComposer.hpp"

#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"
#include "Tools/Tools.hpp"

// music includes
#include "EventClassFactory/EventClassFactory.hpp"

#include "boost/program_options.hpp"
using std::string;
namespace po = boost::program_options;
AnalysisComposer::AnalysisComposer()
    : m_analysisName("MUSiC Classification"), m_outputDirectory("./AnalysisOutput"), runOnData(false),
      // music variables
      m_XSectionsFile("$PXLANA/scales.txt"), m_RunHash("dummyhash")
{
}

po::options_description AnalysisComposer::getCmdArguments()
{
    po::options_description myoptions("Analysis options");
    myoptions.add_options()("xsections,x", po::value<string>(&m_XSectionsFile), "Path to cross-sections file.")(
        "outputfile,e", po::value<string>(&m_outfilename), "Name of the outpufile.\
                         Constructed from process if omitted")("hash", po::value<string>(&m_RunHash),
                                                               "Path to cross-sections file.");
    return myoptions;
}

void AnalysisComposer::endAnalysis()
{
}

pxl::AnalysisFork AnalysisComposer::addForkObjects(const Tools::MConfig &config, string outputDirectory,
                                                   //~ const pxl::AnalysisFork &fork,
                                                   EventSelector &selector, Systematics &syst, const bool debug)
{
    pxl::AnalysisFork fork;
    fork.setName(m_analysisName);
    m_outputDirectory = outputDirectory;
    m_XSectionsFile = Tools::AbsolutePath(m_XSectionsFile);

    // add classification to fork
    runOnData = config.GetItem<bool>("General.RunOnData");

    const Tools::MConfig XSections(m_XSectionsFile);

    // save other configs with output
    system(("cp " + m_XSectionsFile + " . ").c_str());

    EventClassFactory *event_class_factory =
        new EventClassFactory(config, XSections, selector, syst, m_outfilename, m_RunHash);
    fork.insertObject(event_class_factory, "EventClassFactory");

    return fork;
}
