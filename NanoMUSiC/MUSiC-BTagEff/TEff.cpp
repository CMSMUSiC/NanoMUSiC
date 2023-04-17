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


auto BTagEff(std::string file_path, std::string sample_name) -> void {
  //Opens the ROOT file
  TFile* file = new TFile(file_path.c_str(), "READ");
  

  // Check if the file exists
  if(!file || file->IsZombie()) {
    cout << "Error: Failed to open ROOT File:"<<file_path;
    return;
  }
  
  // Get Histograms from the ROOT File
  TH1F* btag_matched_hist = (TH1F*)file->Get("btag_matched_hist");
  TH1F* btag_all_hist = (TH1F*)file->Get("btag_all_hist");
  TH1F* ltag_matched_hist = (TH1F*)file->Get("ltag_matched_hist");
  TH1F* ltag_all_hist = (TH1F*)file->Get("ltag_all_hist");

  // Make TEfficiency histograms
  TEfficiency *pEff_b = new TEfficiency(*btag_matched_hist,*btag_all_hist);
  pEff_b->SetTitle("B Tag Efficiency;pT;Eta");
  TCanvas *c1 = new TCanvas("c1");
  pEff_b->Draw("colz text");
  c1->Print("Outputs/PNG/Btag_TEff.png");
  c1->Print("Outputs/PDF/Btag_TEff.pdf");
  c1->Print("Outputs/C/Btag_TEff.C");
  
  
  TEfficiency *pEff_l = new TEfficiency(*ltag_matched_hist,*ltag_all_hist);
  pEff_l->SetTitle("L Tag Efficiency;pT;Eta");
  TCanvas *c2 = new TCanvas("c2");
  pEff_l->Draw("colz text");
  c2->Print("Outputs/PNG/ltag_TEff.png");
  c2->Print("Outputs/PDF/ltag_TEff.pdf");
  c2->Print("Outputs/C/ltag_TEff.C");
  file->Close();
  
  string rootFileName = "Outputs/RootFiles/"+sample_name+"_Teff.root";
  

  TFile* tfile = new TFile(rootFileName.c_str(),"RECREATE");
  TH2* TEff_b = pEff_b->CreateHistogram();
  TEff_b->SetName("TEff_bjets");
  TEff_b->Write();
  TH2* TEff_l = pEff_l->CreateHistogram();
  TEff_l->Write();
  tfile->Close();
}


void TEff(){}
  
