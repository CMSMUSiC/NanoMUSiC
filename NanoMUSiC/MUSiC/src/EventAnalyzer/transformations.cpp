#include "EventAnalyzer.hpp"

#include <utility>
#include <variant>

#include "ROOT/RVec.hxx"

/// TODO:
auto EventAnalyzer::transform_muons() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_electrons() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_photons() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_taus() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_bjets_and_jets(JetCorrector &jet_corrections) -> EventAnalyzer &
{
    if (*this)
    {
        // Jets
        jet_transformer(jets, good_jets_mask, jet_corrections);
        // BJets
        jet_transformer(bjets, good_bjets_mask, jet_corrections);

        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_met() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}