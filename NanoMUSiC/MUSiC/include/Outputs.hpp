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

////////////////////////////////////////////////////////////////////////////////
/// Creates a iterable range of integers in the interval [first, last].
inline auto make_index(long first, long last) -> const std::vector<long>
{
    auto vec = std::vector<long>(last - first + 1);
    long item = first;
    // for (std::size_t i = 0; i < vec.size(); i++)
    for (auto &this_item : vec)
    {
        this_item = item;
        item++;
    }
    return vec;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a iterable range of integers in the interval [0 (zero), length[.
inline auto make_index(long lenght) -> const std::vector<long>
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
    static constexpr auto Cuts = make_enumerate("NoCuts", "GeneratorWeight", "RunLumi", "nPV", "METFilters",
                                                "TriggerCut", "AtLeastOneSelectedObject", "TriggerMatch");
    static constexpr auto Weights = make_enumerate("Generator", "PileUp", "Lumi", "Trigger");
    // static constexpr auto Variations =
    //     make_enumerate("Default", "JEC", "JER", "MuonScale", "MuonResolution", "ElectronScale",
    //     "ElectronResolution");
    static constexpr auto Shifts = make_enumerate("Nominal", "Up", "Down");

    static constexpr auto kTotalCuts = Outputs::Cuts.size();
    static constexpr unsigned int kTotalWeights = Outputs::Weights.size();
    // static constexpr auto kTotalVariations = Outputs::Variations.size();
    static constexpr auto kTotalshifts = Outputs::Shifts.size();

    static constexpr unsigned int kMaxObjects = 100;
    static constexpr unsigned int buffer_size = 2560000;

    // output file
    std::unique_ptr<TFile> output_file;

    // output event trees
    std::unique_ptr<TTree> output_tree;

    // output data per event
    unsigned int run;
    unsigned int lumi_section;
    unsigned long event_number;
    unsigned long trigger_bits;
    unsigned int lha_id;

    // ROOT won't let us write a const to a TTree 🤔🤬 ...
    int kTotalWeights_non_const = kTotalWeights;
    std::array<float, kTotalWeights> weights_nominal;
    std::array<float, kTotalWeights> weights_up;
    std::array<float, kTotalWeights> weights_down;

    unsigned int nLHEPdfWeight;
    std::array<float, 150> LHEPdfWeight;
    float alpha_s_weight_up;
    float alpha_s_weight_down;

    unsigned int nMuon;
    std::array<float, kMaxObjects> Muon_pt;
    std::array<float, kMaxObjects> Muon_eta;
    std::array<float, kMaxObjects> Muon_phi;

    unsigned int nElectron;
    std::array<float, kMaxObjects> Electron_pt;
    std::array<float, kMaxObjects> Electron_eta;
    std::array<float, kMaxObjects> Electron_phi;

    unsigned int nPhoton;
    std::array<float, kMaxObjects> Photon_pt;
    std::array<float, kMaxObjects> Photon_eta;
    std::array<float, kMaxObjects> Photon_phi;

    unsigned int nTau;
    std::array<float, kMaxObjects> Tau_pt;
    std::array<float, kMaxObjects> Tau_eta;
    std::array<float, kMaxObjects> Tau_phi;

    unsigned int nBJet;
    std::array<float, kMaxObjects> BJet_pt;
    std::array<float, kMaxObjects> BJet_eta;
    std::array<float, kMaxObjects> BJet_phi;

    unsigned int nJet;
    std::array<float, kMaxObjects> Jet_pt;
    std::array<float, kMaxObjects> Jet_eta;
    std::array<float, kMaxObjects> Jet_phi;

    unsigned int nMET;
    std::array<float, kMaxObjects> MET_pt;
    std::array<float, kMaxObjects> MET_phi;

    // cutflow
    TH1F cutflow_histo;

    Outputs(const std::string _filename)
        : filename(_filename),
          output_file(std::unique_ptr<TFile>(TFile::Open(filename.c_str(), "RECREATE")))
    {

        output_tree = std::make_unique<TTree>("nano_music", "nano_music");
        // output_tree->SetAutoFlush(std::numeric_limits<long long>::max());
        output_tree->SetDirectory(output_file.get());

        // tree branches
        // Reference: https://root.cern.ch/doc/v626/classTTree.html#a1b2944a5a65655e135d102883a96daf2
        // meta and counters
        output_tree->Branch("run", &run, "run/i");
        output_tree->Branch("lumi_section", &lumi_section, "lumi_section/i");
        output_tree->Branch("event_number", &event_number, "event_number/g");
        output_tree->Branch("trigger_bits", &trigger_bits, "trigger_bits/g");
        output_tree->Branch("lha_id", &lha_id, "lha_id/i");

        output_tree->Branch("kTotalWeights", &kTotalWeights_non_const, "kTotalWeights/i");

        output_tree->Branch("nLHEPdfWeight", &nLHEPdfWeight, "nLHEPdfWeight/i");
        output_tree->Branch("alpha_s_weight_up", &alpha_s_weight_up, "alpha_s_weight_up/f");
        output_tree->Branch("alpha_s_weight_down", &alpha_s_weight_down, "alpha_s_weight_down/f");
        output_tree->Branch("nMuon", &nMuon, "nMuon/i");
        output_tree->Branch("nElectron", &nElectron, "nElectron/i");
        output_tree->Branch("nPhoton", &nPhoton, "nPhoton/i");
        output_tree->Branch("nTau", &nTau, "nTau/i");
        output_tree->Branch("nBJet", &nBJet, "nBJet/i");
        output_tree->Branch("nJet", &nJet, "nJet/i");
        output_tree->Branch("nMET", &nMET, "nMET/i");

        // weights
        output_tree->Branch("weights_nominal", weights_nominal.data(), "weights_nominal[kTotalWeights]/F", buffer_size);
        output_tree->Branch("weights_up", weights_up.data(), "weights_up[kTotalWeights]/F", buffer_size);
        output_tree->Branch("weights_down", weights_down.data(), "weights_down[kTotalWeights]/F", buffer_size);

        // LHE Info
        output_tree->Branch("LHEPdfWeight", LHEPdfWeight.data(), "LHEPdfWeight[nLHEPdfWeight]/F", buffer_size);

        // physical objects
        // muons
        output_tree->Branch("Muon_pt", Muon_pt.data(), "Muon_pt[nMuon]/F", buffer_size);
        output_tree->Branch("Muon_eta", Muon_eta.data(), "Muon_eta[nMuon]/F", buffer_size);
        output_tree->Branch("Muon_phi", Muon_phi.data(), "Muon_phi[nMuon]/F", buffer_size);

        // electrons
        output_tree->Branch("Electron_pt", Electron_pt.data(), "Electron_pt[nElectron]/F", buffer_size);
        output_tree->Branch("Electron_eta", Electron_eta.data(), "Electron_eta[nElectron]/F", buffer_size);
        output_tree->Branch("Electron_phi", Electron_phi.data(), "Electron_phi[nElectron]/F", buffer_size);

        // photons
        output_tree->Branch("Photon_pt", Photon_pt.data(), "Photon_pt[nPhoton]/F", buffer_size);
        output_tree->Branch("Photon_eta", Photon_eta.data(), "Photon_eta[nPhoton]/F", buffer_size);
        output_tree->Branch("Photon_phi", Photon_phi.data(), "Photon_phi[nPhoton]/F", buffer_size);

        // taus
        output_tree->Branch("Tau_pt", Tau_pt.data(), "Tau_pt[nTau]/F", buffer_size);
        output_tree->Branch("Tau_eta", Tau_eta.data(), "Tau_eta[nTau]/F", buffer_size);
        output_tree->Branch("Tau_phi", Tau_phi.data(), "Tau_phi[nTau]/F", buffer_size);

        // bjets
        output_tree->Branch("BJet_pt", BJet_pt.data(), "BJet_pt[nBJet]/F", buffer_size);
        output_tree->Branch("BJet_eta", BJet_eta.data(), "BJet_eta[nBJet]/F", buffer_size);
        output_tree->Branch("BJet_phi", BJet_phi.data(), "BJet_phi[nBJet]/F", buffer_size);

        // jets
        output_tree->Branch("Jet_pt", Jet_pt.data(), "Jet_pt[nJet]/F", buffer_size);
        output_tree->Branch("Jet_eta", Jet_eta.data(), "Jet_eta[nJet]/F", buffer_size);
        output_tree->Branch("Jet_phi", Jet_phi.data(), "Jet_phi[nJet]/F", buffer_size);

        // MET
        output_tree->Branch("MET_pt", MET_pt.data(), "MET_pt[nMET]/F", buffer_size);
        output_tree->Branch("MET_phi", MET_phi.data(), "MET_phi[nMET]/F", buffer_size);

        // make cutflow histos
        const std::string histo_name = "cutflow";
        cutflow_histo = TH1F("cutflow", "cutflow", kTotalCuts, -0.5, kTotalCuts - 0.5);
        cutflow_histo.Sumw2();
        cutflow_histo.SetDirectory(output_file.get());
    }

    auto set_event_weight(std::string_view weight, std::string_view shift, float value) -> void
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

    auto set_event_weight(std::string_view weight, float value) -> void
    {
        weights_up.at(Outputs::Weights.index_of(weight)) = value;
        weights_down.at(Outputs::Weights.index_of(weight)) = value;
        weights_nominal.at(Outputs::Weights.index_of(weight)) = value;
    }

    auto get_event_weight(std::string_view weight = "", std::string_view shift = "Nominal") -> float
    {
        auto nominal_weight = std::reduce(weights_nominal.cbegin(), weights_nominal.cend(), 1.f, std::multiplies());

        if (shift == "Up")
        {
            return weights_up.at(Outputs::Weights.index_of(weight)) /
                   weights_nominal.at(Outputs::Weights.index_of(weight)) * nominal_weight;
        }
        if (shift == "Down")
        {
            return weights_down.at(Outputs::Weights.index_of(weight)) /
                   weights_nominal.at(Outputs::Weights.index_of(weight)) * nominal_weight;
        }
        return nominal_weight;
    }

    // clear event tree
    void clear_event_tree()
    {
        run = 0;
        lumi_section = 0;
        event_number = 0;
        trigger_bits = 0;
        lha_id = 0;

        weights_nominal.fill(1.);
        weights_up.fill(1.);
        weights_down.fill(1.);

        nLHEPdfWeight = 0;
        LHEPdfWeight.fill(1.);
        alpha_s_weight_up = 1.;
        alpha_s_weight_down = 1.;

        nMuon = 0;
        Muon_pt.fill(0);
        Muon_eta.fill(0);
        Muon_phi.fill(0);

        nElectron = 0;
        Electron_pt.fill(0);
        Electron_eta.fill(0);
        Electron_phi.fill(0);

        nPhoton = 0;
        Photon_pt.fill(0);
        Photon_eta.fill(0);
        Photon_phi.fill(0);

        nTau = 0;
        Tau_pt.fill(0);
        Tau_eta.fill(0);
        Tau_phi.fill(0);

        nBJet = 0;
        BJet_pt.fill(0);
        BJet_eta.fill(0);
        BJet_phi.fill(0);

        nJet = 0;
        Jet_pt.fill(0);
        Jet_eta.fill(0);
        Jet_phi.fill(0);

        nMET = 0;
        MET_pt.fill(0);
        MET_phi.fill(0);
    }

    void fill_branches(RVec<float> &&_lhe_pdf_weights, //
                                                       // muons
                       RVec<float> &&_muon_pt,         //
                       RVec<float> &&_muon_eta,        //
                       RVec<float> &&_muon_phi,        //
                       // electrons
                       RVec<float> &&_electron_pt,  //
                       RVec<float> &&_electron_eta, //
                       RVec<float> &&_electron_phi, //
                       // photons
                       RVec<float> &&_photon_pt,  //
                       RVec<float> &&_photon_eta, //
                       RVec<float> &&_photon_phi, //
                       // taus
                       RVec<float> &&_tau_pt,  //
                       RVec<float> &&_tau_eta, //
                       RVec<float> &&_tau_phi, //
                       // bjets
                       RVec<float> &&_bjet_pt,  //
                       RVec<float> &&_bjet_eta, //
                       RVec<float> &&_bjet_phi, //
                       // jets
                       RVec<float> &&_jet_pt,  //
                       RVec<float> &&_jet_eta, //
                       RVec<float> &&_jet_phi, //
                       // met
                       RVec<float> &&_met_pt, //
                       RVec<float> &&_met_phi)
    {
        nLHEPdfWeight = _lhe_pdf_weights.size();
        std::copy(_lhe_pdf_weights.cbegin(), _lhe_pdf_weights.cend(), LHEPdfWeight.begin());

        nMuon = _muon_pt.size();
        std::copy(_muon_pt.cbegin(), _muon_pt.cend(), Muon_pt.begin());
        std::copy(_muon_eta.cbegin(), _muon_eta.cend(), Muon_eta.begin());
        std::copy(_muon_phi.cbegin(), _muon_phi.cend(), Muon_phi.begin());

        nElectron = _electron_pt.size();
        std::copy(_electron_pt.cbegin(), _electron_pt.cend(), Electron_pt.begin());
        std::copy(_electron_eta.cbegin(), _electron_eta.cend(), Electron_eta.begin());
        std::copy(_electron_phi.cbegin(), _electron_phi.cend(), Electron_phi.begin());

        nPhoton = _photon_pt.size();
        std::copy(_photon_pt.cbegin(), _photon_pt.cend(), Photon_pt.begin());
        std::copy(_photon_eta.cbegin(), _photon_eta.cend(), Photon_eta.begin());
        std::copy(_photon_phi.cbegin(), _photon_phi.cend(), Photon_phi.begin());

        nTau = _tau_pt.size();
        std::copy(_tau_pt.cbegin(), _tau_pt.cend(), Tau_pt.begin());
        std::copy(_tau_eta.cbegin(), _tau_eta.cend(), Tau_eta.begin());
        std::copy(_tau_phi.cbegin(), _tau_phi.cend(), Tau_phi.begin());

        nBJet = _bjet_pt.size();
        std::copy(_bjet_pt.cbegin(), _bjet_pt.cend(), BJet_pt.begin());
        std::copy(_bjet_eta.cbegin(), _bjet_eta.cend(), BJet_eta.begin());
        std::copy(_bjet_phi.cbegin(), _bjet_phi.cend(), BJet_phi.begin());

        nJet = _jet_pt.size();
        std::copy(_jet_pt.cbegin(), _jet_pt.cend(), Jet_pt.begin());
        std::copy(_jet_eta.cbegin(), _jet_eta.cend(), Jet_eta.begin());
        std::copy(_jet_phi.cbegin(), _jet_phi.cend(), Jet_phi.begin());

        nMET = _met_pt.size();
        std::copy(_met_pt.cbegin(), _met_pt.cend(), MET_pt.begin());
        std::copy(_met_phi.cbegin(), _met_phi.cend(), MET_phi.begin());
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

    // save storaged data
    void write_data()
    {
        output_file->cd();
        output_tree->Write();
        cutflow_histo.Write();
    }
};

#endif /*OUTPUTS*/
