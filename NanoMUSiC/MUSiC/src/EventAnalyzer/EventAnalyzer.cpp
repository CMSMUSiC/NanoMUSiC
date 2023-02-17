#include "EventAnalyzer.hpp"

EventAnalyzer::EventAnalyzer(const bool &_is_data, const Year &_year, Outputs &outputs)
    : is_null(false),
      is_data(_is_data),
      year(_year)
{
}

///////////////////////////////////////////////////////////////////////////////////
/// null-ify the event
auto EventAnalyzer::set_null() -> void
{
    this->is_null = true;
}

///////////////////////////////////////////////////////////////////////////////////
/// un-null-ify - not sure when/if it would be needed, but ...
auto EventAnalyzer::unset_null() -> void
{
    this->is_null = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
/// set generator weight
/// should be called before any EventAnalyzer method, but only after all weights are available (should wait for PDF
/// and QCD Scale weights). The naming constant weights means weights that are the sample for the whole event, but
/// could differ from one event to another, e.g. pile-up. As a negative example, Muons resolution corretions are not
/// constants, within the whole event. Weights that are related to physical objects (e.g.: muon SFs) are set later,
/// if the event pass the selection.
auto EventAnalyzer::set_const_weights(Outputs &outputs, Corrector &pu_weight) -> EventAnalyzer &
{
    if (*this)
    {
        // // fmt::print("\nDEBUG - set_const_weights");
        if (!is_data)
        {
            // strangely, this should be correct weight, but in the QCD sample, genWeight and originalXWGTUP are
            // different. for safety, will consider originalXWGTUP
            // outputs.set_event_weight("Generator", event_info.genWeight);
            outputs.set_event_weight("Generator", lhe_info.originalXWGTUP);
            outputs.set_event_weight("L1PreFiring", lhe_info.originalXWGTUP);
            outputs.set_event_weight("PileUp", "Nominal", pu_weight({event_info.Pileup_nTrueInt, "nominal"}));
            outputs.set_event_weight("PileUp", "Up", pu_weight({event_info.Pileup_nTrueInt, "up"}));
            outputs.set_event_weight("PileUp", "Down", pu_weight({event_info.Pileup_nTrueInt, "down"}));
        }
        return *this;
    }
    return *this;
}

auto EventAnalyzer::fill_event_content(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        outputs.run = event_info.run;
        outputs.lumi_section = event_info.lumi;
        outputs.event_number = event_info.event;
        outputs.trigger_bits = trigger_bits.as_ulong();
        outputs.lha_id = lha_id;
        outputs.alpha_s_weight_up = alpha_s_up;
        outputs.alpha_s_weight_down = alpha_s_down;

        outputs.fill_branches(
            // LHE Info
            std::move(lhe_info.LHEPdfWeight), //

            // muons
            muons.pt[good_muons_mask],  //
            muons.eta[good_muons_mask], //
            muons.phi[good_muons_mask], //
            // electrons
            electrons.pt[good_electrons_mask],  //
            electrons.eta[good_electrons_mask], //
            electrons.phi[good_electrons_mask], //
            // photons
            photons.pt[good_photons_mask],  //
            photons.eta[good_photons_mask], //
            photons.phi[good_photons_mask], //
            // taus
            taus.pt[good_taus_mask],  //
            taus.eta[good_taus_mask], //
            taus.phi[good_taus_mask], //
            // bjets
            bjets.pt[good_bjets_mask],  //
            bjets.eta[good_bjets_mask], //
            bjets.phi[good_bjets_mask], //
            // jets
            jets.pt[good_jets_mask],  //
            jets.eta[good_jets_mask], //
            jets.phi[good_jets_mask], //
            // met
            met.pt[good_met_mask], //
            met.phi[good_met_mask]);

        return *this;
    }
    return *this;
}
