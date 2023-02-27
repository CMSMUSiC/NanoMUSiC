#include "EventAnalyzer.hpp"

//////////////////////////////////////////////////////////
/// Set `TriggerBits` for this event. They will be checked latter.
// Clear `TriggerBits` and save the seed trigger.
// Will loop over the `TriggerBits`. Once a fired trigger is found:
//  1 - the fired trigger bit is saved as trigger_seed
//  2 - all others bits are set to false
///
auto EventAnalyzer::set_trigger_bits() -> EventAnalyzer &
{
    if (*this)
    {
        switch (year)
        {
        case Year::Run2016APV:
            trigger_bits.set("SingleMuonLowPt", event_info.HLT_IsoMu24 || event_info.HLT_IsoTkMu24)
                .set("SingleMuonHighPt", event_info.HLT_Mu50 || event_info.HLT_TkMu50)
                .set("SingleElectronLowPt", event_info.HLT_Ele27_WPTight_Gsf || event_info.HLT_Photon175)
                .set("SingleElectronHighPt",
                     event_info.HLT_Ele27_WPTight_Gsf || event_info.HLT_Photon175 ||
                         event_info.HLT_Ele115_CaloIdVT_GsfTrkIdT)
                .set("DoubleMuon", false)
                .set("DoubleElectron", false)
                // .set("Photon", event_info.HLT_Photon200)
                .set("Photon", false)
                .set("Tau", false)
                .set("BJet", false)
                .set("Jet", false)
                .set("MET", false);
            break;
        case Year::Run2016:
            trigger_bits.set("SingleMuonLowPt", event_info.HLT_IsoMu24 || event_info.HLT_IsoTkMu24)
                .set("SingleMuonHighPt", event_info.HLT_Mu50 || event_info.HLT_TkMu50)
                .set("SingleElectronLowPt", event_info.HLT_Ele27_WPTight_Gsf || event_info.HLT_Photon175)
                .set("SingleElectronHighPt",
                     event_info.HLT_Ele27_WPTight_Gsf || event_info.HLT_Photon175 ||
                         event_info.HLT_Ele115_CaloIdVT_GsfTrkIdT)
                .set("DoubleMuon", false)
                .set("DoubleElectron", false)
                // .set("Photon", event_info.HLT_Photon200)
                .set("Photon", false)
                .set("Tau", false)
                .set("BJet", false)
                .set("Jet", false)
                .set("MET", false);
            break;
        case Year::Run2017:
            trigger_bits.set("SingleMuonLowPt", event_info.HLT_IsoMu27)
                .set("SingleMuonHighPt", event_info.HLT_Mu50 || event_info.HLT_TkMu100 || event_info.HLT_OldMu100)
                .set("SingleElectronLowPt", event_info.HLT_Ele35_WPTight_Gsf || event_info.HLT_Photon200)
                .set("SingleElectronHighPt",
                     event_info.HLT_Ele35_WPTight_Gsf || event_info.HLT_Photon200 ||
                         event_info.HLT_Ele115_CaloIdVT_GsfTrkIdT)
                .set("DoubleMuon", false)
                .set("DoubleElectron", false)
                // .set("Photon", event_info.HLT_Photon200)
                .set("Photon", false)
                .set("Tau", false)
                .set("BJet", false)
                .set("Jet", false)
                .set("MET", false);
            break;
        case Year::Run2018:
            trigger_bits.set("SingleMuonLowPt", event_info.HLT_IsoMu24)
                .set("SingleMuonHighPt", event_info.HLT_Mu50 || event_info.HLT_TkMu100 || event_info.HLT_OldMu100)
                .set("SingleElectronLowPt", event_info.HLT_Ele32_WPTight_Gsf || event_info.HLT_Photon200)
                .set("SingleElectronHighPt",
                     event_info.HLT_Ele32_WPTight_Gsf || event_info.HLT_Photon200 ||
                         event_info.HLT_Ele115_CaloIdVT_GsfTrkIdT)
                .set("DoubleMuon", false)
                .set("DoubleElectron", false)
                // .set("Photon", event_info.HLT_Photon200)
                .set("Photon", false)
                .set("Tau", false)
                .set("BJet", false)
                .set("Jet", false)
                .set("MET", false);
            break;
        default:
            throw std::runtime_error("Year (" + std::to_string(year) +
                                     ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
        }

        return *this;
    }
    return *this;
}

//////////////////////////////////////////////////////////////
/// Filter events that did not fired any trigger or do not pass double trigger firing check
///
auto EventAnalyzer::trigger_filter(Outputs &outputs) -> EventAnalyzer &
{
    if (*this)
    {
        // fmt::print("\nDEBUG - trigger_filter: {}\n", trigger_bits.as_string());
        // fmt::print("\nDEBUG - muon pt: {}\n", muons.pt);
        if (trigger_bits.any())
        {
            outputs.fill_cutflow_histo("TriggerCut", outputs.get_event_weight());
            return *this;
        }
        set_null();
        // fmt::print("\nDEBUG - DID NOT PASS hlt_trigger FILTER");
        return *this;
    }
    return *this;
}

/////////////////////////////////////////////////////////////////////
/// Will check if the current event has a matched object to a good TrgObj
// here we have to break the single responsability rule ...
// this filter also get the trigger scale factor,
// otherwise we would have to look over the objects twice
///
auto EventAnalyzer::trigger_match_filter(Outputs &outputs, const std::map<std::string_view, TrgObjMatcher> &matchers)
    -> EventAnalyzer &
{
    if (*this)
    {
        // given the trigger seed produce a triggerobj mask for that seed
        // test the correspondents objects to that seed
        // if fail, try next seed (bit)
        for (auto &&hlt_path : Trigger::ActivatedHLTPath)
        {
            if (trigger_bits.pass(hlt_path))
            {
                if (matchers.count(hlt_path) == 0)
                {
                    throw std::runtime_error(
                        fmt::format("ERROR: There is no matcher defined for path: {}\n", hlt_path));
                }
                if (hlt_path.find("SingleMuonLowPt") != std::string_view::npos)
                {
                    const auto [has_trigger_match, _trigger_sf_nominal, _trigger_sf_up, _trigger_sf_down] =
                        matchers.at(hlt_path)(trgobjs, muons, muons.good_low_pt_muons_mask["nominal"]);
                    if (has_trigger_match)
                    {
                        // set scale factors
                        trigger_sf_nominal = _trigger_sf_nominal;
                        trigger_sf_up = _trigger_sf_up;
                        trigger_sf_down = _trigger_sf_down;
                        outputs.fill_cutflow_histo("TriggerMatch", outputs.get_event_weight());
                        return *this;
                    }
                }
                if (hlt_path.find("SingleMuonHighPt") != std::string_view::npos)
                {
                    const auto [has_trigger_match, _trigger_sf_nominal, _trigger_sf_up, _trigger_sf_down] =
                        matchers.at(hlt_path)(trgobjs, muons, muons.good_high_pt_muons_mask["nominal"]);
                    if (has_trigger_match)
                    {
                        // set scale factors
                        trigger_sf_nominal = _trigger_sf_nominal;
                        trigger_sf_up = _trigger_sf_up;
                        trigger_sf_down = _trigger_sf_down;
                        outputs.fill_cutflow_histo("TriggerMatch", outputs.get_event_weight());
                        return *this;
                    }
                }
                if (hlt_path.find("SingleElectronLowPt") != std::string_view::npos)
                {
                    const auto [has_trigger_match, _trigger_sf_nominal, _trigger_sf_up, _trigger_sf_down] =
                        matchers.at(hlt_path)(trgobjs, electrons, electrons.good_low_pt_electrons_mask["nominal"]);
                    if (has_trigger_match)
                    { // set scale factors
                        trigger_sf_nominal = _trigger_sf_nominal;
                        trigger_sf_up = _trigger_sf_up;
                        trigger_sf_down = _trigger_sf_down;
                        outputs.fill_cutflow_histo("TriggerMatch", outputs.get_event_weight());
                        return *this;
                    }
                }
                if (hlt_path.find("SingleElectronHighPt") != std::string_view::npos)
                {
                    const auto [has_trigger_match, _trigger_sf_nominal, _trigger_sf_up, _trigger_sf_down] =
                        matchers.at(hlt_path)(trgobjs, electrons, electrons.good_high_pt_electrons_mask["nominal"]);
                    if (has_trigger_match)
                    {
                        // set scale factors
                        trigger_sf_nominal = _trigger_sf_nominal;
                        trigger_sf_up = _trigger_sf_up;
                        trigger_sf_down = _trigger_sf_down;
                        outputs.fill_cutflow_histo("TriggerMatch", outputs.get_event_weight());
                        return *this;
                    }
                }
            }
        }

        set_null();
        // fmt::print("\nDEBUG - DID NOT PASS triggermatch FILTER");
        return *this;
    }
    return *this;
}