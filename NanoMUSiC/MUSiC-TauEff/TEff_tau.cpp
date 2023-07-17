//#include "TEff_tau.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <vector>
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
#include "fmt/core.h"
#include<TList.h>
#include <TObjArray.h>


auto TEff_tau(std::string file_path1,
              // std::string file_path2,
              // std::string file_path3, 
              // std::string file_path4, 
              // std::string file_path5, 
              // std::string file_path6,  
              std::string sample_name, 
              std::string sample_year) -> void {
  //Opens the ROOT file
  TFile* file1 = new TFile(file_path1.c_str(), "READ");

  // // Check if the file exists
  // if((!file1 || file1->IsZombie()) or (!file2 || file2->IsZombie()) or (!file3 || file3->IsZombie()) or (!file4 || file4->IsZombie()) or (!file5 || file5->IsZombie()) or (!file6 || file6->IsZombie())) {
  //   cout << "Error: Failed to open ROOT File:"<<file_path1;
  //   cout << "    or Failed to open ROOT File:"<<file_path2;
  //   cout << "    or Failed to open ROOT File:"<<file_path3;
  //   cout << "    or Failed to open ROOT File:"<<file_path4;
  //   cout << "    or Failed to open ROOT File:"<<file_path5;
  //   cout << "    or Failed to open ROOT File:"<<file_path6;
  //   return;
  // }


  // Different creating procedure for not tauola files
              string hist_name_all = "[one_tau_at_least]_[HIG]_[N3LO]_[GluGluHToTauTau_M-125_13TeV_AM]_[2018]_[Nominal]_[h_leptons_all]";
              string hist_name_matched = "[one_tau_at_least]_[HIG]_[N3LO]_[GluGluHToTauTau_M-125_13TeV_AM]_[2018]_[Nominal]_[h_leptons_matched]";

              TH1F* h_all = (TH1F*)file1->Get(hist_name_all.c_str());
              TH1F* h_matched = (TH1F*)file1->Get(hist_name_matched.c_str());

              string sample_id = sample_name + "_" + sample_year;

              // Make TEfficiency histograms
              TEfficiency *plotEff_tau = new TEfficiency(*h_matched,*h_all);
              plotEff_tau->SetTitle("Tau Efficiency;pT;Efficiency");
              TCanvas *c1 = new TCanvas("c1");
              plotEff_tau->Draw("ap"); 
              gPad->Update(); 
              auto graph = plotEff_tau->GetPaintedGraph(); 
              auto x_axis = graph->GetXaxis(); 
              x_axis->SetLimits(0,500);
              gPad->Update();
              c1->Print(("Outputs/tryTauEff_"+sample_id+".png").c_str());
              c1->Print(("Outputs/tryTauEff_"+sample_id+".pdf").c_str());
              c1->Print(("Outputs/tryTauEff_"+sample_id+".C").c_str());
              h_all->Draw("ap");
              c1->Print(("Outputs/tryTauEff_"+sample_id+"h_all.png").c_str());
              h_matched->Draw("ap");
              c1->Print(("Outputs/tryTauEff_"+sample_id+"h_matched.png").c_str());

              file1->Close();
    
  
  //Procedure for WToTauNu_M-X-tauola ...

  // TFile* file2 = new TFile(file_path2.c_str(), "READ");
  // TFile* file3 = new TFile(file_path3.c_str(), "READ");
  // TFile* file4 = new TFile(file_path4.c_str(), "READ");
  // TFile* file5 = new TFile(file_path5.c_str(), "READ");
  // TFile* file6 = new TFile(file_path6.c_str(), "READ");


  // TObjArray fileArray;
  // fileArray.Add(file1);
  // fileArray.Add(file2);
  // fileArray.Add(file3);
  // fileArray.Add(file4);
  // fileArray.Add(file5);
  // fileArray.Add(file6);

  // // Get Histograms from the ROOT File

  // std::vector<std::string> sample_masses = {"200", "500", "1000", "2000", "3000", "4000"};
  // std::vector<std::string> sample_names_all;
  // std::vector<std::string> sample_names_matched;
  
  // for (int j = 0; j < sample_masses.size(); j++)
  //   {
  //     sample_names_all.push_back("[one_tau_at_least]_[W]_[LO]_[WToTauNu_M-"+ sample_masses[j] +"-tauola_13TeV_P8]_[2018]_[Nominal]_[h_leptons_all]");
  //     sample_names_matched.push_back("[one_tau_at_least]_[W]_[LO]_[WToTauNu_M-"+ sample_masses[j] +"-tauola_13TeV_P8]_[2018]_[Nominal]_[h_leptons_matched]");             
  //   }

  // TList* list_all = new TList;
  // TList* list_matched = new TList;

  // TH1F* h_all_0 = (TH1F*)file1->Get(sample_names_all[0].c_str());
  // TH1F* h_matched_0 = (TH1F*)file1->Get(sample_names_matched[0].c_str());

  // for (int j = 0; j < sample_masses.size(); j++)
  //   {
  //     TFile* file = (TFile*)fileArray.At(j);
  //     TH1F* h_all_j = (TH1F*)file->Get(sample_names_all[j].c_str());
  //     TH1F* h_matched_j = (TH1F*)file->Get(sample_names_matched[j].c_str());
  //     list_all->Add(h_all_j);
  //     list_matched->Add(h_matched_j);
  //   }

  // TH1F *h_all = (TH1F*)h_all_0->Clone("h_all");
  // h_all->Reset();
  // h_all->Merge(list_all);

  // TH1F *h_matched = (TH1F*)h_matched_0->Clone("h_matched");
  // h_matched->Reset();
  // h_matched->Merge(list_matched);

  // // Name for outputfile
  // string sample_id = sample_name + "_" + sample_year;

  // // Make TEfficiency histograms
  // TEfficiency *plotEff_tau = new TEfficiency(*h_matched,*h_all);
  // plotEff_tau->SetTitle("Tau Efficiency;pT;Efficiency");
  // TCanvas *c1 = new TCanvas("c1");
  // plotEff_tau->Draw("ap"); 
  // gPad->Update(); 
  // auto graph = plotEff_tau->GetPaintedGraph(); 
  // auto x_axis = graph->GetXaxis(); 
  // x_axis->SetLimits(0,500);
  // gPad->Update();
  // c1->Print(("Outputs/tryTauEff_"+sample_id+".png").c_str());
  // c1->Print(("Outputs/tryTauEff_"+sample_id+".pdf").c_str());
  // c1->Print(("Outputs/tryTauEff_"+sample_id+".C").c_str());
  // h_all->Draw("ap");
  // c1->Print(("Outputs/tryTauEff_"+sample_id+"h_all.png").c_str());
  // h_matched->Draw("ap");
  // c1->Print(("Outputs/tryTauEff_"+sample_id+"h_matched.png").c_str());
 
  // file1->Close();
  // file2->Close();
  // file3->Close();
  // file4->Close();
  // file5->Close();
  // file6->Close();
}
    
// comment for M-200: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-200-tauola_13TeV_P8/WToTauNu_M-200-tauola_13TeV_P8_2018.root", "WToTauNu_M-200-tauola_13TeV_P8", "2018")
// comment for M-500: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-500-tauola_13TeV_P8/WToTauNu_M-500-tauola_13TeV_P8_2018.root", "WToTauNu_M-500-tauola_13TeV_P8", "2018")
// comment for M-1000: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-1000-tauola_13TeV_P8/WToTauNu_M-1000-tauola_13TeV_P8_2018.root", "WToTauNu_M-1000-tauola_13TeV_P8", "2018")
// comment for M-2000: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-2000-tauola_13TeV_P8/WToTauNu_M-2000-tauola_13TeV_P8_2018.root", "WToTauNu_M-2000-tauola_13TeV_P8", "2018")
// comment for M-3000: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-3000-tauola_13TeV_P8/WToTauNu_M-3000-tauola_13TeV_P8_2018.root", "WToTauNu_M-3000-tauola_13TeV_P8", "2018")
// comment for M-4000: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-4000-tauola_13TeV_P8/WToTauNu_M-4000-tauola_13TeV_P8_2018.root", "WToTauNu_M-4000-tauola_13TeV_P8", "2018")

//command tight wp: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-200-tauola_13TeV_P8/WToTauNu_M-200-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-500-tauola_13TeV_P8/WToTauNu_M-500-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-1000-tauola_13TeV_P8/WToTauNu_M-1000-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-2000-tauola_13TeV_P8/WToTauNu_M-2000-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-3000-tauola_13TeV_P8/WToTauNu_M-3000-tauola_13TeV_P8_2018.root","~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/WToTauNu_M-4000-tauola_13TeV_P8/WToTauNu_M-4000-tauola_13TeV_P8_2018.root", "All_samples_added", "2018")
//command loose wp: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms_loose_wp/validation_outputs/2018/WToTauNu_M-200-tauola_13TeV_P8/WToTauNu_M-200-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms_loose_wp/validation_outputs/2018/WToTauNu_M-500-tauola_13TeV_P8/WToTauNu_M-500-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms_loose_wp/validation_outputs/2018/WToTauNu_M-1000-tauola_13TeV_P8/WToTauNu_M-1000-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms_loose_wp/validation_outputs/2018/WToTauNu_M-2000-tauola_13TeV_P8/WToTauNu_M-2000-tauola_13TeV_P8_2018.root", "~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms_loose_wp/validation_outputs/2018/WToTauNu_M-3000-tauola_13TeV_P8/WToTauNu_M-3000-tauola_13TeV_P8_2018.root","~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms_loose_wp/validation_outputs/2018/WToTauNu_M-4000-tauola_13TeV_P8/WToTauNu_M-4000-tauola_13TeV_P8_2018.root", "All_samples_added_loose_wp", "2018")

//command VBFHToTauTau_M125_13TeV_PH tight wp: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/VBFHToTauTau_M125_13TeV_PH/VBFHToTauTau_M125_13TeV_PH_2018.root", "VBFHToTauTau_M125_13TeV_PH", "2018")
//command VBFHToTauTau_M125_13TeV_PH loose wp: TEff_tau("/home/home1/institut_3a/kersten/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms_loose_wp/validation_outputs/2018/VBFHToTauTau_M125_13TeV_PH/VBFHToTauTau_M125_13TeV_PH_2018.root", "VBFHToTauTau_M125_13TeV_PH_loose_wp", "2018")

//command VBFHToTauTau_M125_13TeV_PH tight wp: TEff_tau("~/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/GluGluHToTauTau_M-125_13TeV_AM/GluGluHToTauTau_M-125_13TeV_AM_2018.root", "GluGluHToTauTau_M-125_13TeV_AM", "2018")
//command VBFHToTauTau_M125_13TeV_PH loose wp: TEff_tau("/home/home1/institut_3a/kersten/music/NanoMUSiC/build/validation_output_efficiency/all_matched_histograms/validation_outputs/2018/GluGluHToTauTau_M-125_13TeV_AM/GluGluHToTauTau_M-125_13TeV_AM_2018.root", "GluGluHToTauTau_M-125_13TeV_AM_loose_wp", "2018")

void TEff_tau(){}
  
