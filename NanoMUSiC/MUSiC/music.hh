
#include <algorithm>
#include <csignal>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <time.h>
#include <unordered_set>

// toml++ v3.1.0
// https://github.com/marzer/tomlplusplus
#include "Tools/cpp_helper_libs/toml.h"

#include "Tools/cpp_helper_libs/color.hh"

#include "Tools/cpp_helper_libs/emoji.hh"

// Comand line Tools
// https://github.com/adishavit/argh
#include "Tools/cpp_helper_libs/argh.h"

#include "RunLumiFilter.hh"

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

#include "Main/EventAdaptor.hh"
#include "Main/EventSelector.hh"
#include "Main/JetTypeWriter.hh"
#include "Main/ParticleMatcher.hh"
#include "Main/ReWeighter.hh"
#include "Main/RunLumiRanges.hh"
#include "Main/SkipEvents.hh"
#include "Main/Systematics.hh"
#include "Main/TOMLConfig.hh"
#include "Tools/Tools.hh"

// ROOT Stuff
#include "TFile.h"
#include "TTree.h"

#include "EventClassFactory/EventClassFactory.hh"

#include "Main/NanoAODReader.hh"
#include "nano2pxl_utils.hh"

unsigned int getIntYear(std::string year)
{
    if (year == "2016APV" || year == "2016")
    {
        return 2016;
    }
    if (year == "2017")
    {
        return 2017;
    }
    if (year == "2018")
    {
        return 2018;
    }
    return 1; // dummy
}

// This function will read a NanoAOD event from a tree and return a pxl::Event
// How to access data:
// nano_reader->getVal<UInt_t>("nMuon")
// nano_reader->getVec<Float_t>("Muon_pt")
// nano_reader->getVal<Bool_t>("HLT_Mu18_Mu9")
std::unique_ptr<pxl::Event> make_pxlevent(unsigned int i_evt, NanoAODReader &nano_reader, std::string year,
                                          std::string process, std::string dataset, bool isData, int debug)
{

    if (debug > 3)
    {
        std::cout << std::string(25, '-') << std::endl;
        std::cout << "Build pxl:Event..." << std::endl;
        std::cout << "Event number: " << i_evt << std::endl;
        std::cout << "Year: " << year << std::endl;
        std::cout << "Process: " << process << std::endl;
        std::cout << "Dataset: " << dataset << std::endl;
        std::cout << "Is Data ? " << isData << std::endl;
        std::cout << std::string(25, '-') << std::endl;
    }

    // do what ever initialization is needed
    pxl::Core::initialize();
    // create a new pxl::Event
    std::unique_ptr<pxl::Event> event = std::make_unique<pxl::Event>();

    // setup base variables
    bool IsMC = !isData;
    std::string Process_ = process; // is it really used somewhere?
    std::string Dataset_ = dataset; // is it rally used somewhere?
    // bool GenOnly_ = false;          // for now, it will be only false. Is it
    // ever true? in the past, it was used to find converted photons from SIM
    // event content. bool UseSIM_ = false;                             // for
    // now, it will be only false. Is it ever true? std::string PdfSetFileName_ =
    // "data/pdfsets.txt"; // is it really used somewhere? unsigned int Year_ =
    // getIntYear(year);            // is it really used somewhere?

    event->setUserRecord("MC", IsMC); // distinguish between MC and data
    event->setUserRecord("Run", nano_reader.getVal<UInt_t>("run"));
    event->setUserRecord("LumiSection", nano_reader.getVal<UInt_t>("luminosityBlock"));
    event->setUserRecord("EventNum", static_cast<uint64_t>(nano_reader.getVal<ULong64_t>("event")));
    // event->setUserRecord("BX", 0); // really needed? I don't thik so...
    // event->setUserRecord("Orbit", 0);// really needed? I don't thik so...
    event->setUserRecord("Dataset", Dataset_);

    pxl::EventView *RecEvtView = event->create<pxl::EventView>();
    event->setIndex("Rec", RecEvtView);
    pxl::EventView *GenEvtView = event->create<pxl::EventView>();
    event->setIndex("Gen", GenEvtView);
    pxl::EventView *TrigEvtView = event->create<pxl::EventView>();
    event->setIndex("Trig", TrigEvtView);
    pxl::EventView *FilterEvtView = event->create<pxl::EventView>();
    event->setIndex("Filter", FilterEvtView);
    GenEvtView->setName("Gen");
    RecEvtView->setName("Rec");
    TrigEvtView->setName("Trig");
    FilterEvtView->setName("Filter");

    GenEvtView->setUserRecord("Type", (std::string) "Gen");
    RecEvtView->setUserRecord("Type", (std::string) "Rec");
    TrigEvtView->setUserRecord("Type", (std::string) "Trig");
    FilterEvtView->setUserRecord("Type", (std::string) "Filter");

    // define maps for matching
    // TODO!
    // std::map<const reco::Candidate *, pxl::Particle *> genmap;
    // TODO!
    // std::map<const reco::Candidate *, pxl::Particle *> genjetmap;

    // set process name
    GenEvtView->setUserRecord("Process", Process_);
    RecEvtView->setUserRecord("Process", Process_);

    /////////////////////////////
    /// Store Gen information ///
    /////////////////////////////
    if (IsMC)
    {
        // PDFInfo, scale, pthat, ...
        analyzeGenRelatedInfo(nano_reader, GenEvtView);
        analyzeLHEParticles(nano_reader, GenEvtView);
        analyzeLHEInfo(nano_reader, GenEvtView);
        analyzegenWeight(nano_reader, GenEvtView);
        analyzeGenVertices(nano_reader, GenEvtView);
        analyzeGenParticles(nano_reader, GenEvtView);
        analyzeGenDressedLeptons(nano_reader, GenEvtView);
        analyzeGenPU(nano_reader, GenEvtView);

        analyzeGenJets(nano_reader, GenEvtView);
        analyzeGenJetsAK8(nano_reader, GenEvtView);
        analyzeGenMET(nano_reader, GenEvtView);

        // ttbar id
        GenEvtView->setUserRecord("genTtbarId", nano_reader.getVal<Int_t>("genTtbarId"));
    }

    /////////////////////////////
    /// Store Rec information ///
    /////////////////////////////

    // Trigger bits
    analyzeTrigger(nano_reader, TrigEvtView);

    // MET Filters bits
    analyseMETFilter(nano_reader, FilterEvtView);

    /////////////////////////
    // Reconstructed stuff //
    /////////////////////////

    // rho info
    analyzeRho(nano_reader, RecEvtView);

    // Primary Vertices
    analyzeRecVertices(nano_reader, RecEvtView);

    // taus - could be that those two different collections will be merged in the
    // future?
    analyzeRecTaus(nano_reader, RecEvtView);
    analyzeRecBoostedTaus(nano_reader, RecEvtView);

    // muons
    analyzeRecMuons(nano_reader, RecEvtView);

    // electrons
    analyzeRecElectrons(nano_reader, RecEvtView);

    // photons
    analyzeRecPhotons(nano_reader, RecEvtView);

    // METs
    analyzeRecMET(nano_reader, RecEvtView);
    analyzeRecPuppiMET(nano_reader, RecEvtView);

    // Jets
    analyzeRecJets(nano_reader, RecEvtView);
    analyzeRecFatJets(nano_reader, RecEvtView);

    // btag Weights
    analyzeRecBTagWeights(nano_reader, RecEvtView);

    // L1 Prefiring weights
    analyzePrefiringWeights(nano_reader, RecEvtView);

    //////////////////////////////////
    /// Store matching information ///
    //////////////////////////////////
    // if (IsMC) {
    //   const string met_name = "MET";
    //   Matcher->matchObjects(GenEvtView, RecEvtView, jet_infos, met_name);
    // }

    // return the produced pxl::Event
    return event;
}
