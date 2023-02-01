#ifndef MUSIC_EVENT_DATA
#define MUSIC_EVENT_DATA

#include "LHAPDF/PDF.h"
#include <algorithm>
#include <exception>
#include <functional>
#include <stdexcept>
#include <string_view>

#include <fmt/format.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "LHAPDF/LHAPDF.h"
#pragma GCC diagnostic pop

#include "Configs.hpp"
#include "NanoObjects.hpp"
#include "Outputs.hpp"
#include "RunLumiFilter.hpp"
#include "Trigger.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;

class EventData
{
  private:
    bool is_null = true;

  public:
    TriggerBits trigger_bits;
    float trigger_sf_nominal = 1.;
    float trigger_sf_up = 1.;
    float trigger_sf_down = 1.;

    NanoObjects::EventInfo event_info;

    NanoObjects::GeneratorInfo generator_info;

    NanoObjects::LHEInfo lhe_info;
    int lha_id;
    float alpha_s_up = 1.;
    float alpha_s_down = 1.;
    float scale_envelope_weight_up = 1.;
    float scale_envelope_weight_down = 1.;

    NanoObjects::Muons muons;
    RVec<int> good_muons_mask;
    RVec<int> good_low_pt_muons_mask;
    RVec<int> good_high_pt_muons_mask;

    NanoObjects::Electrons electrons;
    RVec<int> good_electrons_mask;
    RVec<int> good_low_pt_electrons_mask;
    RVec<int> good_high_pt_electrons_mask;

    NanoObjects::Photons photons;
    RVec<int> good_photons_mask;

    NanoObjects::Taus taus;
    RVec<int> good_taus_mask;

    NanoObjects::BJets bjets;
    RVec<int> good_bjets_mask;

    NanoObjects::Jets jets;
    RVec<int> good_jets_mask;

    NanoObjects::MET met;
    RVec<int> good_met_mask;

    NanoObjects::TrgObjs trgobjs;
    RVec<int> good_trgobjs_mask;

    bool is_data = true;
    Year year = Year::kTotalYears;

    EventData(const bool &_is_data, const Year &_year)
        : is_null(false),
          is_data(_is_data),
          year(_year)
    {
    }

    // builder interface
    auto set_event_info(NanoObjects::EventInfo &&_event_info) -> EventData &
    {
        if (*this)
        {
            event_info = _event_info;
            return *this;
        }
        return *this;
    }

    auto set_generator_info(NanoObjects::GeneratorInfo &&_generator_info) -> EventData &
    {
        if (*this)
        {
            generator_info = _generator_info;
            return *this;
        }
        return *this;
    }

    auto set_lhe_info(NanoObjects::LHEInfo &&_lhe_info) -> EventData &
    {
        if (*this)
        {
            lhe_info = _lhe_info;
            return *this;
        }
        return *this;
    }

    auto set_muons(NanoObjects::Muons &&_muons, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            muons = _muons;
            good_muons_mask = mask;
            good_low_pt_muons_mask = mask;
            good_high_pt_muons_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_muons(NanoObjects::Muons &&_muons) -> EventData &
    {
        return set_muons(std::move(_muons), RVec<int>(_muons.size, 1));
    }

    auto set_electrons(NanoObjects::Electrons &&_electrons, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            electrons = _electrons;
            good_electrons_mask = mask;
            good_low_pt_electrons_mask = mask;
            good_high_pt_electrons_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_electrons(NanoObjects::Electrons &&_electrons) -> EventData &
    {
        return set_electrons(std::move(_electrons), RVec<int>(_electrons.size, 1));
    }

    auto set_photons(NanoObjects::Photons &&_photons, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            photons = _photons;
            good_photons_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_photons(NanoObjects::Photons &&_photons) -> EventData &
    {
        return set_photons(std::move(_photons), RVec<int>(_photons.size, 1));
    }

    auto set_taus(NanoObjects::Taus &&_taus, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            taus = _taus;
            good_taus_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_taus(NanoObjects::Taus &&_taus) -> EventData &
    {
        return set_taus(std::move(_taus), RVec<int>(_taus.size, 1));
    }

    auto set_bjets(NanoObjects::BJets &&_bjets, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            bjets = _bjets;
            good_bjets_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_bjets(NanoObjects::BJets &&_bjets) -> EventData &
    {
        return set_bjets(std::move(_bjets), RVec<int>(_bjets.size, 1));
    }

    auto set_jets(NanoObjects::Jets &&_jets, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            jets = _jets;
            good_jets_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_jets(NanoObjects::Jets &&_jets) -> EventData &
    {
        return set_jets(std::move(_jets), RVec<int>(_jets.size, 1));
    }

    auto set_met(NanoObjects::MET &&_met, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            met = _met;
            good_met_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_met(NanoObjects::MET &&_met) -> EventData &
    {
        return set_met(std::move(_met), RVec<int>(_met.size, 1));
    }

    auto set_trgobjs(NanoObjects::TrgObjs &&_trgobjs, RVec<int> &&mask) -> EventData &
    {
        if (*this)
        {
            trgobjs = _trgobjs;
            good_trgobjs_mask = mask;
            return *this;
        }
        return *this;
    }

    auto set_trgobjs(NanoObjects::TrgObjs &&_trgobjs) -> EventData &
    {
        return set_trgobjs(std::move(_trgobjs), RVec<int>(_trgobjs.size, 1));
    }

    ///////////////////////////////////////////////////////////////////////////////////
    /// is it a null event
    operator bool() const
    {
        return !is_null;
    }

    ///////////////////////////////////////////////////////////////////////////////////
    /// null-ify the event
    void set_null()
    {
        this->is_null = true;
    }

    ///////////////////////////////////////////////////////////////////////////////////
    /// un-null-ify - not sure when/if it would be needed, but ...
    void unset_null()
    {
        this->is_null = true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set PDF and Alpha_S uncertainties.
    /// Those are tricky beasts, since they are not simple weights added to the event, but rather, should be treated as
    /// variations and have their uncert. squared-summed in the end of the processing (classification).
    /// This method also saves the LHA ID that was used during generation or rescaling.
    auto set_pdf_alpha_s_weights(
        const std::optional<std::pair<unsigned int, unsigned int>> &lha_indexes,
        const std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>, std::unique_ptr<LHAPDF::PDF>,
                         std::unique_ptr<LHAPDF::PDF>> &default_pdf_sets) -> EventData &
    {
        if (*this)
        {
            // references are dangerous!!!!!
            // be carefull with life time
            auto &default_pdf = std::get<0>(default_pdf_sets);
            auto &alpha_s_up_pdf = std::get<1>(default_pdf_sets);
            auto &alpha_s_down_pdf = std::get<2>(default_pdf_sets);

            if (lhe_info.nLHEPdfWeight > 0)
            {
                // set LHA ID
                auto [lha_id, _] = lha_indexes.value_or(std::pair<unsigned int, unsigned int>());
                if (lha_id == 0)
                {
                    throw std::runtime_error(fmt::format(
                        "There are PDF weights written in the file, but the REGEX failed to get a proper LHA ID."));
                }

                // has alpha_s weights
                if (lhe_info.nLHEPdfWeight == 103 or lhe_info.nLHEPdfWeight == 33)
                {
                    alpha_s_up = lhe_info.LHEPdfWeight[101];
                    alpha_s_down = lhe_info.LHEPdfWeight[102];

                    // remove the first weight (always 1.)
                    lhe_info.LHEPdfWeight.erase(lhe_info.LHEPdfWeight.begin());

                    // remove last two elements (Alpha_S weights)
                    lhe_info.LHEPdfWeight.erase(lhe_info.LHEPdfWeight.end() - 1);
                    lhe_info.LHEPdfWeight.erase(lhe_info.LHEPdfWeight.end() - 1);
                }
                // don't have alpha_s weights, should get the one from the 5f LHAPDF set.
                // REF:
                // https://cms-pdmv.gitbook.io/project/mccontact/info-for-mc-production-for-ultra-legacy-campaigns-2016-2017-2018#recommendations-on-the-usage-of-pdfs-and-cpx-tunes
                else if (lhe_info.nLHEPdfWeight == 101 or lhe_info.nLHEPdfWeight == 31)
                {
                    // Those are some possible convertion from for NNPDF31, without to with alpha_s
                    // During the classification, the coded should check the status of alpha_s_up and alpha_s_down
                    // and react accordinly.
                    // 304400 --> 306000
                    // 316200 --> 325300
                    // 325500 --> 325300
                    // 320900 --> 306000

                    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0120 (319500) and divide the new
                    // weight by the weight from the PDF the event was produced with.
                    alpha_s_up = alpha_s_up_pdf->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
                                 alpha_s_up_pdf->xfxQ(generator_info.id2, generator_info.x2, generator_info.scalePDF) /
                                 lhe_info.originalXWGTUP;

                    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0116 (319300) and divide the new
                    // weight by the weight from the PDF the event was produced with.
                    alpha_s_down =
                        alpha_s_down_pdf->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
                        alpha_s_down_pdf->xfxQ(generator_info.id2, generator_info.x2, generator_info.scalePDF) /
                        lhe_info.originalXWGTUP;
                }
                else
                {
                    throw std::runtime_error(fmt::format(
                        "Unexpected number of PDF weights ({}). According to CMSSW "
                        "(https://github.dev/cms-sw/cmssw/blob/6ef534126e6db3dfdea86c3f0eedb773f0117cbc/PhysicsTools/"
                        "NanoAOD/python/genWeightsTable_cfi.py#L20) if should be eighther 101, 103, 31 or 33.\n",
                        lhe_info.nLHEPdfWeight));
                }
            }
            else
            {
                if (not is_data)
                {
                    // NNPDF31_nnlo_as_0118_hessian
                    lha_id = 304400;

                    // Compute the PDF weight for this event using NNPDF31_nnlo_as_0118_hessian (304400) and divide the
                    // new weight by the weight from the PDF the event was produced with.
                    lhe_info.LHEPdfWeight.reserve(default_pdf.size() - 1);
                    lhe_info.originalXWGTUP =
                        default_pdf[0]->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
                        default_pdf[0]->xfxQ(generator_info.id2, generator_info.x2, generator_info.scalePDF);

                    // skip the first, since it correspondeds to the originalXWGTUP (nominal)
                    for (std::size_t i = 1; i < default_pdf.size(); i++)
                    {
                        lhe_info.LHEPdfWeight.push_back(
                            default_pdf[i]->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
                            default_pdf[i]->xfxQ(generator_info.id2, generator_info.x2, generator_info.scalePDF) /
                            lhe_info.originalXWGTUP);
                    }

                    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0120 (319500) and divide the new
                    // weight by the weight from the PDF the event was produced with.
                    alpha_s_up = alpha_s_up_pdf->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
                                 alpha_s_up_pdf->xfxQ(generator_info.id2, generator_info.x2, generator_info.scalePDF) /
                                 lhe_info.originalXWGTUP;

                    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0116 (319300) and divide the new
                    // weight by the weight from the PDF the event was produced with.
                    alpha_s_down =
                        alpha_s_down_pdf->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
                        alpha_s_down_pdf->xfxQ(generator_info.id2, generator_info.x2, generator_info.scalePDF) /
                        lhe_info.originalXWGTUP;
                }
            }
            return *this;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    /// Set the QCD Scaling weights, using the envelope method. If the sample has no weights are kept as 1.
    auto set_scale_weights() -> EventData &
    {
        if (*this)
        {
            if (lhe_info.nLHEScaleWeight > 0)
            {

                if (lhe_info.nLHEScaleWeight != 9)
                {
                    throw std::runtime_error(fmt::format("Unexpected number of Scale weights ({}). Expected to be 9.\n",
                                                         lhe_info.nLHEScaleWeight));
                }
                auto murf_nominal = lhe_info.LHEScaleWeight[4];

                // remove indexes 2 and 6 since they corresponds to unphysical values ()
                lhe_info.LHEScaleWeight.erase(lhe_info.LHEScaleWeight.begin() + 2);
                lhe_info.LHEScaleWeight.erase(lhe_info.LHEScaleWeight.begin() + 6);

                // The nominal LHEScaleWeight is expected to be 1.
                // All variations will be normalized to the nominal LHEScaleWeight and it is assumed that the nominal
                // weight is already included in the LHEWeight.
                // rescale, just in case, scale all weights to the nominal
                for (auto &scale_weight : lhe_info.LHEScaleWeight)
                {
                    scale_weight /= murf_nominal;
                }

                scale_envelope_weight_up = VecOps::Max(lhe_info.LHEScaleWeight);
                scale_envelope_weight_down = VecOps::Min(lhe_info.LHEScaleWeight);
            }
            else
            {
                if (not is_data)
                {
                    // it is already set, but just to be sure ...
                    // not much to do here ...
                    scale_envelope_weight_up = 1.;
                    scale_envelope_weight_down = 1.;
                }
            }

            return *this;
        }
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    /// set generator weight
    /// should be called before any EventData method, but only after all weights are available (should wait for PDF and
    /// QCD Scale weights).
    /// The naming constant weights means weights that are the sample for the whole event, but could differ from one
    /// event to another, e.g. pile-up. As a negative example, Muons resolution corretions are not constants, within the
    /// whole event.
    /// Weights that are related to physical objects (e.g.: muon SFs) are set later, if the event pass the selection.
    auto set_const_weights(Outputs &outputs, Corrector &pu_weight) -> EventData &
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
                outputs.set_event_weight("PileUp", "Nominal", pu_weight({event_info.Pileup_nTrueInt, "nominal"}));
                outputs.set_event_weight("PileUp", "Up", pu_weight({event_info.Pileup_nTrueInt, "up"}));
                outputs.set_event_weight("PileUp", "Down", pu_weight({event_info.Pileup_nTrueInt, "down"}));
            }
            return *this;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    /// Filter events based on their Generator process. This is implemented in order to avoid overlap of phase-space
    /// between MC samples.
    /// Should come after all constant weights are available.
    auto generator_filter(Outputs &outputs) -> EventData &
    {
        if (*this)
        {
            bool is_good_gen = true;
            // if MC
            if (!is_data)
            {
                /////////////////////////////////////////////////
                // FIXME: check if it is good gen event
                /////////////////////////////////////////////////
                if (true)
                {
                    is_good_gen = true;
                }
            }
            if (is_good_gen)
            {
                // // fmt::print("\nDEBUG - generator_filter");
                outputs.fill_cutflow_histo("NoCuts", 1.);
                outputs.fill_cutflow_histo("GeneratorWeight", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS generator FILTER");
            return *this;
        }
        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Filter events based on their lumi sectiona and run numbers, following the recommendations from the LUMI-POG (aka
    /// "golden JSON").
    auto run_lumi_filter(Outputs &outputs, const RunLumiFilter &_run_lumi_filter) -> EventData &
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
    auto npv_filter(Outputs &outputs) -> EventData &
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
    auto met_filter(Outputs &outputs) -> EventData &
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
                // // fmt::print("\nDEBUG - met_filter");
                outputs.fill_cutflow_histo("METFilters", outputs.get_event_weight());
                return *this;
            }
            set_null();
            // fmt::print("\nDEBUG - DID NOT PASS met FILTER");
            return *this;
        }
        return *this;
    }

    //////////////////////////////////////////////////////////
    /// Set `TriggerBits` for this event. They will be checked latter.
    // Clear `TriggerBits` and save the seed trigger.
    // Will loop over the `TriggerBits`. Once a fired trigger is found:
    //  1 - the fired trigger bit is saved as trigger_seed
    //  2 - all others bits are set to false
    ///
    auto set_trigger_bits() -> EventData &
    {
        if (*this)
        {
            switch (year)
            {
            case Year::Run2016APV:
                trigger_bits.set("SingleMuonLowPt", event_info.HLT_IsoMu24 || event_info.HLT_IsoTkMu24)
                    .set("SingleMuonHighPt", event_info.HLT_Mu50 || event_info.HLT_TkMu50)
                    .set("SingleElectronLowPt", event_info.HLT_Ele27_WPTight_Gsf || event_info.HLT_Photon175)
                    .set("SingleElectronHighPt", event_info.HLT_Ele27_WPTight_Gsf || event_info.HLT_Photon175 ||
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
                    .set("SingleElectronHighPt", event_info.HLT_Ele27_WPTight_Gsf || event_info.HLT_Photon175 ||
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
                    .set("SingleElectronHighPt", event_info.HLT_Ele35_WPTight_Gsf || event_info.HLT_Photon200 ||
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
                    .set("SingleElectronHighPt", event_info.HLT_Ele32_WPTight_Gsf || event_info.HLT_Photon200 ||
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
    auto trigger_filter(Outputs &outputs) -> EventData &
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

    // Low pT muon filter
    auto get_low_pt_muons_selection_mask() -> RVec<int>
    {

        // fmt::print("Eta: {}\n", muons.eta);
        // fmt::print("Eta mask: {}\n", VecOps::abs(muons.eta) <= 2.4);
        // fmt::print("Eta mask from Config: {}\n", VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta);
        return (muons.pt >= ObjConfig::Muons[year].MinLowPt)                   //
               && (muons.pt < ObjConfig::Muons[year].MaxLowPt)                 //
               && (VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta) //
               && (muons.tightId)                                              //
               && (muons.pfRelIso03_all < ObjConfig::Muons[year].PFRelIso_WP);
    }

    // High pT muon filter
    auto get_high_pt_muons_selection_mask() -> RVec<int>
    {
        return (muons.pt >= ObjConfig::Muons[year].MaxLowPt)                   //
               && (VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta) //
               && (muons.highPtId >= 1)                                        //
               && (muons.tkRelIso < ObjConfig::Muons[year].TkRelIso_WP);
    }

    // Low pT Electrons
    auto get_low_pt_electrons_selection_mask() -> RVec<int>
    {
        return ((electrons.pt >= ObjConfig::Electrons[year].MinLowPt) &&
                (electrons.pt < ObjConfig::Electrons[year].MaxLowPt)) //
               && ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 1.442) ||
                   ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) >= 1.566) &&
                    (VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 2.5))) //
               && (electrons.cutBased >= 4);
    }

    // High pT Electrons
    auto get_high_pt_electrons_selection_mask() -> RVec<int>
    {
        return (electrons.pt >= ObjConfig::Electrons[year].MaxLowPt) //
               && ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 1.442) ||
                   ((VecOps::abs(electrons.eta + electrons.deltaEtaSC) >= 1.566) &&
                    (VecOps::abs(electrons.eta + electrons.deltaEtaSC) <= 2.5))) //
               && (electrons.cutBased_HEEP);
    }

    // Photons
    auto get_photons_selection_mask() -> RVec<int>
    {
        return (photons.pt >= ObjConfig::Photons[year].MinPt) //
                                                              //    && (VecOps::abs(photons.eta) <= 1.442)         //
               && (photons.isScEtaEB)                         //
               && (not photons.isScEtaEE)                     //
               && (photons.cutBased >= 3)                     //
               && (photons.pixelSeed == false);
    }

    // Taus
    auto get_taus_selection_mask() -> RVec<int>
    {
        return taus.pt >= ObjConfig::Taus[year].MinPt;
    }

    // BJets
    auto get_bjets_selection_mask() -> RVec<int>
    {
        return (bjets.pt >= ObjConfig::BJets[year].MinPt)                      //
               && (VecOps::abs(bjets.eta) <= ObjConfig::BJets[year].MaxAbsEta) //
               && (bjets.jetId >= ObjConfig::BJets[year].MinJetID)             //
               && (bjets.btagDeepFlavB >= ObjConfig::BJets[year].MinBTagWPTight);
    }

    // Jets
    auto get_jets_selection_mask() -> RVec<int>
    {
        return (jets.pt >= ObjConfig::Jets[year].MinPt)                      //
               && (VecOps::abs(jets.eta) <= ObjConfig::Jets[year].MaxAbsEta) //
               && (jets.jetId >= ObjConfig::Jets[year].MinJetID)             //
               && (jets.btagDeepFlavB < ObjConfig::Jets[year].MaxBTagWPTight);
    }

    // MET
    auto get_met_selection_mask() -> RVec<int>
    {
        return met.pt >= ObjConfig::MET[year].MinPt;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Fill masks in order to select objects.
    // ATTENTION: Care should be taken to do not forget to merge (AND operation) all different masks per object. It is
    // need in order to filter out events that have no objects selected.
    auto object_selection() -> EventData &
    {
        if (*this)
        {
            // launch selection tasks
            good_low_pt_muons_mask = good_low_pt_muons_mask && get_low_pt_muons_selection_mask();
            good_high_pt_muons_mask = good_high_pt_muons_mask && get_high_pt_muons_selection_mask();
            good_muons_mask = good_low_pt_muons_mask || good_high_pt_muons_mask;

            good_low_pt_electrons_mask = good_low_pt_electrons_mask && get_low_pt_electrons_selection_mask();
            good_high_pt_electrons_mask = good_high_pt_electrons_mask && get_high_pt_electrons_selection_mask();
            good_electrons_mask = good_low_pt_electrons_mask || good_high_pt_electrons_mask;

            good_photons_mask = good_photons_mask && get_photons_selection_mask();
            good_taus_mask = good_taus_mask && get_taus_selection_mask();
            good_bjets_mask = good_bjets_mask && get_bjets_selection_mask();
            good_jets_mask = good_jets_mask && get_jets_selection_mask();
            good_met_mask = good_met_mask && get_met_selection_mask();
            return *this;
        }
        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Returns `true` if the event has at least one object selected.
    auto has_selected_objects_filter(Outputs &outputs) -> EventData &
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

    /////////////////////////////////////////////////////////
    /// Not actually being used. Can stay, just in case ...
    ///
    // EventData &dummy()
    // {
    //     if (*this)
    //     {
    //         fmt::print("--> Muon_pt (inside matcher) : {}\n", muons.pt);
    //         fmt::print("--> Electron_pt (inside matcher) : {}\n", electrons.pt);
    //         fmt::print("--> trigger filter bits (inside matcher 1): {}\n", trgobjs.filterBits);
    //         return *this;
    //     }
    //     return *this;
    // }

    /////////////////////////////////////////////////////////////////////
    /// Will check if the current event has a matched object to a good TrgObj
    // here we have to break the single responsability rule ...
    // this filter also get the trigger scale factor,
    // otherwise we would have to look over the objects twice
    ///
    auto trigger_match_filter(Outputs &outputs, const std::map<std::string_view, TrgObjMatcher> &matchers)
        -> EventData &
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
                            matchers.at(hlt_path)(trgobjs, muons, good_low_pt_muons_mask);
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
                            matchers.at(hlt_path)(trgobjs, muons, good_high_pt_muons_mask);
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
                            matchers.at(hlt_path)(trgobjs, electrons, good_low_pt_electrons_mask);
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
                            matchers.at(hlt_path)(trgobjs, electrons, good_high_pt_electrons_mask);
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

    auto set_scale_factors_and_weights(Outputs &outputs) -> EventData &
    {
        if (*this)
        {
            // TODO: L1 prefiring
            // TODO: Muons
            // Low Pt Muons should consider:
            // --  tracking efficiency: ~1.
            // -- Reconstruction: NUM_TrackerMuons_DEN_genTracks
            // -- ID: NUM_TightID_DEN_TrackerMuons
            // -- Isolation:
            // -- Trigger: NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight
            // TODO: Electrons
            // TODO: Photons
            // TODO: Taus
            // TODO: Jets
            // TODO: BTag
            // TODO: MET
            //  trigger - this has already been procecssed during th trigger matching
            outputs.set_event_weight("Trigger", "Nominal", trigger_sf_nominal);
            outputs.set_event_weight("Trigger", "Up", trigger_sf_up);
            outputs.set_event_weight("Trigger", "Down", trigger_sf_down);
            return *this;
        }
        return *this;
    }

    auto muon_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    auto electron_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    auto photon_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    auto tau_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    auto bjet_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    auto jet_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }
    auto met_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    auto fill_event_content(Outputs &outputs) -> EventData &
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
};

#endif /*MUSIC_EVENT_DATA*/
