#include <iostream>
#include "NanoAODReader.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"

// UInt_t nMuon;

// This function will read a NanoAOD event from a tree and return a pxl::Event
pxl::Event *buildPxlEvent(int i_evt, NanoAODReader *nano_reader, bool isData)
{
  // get entry from NanoAOD file
  // tree_ptr->GetEntry(i_evt);
  nano_reader->fReader->SetEntry(i_evt);

  // create a new pxl::Event
  pxl::Event *event_ptr = new pxl::Event();

  // dummy stuff
  if (i_evt % 100 == 0)
  {
    printf("nMuon: %d\n", nano_reader->nMuon);
  }

  // return the produced pxl::Event
  return event_ptr;
}
