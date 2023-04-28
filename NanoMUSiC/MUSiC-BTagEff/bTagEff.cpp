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

float DeltaPhi(const float v1, const float v2, const double c = M_PI)
{
  float r = std::fmod(v2 - v1, 2.0 * c);
  if (r < -c) {
    r += 2.0 * c;
  }
  else if (r > c) {
    r -= 2.0 * c;
  }
  return r;
}

float DeltaEta(const float v1, const float v2)
{
  float r = v1-v2;
  return r;
}


auto bTag_Eff(std::vector<string> my_list) -> void {
  TChain chain("Events");
  for (auto &&file : my_list)
    {
      chain.Add(file.c_str());
    }
  
  
  TTreeReader t(&chain);
  TTreeReaderArray<float> Jet_pt(t, "Jet_pt");
  TTreeReaderArray<float> Jet_eta(t, "Jet_eta");
  TTreeReaderArray<float> Jet_phi(t, "Jet_phi");
  TTreeReaderArray<float> Jet_btagDeepFlavB(t, "Jet_btagDeepFlavB"); 
  TTreeReaderArray<int> Jet_partonFlavour(t, "Jet_partonFlavour"); 
  TTreeReaderValue<unsigned int> nJet(t, "nJet");
  double edges[]={0, 50, 100, 150, 200, 250, 300, 350, 400, 500, 600, 700, 800, 900, 1000, 2000, 13000};
  TH2F btag_matched_hist =  TH2F("btag_matched_hist", "Hist for matched b-jets",16,edges,10,0,4);
  TH2F btag_all_hist =  TH2F("btag_all_hist", "Hist for all b-jets",16,edges,10,0,4);
  TH2F ltag_all_hist =  TH2F("ltag_all_hist", "Hist for all light-jets",16,edges,10,0,4);
  TH2F ltag_matched_hist =  TH2F("ltag_matched_hist", "Hist for matched l-jets",16,edges,10,0,4);
  while(t.Next()){
    
    for (unsigned int _nJet = 0; _nJet < *nJet; _nJet++) {
      const auto btag = Jet_btagDeepFlavB[_nJet];
      const auto eta = std::fabs(Jet_eta[_nJet]);
      const auto phi = Jet_phi[_nJet];
      const auto pt = Jet_pt[_nJet];
      const auto weight = 1;
      const auto pId = std::abs(Jet_partonFlavour[_nJet]);
      
      if(btag>=0.71)
	{
          btag_all_hist.Fill(pt,eta,weight);
	    
        }
      if(pId ==5 and btag>=0.71)
	{
	  btag_matched_hist.Fill(pt,eta,weight);
	}

  
      if(btag<0.71)
	{
          ltag_all_hist.Fill(pt,eta,weight);
	    
        }

      if(pId !=5 and pId !=4  and btag<0.71)
	{
	  ltag_matched_hist.Fill(pt,eta,weight);
	}
      
    
    }
    
  }
  TFile* file = new TFile("outputs/efficiency_hist.root","RECREATE");
  btag_matched_hist.Write();
  btag_all_hist.Write();
  ltag_matched_hist.Write();
  ltag_all_hist.Write();
  file->Close();
  
  
}


void bTagEff(){}
