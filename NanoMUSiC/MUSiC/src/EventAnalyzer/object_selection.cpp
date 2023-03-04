#include "EventAnalyzer.hpp"
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "ROOT/RVec.hxx"
#include <cmath>

// Low pT muon filter
auto EventAnalyzer::get_low_pt_muons_selection_mask() -> RVec<int>
{
    return (muons.pt >= ObjConfig::Muons[year].MinLowPt)                   //
           && (muons.pt < ObjConfig::Muons[year].MaxLowPt)                 //
           && (VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta) //
           && (muons.tightId)                                              //
           && (muons.pfRelIso04_all < ObjConfig::Muons[year].PFRelIso_WP);
}

// High pT muon filter
auto EventAnalyzer::get_high_pt_muons_selection_mask() -> RVec<int>
{
    return (muons.pt >= ObjConfig::Muons[year].MaxLowPt)                   //
           && (VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta) //
           && (muons.highPtId >= 2)                                        //
           //    && (muons.pfRelIso04_all < ObjConfig::Muons[year].PFRelIso_WP);
           && (muons.tkRelIso < ObjConfig::Muons[year].TkRelIso_WP); // only RelTkIso is available as SFs
}

// Low pT Electrons
auto EventAnalyzer::get_low_pt_electrons_selection_mask() -> RVec<int>
{
    return ((electrons.pt >= ObjConfig::Electrons[year].MinLowPt) &&
            (electrons.pt < ObjConfig::Electrons[year].MaxLowPt)) //
           && ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 1.442) ||
               ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) >= 1.566) &&
                (VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 2.5))) //
           && (electrons.cutBased >= ObjConfig::Electrons[year].cutBasedId);
}

// High pT Electrons
auto EventAnalyzer::get_high_pt_electrons_selection_mask() -> RVec<int>
{
    return (electrons.pt >= ObjConfig::Electrons[year].MaxLowPt) //
           && ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 1.442) ||
               ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) >= 1.566) &&
                (VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 2.5))) //
           && (electrons.cutBased_HEEP);
}

// Photons
auto EventAnalyzer::get_photons_selection_mask() -> RVec<int>
{
    return (photons.pt >= ObjConfig::Photons[year].MinPt) //
                                                          //    && (VecOps::abs(photons.eta) <= 1.442)         //
           && (photons.isScEtaEB)                         //
           && (not photons.isScEtaEE)                     // only EB photons
           && (photons.cutBased >= ObjConfig::Photons[year].cutBasedId) //
           && (photons.pixelSeed == false);
}

// Taus
auto EventAnalyzer::get_taus_selection_mask() -> RVec<int>
{
    return taus.pt >= ObjConfig::Taus[year].MinPt;
}

// BJets
auto EventAnalyzer::get_bjets_selection_mask() -> RVec<int>
{
    return (bjets.pt >= ObjConfig::BJets[year].MinPt)                      //
           && (VecOps::abs(bjets.eta) <= ObjConfig::BJets[year].MaxAbsEta) //
           && (bjets.jetId >= ObjConfig::BJets[year].MinJetID)             //
           && (bjets.btagDeepFlavB >= ObjConfig::BJets[year].MinBTagWPTight);
}

// Jets
auto EventAnalyzer::get_jets_selection_mask() -> RVec<int>
{
    return (jets.pt >= ObjConfig::Jets[year].MinPt)                      //
           && (VecOps::abs(jets.eta) <= ObjConfig::Jets[year].MaxAbsEta) //
           && (jets.jetId >= ObjConfig::Jets[year].MinJetID)             //
           && (jets.btagDeepFlavB < ObjConfig::Jets[year].MaxBTagWPTight);
}

// MET
auto EventAnalyzer::get_met_selection_mask() -> RVec<int>
{
    return met.pt >= ObjConfig::MET[year].MinPt;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Fill masks in order to select objects.
// ATTENTION: Care should be taken to do not forget to merge (AND operation) all different masks per object. It is
// need in order to filter out events that have no objects selected.
auto EventAnalyzer::object_selection() -> EventAnalyzer &
{
    if (*this)
    {
        //////////////////////////////////////
        // muons
        muons.good_low_pt_muons_mask["nominal"] =
            muons.good_low_pt_muons_mask["nominal"] && get_low_pt_muons_selection_mask();
        muons.good_high_pt_muons_mask["nominal"] =
            muons.good_high_pt_muons_mask["nominal"] && get_high_pt_muons_selection_mask();

        // loop over HighPt muons and ajust their pT to tunepRelPt*PF_pt
        auto dX = RVec<float>(muons.size);
        auto dY = RVec<float>(muons.size);
        for (std::size_t i = 0; i < muons.good_high_pt_muons_mask["nominal"].size(); i++)
        {
            if (muons.good_high_pt_muons_mask["nominal"][i] == 1)
            {
                dX[i] = (muons.tunepRelPt[i] * muons.pt[i] - muons.pt[i]) * std::cos(muons.phi[i]);
                dY[i] = (muons.tunepRelPt[i] * muons.pt[i] - muons.pt[i]) * std::sin(muons.phi[i]);
                muons.pt[i] = muons.tunepRelPt[i] * muons.pt[i];
            }
        }

        // transform MET accordingly
        auto current_met = Math::PtEtaPhiMVector(met.pt[0], 0., met.phi[0], 0.);
        auto new_met =
            Math::PxPyPzMVector(current_met.X() - VecOps::Sum(dX), current_met.Y() - VecOps::Sum(dY), 0., 0.);
        met.pt[0] = new_met.pt();
        met.phi[0] = new_met.phi();

        // set global muon mask ("cocktail")
        muons.good_muons_mask["nominal"] =
            muons.good_low_pt_muons_mask["nominal"] || muons.good_high_pt_muons_mask["nominal"];

        // clear muons against themselves
        // should we do it????
        // muons.good_muons_mask["nominal"] =
        //     muons.good_muons_mask["nominal"] &&
        //     get_self_cleanning_mask(muons,
        //                             muons.good_muons_mask["nominal"],
        //                             0.4,
        //                             [&](const NanoObjects::Muons &muons, std::size_t i, std::size_t j)
        //                             {
        //                                 if (not(muons.highPurity[i]) and muons.highPurity[j])
        //                                 {
        //                                     return false;
        //                                 }
        //                                 return true;
        //                             });

        //////////////////////////////////////
        // electrons
        electrons.good_low_pt_electrons_mask["nominal"] =
            electrons.good_low_pt_electrons_mask["nominal"] && get_low_pt_electrons_selection_mask();
        electrons.good_high_pt_electrons_mask["nominal"] =
            electrons.good_high_pt_electrons_mask["nominal"] && get_high_pt_electrons_selection_mask();
        electrons.good_electrons_mask["nominal"] =
            electrons.good_low_pt_electrons_mask["nominal"] || electrons.good_high_pt_electrons_mask["nominal"];

        // clear electrons against muons
        electrons.good_electrons_mask["nominal"] =
            electrons.good_electrons_mask["nominal"] &&
            get_cross_cleanning_mask(electrons, muons, electrons.good_electrons_mask["nominal"], 0.4);

        //////////////////////////////////////
        // photons
        photons.good_photons_mask["nominal"] = photons.good_photons_mask["nominal"] && get_photons_selection_mask();
        photons.good_photons_mask["nominal"] =
            photons.good_photons_mask["nominal"] &&
            get_cross_cleanning_mask(photons, muons, photons.good_photons_mask["nominal"], 0.4);
        photons.good_photons_mask["nominal"] =
            photons.good_photons_mask["nominal"] &&
            get_cross_cleanning_mask(photons, electrons, photons.good_photons_mask["nominal"], 0.4);

        //////////////////////////////////////
        // taus
        taus.good_taus_mask["nominal"] = taus.good_taus_mask["nominal"] && get_taus_selection_mask();
        taus.good_taus_mask["nominal"] = taus.good_taus_mask["nominal"] &&
                                         get_cross_cleanning_mask(taus, muons, taus.good_taus_mask["nominal"], 0.4);
        taus.good_taus_mask["nominal"] = taus.good_taus_mask["nominal"] &&
                                         get_cross_cleanning_mask(taus, electrons, taus.good_taus_mask["nominal"], 0.4);
        taus.good_taus_mask["nominal"] = taus.good_taus_mask["nominal"] &&
                                         get_cross_cleanning_mask(taus, photons, taus.good_taus_mask["nominal"], 0.4);

        //////////////////////////////////////
        // bjets
        bjets.good_jets_mask["nominal"] = bjets.good_jets_mask["nominal"] && get_bjets_selection_mask();
        bjets.good_jets_mask["nominal"] = bjets.good_jets_mask["nominal"] &&
                                          get_cross_cleanning_mask(bjets, muons, bjets.good_jets_mask["nominal"], 0.5);
        bjets.good_jets_mask["nominal"] =
            bjets.good_jets_mask["nominal"] &&
            get_cross_cleanning_mask(bjets, electrons, bjets.good_jets_mask["nominal"], 0.5);
        bjets.good_jets_mask["nominal"] =
            bjets.good_jets_mask["nominal"] &&
            get_cross_cleanning_mask(bjets, photons, bjets.good_jets_mask["nominal"], 0.5);
        bjets.good_jets_mask["nominal"] = bjets.good_jets_mask["nominal"] &&
                                          get_cross_cleanning_mask(bjets, taus, bjets.good_jets_mask["nominal"], 0.5);

        //////////////////////////////////////
        // jets
        jets.good_jets_mask["nominal"] = jets.good_jets_mask["nominal"] && get_jets_selection_mask();
        jets.good_jets_mask["nominal"] = jets.good_jets_mask["nominal"] &&
                                         get_cross_cleanning_mask(jets, muons, jets.good_jets_mask["nominal"], 0.5);
        jets.good_jets_mask["nominal"] = jets.good_jets_mask["nominal"] &&
                                         get_cross_cleanning_mask(jets, electrons, jets.good_jets_mask["nominal"], 0.5);
        jets.good_jets_mask["nominal"] = jets.good_jets_mask["nominal"] &&
                                         get_cross_cleanning_mask(jets, photons, jets.good_jets_mask["nominal"], 0.5);
        jets.good_jets_mask["nominal"] =
            jets.good_jets_mask["nominal"] && get_cross_cleanning_mask(jets, taus, jets.good_jets_mask["nominal"], 0.5);

        //////////////////////////////////////
        // met
        met.good_met_mask["nominal"] = met.good_met_mask["nominal"] && get_met_selection_mask();

        return *this;
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Returns `true` if the event has at least one object selected.
auto EventAnalyzer::has_selected_objects_filter(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        if (                                                               //
            (VecOps::Sum(muons.good_muons_mask["nominal"]) > 0) ||         //
            (VecOps::Sum(electrons.good_electrons_mask["nominal"]) > 0) || //
            (VecOps::Sum(photons.good_photons_mask["nominal"]) > 0) ||     //
            (VecOps::Sum(taus.good_taus_mask["nominal"]) > 0) ||           //
            (VecOps::Sum(bjets.good_jets_mask["nominal"]) > 0) ||          //
            (VecOps::Sum(jets.good_jets_mask["nominal"]) > 0) ||           //
            (VecOps::Sum(met.good_met_mask["nominal"]) > 0)                //
            // (VecOps::Sum(muons.good_muons_mask["nominal"]) > 1)//
        )
        {
            outputs.fill_cutflow_histo("AtLeastOneSelectedObject", outputs.get_event_weight());
            return *this;
        }
        set_null();
        // fmt::print("\nDEBUG - DID NOT PASS has_selected_objects_filter FILTER");
        return *this;
    }
    return *this;
}