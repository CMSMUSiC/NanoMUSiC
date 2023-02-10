#ifndef MUSIC_EVENT_DATA
#define MUSIC_EVENT_DATA

#include "LHAPDF/PDF.h"
#include "ROOT/RVec.hxx"
#include <algorithm>
#include <complex>
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
#include "GeneratorFilters.hpp"
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

    NanoObjects::GenParticles gen_particles;

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

    EventData(const bool &_is_data, const Year &_year, Outputs &outputs)
        : is_null(false),
          is_data(_is_data),
          year(_year)
    {
        outputs.fill_cutflow_histo("NoCuts", 1.);
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

    auto set_gen_particles(NanoObjects::GenParticles &&_gen_particles) -> EventData &
    {
        if (*this)
        {
            gen_particles = _gen_particles;
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
    /// TODO: Filter events based on their Generator process. This is implemented in order to avoid overlap of
    /// phase-space between MC samples. Should come after all constant weights are available.
    auto generator_filter(Outputs &outputs, const std::string &process) -> EventData &
    {
        if (*this)
        {
            bool is_good_gen = true;
            // if MC
            if (!is_data)
            {
                /////////////////////////////////////////////////
                // TODO: check if it is good gen event
                /////////////////////////////////////////////////
                if (true)
                {
                    // fmt::print("{}\n", generator_info.binvar);
                    // fmt::print("{}\n", lhe_info.);
                    if (GeneratorFilters::filters.count(process) > 0)
                    {
                        GeneratorFilters::Inputs_t foo = {
                            1, "foo", 4.20f, true, RVec<float>({1.2, 3.4}), RVec<int>({1, 3})};
                        is_good_gen = GeneratorFilters::filters.at(process)(foo);
                    }
                }
            }
            if (is_good_gen)
            {
                // // fmt::print("\nDEBUG - generator_filter");
                outputs.fill_cutflow_histo("GeneratorFilter", 1.);
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
        return (muons.pt >= ObjConfig::Muons[year].MinLowPt)                   //
               && (muons.pt < ObjConfig::Muons[year].MaxLowPt)                 //
               && (VecOps::abs(muons.eta) <= ObjConfig::Muons[year].MaxAbsEta) //
               && (muons.tightId)                                              //
               && (muons.pfRelIso04_all < ObjConfig::Muons[year].PFRelIso_WP);
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
               && (electrons.cutBased >= ObjConfig::Electrons[year].cutBasedId);
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
               && (not photons.isScEtaEE)                     // only EB photons
               && (photons.cutBased >= ObjConfig::Photons[year].cutBasedId) //
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
    auto set_l1_pre_firing_SFs(Outputs &outputs) -> EventData &
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

    ///////////////////////////////////////////////////////////////
    /// Muons
    /// Ref (2018): https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2018_UL_muon_Z.html
    /// Low Pt Muons should consider:
    /// -- Tracking efficiency: ~1.0
    /// -- Reconstruction: NUM_TrackerMuons_DEN_genTracks
    /// -- ID: NUM_TightID_DEN_TrackerMuons
    /// -- Isolation: NUM_TightRelIso_DEN_TightIDandIPCut
    /// -- Trigger: NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight (already implemented during trigger matching)
    /// High Pt Muons should consider:
    /// -- Tracking efficiency: ~1.0
    /// -- Reconstruction: NUM_TrackerMuons_DEN_genTracks
    /// -- ID: NUM_HighPtID_DEN_TrackerMuons
    /// -- Isolation: NUM_TightRelTkIso_DEN_HighPtIDandIPCut
    /// -- Trigger: NUM_Mu50_or_OldMu100_or_TkMu100_DEN_CutBasedIdGlobalHighPt_and_TkIsoLoose (already
    /// implemented during trigger matching)
    auto set_muon_SFs(Outputs &outputs,                    //
                      const Corrector &muon_sf_reco,       //
                      const Corrector &muon_sf_id_low_pt,  //
                      const Corrector &muon_sf_id_high_pt, //
                      const Corrector &muon_sf_iso_low_pt, //
                      const Corrector &muon_sf_iso_high_pt) -> EventData &
    {
        if (*this)
        {

            RVec<float> good_muons_pt = muons.pt[good_muons_mask];
            RVec<float> good_muons_pt_low_pt = muons.pt[good_low_pt_muons_mask];
            RVec<float> good_muons_pt_high_pt = muons.pt[good_high_pt_muons_mask];
            RVec<float> good_muons_eta = VecOps::abs(muons.eta[good_muons_mask]);
            RVec<float> good_muons_eta_low_pt = VecOps::abs(muons.eta[good_low_pt_muons_mask]);
            RVec<float> good_muons_eta_high_pt = VecOps::abs(muons.eta[good_high_pt_muons_mask]);

            // Muon Reco
            outputs.set_event_weight("MuonReco", "Nominal", muon_sf_reco(year, good_muons_pt, good_muons_eta, "sf"));
            outputs.set_event_weight("MuonReco", "Up", muon_sf_reco(year, good_muons_pt, good_muons_eta, "systup"));
            outputs.set_event_weight("MuonReco", "Down", muon_sf_reco(year, good_muons_pt, good_muons_eta, "systdown"));

            // Muon Id
            outputs.set_event_weight("MuonId", "Nominal",
                                     muon_sf_id_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "sf") *
                                         muon_sf_id_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "sf"));
            outputs.set_event_weight(
                "MuonId", "Up",
                muon_sf_id_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systup") *
                    muon_sf_id_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systup"));
            outputs.set_event_weight(
                "MuonId", "Down",
                muon_sf_id_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systdown") *
                    muon_sf_id_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systdown"));

            // Muon Iso
            outputs.set_event_weight(
                "MuonIso", "Nominal",
                muon_sf_iso_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "sf") *
                    muon_sf_iso_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "sf"));
            outputs.set_event_weight(
                "MuonIso", "Up",
                muon_sf_iso_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systup") *
                    muon_sf_iso_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systup"));
            outputs.set_event_weight(
                "MuonIso", "Down",
                muon_sf_iso_low_pt(year, good_muons_pt_low_pt, good_muons_eta_low_pt, "systdown") *
                    muon_sf_iso_high_pt(year, good_muons_pt_high_pt, good_muons_eta_high_pt, "systdown"));

            return *this;
        }
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    /// Electron  SFs, in the correctionlib JSONs, are implemented in a single key: UL-Electron-ID-SF
    /// inputs: year (string), variation (string), WorkingPoint (string), eta_SC (real), pt (real)
    /// Examples:
    /// - year: 2016preVFP, 2016postVFP, 2017, 2018
    /// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
    /// - WorkingPoint: Loose, Medium, RecoAbove20, RecoBelow20, Tight, Veto, wp80iso, wp80noiso, wp90iso, wp90noiso
    /// - eta: [-inf, inf)
    /// - pt [10., inf)
    ///
    /// Low pT
    /// RECO: RecoAbove20
    /// ID: Tight
    /// ISO: No recomendations (already incorporated).
    ///
    /// TODO: High Pt - Doesn't use the Correctionlib
    /// RECO: Same as Low Pt.
    /// ID: Example:
    /// https://github.com/CMSLQ/rootNtupleAnalyzerV2/blob/2dd8f9415e7a9c3465c7e28916eb68866ff337ff/src/ElectronScaleFactors.C
    /// 2016 prompt: 0.971±0.001 (stat) (EB), 0.983±0.001 (stat) (EE)
    ///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
    ///              uncertainty (syst?): EE ET < 90 GeV: 1% else min(1+(ET-90)*0.0143)%,4%)
    ///
    /// 2016 legacy: 0.983±0.000 (stat) (EB), 0.991±0.001 (stat) (EE) (taken from slide 10 of [0])
    ///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
    ///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
    ///
    /// 2017 prompt: 0.968±0.001 (stat) (EB), 0.973±0.002 (stat) (EE)
    ///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
    ///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
    ///
    /// 2018 rereco (Autumn 18): 0.969 +/- 0.000 (stat) (EB), and 0.984 +/- 0.001 (stat) (EE).
    ///                          uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
    ///                          uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)

    /// For more details see here https://twiki.cern.ch/twiki/bin/view/CMS/HEEPElectronIdentificationRun2#Scale_Factor.
    /// As always, HEEP ID SF are just two numbers, one for EB and one for EE.
    ///
    /// [0] -
    /// https://indico.cern.ch/event/831669/contributions/3485543/attachments/1871797/3084930/ApprovalSlides_EE_v3.pdf

    auto set_electron_SFs(Outputs &outputs, const ElectronSFCorrector &electron_sf) -> EventData &
    {
        if (*this)
        {
            RVec<float> good_electrons_pt = electrons.pt[good_electrons_mask];
            RVec<float> good_electrons_pt_low_pt = electrons.pt[good_low_pt_electrons_mask];
            RVec<float> good_electrons_pt_high_pt = electrons.pt[good_high_pt_electrons_mask];

            RVec<float> good_electrons_eta_sc = (electrons.eta + electrons.deltaEtaSC)[good_electrons_mask];
            RVec<float> good_electrons_eta_sc_low_pt =
                (electrons.eta + electrons.deltaEtaSC)[good_low_pt_electrons_mask];
            RVec<float> good_electrons_eta_sc_high_pt =
                (electrons.eta + electrons.deltaEtaSC)[good_high_pt_electrons_mask];

            // Electron Reco
            outputs.set_event_weight("ElectronReco", "Nominal",
                                     electron_sf("sf", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));
            outputs.set_event_weight("ElectronReco", "Up",
                                     electron_sf("sfup", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));
            outputs.set_event_weight("ElectronReco", "Down",
                                     electron_sf("sfup", "RecoAbove20", good_electrons_pt, good_electrons_eta_sc));

            // Electron ID
            outputs.set_event_weight(
                "ElectronId", "Nominal",
                electron_sf("sf", "Tight", good_electrons_pt_low_pt, good_electrons_eta_sc_low_pt) *
                    electron_sf("sf", "HEEPId", good_electrons_pt_high_pt, good_electrons_eta_sc_high_pt));
            outputs.set_event_weight(
                "ElectronId", "Up",
                electron_sf("sfup", "Tight", good_electrons_pt_low_pt, good_electrons_eta_sc_low_pt) *
                    electron_sf("sfup", "HEEPId", good_electrons_pt_high_pt, good_electrons_eta_sc_high_pt));
            outputs.set_event_weight(
                "ElectronId", "Down",
                electron_sf("sfdown", "Tight", good_electrons_pt_low_pt, good_electrons_eta_sc_low_pt) *
                    electron_sf("sfdown", "HEEPId", good_electrons_pt_high_pt, good_electrons_eta_sc_high_pt));

            // return modified event_data
            return *this;
        }
        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    /// Photons ID SFs, in the correctionlib JSONs, are implemented in: UL-Photon-ID-SF
    /// inputs: year (string), variation (string), WorkingPoint (string), eta_SC (real), pt (real)
    /// - year: 2016preVFP, 2016postVFP, 2017, 2018
    /// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
    /// - WorkingPoint: Loose, Medium, Tight, wp80, wp90
    /// - eta: [-inf, inf)
    /// - pt [20., inf)
    ///
    /// Low pT
    /// RECO: From Twiki [0]: "The scale factor to reconstruct a supercluster with H/E<0.5 is assumed to be 100%."
    /// ID: Tight
    /// ISO: No recomendations (already incorporated).
    ///
    /// [0] - https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations#E_gamma_RECO
    ///
    /// Photons PixelSeed SFs, in the correctionlib JSONs, are implemented in:  UL-Photon-PixVeto-SF
    /// These are the Photon Pixel Veto Scale Factors (nominal, up or down) for 2018 Ultra Legacy dataset.
    /// - year: 2016preVFP, 2016postVFP, 2017, 2018
    /// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
    /// - WorkingPoint (SFs available for the cut-based and MVA IDs): Loose, MVA, Medium, Tight
    /// - HasPixBin: For each working point of choice, they are dependent on the photon pseudorapidity and R9: Possible
    /// bin choices: ['EBInc','EBHighR9','EBLowR9','EEInc','EEHighR9','EELowR9']
    auto set_photon_SFs(Outputs &outputs, const PhotonSFCorrector &photon_id_sf,
                        const PhotonSFCorrector &photon_pixel_seed_sf) -> EventData &
    {
        if (*this)
        {
            RVec<float> good_photons_pt = photons.pt[good_photons_mask];
            RVec<float> good_photons_eta = photons.eta[good_photons_mask];

            // PhotonId
            outputs.set_event_weight("PhotonId", "Nominal", photon_id_sf("sf", good_photons_pt, good_photons_eta));
            outputs.set_event_weight("PhotonId", "Up", photon_id_sf("sfup", good_photons_pt, good_photons_eta));
            outputs.set_event_weight("PhotonId", "Down", photon_id_sf("sfup", good_photons_pt, good_photons_eta));

            // Photon PixelSeed
            // here the pt RVec is needed just to get the number of photons.
            // the PixelSeed SF is independent of pt and eta
            outputs.set_event_weight("PhotonPixelSeed", "Nominal", photon_pixel_seed_sf("sf", good_photons_pt));
            outputs.set_event_weight("PhotonPixelSeed", "Up", photon_pixel_seed_sf("sfup", good_photons_pt));
            outputs.set_event_weight("PhotonPixelSeed", "Down", photon_pixel_seed_sf("sfup", good_photons_pt));

            return *this;
        }
        return *this;
    }

    /// TODO: Taus
    auto set_tau_SFs(Outputs &outputs) -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// Jets
    /// No SFs are assigned to Jets. They have been measured to be close to 1.
    auto set_jet_SFs(Outputs &outputs) -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

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
    auto set_bjet_SFs(Outputs &outputs, const BTagSFCorrector &btag_sf) -> EventData &
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
            outputs.set_event_weight("BJetCorrelated", "Nominal",
                                     btag_sf("central", "central", good_bjets_pt, //
                                             good_bjets_abseta,                   //
                                             good_bjets_hadronFlavour,            //
                                             good_jets_pt,                        //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("BJetCorrelated", "Up",
                                     btag_sf("up_correlated", "central", good_bjets_pt, //
                                             good_bjets_abseta,                         //
                                             good_bjets_hadronFlavour,                  //
                                             good_jets_pt,                              //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("BJetCorrelated", "Down",
                                     btag_sf("down_correlated", "central", good_bjets_pt, //
                                             good_bjets_abseta,                           //
                                             good_bjets_hadronFlavour,                    //
                                             good_jets_pt,                                //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));

            // LightJetCorrelated
            outputs.set_event_weight("LightJetCorrelated", "Nominal",
                                     btag_sf("central", "central", good_bjets_pt, //
                                             good_bjets_abseta,                   //
                                             good_bjets_hadronFlavour,            //
                                             good_jets_pt,                        //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("LightJetCorrelated", "Up",
                                     btag_sf("central", "up_correlated", good_bjets_pt, //
                                             good_bjets_abseta,                         //
                                             good_bjets_hadronFlavour,                  //
                                             good_jets_pt,                              //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("LightJetCorrelated", "Down",
                                     btag_sf("central", "down_correlated", good_bjets_pt, //
                                             good_bjets_abseta,                           //
                                             good_bjets_hadronFlavour,                    //
                                             good_jets_pt,                                //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));

            // BJetUncorrelated
            outputs.set_event_weight("BJetUncorrelated", "Nominal",
                                     btag_sf("central", "central", good_bjets_pt, //
                                             good_bjets_abseta,                   //
                                             good_bjets_hadronFlavour,            //
                                             good_jets_pt,                        //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("BJetUncorrelated", "Up",
                                     btag_sf("up_uncorrelated", "central", good_bjets_pt, //
                                             good_bjets_abseta,                           //
                                             good_bjets_hadronFlavour,                    //
                                             good_jets_pt,                                //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("BJetUncorrelated", "Down",
                                     btag_sf("down_uncorrelated", "central", good_bjets_pt, //
                                             good_bjets_abseta,                             //
                                             good_bjets_hadronFlavour,                      //
                                             good_jets_pt,                                  //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));

            // LightJetUncorrelated
            outputs.set_event_weight("LightJetUncorrelated", "Nominal",
                                     btag_sf("central", "central", good_bjets_pt, //
                                             good_bjets_abseta,                   //
                                             good_bjets_hadronFlavour,            //
                                             good_jets_pt,                        //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("LightJetUncorrelated", "Up",
                                     btag_sf("central", "up_uncorrelated", good_bjets_pt, //
                                             good_bjets_abseta,                           //
                                             good_bjets_hadronFlavour,                    //
                                             good_jets_pt,                                //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));
            outputs.set_event_weight("LightJetUncorrelated", "Down",
                                     btag_sf("central", "down_uncorrelated", good_bjets_pt, //
                                             good_bjets_abseta,                             //
                                             good_bjets_hadronFlavour,                      //
                                             good_jets_pt,                                  //
                                             good_jets_abseta, //                                       //
                                             good_jets_hadronFlavour));

            return *this;
        }
        return *this;
    }

    /// MET
    /// MET is present in, virtually, all events. It is not possible to assign SFs.
    auto set_met_SFs(Outputs &outputs) -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// Trigger
    /// These weights have already been calculated during the trigger matching.
    auto set_trigger_SFs(Outputs &outputs) -> EventData &
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

    /// TODO:
    auto muon_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// TODO:
    auto electron_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// TODO:
    auto photon_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// TODO:
    auto tau_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// TODO:
    auto bjet_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// TODO:
    auto jet_corrections() -> EventData &
    {
        if (*this)
        {
            return *this;
        }
        return *this;
    }

    /// TODO:
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
