#include<iostream>

#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "Pxl/Pxl/interface/pxl/core.hh"

pxl::Event * buildPxlEvent() {
  pxl::Event * event_ptr = new pxl::Event();
  return event_ptr;
} 


