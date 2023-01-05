#ifndef OUTPUTS
#define OUTPUTS

#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

// ROOT stuff
#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TH1.h"
#include "TObjString.h"
#include "TObject.h"
#include "TTree.h"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include "Enumerate.hpp"
#include "MUSiCTools.hpp"
#include "NanoObjects.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

namespace IndexHelpers
{
const std::vector<long> make_index(long first, long last)
{
    auto vec = std::vector<long>(last - first + 1);
    long item = first;
    for (std::size_t i = 0; i < vec.size(); i++)
    {
        vec[i] = item;
        item++;
    }
    return vec;
}

const std::vector<long> make_index(long lenght)
{
    return make_index(0, lenght - 1);
}

} // namespace IndexHelpers

class Outputs
{
  private:
    const std::string filename;

  public:
    // variations, shifts, weights and cuts
    static constexpr auto Cuts = make_enumerate("NoCuts", "GeneratorWeight", "RunLumi", "nPV", "METFilters", "TriggerCut",
                                                "TriggerMatch", "AtLeastOneSelectedObject");
    static constexpr auto Weights = make_enumerate("Generator", "PDF", "Alpha_S", "PileUp", "Lumi", "Trigger");
    // static constexpr auto Variations =
    //     make_enumerate("Default", "JEC", "JER", "MuonScale", "MuonResolution", "ElectronScale", "ElectronResolution");
    static constexpr auto Shifts = make_enumerate("Nominal", "Up", "Down");

    static constexpr auto kTotalCuts = Outputs::Cuts.size();
    static constexpr auto kTotalWeights = Outputs::Weights.size();
    // static constexpr auto kTotalVariations = Outputs::Variations.size();
    static constexpr auto kTotalshifts = Outputs::Shifts.size();

    // output file
    std::unique_ptr<TFile> output_file;

    // output event trees
    std::unique_ptr<TTree> output_tree;

    // output data per event
    unsigned int run;
    unsigned int lumi_section;
    unsigned long event_number;
    unsigned long trigger_bits;

    std::array<float, kTotalWeights> weights_nominal;
    std::array<float, kTotalWeights> weights_up;
    std::array<float, kTotalWeights> weights_down;

    unsigned int nMuon;
    RVec<float> Muon_pt;
    RVec<float> Muon_eta;
    RVec<float> Muon_phi;

    unsigned int nElectron;
    RVec<float> Electron_pt;
    RVec<float> Electron_eta;
    RVec<float> Electron_phi;

    unsigned int nPhoton;
    RVec<float> Photon_pt;
    RVec<float> Photon_eta;
    RVec<float> Photon_phi;

    unsigned int nTau;
    RVec<float> Tau_pt;
    RVec<float> Tau_eta;
    RVec<float> Tau_phi;

    unsigned int nBJet;
    RVec<float> BJet_pt;
    RVec<float> BJet_eta;
    RVec<float> BJet_phi;

    unsigned int nJet;
    RVec<float> Jet_pt;
    RVec<float> Jet_eta;
    RVec<float> Jet_phi;

    unsigned int nMET;
    RVec<float> MET_pt;
    RVec<float> MET_phi;

    // cutflow
    TH1F cutflow_histo;

    Outputs(const std::string _filename)
        : filename(_filename), output_file(std::unique_ptr<TFile>(TFile::Open(filename.c_str(), "RECREATE")))
    {

        output_tree = std::make_unique<TTree>("nano_music", "nano_music");
        output_tree->SetDirectory(output_file.get());

        // tree branches
        output_tree->Branch("run", &run);
        output_tree->Branch("lumi_section", &lumi_section);
        output_tree->Branch("event_number", &event_number);
        output_tree->Branch("trigger_bits", &trigger_bits);
        output_tree->Branch("weights_nominal", &weights_nominal);
        output_tree->Branch("weights_up", &weights_up);
        output_tree->Branch("weights_down", &weights_down);
        output_tree->Branch("nMuon", &nMuon);
        output_tree->Branch("Muon_pt", &Muon_pt);
        output_tree->Branch("Muon_eta", &Muon_eta);
        output_tree->Branch("Muon_phi", &Muon_phi);
        output_tree->Branch("nElectron", &nElectron);
        output_tree->Branch("Electron_pt", &Electron_pt);
        output_tree->Branch("Electron_eta", &Electron_eta);
        output_tree->Branch("Electron_phi", &Electron_phi);
        output_tree->Branch("nPhoton", &nPhoton);
        output_tree->Branch("Photon_pt", &Photon_pt);
        output_tree->Branch("Photon_eta", &Photon_eta);
        output_tree->Branch("Photon_phi", &Photon_phi);
        output_tree->Branch("nTau", &nTau);
        output_tree->Branch("Tau_pt", &Tau_pt);
        output_tree->Branch("Tau_eta", &Tau_eta);
        output_tree->Branch("Tau_phi", &Tau_phi);
        output_tree->Branch("nBJet", &nBJet);
        output_tree->Branch("BJet_pt", &BJet_pt);
        output_tree->Branch("BJet_eta", &BJet_eta);
        output_tree->Branch("BJet_phi", &BJet_phi);
        output_tree->Branch("nJet", &nJet);
        output_tree->Branch("Jet_pt", &Jet_pt);
        output_tree->Branch("Jet_eta", &Jet_eta);
        output_tree->Branch("Jet_phi", &Jet_phi);
        output_tree->Branch("nMET", &nMET);
        output_tree->Branch("MET_pt", &MET_pt);
        output_tree->Branch("MET_phi", &MET_phi);

        // make cutflow histos
        const std::string histo_name = "cutflow";
        cutflow_histo = TH1F("cutflow", "cutflow", kTotalCuts, -0.5, kTotalCuts - 0.5);
        cutflow_histo.Sumw2();
        cutflow_histo.SetDirectory(output_file.get());
    }

    void set_event_weight(std::string_view weight, std::string_view shift, float value)
    {
        if (shift == "Up")
        {
            weights_up.at(Outputs::Weights.index_of(weight)) = value;
        }
        else if (shift == "Down")
        {
            weights_down.at(Outputs::Weights.index_of(weight)) = value;
        }
        else
        {
            weights_nominal.at(Outputs::Weights.index_of(weight)) = value;
        }
    }

    void set_event_weight(std::string_view weight, float value)
    {
        weights_up.at(Outputs::Weights.index_of(weight)) = value;
        weights_down.at(Outputs::Weights.index_of(weight)) = value;
        weights_nominal.at(Outputs::Weights.index_of(weight)) = value;
    }

    float get_event_weight(std::string_view weight = "", std::string_view shift = "Nominal")
    {
        auto nominal_weight = std::reduce(weights_nominal.cbegin(), weights_nominal.cend(), 1., std::multiplies<float>());

        if (shift == "Up")
        {
            return weights_up.at(Outputs::Weights.index_of(weight)) / weights_nominal.at(Outputs::Weights.index_of(weight)) *
                   nominal_weight;
        }
        else if (shift == "Down")
        {
            return weights_down.at(Outputs::Weights.index_of(weight)) / weights_nominal.at(Outputs::Weights.index_of(weight)) *
                   nominal_weight;
        }
        else
        {
            return nominal_weight;
        }
    }

    // clear event tree
    void clear_event_tree()
    {
        run = 0;
        lumi_section = 0;
        event_number = 0;
        trigger_bits = 0;

        for (auto &&idx_weight : Outputs::Weights)
        {
            weights_nominal.at(Outputs::Weights.index_of(idx_weight)) = 1.;
            weights_up.at(Outputs::Weights.index_of(idx_weight)) = 1.;
            weights_down.at(Outputs::Weights.index_of(idx_weight)) = 1.;
        }

        nMuon = 0;
        Muon_pt.clear();
        Muon_eta.clear();
        Muon_phi.clear();

        nElectron = 0;
        Electron_pt.clear();
        Electron_eta.clear();
        Electron_phi.clear();

        nPhoton = 0;
        Photon_pt.clear();
        Photon_eta.clear();
        Photon_phi.clear();

        nTau = 0;
        Tau_pt.clear();
        Tau_eta.clear();
        Tau_phi.clear();

        nBJet = 0;
        BJet_pt.clear();
        BJet_eta.clear();
        BJet_phi.clear();

        nJet = 0;
        Jet_pt.clear();
        Jet_eta.clear();
        Jet_phi.clear();

        nMET = 0;
        MET_pt.clear();
        MET_phi.clear();
    }

    void fill_event_tree()
    {
        output_tree->Fill();
    }

    // fill cutflow
    void fill_cutflow_histo(std::string_view cut, float weight)
    {
        cutflow_histo.Fill(Outputs::Cuts.index_of(cut), weight);
    }

    void fill_branches(RVec<float> &&_muon_pt, RVec<float> &&_muon_eta, RVec<float> &&_muon_phi, RVec<float> &&_electron_pt,
                       RVec<float> &&_electron_eta, RVec<float> &&_electron_phi, RVec<float> &&_photon_pt,
                       RVec<float> &&_photon_eta, RVec<float> &&_photon_phi, RVec<float> &&_tau_pt, RVec<float> &&_tau_eta,
                       RVec<float> &&_tau_phi, RVec<float> &&_bjet_pt, RVec<float> &&_bjet_eta, RVec<float> &&_bjet_phi,
                       RVec<float> &&_jet_pt, RVec<float> &&_jet_eta, RVec<float> &&_jet_phi, RVec<float> &&_met_pt,
                       RVec<float> &&_met_phi)
    {
        Muon_pt = std::move(_muon_pt);
        Muon_eta = std::move(_muon_eta);
        Muon_phi = std::move(_muon_phi);
        nMuon = Muon_pt.size();

        Electron_pt = std::move(_electron_pt);
        Electron_eta = std::move(_electron_eta);
        Electron_phi = std::move(_electron_phi);
        nElectron = Electron_pt.size();

        Photon_pt = std::move(_photon_pt);
        Photon_eta = std::move(_photon_eta);
        Photon_phi = std::move(_photon_phi);
        nPhoton = Photon_pt.size();

        Tau_pt = std::move(_tau_pt);
        Tau_eta = std::move(_tau_eta);
        Tau_phi = std::move(_tau_phi);
        nTau = Tau_pt.size();

        BJet_pt = std::move(_bjet_pt);
        BJet_eta = std::move(_bjet_eta);
        BJet_phi = std::move(_bjet_phi);
        nBJet = BJet_pt.size();

        Jet_pt = std::move(_jet_pt);
        Jet_eta = std::move(_jet_eta);
        Jet_phi = std::move(_jet_phi);
        nJet = Jet_pt.size();

        MET_pt = std::move(_met_pt);
        MET_phi = std::move(_met_phi);
        nMET = MET_pt.size();
    }

    // save storaged data
    void write_data()
    {
        output_file->cd();
        output_tree->Write();
        cutflow_histo.Write();
    }
};

#endif /*OUTPUTS*/
