#include "EventAnalyzer.hpp"

////////////////////////////////////////////////////////
/// BTagging
/// Using Method 1A - Per event weight
/// References:
/// - https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods#1a_Event_reweighting_using_scale
/// - https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideCMSDataAnalysisSchoolLPC2023TaggingExercise
/// - https://github.com/IreneZoi/CMSDAS2023-BTV/tree/master/BTaggingExercise
/// - https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation#UltraLegacy_scale_factor_uncerta
///
/// systematic (string): central, down, down_correlated, down_uncorrelated, up, up_correlated,
/// working_point (string): L, M, T
/// flavor (int): 5=b, 4=c, 0=udsg
/// abseta (real)
/// pt (real)
/// Official instructions on systematics:
/// - Simple "up" and "down" uncertainties are only to be used when one single data era is analyzed
/// - A breakdown of SFbc and SFlight uncertainties into "up/down_correlated/uncorrelated" is to be used when the
/// fullRunII dataset is analyzed. The "uncorrelated" uncertainties are to be decorrelated between years, and the
/// "correlated" uncertainties are to be correlated between years With this scheme you should have 10 uncertainties
/// - related to the b-tagging SFs in the end:
///- btagSFbc_correlated
///- btagSFlight_correlated
///- btagSFbc_2018
///- btagSFlight_2018
///- btagSFbc_2017
///- btagSFlight_2017
///- btagSFbc_2016postVFP
///- btagSFlight_2016postVFP
///- btagSFbc_2016preVFP
///- btagSFlight_2016preVFP
auto EventAnalyzer::set_bjet_SFs(Outputs &outputs, const BTagSFCorrector &btag_sf) -> EventAnalyzer &
{
    if (*this)
    {
        RVec<float> good_bjets_pt = bjets.pt[good_bjets_mask];
        RVec<float> good_bjets_abseta = VecOps::abs(bjets.eta[good_bjets_mask]);
        RVec<float> good_bjets_hadronFlavour = bjets.hadronFlavour[good_bjets_mask];

        RVec<float> good_jets_pt = jets.pt[good_jets_mask];
        RVec<float> good_jets_abseta = VecOps::abs(jets.eta[good_jets_mask]);
        RVec<float> good_jets_hadronFlavour = jets.hadronFlavour[good_jets_mask];

        // BJetCorrelated
        outputs.set_event_weight("BJetCorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetCorrelated",
                                 "Up",
                                 btag_sf("up_correlated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetCorrelated",
                                 "Down",
                                 btag_sf("down_correlated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        // LightJetCorrelated
        outputs.set_event_weight("LightJetCorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetCorrelated",
                                 "Up",
                                 btag_sf("central",
                                         "up_correlated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetCorrelated",
                                 "Down",
                                 btag_sf("central",
                                         "down_correlated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        // BJetUncorrelated
        outputs.set_event_weight("BJetUncorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetUncorrelated",
                                 "Up",
                                 btag_sf("up_uncorrelated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("BJetUncorrelated",
                                 "Down",
                                 btag_sf("down_uncorrelated",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        // LightJetUncorrelated
        outputs.set_event_weight("LightJetUncorrelated",
                                 "Nominal",
                                 btag_sf("central",
                                         "central",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetUncorrelated",
                                 "Up",
                                 btag_sf("central",
                                         "up_uncorrelated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));
        outputs.set_event_weight("LightJetUncorrelated",
                                 "Down",
                                 btag_sf("central",
                                         "down_uncorrelated",
                                         good_bjets_pt,            //
                                         good_bjets_abseta,        //
                                         good_bjets_hadronFlavour, //
                                         good_jets_pt,             //
                                         good_jets_abseta,         //                                       //
                                         good_jets_hadronFlavour));

        return *this;
    }
    return *this;
}
