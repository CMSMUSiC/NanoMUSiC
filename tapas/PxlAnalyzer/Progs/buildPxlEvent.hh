#include <iostream>

#include "TTree.h"
#include "Pxl/Pxl/interface/pxl/core.hh"

// This function will read a NanoAOD event from a tree and return a pxl::Event
pxl::Event *buildPxlEvent(int i_evt, TTree *tree_ptr, bool isData)
{
  // set branch addresses
  unsigned int nMuon;
  tree_ptr->SetBranchAddress("nMuon", &nMuon);

  // get entry from NanoAOD file
  tree_ptr->GetEntry(i_evt);

  // create a new pxl::Event
  pxl::Event *event_ptr = new pxl::Event();

  // dummy stuff
  if (i_evt % 100 == 0)
  {
    printf("nMuon: %d\n", nMuon);
  }

  // return the produced pxl::Event
  return event_ptr;
}
