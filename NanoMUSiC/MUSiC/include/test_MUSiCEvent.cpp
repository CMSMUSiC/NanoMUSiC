#include <iostream>

#include "TFile.h"
#include "TTree.h"

// #include "MUSiCEvent.hpp"

gSystem->Load("MUSiCEvent_hh.so");

// void test_MUSiCEvent()
// {

TFile f("test_MUSiCEvent.root", "recreate");
TTree t2("test_MUSiCEvent", "test_MUSiCEvent");
auto music_content = MUSiCEvent();
t2.Branch("music_content", &music_content, 256000, 99);
std::cout << "test ..." << std::endl;

for (int i = 0; i < 10000000; i++)
{
    t2.Fill();
    music_content = MUSiCEvent();
}

t2.Write();

// }
