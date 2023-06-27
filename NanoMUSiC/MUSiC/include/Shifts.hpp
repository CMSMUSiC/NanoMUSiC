#ifndef SHIFTS
#define SHIFTS
#include <string>
#include <vector>

#include <fmt/format.h>

#include "ObjectFactories/music_objects.hpp"
#include "PDFAlphaSWeights.hpp"

//////////////////////////////////////////////////////////
/// All systematics implemented in one of Yannik's bg.root
/// "alphasDown", "alphasUp",
/// "charge",
/// "Ele_systScaleDown", "Ele_systScaleUp",
/// "fakeDown", "fakeUp",
/// "Gamma_systScaleDown", "Gamma_systScaleUp",
/// "Jet_systBJetsSF2ADown", "Jet_systBJetsSF2AUp",
/// "Jet_systResolutionDown", "Jet_systResolutionUp",
/// "Jet_systScaleDown", "Jet_systScaleUp",
/// "luminosity",
/// "Muon_systResolutionDown", "Muon_systResolutionUp", //
/// "Muon_systScaleDown", "Muon_systScaleUp",
/// "pdfuponly",
/// "pileupDown", "pileupUp",
/// "prefireDown", "prefireUp",
/// "qcdWeightDrellYan_Down", "qcdWeightDrellYan_Up",
/// "qcdWeightGamma_Down", "qcdWeightGamma_Up",
/// "qcdWeightGG_Down", "qcdWeightGG_Up",
/// "qcdWeightHIG_Down", "qcdWeightHIG_Up",
/// "qcdWeightQCD_Down", "qcdWeightQCD_Up",
/// "qcdWeighttG_Down", "qcdWeighttG_Up",
/// "qcdWeightTop_Down", "qcdWeightTop_Up",
/// "qcdWeightTTbar_Down", "qcdWeightTTbar_Up",
/// "qcdWeightTTbarTTbar_Down", "qcdWeightTTbarTTbar_Up",
/// "qcdWeightTTG_Down", "qcdWeightTTG_Up",
/// "qcdWeightTTGG_Down", "qcdWeightTTGG_Up",
/// "qcdWeightTTW_Down", "qcdWeightTTW_Up",
/// "qcdWeightTTZ_Down", "qcdWeightTTZ_Up",
/// "qcdWeighttZQ_Down", "qcdWeighttZQ_Up",
/// "qcdWeightW_Down", "qcdWeightW_Up",
/// "qcdWeightWG_Down", "qcdWeightWG_Up",
/// "qcdWeightWGG_Down", "qcdWeightWGG_Up",
/// "qcdWeightWW_Down", "qcdWeightWW_Up",
/// "qcdWeightWWG_Down", "qcdWeightWWG_Up",
/// "qcdWeightWWW_Down", "qcdWeightWWW_Up",
/// "qcdWeightWWZ_Down", "qcdWeightWWZ_Up",
/// "qcdWeightWZ_Down", "qcdWeightWZ_Up",
/// "qcdWeightWZG_Down", "qcdWeightWZG_Up",
/// "qcdWeightWZZ_Down", "qcdWeightWZZ_Up",
/// "qcdWeightZToInvisible_Down", "qcdWeightZToInvisible_Up",
/// "qcdWeightZZ_Down", "qcdWeightZZ_Up",
/// "qcdWeightZZZ_Down", "qcdWeightZZZ_Up",
/// "scale_factor_syst",
/// "slimmedMETs_systScaleDown", "slimmedMETs_systScaleUp",
/// "xsDrellYanNLO",
/// "xsGammaLO",
/// "xsGGLO",
/// "xsGGNLO",
/// "xsHIGN3LO",
/// "xsHIGNLO",
/// "xsHIGNNLO",
/// "xsQCDLO",
/// "xstGNLO",
/// "xsTopNLO",
/// "xsTTbarNNLO",
/// "xsTTbarTTbarNLO",
/// "xsTTGGNLO",
/// "xsTTGNLO",
/// "xsTTWNLO",
/// "xsTTZNLO",
/// "xstZQNLO"
/// "xsWGGNLO",
/// "xsWGLO",
/// "xsWGNLO",
/// "xsWLO",
/// "xsWNNLO",
/// "xsWWGNLO",
/// "xsWWNLO",
/// "xsWWNNLO",
/// "xsWWWNLO",
/// "xsWWZNLO",
/// "xsWZGNLO",
/// "xsWZNLO",
/// "xsWZZNLO",
/// "xsZToInvisibleNLO",
/// "xsZZNLO",
/// "xsZZZNLO",

class Shifts
{
  private:
    const std::vector<std::string> m_shifts;

  public:
    Shifts(bool is_data)
        : m_shifts(is_data ? std::vector<std::string>{"Nominal"}

                           : std::vector<std::string>{"Nominal",                 //
                                                      "PU_Up",                   //
                                                      "PU_Down",                 //
                                                      "Luminosity_Up",           //
                                                      "Luminosity_Down",         //
                                                      "xSecOrder_Up",            //
                                                      "xSecOrder_Down",          //
                                                      "Fake_Up",                 //
                                                      "Fake_Down",               //
                                                      "PDF_As_Up",               //
                                                      "PDF_As_Down",             //
                                                      "ScaleFactor_Up",          //
                                                      "ScaleFactor_Down",        //
                                                      "PreFiring_Up",            //
                                                      "PreFiring_Down",          //
                                                      "MuonResolution_Up",       //
                                                      "MuonResolution_Down",     //
                                                      "MuonScale_Up",            //
                                                      "MuonScale_Down",          //
                                                      "ElectronResolution_Up",   //
                                                      "ElectronResolution_Down", //
                                                      "ElectronScale_Up",        //
                                                      "ElectronScale_Down",      //
                                                      "PhotonResolution_Up",     //
                                                      "PhotonResolution_Down",   //
                                                      "PhotonScale_Up",          //
                                                      "PhotonScale_Down",        //
                                                      "JetResolution_Up",        //
                                                      "JetResolution_Down",      //
                                                      "JetScale_Up",             //
                                                      "JetScale_Down"})
    {
    }

    // Iterator types
    using const_iterator = std::vector<std::string>::const_iterator;

    // Member functions for iterators
    const_iterator begin() const
    {
        return m_shifts.cbegin();
    }

    const_iterator end() const
    {
        return m_shifts.cend();
    }

    const_iterator cbegin() const
    {
        return m_shifts.cbegin();
    }

    const_iterator cend() const
    {
        return m_shifts.cend();
    }

    auto get_shifts() const -> std::vector<std::string>
    {
        return m_shifts;
    }

    auto size() const -> std::size_t
    {
        return m_shifts.size();
    }

    auto is_valid(const std::string &shift) const -> bool
    {
        auto it = std::find(m_shifts.cbegin(), m_shifts.cend(), shift);
        if (it != m_shifts.end())
        {
            return true;
        }
        return false;
    }

    static auto get_pu_variation(const std::string &shift) -> std::string
    {
        if (shift == "PU_Up")
        {
            return "up";
        }

        if (shift == "PU_Down")
        {
            return "down";
        }

        return "nominal";
    }

    static auto get_prefiring_weight(float L1PreFiringWeight_Nom,
                                     float L1PreFiringWeight_Up,
                                     float L1PreFiringWeight_Dn,
                                     const std::string &shift) -> float
    {
        if (shift == "PreFiring_Up")
        {
            return L1PreFiringWeight_Up;
        }
        if (shift == "PreFiring_Down")
        {
            return L1PreFiringWeight_Dn;
        }
        return L1PreFiringWeight_Nom;
    }

    static auto scale_luminosity(const double luminosity, const std::string &shift) -> float
    {
        if (shift == "Luminosity_Up")
        {
            return luminosity * (1 + 2.5 / 100.);
        }
        if (shift == "Luminosity_Down")
        {
            return luminosity * (1 - 2.5 / 100.);
        }

        return 1.;
    }

    static auto get_scale_factor(const std::string &shift,
                                 unsigned int n_muons,
                                 unsigned int n_electrons,
                                 unsigned int n_photons,
                                 unsigned int n_bjets,
                                 unsigned int n_jets,
                                 unsigned int n_met,
                                 const MUSiCObjects &muons,
                                 const MUSiCObjects &electrons,
                                 const MUSiCObjects &photons,
                                 const MUSiCObjects &bjets,
                                 const MUSiCObjects &jets,
                                 const MUSiCObjects &met) -> double
    {
        if (shift == "ScaleFactor_Up")
        {
            return std::reduce(muons.scale_factor_up.begin(),
                               muons.scale_factor_up.begin() + n_muons,
                               1.,
                               std::multiplies<double>()) //
                   * std::reduce(electrons.scale_factor_up.begin(),
                                 electrons.scale_factor_up.begin() + n_electrons,
                                 1.,
                                 std::multiplies<double>()) //
                   * std::reduce(photons.scale_factor_up.begin(),
                                 photons.scale_factor_up.begin() + n_photons,
                                 1.,
                                 std::multiplies<double>()) //
                   * std::reduce(bjets.scale_factor_up.begin(),
                                 bjets.scale_factor_up.begin() + n_bjets,
                                 1.,
                                 std::multiplies<double>()) //
                   * std::reduce(jets.scale_factor_up.begin(),
                                 jets.scale_factor_up.begin() + n_jets,
                                 1.,
                                 std::multiplies<double>()) //
                   *
                   std::reduce(
                       met.scale_factor_up.begin(), met.scale_factor_up.begin() + n_met, 1., std::multiplies<double>());
        }

        if (shift == "ScaleFactor_Down")
        {
            return std::reduce(muons.scale_factor_down.begin(),
                               muons.scale_factor_down.begin() + n_muons,
                               1.,
                               std::multiplies<double>()) //
                   * std::reduce(electrons.scale_factor_down.begin(),
                                 electrons.scale_factor_down.begin() + n_electrons,
                                 1.,
                                 std::multiplies<double>()) //
                   * std::reduce(photons.scale_factor_down.begin(),
                                 photons.scale_factor_down.begin() + n_photons,
                                 1.,
                                 std::multiplies<double>()) //
                   * std::reduce(bjets.scale_factor_down.begin(),
                                 bjets.scale_factor_down.begin() + n_bjets,
                                 1.,
                                 std::multiplies<double>()) //
                   * std::reduce(jets.scale_factor_down.begin(),
                                 jets.scale_factor_down.begin() + n_jets,
                                 1.,
                                 std::multiplies<double>()) //
                   * std::reduce(met.scale_factor_down.begin(),
                                 met.scale_factor_down.begin() + n_met,
                                 1.,
                                 std::multiplies<double>());
        }

        return std::reduce(
                   muons.scale_factor.begin(), muons.scale_factor.begin() + n_muons, 1., std::multiplies<double>()) //
               * std::reduce(electrons.scale_factor.begin(),
                             electrons.scale_factor.begin() + n_electrons,
                             1.,
                             std::multiplies<double>()) //
               * std::reduce(photons.scale_factor.begin(),
                             photons.scale_factor.begin() + n_photons,
                             1.,
                             std::multiplies<double>()) //
               * std::reduce(
                     bjets.scale_factor.begin(), bjets.scale_factor.begin() + n_bjets, 1., std::multiplies<double>()) //
               * std::reduce(
                     jets.scale_factor.begin(), jets.scale_factor.begin() + n_jets, 1., std::multiplies<double>()) //
               * std::reduce(met.scale_factor.begin(), met.scale_factor.begin() + n_met, 1., std::multiplies<double>());
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set PDF and Alpha_S uncertainties.
    /// Those are tricky beasts, since they are not simple weights added to the event, but rather, should be treated as
    /// variations and have their uncert. squared-summed in the end of the processing (classification).
    /// This method also saves the LHA ID that was used during generation or rescaling.
    /// Ref: https://arxiv.org/pdf/1510.03865.pdf
    static auto get_pdf_alpha_s_weights(const std::string &shift,                                                //
                                        const std::optional<std::pair<unsigned int, unsigned int>> &lha_indexes, //
                                        const std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>,              //
                                                         std::unique_ptr<LHAPDF::PDF>,                           //
                                                         std::unique_ptr<LHAPDF::PDF>> &default_pdf_sets,        //
                                        RVec<float> LHEPdfWeight,                                                //
                                        float Generator_scalePDF,                                                //
                                        float Generator_x1,                                                      //
                                        float Generator_x2,                                                      //
                                        int Generator_id1,                                                       //
                                        int Generator_id2,                                                       //
                                        float LHEWeight_originalXWGTUP) -> double
    {
        if (shift == "PDF_As_Up" or shift == "PDF_As_Down")
        {
            // references are dangerous!!!!!
            // be carefull with life time
            auto &default_pdf = std::get<0>(default_pdf_sets);
            auto &alpha_s_up_pdf = std::get<1>(default_pdf_sets);
            auto &alpha_s_down_pdf = std::get<2>(default_pdf_sets);

            if (LHEPdfWeight.size() > 0)
            {
                float alpha_s_up = 1.;
                float alpha_s_down = 1.;

                // set LHA ID
                auto [lha_id, _] = lha_indexes.value_or(std::pair<unsigned int, unsigned int>());
                if (lha_id == 0)
                {
                    throw std::runtime_error(fmt::format(
                        "ERROR: There are PDF weights written in the file, but the REGEX parser failed to get "
                        "a proper LHA ID."));
                }

                // The nominal LHEPdfWeight (first element) is expected to be 1 but also found values  All variations
                // will be normalized to the nominal LHEPdfWeight.
                LHEPdfWeight = LHEPdfWeight / LHEPdfWeight[0];

                // has alpha_s weights
                // if (LHEPdfWeight.size() == 103 or LHEPdfWeight.size() == 33)
                if (LHEPdfWeight.size() == 103)
                {
                    alpha_s_up = LHEPdfWeight[101];
                    alpha_s_down = LHEPdfWeight[102];

                    // remove the first weight (always 1.)
                    // remove last two elements (Alpha_S weights)
                    LHEPdfWeight.erase(LHEPdfWeight.begin());
                    LHEPdfWeight.erase(LHEPdfWeight.end() - 1);
                    LHEPdfWeight.erase(LHEPdfWeight.end() - 1);
                }

                // don't have alpha_s weights, should get the one from the 5f LHAPDF set.
                // REF:
                // https://cms-pdmv.gitbook.io/project/mccontact/info-for-mc-production-for-ultra-legacy-campaigns-2016-2017-2018#recommendations-on-the-usage-of-pdfs-and-cpx-tunes
                // else if (LHEPdfWeight.size() == 101 or LHEPdfWeight.size() == 31)
                else if (LHEPdfWeight.size() == 101)
                {
                    // Those are some possible convertion from for NNPDF31, without to with alpha_s
                    // During the classification, the code should check the status of alpha_s_up and alpha_s_down
                    // and react accordinly.
                    // 304400 --> 306000
                    // 316200 --> 325300
                    // 325500 --> 325300
                    // 320900 --> 306000

                    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0120 (319500) and divide the new
                    // weight by the weight from the PDF the event was produced with.
                    alpha_s_up = alpha_s_up_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                 alpha_s_up_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                                 LHEWeight_originalXWGTUP;

                    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0116 (319300) and divide the new
                    // weight by the weight from the PDF the event was produced with.
                    alpha_s_down = alpha_s_down_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                   alpha_s_down_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                                   LHEWeight_originalXWGTUP;
                }
                else
                {
                    throw std::runtime_error(fmt::format(
                        "ERROR: Unexpected number of PDF weights ({}). According to CMSSW "
                        "(https://github.dev/cms-sw/cmssw/blob/6ef534126e6db3dfdea86c3f0eedb773f0117cbc/PhysicsTools/"
                        // "NanoAOD/python/genWeightsTable_cfi.py#L20) if should be eighther 101, 103, 31 or 33.\n",
                        "NanoAOD/python/genWeightsTable_cfi.py#L20) if should be eighther 101 or 103.\n",
                        LHEPdfWeight.size()));
                }

                auto sorted_weights = VecOps::Sort(LHEPdfWeight);
                auto pdf_shift = (sorted_weights[83] - sorted_weights[15]) / 2.f;
                auto alpha_s_shift = (alpha_s_up - alpha_s_down) / 2.f;

                if (shift == "PDF_As_Up")
                {
                    return 1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
                }

                if (shift == "PDF_As_Down")
                {
                    return 1. - std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
                }
            }
            // NNPDF31_nnlo_as_0118_hessian - 304400
            float alpha_s_up = 1.;
            float alpha_s_down = 1.;

            // Compute the PDF weight for this event using NNPDF31_nnlo_as_0118_hessian (304400) and divide the
            // new weight by the weight from the PDF the event was produced with.
            LHEWeight_originalXWGTUP = default_pdf[0]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                       default_pdf[0]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF);

            // skip the first, since it corresponds to the originalXWGTUP (nominal)
            for (std::size_t i = 1; i < default_pdf.size(); i++)
            {
                LHEPdfWeight.push_back(default_pdf[i]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                       default_pdf[i]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                                       LHEWeight_originalXWGTUP);
            }

            // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0120 (319500) and divide the new
            // weight by the weight from the PDF the event was produced with.
            alpha_s_up = alpha_s_up_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                         alpha_s_up_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                         LHEWeight_originalXWGTUP;

            // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0116 (319300) and divide the new
            // weight by the weight from the PDF the event was produced with.
            alpha_s_down = alpha_s_down_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                           alpha_s_down_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                           LHEWeight_originalXWGTUP;

            auto sorted_weights = VecOps::Sort(LHEPdfWeight);
            auto pdf_shift = (sorted_weights[83] - sorted_weights[15]) / 2.f;
            auto alpha_s_shift = (alpha_s_up - alpha_s_down) / 2.f;

            if (shift == "PDF_As_Up")
            {
                return 1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
            }

            if (shift == "PDF_As_Down")
            {
                return 1. - std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
            }
        }

        return 1.;
    }
};

#endif /*SHIFTS*/
