#include<iostream>

#include "TTree.h"
#include "Pxl/Pxl/interface/pxl/core.hh"

// This function will read a NanoAOD event from a tree and return a pxl::Event
pxl::Event * buildPxlEvent(int i_evt, TTree * tree_ptr, bool isData) {
  pxl::Event * event_ptr = new pxl::Event();
  tree_ptr->GetEntry(i_evt);

  // return the produced pxl:Event
  return event_ptr;
} 


