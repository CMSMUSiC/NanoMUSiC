#include <iostream>
#include <string>
#include <sstream>

#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"

#include "Main/NanoAODReader.hh"

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

void fillParticle(
    pxl::Particle *particle_ptr,
    std::string particle_type,
    std::string year,
    bool isData,
    int debug)
{
  // fill pxl::Particle
}

// This function will read a NanoAOD event from a tree and return a pxl::Event
// How to access data:
// nano_reader->getVal<UInt_t>("nMuon")
// nano_reader->getRVec<Float_t>("Muon_pt")
// nano_reader->getVal<Bool_t>("HLT_Mu18_Mu9")
pxl::Event *buildPxlEvent(
    unsigned int i_evt,
    NanoAODReader *nano_reader,
    std::string year,
    std::string process,
    std::string dataset,
    bool isData,
    int debug)
{

  std::cout << "aaaa" << std::endl;

  if (debug > 0)
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
  pxl::Event *event = new pxl::Event();
  std::cout << "aaaa" << std::endl;

  std::cout << nano_reader->getVal<UInt_t>("nMuon") << std::endl;
  std::cout << "aaaa" << std::endl;

  // std::cout << nano_reader->getRVec<Float_t>("Muon_pt") << std::endl;
  // std::cout << nano_reader->getVal<Bool_t>("HLT_Mu18_Mu9") << std::endl;

  /*
    // setup base variables
    bool IsMC = !isData;
    std::string Process_ = process; // is it really used somewhere?
    std::string Dataset_ = dataset; // is it rally used somewhere?
    bool GenOnly_ = false;          // for now, it will be only false. Is it ever true?
    // in the past, it was used to find converted photons from SIM event content.
    bool UseSIM_ = false;                             // for now, it will be only false. Is it ever true?
    std::string PdfSetFileName_ = "data/pdfsets.txt"; // is it really used somewhere?
    unsigned int Year_ = getIntYear(year);            // is it really used somewhere?

    event->setUserRecord("MC", IsMC); // distinguish between MC and data
    event->setUserRecord("Run", nano_reader->getVal<Int_t>("run"));
    event->setUserRecord("LumiSection", nano_reader->getVal<Int_t>("luminosityBlock"));
    event->setUserRecord("EventNum", static_cast<uint64_t>(nano_reader->getVal<Int_t>("event")));
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

    // Generator stuff
    // if (IsMC)
    // {
    //   // PDFInfo, Process ID, scale, pthat
    //   analyzeGenRelatedInfo(nano_reader, GenEvtView);
    //   analyzeGenInfo(nano_reader, GenEvtView, genmap);

    //   for (std::vector<string>::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info)
    //   {
    //     if (not jet_info->recoOnly)
    //     {
    //       analyzeGenJets(nano_reader, GenEvtView, genjetmap, *jet_info);
    //     }
    //   }
    //   analyzeGenMET(nano_reader, GenEvtView);
    // }

  */
  // dummy stuff
  if (i_evt % 100 == 0)
  {
    // printf("nMuon: %d\n", nano_reader->nMuon);
  }

  // return the produced pxl::Event
  return event;
}
