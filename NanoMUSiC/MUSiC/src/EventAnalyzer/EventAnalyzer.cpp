#include "EventAnalyzer.hpp"
#include <stdexcept>

EventAnalyzer::EventAnalyzer(const bool &_is_data,
                             const Year &_year,
                             const std::optional<std::string> &generator_filter_key,
                             Outputs &outputs)
    : is_null(false),
      is_data(_is_data),
      year(_year)
{
    // checks if the requested generator_filter_key (if any) is available
    if (generator_filter_key)
    {
        if (GeneratorFilters::filters.count(*generator_filter_key) == 0)
        {
            fmt::print("ERROR: Requested Generator Filter ({}) not found.\n", *generator_filter_key);
            exit(-1);
        }
    }
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
            // outputs.set_event_weight("L1PreFiring", lhe_info.originalXWGTUP);
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
            muons.pt[muons.good_muons_mask["nominal"]],  //
            muons.eta[muons.good_muons_mask["nominal"]], //
            muons.phi[muons.good_muons_mask["nominal"]], //
            // electrons
            electrons.pt[electrons.good_electrons_mask["nominal"]],  //
            electrons.eta[electrons.good_electrons_mask["nominal"]], //
            electrons.phi[electrons.good_electrons_mask["nominal"]], //
            // photons
            photons.pt[photons.good_photons_mask["nominal"]],  //
            photons.eta[photons.good_photons_mask["nominal"]], //
            photons.phi[photons.good_photons_mask["nominal"]], //
            // taus
            taus.pt[taus.good_taus_mask["nominal"]],  //
            taus.eta[taus.good_taus_mask["nominal"]], //
            taus.phi[taus.good_taus_mask["nominal"]], //
            // bjets
            // bjets.pt[bjets.good_jets_mask["nominal"]],         //
            bjets.pt_nominal[bjets.good_jets_mask["nominal"]], //
            bjets.eta[bjets.good_jets_mask["nominal"]],        //
            bjets.phi[bjets.good_jets_mask["nominal"]],        //
            bjets.mass[bjets.good_jets_mask["nominal"]],       //
            // jets
            // jets.pt[jets.good_jets_mask["nominal"]],         //
            jets.pt_nominal[jets.good_jets_mask["nominal"]], //
            jets.eta[jets.good_jets_mask["nominal"]],        //
            jets.phi[jets.good_jets_mask["nominal"]],        //
            jets.mass[jets.good_jets_mask["nominal"]],       //
            // met
            // met.pt[met.good_met_mask["nominal"]], //
            met.pt[met.good_met_mask["nominal"]], //
            met.phi[met.good_met_mask["nominal"]]);

        return *this;
    }
    return *this;
}
