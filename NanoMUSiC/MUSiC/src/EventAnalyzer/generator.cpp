#include "EventAnalyzer.hpp"
#include <optional>

////////////////////////////////////////////////////////////////////////////
/// TODO: Filter events based on their Generator process. This is implemented in order to avoid overlap of
/// phase-space between MC samples. Should come after all constant weights are available.
auto EventAnalyzer::generator_filter(Outputs &outputs,
                                     const std::optional<std::string> &generator_filter_key,
                                     debugger_t &h_debug) -> EventAnalyzer &
{
    if (*this)
    {
        bool pass_gen_filter = true;
        // if MC
        if (!is_data)
        {
            if (generator_filter_key)
            {
                pass_gen_filter =
                    GeneratorFilters::filters.at(*generator_filter_key)(lhe_particles, gen_particles, year, h_debug);
            }
            else
            {
                pass_gen_filter = true;
            }
        }

        // first filter - it is needed in order to calculate the effective x_section_file
        // xSec_eff = xSec*pass(GeneratorFilter)/pass(NoCuts)
        outputs.fill_cutflow_histo("NoCuts", outputs.get_event_weight("Generator"));
        if (pass_gen_filter)
        {
            // fmt::print("\nDEBUG - generator_filter");
            // fmt::print("Event weight: {}\n", outputs.get_event_weight());
            // fmt::print("Event weights: {}\n", outputs.weights_nominal);
            outputs.fill_cutflow_histo("GeneratorFilter", outputs.get_event_weight("Generator"));
            outputs.fill_cutflow_histo("GeneratorWeight", outputs.get_event_weight());
            return *this;
        }
        set_null();
        // fmt::print("\nDEBUG - DID NOT PASS generator FILTER");
        return *this;
    }
    return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Set PDF and Alpha_S uncertainties.
/// Those are tricky beasts, since they are not simple weights added to the event, but rather, should be treated as
/// variations and have their uncert. squared-summed in the end of the processing (classification).
/// This method also saves the LHA ID that was used during generation or rescaling.
auto EventAnalyzer::set_pdf_alpha_s_weights(const std::optional<std::pair<unsigned int, unsigned int>> &lha_indexes,
                                            const std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>,
                                                             std::unique_ptr<LHAPDF::PDF>,
                                                             std::unique_ptr<LHAPDF::PDF>> &default_pdf_sets)
    -> EventAnalyzer &
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
                throw std::runtime_error(
                    fmt::format("ERROR: There are PDF weights written in the file, but the REGEX parser failed to get "
                                "a proper LHA ID."));
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
                alpha_s_down = alpha_s_down_pdf->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
                               alpha_s_down_pdf->xfxQ(generator_info.id2, generator_info.x2, generator_info.scalePDF) /
                               lhe_info.originalXWGTUP;
            }
            else
            {
                throw std::runtime_error(fmt::format(
                    "ERROR: Unexpected number of PDF weights ({}). According to CMSSW "
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
                alpha_s_down = alpha_s_down_pdf->xfxQ(generator_info.id1, generator_info.x1, generator_info.scalePDF) *
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
auto EventAnalyzer::set_scale_weights() -> EventAnalyzer &
{
    if (*this)
    {
        if (lhe_info.nLHEScaleWeight > 0)
        {

            if (not(lhe_info.nLHEScaleWeight == 9 or lhe_info.nLHEScaleWeight == 8))
            {
                throw std::runtime_error(fmt::format(
                    "ERROR: Unexpected number of Scale weights ({}). Expected to be 8 or 9. \nWeights: {}\n",
                    lhe_info.nLHEScaleWeight,
                    lhe_info.LHEScaleWeight));
            }

            // fmt::print("{}\n", lhe_info.LHEScaleWeight);

            auto murf_nominal = lhe_info.LHEScaleWeight[4];
            // REFERENCE on how to treat nLHEScaleWeight == 8:
            // https://github.com/rappoccio/QJetMassUproot/blob/3b12e0d16adf4fe2c8a50aac55d6a8a2a360d4d7/cms_utils.py
            if (lhe_info.nLHEScaleWeight == 8)
            {
                murf_nominal = 1.;
            }

            // remove indexes 2 and 6 or 5 (n ==8) since they corresponds to unphysical values
            if (lhe_info.nLHEScaleWeight == 9)
            {
                lhe_info.LHEScaleWeight.erase(lhe_info.LHEScaleWeight.begin() + 2);
                lhe_info.LHEScaleWeight.erase(lhe_info.LHEScaleWeight.begin() + 4);
                lhe_info.LHEScaleWeight.erase(lhe_info.LHEScaleWeight.begin() + 6);
            }
            if (lhe_info.nLHEScaleWeight == 8)
            {
                lhe_info.LHEScaleWeight.erase(lhe_info.LHEScaleWeight.begin() + 2);
                lhe_info.LHEScaleWeight.erase(lhe_info.LHEScaleWeight.begin() + 5);
            }
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