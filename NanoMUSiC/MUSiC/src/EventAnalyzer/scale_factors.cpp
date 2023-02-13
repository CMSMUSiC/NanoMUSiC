#include "EventAnalyzer.hpp"

auto EventAnalyzer::set_l1_pre_firing_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        // L1 prefiring
        outputs.set_event_weight("L1PreFiring", "Nominal", event_info.L1PreFiringWeight_Nom);
        outputs.set_event_weight("L1PreFiring", "Up", event_info.L1PreFiringWeight_Up);
        outputs.set_event_weight("L1PreFiring", "Down", event_info.L1PreFiringWeight_Dn);

        return *this;
    }
    return *this;
}

/// TODO: Taus
auto EventAnalyzer::set_tau_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// Jets
/// No SFs are assigned to Jets. They have been measured to be close to 1.
auto EventAnalyzer::set_jet_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// MET
/// MET is present in, virtually, all events. It is not possible to assign SFs.
auto EventAnalyzer::set_met_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// Trigger
/// These weights have already been calculated during the trigger matching.
auto EventAnalyzer::set_trigger_SFs(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        outputs.set_event_weight("Trigger", "Nominal", trigger_sf_nominal);
        outputs.set_event_weight("Trigger", "Up", trigger_sf_up);
        outputs.set_event_weight("Trigger", "Down", trigger_sf_down);
        return *this;
    }
    return *this;
}