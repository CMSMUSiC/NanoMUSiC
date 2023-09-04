#ifndef SHIFTS
#define SHIFTS
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "ObjectFactories/music_objects.hpp"
#include "PDFAlphaSWeights.hpp"

inline auto contains(std::string &&str, const std::string &substring) -> bool
{
    std::transform(str.begin(),
                   str.end(),
                   str.begin(),
                   [](unsigned char c) -> unsigned char
                   {
                       return std::tolower(c);
                   });

    return str.find(substring) != std::string::npos;
}

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
    const std::vector<std::string> m_constant_shifts;
    const std::vector<std::string> m_differential_shifts;

  public:
    Shifts(bool is_data)
        // : m_constant_shifts(std::vector<std::string>{"Nominal"}),
        : m_constant_shifts(is_data ? std::vector<std::string>{"Nominal"}
                                    : std::vector<std::string>{"Nominal",          //
                                                               "PU_Up",            //
                                                               "PU_Down",          //
                                                               "Fakes_Up",          //
                                                               "Fakes_Down",        //
                                                               "PDF_As_Up",        //
                                                               "PDF_As_Down",      //
                                                               "ScaleFactor_Up",   //
                                                               "ScaleFactor_Down", //
                                                               "PreFiring_Up",     //
                                                               "PreFiring_Down"//
                                                               }),

        //   m_differential_shifts(std::vector<std::string>{"Nominal"})
      m_differential_shifts(is_data ? std::vector<std::string>{"Nominal"}
                                    : std::vector<std::string>{"Nominal", //
                                                                          //    "MuonResolution_Up",       //
                                                                          //    "MuonResolution_Down",     //
                                                                          //    "MuonScale_Up",            //
                                                                          //    "MuonScale_Down",          //
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
                                                               "JetScale_Down",
                                                               "UnclusteredEnergy_Up",//
                                                               "UnclusteredEnergy_Down",//
                                                               "TauEnergy_Up",//
                                                               "TauEnergy_Down",//
                                                               }                                                               )
    {
    }

    ///
    /// Will loop and map opver all shifts
    template <typename F>
    auto for_each(F &&f) const -> void
    {
        for (auto &&shift : this->get_constant_shifts())
        {
            f(shift);
        }
        for (auto &&shift : this->get_differential_shifts())
        {
            if (shift != "Nominal")
            {
                f(shift);
            }
        }
    }

    auto get_constant_shifts() const -> std::vector<std::string>
    {
        return m_constant_shifts;
    }

    auto get_constant_shifts(const std::string &diff_shift) const -> std::vector<std::string>
    {
        if (diff_shift == "Nominal")
        {
            auto _m_constant_shifts = m_constant_shifts;
            _m_constant_shifts.push_back("Nominal");
            return _m_constant_shifts;
        }
        return {"Nominal"};
    }

    auto get_differential_shifts() const -> std::vector<std::string>
    {
        return m_differential_shifts;
    }

    auto size() const -> std::size_t
    {
        return m_constant_shifts.size() + m_differential_shifts.size();
    }

    static auto resolve_shifts(const std::string &const_shift, const std::string &diff_shift) -> std::string
    {
        if (diff_shift != "Nominal" and const_shift != "Nominal")
        {
            throw std::runtime_error(fmt::format(
                "ERROR: Could not resolve shift. Differential ({}) and Constant ({}) can not be both variations.",
                diff_shift,
                const_shift));
        }

        if (diff_shift == "Nominal")
        {
            return const_shift;
        }
        return diff_shift;
    }

    auto is_valid(const std::string &shift) const -> bool
    {
        auto it = std::find(m_constant_shifts.cbegin(), m_constant_shifts.cend(), shift);
        if (it != m_constant_shifts.end())
        {
            return true;
        }
        it = std::find(m_differential_shifts.cbegin(), m_differential_shifts.cend(), shift);
        if (it != m_differential_shifts.end())
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
            return luminosity * (1 + 1.6 / 100.);
        }
        if (shift == "Luminosity_Down")
        {
            return luminosity * (1 - 1.6 / 100.);
        }

        return luminosity;
    }

    static auto get_reco_scale_factor(const std::string &shift,
                                      std::pair<std::size_t, const MUSiCObjects &> muons,
                                      std::pair<std::size_t, const MUSiCObjects &> electrons,
                                      std::pair<std::size_t, const MUSiCObjects &> taus,
                                      std::pair<std::size_t, const MUSiCObjects &> photons,
                                      std::pair<std::size_t, const MUSiCObjects &> bjets,
                                      std::pair<std::size_t, const MUSiCObjects &> jets,
                                      std::pair<std::size_t, const MUSiCObjects &> met) -> double
    {
        auto [n_muons, this_muons] = muons;
        auto [n_electrons, this_electrons] = electrons;
        auto [n_taus, this_taus] = taus;
        auto [n_photons, this_photons] = photons;
        auto [n_bjets, this_bjets] = bjets;
        auto [n_jets, this_jets] = jets;
        auto [n_met, this_met] = met;

        auto nominal =
            std::reduce(this_muons.scale_factor.begin(),
                        this_muons.scale_factor.begin() + n_muons,
                        1.,
                        std::multiplies<double>()) *
            std::reduce(this_electrons.scale_factor.begin(),
                        this_electrons.scale_factor.begin() + n_electrons,
                        1.,
                        std::multiplies<double>()) //
            * std::reduce(this_taus.scale_factor.begin(),
                          this_taus.scale_factor.begin() + n_taus,
                          1.,
                          std::multiplies<double>()) //
            * std::reduce(this_photons.scale_factor.begin(),
                          this_photons.scale_factor.begin() + n_photons,
                          1.,
                          std::multiplies<double>()) //
            * std::reduce(this_bjets.scale_factor.begin(),
                          this_bjets.scale_factor.begin() + n_bjets,
                          1.,
                          std::multiplies<double>()) *
            std::reduce(this_jets.scale_factor.begin(),
                        this_jets.scale_factor.begin() + n_jets,
                        1.,
                        std::multiplies<double>()) //
            * std::reduce(
                  this_met.scale_factor.begin(), this_met.scale_factor.begin() + n_met, 1., std::multiplies<double>());

        auto delta = 0.;

        if (shift == "ScaleFactor_Up" or shift == "ScaleFactor_Down")
        {
            delta = std::sqrt(std::pow(std::reduce(this_muons.scale_factor_shift.begin(),
                                                   this_muons.scale_factor_shift.begin() + n_muons,
                                                   0.,
                                                   std::plus<double>()),
                                       2.) //
                              + std::pow(std::reduce(this_electrons.scale_factor_shift.begin(),
                                                     this_electrons.scale_factor_shift.begin() + n_electrons,
                                                     0.,
                                                     std::plus<double>()),
                                         2.) //
                              + std::pow(std::reduce(this_taus.scale_factor_shift.begin(),
                                                     this_taus.scale_factor_shift.begin() + n_taus,
                                                     0.,
                                                     std::plus<double>()),
                                         2.) //
                              + std::pow(std::reduce(this_photons.scale_factor_shift.begin(),
                                                     this_photons.scale_factor_shift.begin() + n_photons,
                                                     0.,
                                                     std::plus<double>()),
                                         2.) //
                              + std::pow(std::reduce(this_bjets.scale_factor_shift.begin(),
                                                     this_bjets.scale_factor_shift.begin() + n_bjets,
                                                     0.,
                                                     std::plus<double>()),
                                         2.) //
                              + std::pow(std::reduce(this_jets.scale_factor_shift.begin(),
                                                     this_jets.scale_factor_shift.begin() + n_jets,
                                                     0.,
                                                     std::plus<double>()),
                                         2.) //
                              + std::pow(std::reduce(this_met.scale_factor_shift.begin(),
                                                     this_met.scale_factor_shift.begin() + n_met,
                                                     0.,
                                                     std::plus<double>()),
                                         2.) //
            );
        }

        if (shift == "ScaleFactor_Down")
        {
            delta = -delta;
        }

        return nominal + delta;
    }

    static auto get_xsec_order_modifier(const std::string &shift, const std::string &xs_order) -> double
    {
        if (xs_order == "LO")
        {
            if (shift == "xSecOrder_Up")
            {
                return 1. + .5;
            }
            if (shift == "xSecOrder_Down")
            {
                return 1. - .5;
            }
        }
        return 1.;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set PDF and Alpha_S uncertainties.
    /// Those are tricky beasts, since they are not simple weights added to the event, but rather, should be treated
    /// as variations and have their uncert. squared-summed in the end of the processing (classification). This
    /// method also saves the LHA ID that was used during generation or rescaling. Ref:
    /// https://arxiv.org/pdf/1510.03865.pdf
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
            // be carefull with lifetime
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

                // don't have alpha_s weights, should get the one from the 5f LHAPDF set.
                // REF:
                // https://cms-pdmv.gitbook.io/project/mccontact/info-for-mc-production-for-ultra-legacy-campaigns-2016-2017-2018#recommendations-on-the-usage-of-pdfs-and-cpx-tunes
                else if (LHEPdfWeight.size() == 101 or LHEPdfWeight.size() == 31)
                // else if (LHEPdfWeight.size() == 101)
                {
                    // Those are some possible convertion from for NNPDF31, without to with alpha_s
                    // During the classification, the code should check the status of alpha_s_up and alpha_s_down
                    // and react accordinly.
                    // 304400 --> 306000
                    // 316200 --> 325300
                    // 325500 --> 325300
                    // 320900 --> 306000

                    // remove the first weight (always 1.)
                    LHEPdfWeight.erase(LHEPdfWeight.begin());

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

                // calculate shifts
                auto alpha_s_shift = (alpha_s_up - alpha_s_down) / 2.f;
                float pdf_shift = std::numeric_limits<float>::max();
                if (contains(default_pdf.at(0)->set().errorType(), "hessian"))
                {
                    auto sum_shifts_squared = 0.;
                    for (std::size_t i = 1; i < LHEPdfWeight.size(); i++)
                    {
                        sum_shifts_squared += std::pow(LHEPdfWeight[i] - LHEWeight_originalXWGTUP, 2.);
                    }
                    pdf_shift = std::sqrt(sum_shifts_squared) / LHEWeight_originalXWGTUP;
                }
                else if (contains(default_pdf.at(0)->set().errorType(), "replica"))
                {
                    if (LHEPdfWeight.size() == 100)
                    {
                        auto sorted_weights = VecOps::Sort(LHEPdfWeight);
                        pdf_shift = (sorted_weights[83] - sorted_weights[15]) / 2.f;
                    }
                    else if (LHEPdfWeight.size() == 30)
                    {
                        pdf_shift = VecOps::StdDev(LHEPdfWeight);
                    }
                }
                else
                {
                    throw std::runtime_error(
                        fmt::format("ERROR: Could not get PDF error type. Unexpected error type ({}).\n",
                                    default_pdf.at(0)->set().errorType()));
                }

                if (shift == "PDF_As_Up")
                {
                    return 1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
                }

                if (shift == "PDF_As_Down")
                {
                    return 1. - std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
                }
            }

            // Assuming: NNPDF31_nnlo_as_0118_hessian - 304400 (Hessian)
            float alpha_s_up = 1.;
            float alpha_s_down = 1.;

            // Compute the PDF weight for this event using NNPDF31_nnlo_as_0118_hessian (304400) and divide the
            // new weight by the weight from the PDF the event was produced with.
            LHEWeight_originalXWGTUP = default_pdf[0]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                       default_pdf[0]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF);

            // skip the first, since it corresponds to the originalXWGTUP (nominal)
            auto sum_shifts_squared = 0.;
            for (std::size_t i = 1; i < default_pdf.size(); i++)
            {
                //     LHEPdfWeight.push_back(default_pdf[i]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                //                            default_pdf[i]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                //                            LHEWeight_originalXWGTUP);
                sum_shifts_squared += std::pow((default_pdf[i]->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                                                default_pdf[i]->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF)) -
                                                   LHEWeight_originalXWGTUP,
                                               2.);
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

            // auto sorted_weights = VecOps::Sort(LHEPdfWeight);
            // auto pdf_shift = (sorted_weights[83] - sorted_weights[15]) / 2.f;
            auto pdf_shift = std::sqrt(sum_shifts_squared) / LHEWeight_originalXWGTUP;
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

    constexpr static auto get_fake_shift(bool x) -> double
    {
        if (x)
        {
            return 0.5;
        }
        return 0.;
    }

    static auto get_fakes_variation_weight(const std::string &shift,
                                           std::pair<std::size_t, const MUSiCObjects &> muons,
                                           std::pair<std::size_t, const MUSiCObjects &> electrons,
                                           std::pair<std::size_t, const MUSiCObjects &> taus,
                                           std::pair<std::size_t, const MUSiCObjects &> photons,
                                           std::pair<std::size_t, const MUSiCObjects &> bjets,
                                           std::pair<std::size_t, const MUSiCObjects &> jets
                                           //    std::pair<std::size_t, const MUSiCObjects &> met//
                                           ) -> double
    {
        if (shift == "Fakes_Up" or shift == "Fakes_Down")
        {
            auto [n_muons, this_muons] = muons;
            auto [n_electrons, this_electrons] = electrons;
            auto [n_taus, this_taus] = taus;
            auto [n_photons, this_photons] = photons;
            auto [n_bjets, this_bjets] = bjets;
            auto [n_jets, this_jets] = jets;
            // auto [n_met, this_met] = met

            auto variation_squared =
                std::pow(
                    std::reduce(
                        this_muons.is_fake.cbegin(), this_muons.is_fake.cbegin() + n_muons, 0., std::plus<double>()) *
                        0.5,
                    2.) +
                std::pow(std::reduce(this_electrons.is_fake.cbegin(),
                                     this_electrons.is_fake.cbegin() + n_electrons,
                                     0.,
                                     std::plus<double>()) *
                             0.5,
                         2.) +

                std::pow(std::reduce(
                             this_taus.is_fake.cbegin(), this_taus.is_fake.cbegin() + n_taus, 0., std::plus<double>()) *
                             0.5,
                         2.) +

                std::pow(std::reduce(this_photons.is_fake.cbegin(),
                                     this_photons.is_fake.cbegin() + n_photons,
                                     0.,
                                     std::plus<double>()) *
                             0.5,
                         2.) +
                std::pow(
                    std::reduce(
                        this_bjets.is_fake.cbegin(), this_bjets.is_fake.cbegin() + n_bjets, 0., std::plus<double>()) *
                        0.5,
                    2.) +

                std::pow(std::reduce(
                             this_jets.is_fake.cbegin(), this_jets.is_fake.cbegin() + n_jets, 0., std::plus<double>()) *
                             0.5,
                         2.);

            if (shift == "Fakes_Up")
            {
                return 1. + std::sqrt(variation_squared);
            }

            if (shift == "Fakes_Down")
            {
                return 1. - std::sqrt(variation_squared);
            }
        }

        return 1.;
    }

    ///////////////////////////////////////////////////////////////
    // IMPORTANT: LHEScaleWeight is passed by value on purpose!
    ///////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    /// Set the QCD Scaling weights, using the envelope method. If the sample has no weights are kept as 1.
    // Ref: https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopSystematics#Factorization_and_renormalizatio
    // Python (Awkward Array) implementation:
    // https://raw.githubusercontent.com/columnflow/columnflow/99a864ef4c6fbb9a80ed21dbe8f0f70d5e3a64cf/columnflow/production/cms/scale.py
    static auto get_qcd_scale_weight(const std::string &shift, RVec<float> LHEScaleWeight) -> double
    {
        if (shift == "QCDScale_Up" or shift == "QCDScale_Down")
        {
            if (LHEScaleWeight.size() > 0)
            {
                if (not(LHEScaleWeight.size() == 9 or LHEScaleWeight.size() == 8))
                {
                    throw std::runtime_error(fmt::format(
                        "ERROR: Unexpected number of QCD scale weights ({}). Expected to be 8 or 9. \nWeights: [{}]\n",
                        LHEScaleWeight.size(),
                        fmt::join(LHEScaleWeight, ", ")));
                }

                auto murf_nominal = LHEScaleWeight[4];

                // REFERENCE on how to treat nLHEScaleWeight == 8:
                // https://github.com/rappoccio/QJetMassUproot/blob/3b12e0d16adf4fe2c8a50aac55d6a8a2a360d4d7/cms_utils.py
                if (LHEScaleWeight.size() == 8)
                {
                    murf_nominal = 1.;
                }

                // remove indexes 2 and 6 or 5 (n ==8) since they corresponds to unphysical values
                if (LHEScaleWeight.size() == 9)
                {
                    LHEScaleWeight.erase(LHEScaleWeight.begin() + 2);
                    LHEScaleWeight.erase(LHEScaleWeight.begin() + 4);
                    LHEScaleWeight.erase(LHEScaleWeight.begin() + 6);
                }
                if (LHEScaleWeight.size() == 8)
                {
                    LHEScaleWeight.erase(LHEScaleWeight.begin() + 2);
                    LHEScaleWeight.erase(LHEScaleWeight.begin() + 5);
                }

                // The nominal LHEScaleWeight is expected to be 1.
                // All variations will be normalized to the nominal LHEScaleWeight and it is assumed that the nominal
                // weight is already included in the LHEWeight. rescale, just in case, scale all weights to the nominal
                for (auto &scale_weight : LHEScaleWeight)
                {
                    scale_weight /= murf_nominal;
                }

                if (shift == "QCDScale_Up")
                {
                    return VecOps::Max(LHEScaleWeight);
                }
                if (shift == "QCDScale_Down")
                {
                    return VecOps::Min(LHEScaleWeight);
                }
            }
        }

        return 1.;
    }
};

#endif /*SHIFTS*/
