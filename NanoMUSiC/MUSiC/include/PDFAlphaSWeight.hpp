#include "Shifts.hpp"

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// Set PDF and Alpha_S uncertainties.
/// Those are tricky beasts, since they are not simple weights added to the
/// event, but rather, should be treated as variations and have their uncert.
/// squared-summed in the end of the processing (classification). This method
/// also saves the LHA ID that was used during generation or rescaling. Ref:
/// https://arxiv.org/pdf/1510.03865.pdf
inline auto get_pdf_alpha_s_weights(const Shifts::Variations &shift,                                         //
                                    const std::optional<std::pair<unsigned int, unsigned int>> &lha_indexes, //
                                    const std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>,              //
                                                     std::unique_ptr<LHAPDF::PDF>,                           //
                                                     std::unique_ptr<LHAPDF::PDF>> &default_pdf_sets,        //
                                    const RVec<float> &_LHEPdfWeight,                                        //
                                    float Generator_scalePDF,                                                //
                                    float Generator_x1,                                                      //
                                    float Generator_x2,                                                      //
                                    int Generator_id1,                                                       //
                                    int Generator_id2,                                                       //
                                    const std::optional<std::unique_ptr<LHAPDF::PDF>> &this_sample_pdf
                                    // float LHEWeight_originalXWGTUP
                                    ) -> double
{
    if (shift == Shifts::Variations::PDF_As_Up or shift == Shifts::Variations::PDF_As_Down)
    {
        // references are dangerous!!!!!
        // be carefull with lifetime
        auto &default_pdf = std::get<0>(default_pdf_sets);
        auto &alpha_s_up_pdf = std::get<1>(default_pdf_sets);
        auto &alpha_s_down_pdf = std::get<2>(default_pdf_sets);

        auto LHEPdfWeight = RVec<float>();

        // some events have weird negative PDF weights
        bool has_negatives = false;
        for (auto &&weight : _LHEPdfWeight)
        {
            LHEPdfWeight.push_back(std::max(weight, 0.f));
            if (weight < 0.)
            {
                has_negatives = true;
            }
        }

        // should fall back to manual PDF+As calculation, for negative weights
        if (LHEPdfWeight.size() > 0 and not(has_negatives))
        {
            double alpha_s_up = 1.;
            double alpha_s_down = 1.;

            // set LHA ID
            auto [lha_id_first, lha_id_last] = lha_indexes.value_or(std::pair<unsigned int, unsigned int>());
            if (lha_id_first == 0)
            {
                fmt::print(stderr,
                           "ERROR: There are PDF weights written in the "
                           "file, but the REGEX parser failed to get "
                           "a proper LHA ID.");
                std::exit(EXIT_FAILURE);
            }
            if (not(this_sample_pdf))
            {
                fmt::print(stderr,
                           "ERROR: Could not find valid PDF Set, even though the the indexes ({} and {}) could be "
                           "read from the branch title.",
                           lha_id_first,
                           lha_id_last);
                std::exit(EXIT_FAILURE);
            }

            // The nominal LHEPdfWeight (first element) is expected to be 1
            // but also found values  All variations will be normalized to the
            // nominal LHEPdfWeight.
            auto LHEWeight_originalXWGTUP = LHEPdfWeight[0];
            LHEPdfWeight = LHEPdfWeight / LHEWeight_originalXWGTUP;

            // has alpha_s weights
            if (LHEPdfWeight.size() == 103 or LHEPdfWeight.size() == 33)
            // if (LHEPdfWeight.size() == 103)
            {
                alpha_s_up = LHEPdfWeight[101];
                alpha_s_down = LHEPdfWeight[102];

                // remove the first weight (always 1.)
                // remove last two elements (Alpha_S weights)
                LHEPdfWeight.erase(LHEPdfWeight.begin());
                LHEPdfWeight.erase(LHEPdfWeight.end() - 1);
                LHEPdfWeight.erase(LHEPdfWeight.end() - 1);
            }

            // don't have alpha_s weights, should get the one from the 5f
            // LHAPDF set. REF:
            // https://cms-pdmv.gitbook.io/project/mccontact/info-for-mc-production-for-ultra-legacy-campaigns-2016-2017-2018#recommendations-on-the-usage-of-pdfs-and-cpx-tunes
            // else if (LHEPdfWeight.size() == 101)
            else if (LHEPdfWeight.size() == 101 or LHEPdfWeight.size() == 31)
            {
                // Those are some possible convertion from for NNPDF31,
                // without to with alpha_s During the classification, the code
                // should check the status of alpha_s_up and alpha_s_down and
                // react accordinly. 304400 --> 306000 316200 --> 325300
                // 325500 --> 325300
                // 320900 --> 306000

                // remove the first weight (always 1.)
                LHEPdfWeight.erase(LHEPdfWeight.begin());

                auto alternative_LHEWeight_originalXWGTUP =
                    default_pdf[0]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                    default_pdf[0]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF);

                // Compute the Alpha_S weight for this event using
                // NNPDF31_nnlo_as_0120 (319500) and divide the new weight by
                // the weight from the PDF the event was produced with.
                alpha_s_up = alpha_s_up_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                             alpha_s_up_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                             alternative_LHEWeight_originalXWGTUP;

                // Compute the Alpha_S weight for this event using
                // NNPDF31_nnlo_as_0116 (319300) and divide the new weight by
                // the weight from the PDF the event was produced with.
                alpha_s_down = alpha_s_down_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                               alpha_s_down_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                               alternative_LHEWeight_originalXWGTUP;
            }
            else
            {
                fmt::print(stderr,
                           "ERROR: Unexpected number of PDF weights ({}). According "
                           "to CMSSW "
                           "(https://github.dev/cms-sw/cmssw/blob/"
                           "6ef534126e6db3dfdea86c3f0eedb773f0117cbc/PhysicsTools/"
                           "NanoAOD/python/genWeightsTable_cfi.py#L20) if should be "
                           "eighther 101 or 103.\n",
                           LHEPdfWeight.size());
                std::exit(EXIT_FAILURE);
            }

            // calculate shifts
            auto alpha_s_shift = (alpha_s_up - alpha_s_down) / 2.f;
            auto pdf_shift = std::numeric_limits<double>::max();

            if (contains((*this_sample_pdf)->set().errorType(), "hessian"))
            {
                auto sum_shifts_squared = 0.;
                for (std::size_t i = 1; i < LHEPdfWeight.size(); i++)
                {
                    if (not(std::isnan(LHEPdfWeight[i])) and not(std::isinf(LHEPdfWeight[i])))
                    {
                        // sum_shifts_squared += std::pow(LHEPdfWeight[i] - LHEWeight_originalXWGTUP, 2.);
                        sum_shifts_squared += std::pow(LHEPdfWeight[i] - 1., 2.);
                    }
                }
                // pdf_shift = std::sqrt(sum_shifts_squared) / LHEWeight_originalXWGTUP;
                pdf_shift = std::sqrt(sum_shifts_squared);
            }
            else if (contains((*this_sample_pdf)->set().errorType(), "replica"))
            {
                if (LHEPdfWeight.size() == 100)
                {
                    auto sorted_weights = ROOT::VecOps::Sort(LHEPdfWeight);
                    pdf_shift = (sorted_weights[83] - sorted_weights[15]) / 2.f;
                }
                else if (LHEPdfWeight.size() == 30)
                {
                    pdf_shift = ROOT::VecOps::StdDev(LHEPdfWeight);
                }
            }
            else
            {
                fmt::print(stderr,
                           "ERROR: Could not get PDF error type. "
                           "Unexpected error type ({}).\n",
                           default_pdf.at(0)->set().errorType());
                std::exit(EXIT_FAILURE);
            }

            if (std::isnan(pdf_shift) or std::isnan(alpha_s_shift) or std::isinf(pdf_shift) or
                std::isinf(alpha_s_shift))
            {
                // fmt::print("NaN found!!!!! (PDF)\n");

                pdf_shift = 0.;
                alpha_s_shift = 0.;
            }

            // fmt::print("\n[{}]\n", fmt::join(LHEPdfWeight, ", "));
            // fmt::print("-- PDF Weight (From NanoAOD): {} - {} => {} \n",
            //            pdf_shift,
            //            alpha_s_shift,
            //            1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.)));

            if (shift == Shifts::Variations::PDF_As_Up)
            {
                return 1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
            }

            if (shift == Shifts::Variations::PDF_As_Down)
            {
                return 1. - std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
            }
        }

        // If LHE PDF Weights are not available ....
        // Assuming: NNPDF31_nnlo_as_0118_hessian - 304400 (Hessian)
        float alpha_s_up = 1.;
        float alpha_s_down = 1.;

        // Compute the PDF weight for this event using
        // NNPDF31_nnlo_as_0118_hessian (304400) and divide the new weight by
        // the weight from the PDF the event was produced with.
        auto LHEWeight_originalXWGTUP = default_pdf[0]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                        default_pdf[0]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF);

        // skip the first, since it corresponds to the originalXWGTUP
        // (nominal)
        auto sum_shifts_squared = 0.;
        for (std::size_t i = 1; i < default_pdf.size(); i++)
        {
            //     LHEPdfWeight.push_back(default_pdf[i]->xfxQ(Generator_id1,
            //     Generator_x1, Generator_scalePDF) *
            //                            default_pdf[i]->xfxQ(Generator_id2,
            //                            Generator_x2, Generator_scalePDF) /
            //                            LHEWeight_originalXWGTUP);
            sum_shifts_squared += std::pow((default_pdf[i]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                            default_pdf[i]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF)) -
                                               LHEWeight_originalXWGTUP,
                                           2.);
        }

        // Compute the Alpha_S weight for this event using
        // NNPDF31_nnlo_as_0120 (319500) and divide the new weight by the
        // weight from the PDF the event was produced with.
        alpha_s_up = alpha_s_up_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                     alpha_s_up_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) / LHEWeight_originalXWGTUP;

        // Compute the Alpha_S weight for this event using
        // NNPDF31_nnlo_as_0116 (319300) and divide the new weight by the
        // weight from the PDF the event was produced with.
        alpha_s_down = alpha_s_down_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                       alpha_s_down_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                       LHEWeight_originalXWGTUP;

        // auto sorted_weights = ROOT::VecOps::Sort(LHEPdfWeight);
        // auto pdf_shift = (sorted_weights[83] - sorted_weights[15]) / 2.f;
        auto pdf_shift = std::sqrt(sum_shifts_squared) / LHEWeight_originalXWGTUP;
        auto alpha_s_shift = (alpha_s_up - alpha_s_down) / 2.f;

        if (std::isnan(pdf_shift) or std::isnan(alpha_s_shift) or std::isinf(pdf_shift) or std::isinf(alpha_s_shift))
        {
            // fmt::print("[{}]\n", fmt::join(LHEPdfWeight, ", "));
            // fmt::print("-- PDF Weight (From NanoAOD): {} - {} => {} \n",
            //            pdf_shift,
            //            alpha_s_shift,
            //            1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.)));

            pdf_shift = 0.;
            alpha_s_shift = 0.;
        }
        // fmt::print("\n-- PDF Weight (From NanoAOD): {} - {} - {} => {} \n",
        //            has_negatives,
        //            pdf_shift,
        //            alpha_s_shift,
        //            1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.)));
        // fmt::print("[{}]\n", fmt::join(LHEPdfWeight, ", "));

        if (shift == Shifts::Variations::PDF_As_Up)
        {
            return 1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
        }

        if (shift == Shifts::Variations::PDF_As_Down)
        {
            return 1. - std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
        }
    }

    return 1.;
}
