///----------------------Version 1.0-------------------------//
///Plotting Macro for E -> Tau Fake Rates study
///Author: Yiwen Wen
///DESY
///-----------------------------------------------------------//
#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include "TFile.h"
#include "TH1.h"
#include "TH1D.h"
#include "TROOT.h"
#include "TColor.h"
#include "TEfficiency.h"
#include "TMath.h"
void NormHist()
{
 
  cout << "Accessing the File" <<endl;
  TFile * file = new TFile("MC_PileUp_UL2017.root");
  cout << "File opened!" <<endl;
  TH1D * h0 = (TH1D*)file->Get("pileup");
  cout << "data_obs found!" <<endl;
  
  //TH1D * h1 = (TH1D*)h0->Clone("h1");
  cout << "dummy TH1D is created" <<endl; 
  //h1->SetDirectory(0);
  h0->SetDirectory(0);
  file->Close();
  
  h0->SetTitle("MC pileup");
  h0->Scale(1./h0->Integral());

  //h1->SetTitle("MC pileup");
  //h1->Scale(1./h1->Integral());
  cout << "dummy is scaled" <<endl; 
  TFile * file2 = new TFile("mc_pileup_UL2017.root", "RECREATE");
  //h1->Write("pileup");
  h0->Write("pileup");
  file2->Close();

}
