#include "AnalysisComposer.hh"

#include "Tools/Tools.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

// music includes
#include "EventClassFactory/CcEventClass.hh"

#include "boost/program_options.hpp"
using std::string;
namespace po = boost::program_options;
AnalysisComposer::AnalysisComposer() : m_analysisName("MUSiC Classification"),
                                       m_outputDirectory("./AnalysisOutput"),
                                       runOnData(false),
                                       // music variables
                                       m_XSectionsFile("$PXLANA/scales.txt"),
                                       m_RunHash("dummyhash")
{
}

po::options_description AnalysisComposer::getCmdArguments()
{
   po::options_description myoptions("Analysis options");
   myoptions.add_options()("XSections,x", po::value<string>(&m_XSectionsFile),
                           "Path to cross-sections file.")("outfilename,e", po::value<string>(&m_outfilename), "Name of the outpufile.\
                         Constructed from process if omitted")("hash", po::value<string>(&m_RunHash),
                                                               "Path to cross-sections file.");
   return myoptions;
}

void AnalysisComposer::endAnalysis()
{
}

pxl::AnalysisFork AnalysisComposer::addForkObjects(const Tools::MConfig &config,
                                                   string outputDirectory,
                                                   //~ const pxl::AnalysisFork &fork,
                                                   EventSelector &selector,
                                                   Systematics &syst,
                                                   const bool debug)
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

   CcEventClass *cc_event = new CcEventClass(config,
                                             XSections,
                                             &selector,
                                             &syst,
                                             m_outfilename,
                                             m_RunHash);
   fork.insertObject(cc_event, "CcEventClass");

   return fork;
}