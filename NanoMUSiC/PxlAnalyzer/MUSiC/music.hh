#include <iostream>
#include <string>
#include <sstream>

#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"

#include "Main/NanoAODReader.hh"

// This function will read a NanoAOD event from a tree and return a pxl::Event
// How to access data:
// nano_reader->getVal<UInt_t>("nMuon")
// nano_reader->getRVec<Float_t>("Muon_pt")
// nano_reader->getVal<Bool_t>("HLT_Mu18_Mu9")
pxl::Event *buildPxlEvent(unsigned int i_evt, NanoAODReader *nano_reader, bool isData)
{
  pxl::Core::initialize();

  // create a new pxl::Event
  pxl::Event *event = new pxl::Event();

  bool IsMC = !isData;
  std::string Process_ = "dummy";
  event->setUserRecord("MC", IsMC); // distinguish between MC and data
  event->setUserRecord("Run", nano_reader->getVal<Int_t>("run"));
  event->setUserRecord("LumiSection", nano_reader->getVal<Int_t>("luminosityBlock"));
  event->setUserRecord("EventNum", static_cast<uint64_t>(nano_reader->getVal<Int_t>("luminosityBlock")));
  event->setUserRecord("BX", 0);
  event->setUserRecord("Orbit", 0);
  event->setUserRecord("Dataset", "dummy");

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

  // // maps for matching
  // TODO!
  // std::map<const reco::Candidate *, pxl::Particle *> genmap;
  // TODO!
  // std::map<const reco::Candidate *, pxl::Particle *> genjetmap;

  // set process name
  GenEvtView->setUserRecord("Process", Process_);
  RecEvtView->setUserRecord("Process", Process_);

  // Isolation Functor for miniIsolation and multiIsolation
  // multiIsolation_.init(iEvent); // not needed in NanoAOD

  // // Generator stuff
  // if (IsMC)
  // {
  //   analyzeGenRelatedInfo(iEvent, GenEvtView); // PDFInfo, Process ID, scale, pthat
  //   analyzeGenInfo(iEvent, GenEvtView, genmap);
  //   for (vector<jet_def>::const_iterator jet_info = jet_infos.begin(); jet_info != jet_infos.end(); ++jet_info)
  //   {
  //     if (not jet_info->recoOnly)
  //     {
  //       analyzeGenJets(iEvent, GenEvtView, genjetmap, *jet_info);
  //     }
  //   }
  //   analyzeGenMET(iEvent, GenEvtView);

  //   if (UseSIM_)
  //   {
  //     analyzeSIM(iEvent, GenEvtView);
  //   }
  // }

  // dummy stuff
  if (i_evt % 100 == 0)
  {
    // printf("nMuon: %d\n", nano_reader->nMuon);
  }

  // return the produced pxl::Event
  return event;
}
