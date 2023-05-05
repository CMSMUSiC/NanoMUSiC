#include "EventAnalyzer.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Filter events based on their lumi sectiona and run numbers, following the recommendations from the LUMI-POG (aka
/// "golden JSON").
auto EventAnalyzer::run_lumi_filter(Outputs &outputs, const RunLumiFilter &_run_lumi_filter) -> EventAnalyzer &
{
    if (*this)
    {
        if (_run_lumi_filter(event_info.run, event_info.lumi, is_data))
        {
            // // fmt::print("\nDEBUG - run_lumi_filter");
            outputs.fill_cutflow_histo("RunLumi", outputs.get_event_weight());
            return *this;
        }
        set_null();
        // fmt::print("\nDEBUG - DID NOT PASS RUN_LUMI FILTER: {} - {}", event_info.run,
        // event_info.lumi);
        return *this;
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Apply good primary vertex filters.
auto EventAnalyzer::npv_filter(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        if (event_info.PV_npvsGood > 0)
        {
            // // fmt::print("\nDEBUG - PV_npvsGood");
            outputs.fill_cutflow_histo("nPV", outputs.get_event_weight());
            return *this;
        }
        set_null();
        // fmt::print("\nDEBUG - DID NOT PASS n_pv FILTER");
        return *this;
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Apply JEMMET-POG recommendations on calorimeter detection quality.
auto EventAnalyzer::met_filter(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        //////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////
        // MET event filters
        // https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2#MET_Filter_Recommendations_for_R
        //////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////
        bool pass_MET_filters = true;
        if (year == Year::Run2016APV || year == Year::Run2016)
        {
            // clang-format off
                pass_MET_filters = pass_MET_filters
                                   && event_info.Flag_goodVertices
                                   && event_info.Flag_globalSuperTightHalo2016Filter
                                   && event_info.Flag_HBHENoiseFilter
                                   && event_info.Flag_HBHENoiseIsoFilter
                                   && event_info.Flag_EcalDeadCellTriggerPrimitiveFilter
                                   && event_info.Flag_BadPFMuonFilter
                                   && event_info.Flag_BadPFMuonDzFilter
                                   && event_info.Flag_eeBadScFilter;
            // clang-format on
            // event_info.Flag_BadChargedCandidateFilter;
            // event_info.Flag_hfNoisyHitsFilter;
        }

        if (year == Year::Run2017 || year == Year::Run2018)
        {
            // clang-format off
                pass_MET_filters = pass_MET_filters
                                   && event_info.Flag_goodVertices
                                   && event_info.Flag_globalSuperTightHalo2016Filter
                                   && event_info.Flag_HBHENoiseFilter
                                   && event_info.Flag_HBHENoiseIsoFilter
                                   && event_info.Flag_EcalDeadCellTriggerPrimitiveFilter
                                   && event_info.Flag_BadPFMuonFilter
                                   && event_info.Flag_BadPFMuonDzFilter
                                   && event_info.Flag_eeBadScFilter
                                   && event_info.Flag_ecalBadCalibFilter;
            // clang-format on
            // event_info.Flag_hfNoisyHitsFilter;
            // event_info.Flag_BadChargedCandidateFilter;
        }

        if (pass_MET_filters)
        {
            outputs.fill_cutflow_histo("METFilters", outputs.get_event_weight());
            return *this;
        }
        set_null();
        // fmt::print("\nDEBUG - DID NOT PASS met FILTER");
        return *this;
    }
    return *this;
}