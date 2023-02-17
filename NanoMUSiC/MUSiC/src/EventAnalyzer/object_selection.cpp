#include "EventAnalyzer.hpp"
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
           && (muons.highPtId >= 1)                                        //
           && (muons.tkRelIso < ObjConfig::Muons[year].TkRelIso_WP);
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
        good_low_pt_muons_mask = good_low_pt_muons_mask && get_low_pt_muons_selection_mask();
        good_high_pt_muons_mask = good_high_pt_muons_mask && get_high_pt_muons_selection_mask();
        good_muons_mask = good_low_pt_muons_mask || good_high_pt_muons_mask;

        // clear muons against themselves
        // should we do it????
        good_muons_mask = good_muons_mask &&
                          get_self_cleanning_mask(muons,
                                                  good_muons_mask,
                                                  0.4,
                                                  [&](const NanoObjects::Muons &muons, std::size_t i, std::size_t j)
                                                  {
                                                      if (not(muons.highPurity[i]) and muons.highPurity[j])
                                                      {
                                                          return false;
                                                      }
                                                      return true;
                                                  });

        //////////////////////////////////////
        // electrons
        good_low_pt_electrons_mask = good_low_pt_electrons_mask && get_low_pt_electrons_selection_mask();
        good_high_pt_electrons_mask = good_high_pt_electrons_mask && get_high_pt_electrons_selection_mask();
        good_electrons_mask = good_low_pt_electrons_mask || good_high_pt_electrons_mask;

        // clear electrons against muons
        good_electrons_mask =
            good_electrons_mask && get_cross_cleanning_mask(electrons, muons, good_electrons_mask, 0.4);

        //////////////////////////////////////
        // photons
        good_photons_mask = good_photons_mask && get_photons_selection_mask();
        good_photons_mask = good_photons_mask && get_cross_cleanning_mask(photons, muons, good_photons_mask, 0.4);
        good_photons_mask = good_photons_mask && get_cross_cleanning_mask(photons, electrons, good_photons_mask, 0.4);

        //////////////////////////////////////
        // taus
        good_taus_mask = good_taus_mask && get_taus_selection_mask();
        good_taus_mask = good_taus_mask && get_cross_cleanning_mask(taus, muons, good_taus_mask, 0.4);
        good_taus_mask = good_taus_mask && get_cross_cleanning_mask(taus, electrons, good_taus_mask, 0.4);
        good_taus_mask = good_taus_mask && get_cross_cleanning_mask(taus, photons, good_taus_mask, 0.4);

        //////////////////////////////////////
        // bjets
        good_bjets_mask = good_bjets_mask && get_bjets_selection_mask();
        good_bjets_mask = good_bjets_mask && get_cross_cleanning_mask(bjets, muons, good_bjets_mask, 0.5);
        good_bjets_mask = good_bjets_mask && get_cross_cleanning_mask(bjets, electrons, good_bjets_mask, 0.5);
        good_bjets_mask = good_bjets_mask && get_cross_cleanning_mask(bjets, photons, good_bjets_mask, 0.5);
        good_bjets_mask = good_bjets_mask && get_cross_cleanning_mask(bjets, taus, good_bjets_mask, 0.5);

        //////////////////////////////////////
        // jets
        good_jets_mask = good_jets_mask && get_jets_selection_mask();
        good_jets_mask = good_jets_mask && get_cross_cleanning_mask(jets, muons, good_jets_mask, 0.5);
        good_jets_mask = good_jets_mask && get_cross_cleanning_mask(jets, electrons, good_jets_mask, 0.5);
        good_jets_mask = good_jets_mask && get_cross_cleanning_mask(jets, photons, good_jets_mask, 0.5);
        good_jets_mask = good_jets_mask && get_cross_cleanning_mask(jets, taus, good_jets_mask, 0.5);

        //////////////////////////////////////
        // met
        good_met_mask = good_met_mask && get_met_selection_mask();
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
        if (                                          //
            (VecOps::Sum(good_muons_mask) > 0) ||     //
            (VecOps::Sum(good_electrons_mask) > 0) || //
            (VecOps::Sum(good_photons_mask) > 0) ||   //
            (VecOps::Sum(good_taus_mask) > 0) ||      //
            (VecOps::Sum(good_bjets_mask) > 0) ||     //
            (VecOps::Sum(good_jets_mask) > 0) ||      //
            (VecOps::Sum(good_met_mask) > 0)          //
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