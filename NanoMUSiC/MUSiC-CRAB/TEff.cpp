#include <vector>
#include <string>
#include <iostream>
#include<TTree.h>
#include<TTreeReader.h>
#include<TTreeReaderArray.h>
#include<TTreeReaderValue.h>
#include<TH1F.h>
#include<TH2F.h>
#include<TFile.h>
#include<TCanvas.h>
#include<TEfficiency.h>
#include<TChain.h>


auto TEff(std::vector<string> output_list) -> void {
  
