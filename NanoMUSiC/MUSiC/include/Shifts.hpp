#ifndef SHIFTS_HPP
#define SHIFTS_HPP

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "ObjectFactories/music_objects.hpp"
#include "PDFAlphaSWeights.hpp"
#include "ROOT/RVec.hxx"

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
  public:
    enum class Variations
    {
        Nominal,

        // Constant
        PU_Up,
        PU_Down,
        Fakes_Up,
        PDF_As_Up,
        ScaleFactor_Up,
        PreFiring_Up,
        PreFiring_Down,
        QCDScale_Up,
        QCDScale_Down,

        // Differential
        ElectronResolution_Up,
        ElectronResolution_Down,
        ElectronScale_Up,
        ElectronScale_Down,
        PhotonResolution_Up,
        PhotonResolution_Down,
        PhotonScale_Up,
        PhotonScale_Down,
        JetResolution_Up,
        JetResolution_Down,
        JetScale_Up,
        JetScale_Down,
        UnclusteredEnergy_Up,
        UnclusteredEnergy_Down,
        TauEnergy_Up,
        TauEnergy_Down,

        // always the last one
        kTotalVariations,

        // backup
        Fakes_Down,
        PDF_As_Down,
        ScaleFactor_Down,
        MuonResolution_Up,
        MuonResolution_Down,
        MuonScale_Up,
        MuonScale_Down,
    };

    static auto variation_to_string(const Variations var) -> std::string
    {
        switch (var)
        {
        case Variations::Nominal:
            return "Nominal";
        case Variations::PU_Up:
            return "PU_Up";
        case Variations::PU_Down:
            return "PU_Down";
        case Variations::Fakes_Up:
            return "Fakes_Up";
        case Variations::Fakes_Down:
            return "Fakes_Down";
        case Variations::PDF_As_Up:
            return "PDF_As_Up";
        case Variations::PDF_As_Down:
            return "PDF_As_Down";
        case Variations::ScaleFactor_Up:
            return "ScaleFactor_Up";
        case Variations::ScaleFactor_Down:
            return "ScaleFactor_Down";
        case Variations::PreFiring_Up:
            return "PreFiring_Up";
        case Variations::PreFiring_Down:
            return "PreFiring_Down";
        case Variations::QCDScale_Up:
            return "QCDScale_Up";
        case Variations::QCDScale_Down:
            return "QCDScale_Down";
        case Variations::MuonResolution_Up:
            return "MuonResolution_Up";
        case Variations::MuonResolution_Down:
            return "MuonResolution_Down";
        case Variations::MuonScale_Up:
            return "MuonScale_Up";
        case Variations::MuonScale_Down:
            return "MuonScale_Down";
        case Variations::ElectronResolution_Up:
            return "ElectronResolution_Up";
        case Variations::ElectronResolution_Down:
            return "ElectronResolution_Down";
        case Variations::ElectronScale_Up:
            return "ElectronScale_Up";
        case Variations::ElectronScale_Down:
            return "ElectronScale_Down";
        case Variations::PhotonResolution_Up:
            return "PhotonResolution_Up";
        case Variations::PhotonResolution_Down:
            return "PhotonResolution_Down";
        case Variations::PhotonScale_Up:
            return "PhotonScale_Up";
        case Variations::PhotonScale_Down:
            return "PhotonScale_Down";
        case Variations::JetResolution_Up:
            return "JetResolution_Up";
        case Variations::JetResolution_Down:
            return "JetResolution_Down";
        case Variations::JetScale_Up:
            return "JetScale_Up";
        case Variations::JetScale_Down:
            return "JetScale_Down";
        case Variations::UnclusteredEnergy_Up:
            return "UnclusteredEnergy_Up";
        case Variations::UnclusteredEnergy_Down:
            return "UnclusteredEnergy_Down";
        case Variations::TauEnergy_Up:
            return "TauEnergy_Up";
        case Variations::TauEnergy_Down:
            return "TauEnergy_Down";
        default:
            throw std::runtime_error(fmt::format("ERROR: Could not convert variation ({}) to string.", var));
        }
    }

  private:
    const std::vector<Variations> m_constant_shifts;
    const std::vector<Variations> m_differential_shifts;

  public:
    Shifts(bool is_data)
        // : m_constant_shifts(std::vector<Variations>{Variations::Nominal}),
        : m_constant_shifts(is_data ? std::vector<Variations>{Variations::Nominal}
                                    : std::vector<Variations>{Variations::Nominal,  //
                                                              Variations::PU_Up,    //
                                                              Variations::PU_Down,  //
                                                              Variations::Fakes_Up, //
                                                              //    Variations::Fakes_Down,        //
                                                              Variations::PDF_As_Up, //
                                                              //    Variations::PDF_As_Down,      //
                                                              Variations::ScaleFactor_Up, //
                                                              //    Variations::ScaleFactor_Down, //
                                                              Variations::PreFiring_Up,   //
                                                              Variations::PreFiring_Down, //
                                                              Variations::QCDScale_Up,
                                                              Variations::QCDScale_Down}),

          //   m_differential_shifts(std::vector<Variations>{Variations::Nominal})
          m_differential_shifts(is_data ? std::vector<Variations>{Variations::Nominal}
                                        : std::vector<Variations>{
                                              Variations::Nominal, //
                                                                   //    Variations::MuonResolution_Up,       //
                                                                   //    Variations::MuonResolution_Down,     //
                                                                   //    Variations::MuonScale_Up,            //
                                                                   //    Variations::MuonScale_Down,          //
                                              Variations::ElectronResolution_Up,   //
                                              Variations::ElectronResolution_Down, //
                                              Variations::ElectronScale_Up,        //
                                              Variations::ElectronScale_Down,      //
                                              Variations::PhotonResolution_Up,     //
                                              Variations::PhotonResolution_Down,   //
                                              Variations::PhotonScale_Up,          //
                                              Variations::PhotonScale_Down,        //
                                              Variations::JetResolution_Up,        //
                                              Variations::JetResolution_Down,      //
                                              Variations::JetScale_Up,             //
                                              Variations::JetScale_Down,           //
                                              Variations::UnclusteredEnergy_Up,    //
                                              Variations::UnclusteredEnergy_Down,  //
                                              Variations::TauEnergy_Up,            //
                                              Variations::TauEnergy_Down           //
                                          })
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
            if (shift != Variations::Nominal)
            {
                f(shift);
            }
        }
    }

    auto get_constant_shifts() const -> std::vector<Variations>
    {
        return m_constant_shifts;
    }

    auto get_constant_shifts(const Variations &diff_shift) const -> std::vector<Variations>
    {
        if (diff_shift == Variations::Nominal)
        {
            return m_constant_shifts;
        }
        return {Variations::Nominal};
    }

    auto get_differential_shifts() const -> std::vector<Variations>
    {
        return m_differential_shifts;
    }

    auto size() const -> std::size_t
    {
        return m_constant_shifts.size() + m_differential_shifts.size() - 1;
    }

    static auto resolve_shifts(const Variations &const_shift, const Variations &diff_shift) -> Variations
    {
        if (diff_shift != Variations::Nominal and const_shift != Variations::Nominal)
        {
            fmt::print(stderr,
                       "ERROR: Could not resolve shift. Differential ({}) "
                       "and Constant ({}) can not be both variations.",
                       diff_shift,
                       const_shift);
            std::exit(EXIT_FAILURE);
        }

        if (diff_shift == Variations::Nominal)
        {
            return const_shift;
        }
        return diff_shift;
    }

    auto is_valid(const Variations &shift) const -> bool
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

    static auto get_pu_variation(const Variations &shift) -> std::string
    {
        if (shift == Variations::PU_Up)
        {
            return "up";
        }

        if (shift == Variations::PU_Down)
        {
            return "down";
        }

        return "nominal";
    }

    static auto get_prefiring_weight(float L1PreFiringWeight_Nom,
                                     float L1PreFiringWeight_Up,
                                     float L1PreFiringWeight_Dn,
                                     const Variations &shift) -> double
    {
        if (shift == Variations::PreFiring_Up)
        {
            return L1PreFiringWeight_Up;
        }
        if (shift == Variations::PreFiring_Down)
        {
            return L1PreFiringWeight_Dn;
        }
        return L1PreFiringWeight_Nom;
    }

    // static auto scale_luminosity(const double luminosity, const Variations
    // &shift) -> double
    // {
    //     if (shift == Variations::Luminosity_Up)
    //     {
    //         return luminosity * (1 + 1.6 / 100.);
    //     }
    //     if (shift == Variations::Luminosity_Down)
    //     {
    //         return luminosity * (1 - 1.6 / 100.);
    //     }

    //     return luminosity;
    // }

    static auto get_reco_scale_factor(const Variations &shift,
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

        if (shift == Variations::ScaleFactor_Up or shift == Variations::ScaleFactor_Down)
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

        if (shift == Variations::ScaleFactor_Down)
        {
            delta = -delta;
        }

        return nominal + delta;
    }

    // static auto get_xsec_order_modifier(const Variations &shift, const
    // std::string &xs_order) -> double
    // {
    //     if (xs_order == "LO")
    //     {
    //         if (shift == Variations::xSecOrder_Up)
    //         {
    //             return 1. + .5;
    //         }
    //         if (shift == Variations::xSecOrder_Down)
    //         {
    //             return 1. - .5;
    //         }
    //     }
    //     return 1.;
    // }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Set PDF and Alpha_S uncertainties.
    /// Those are tricky beasts, since they are not simple weights added to the
    /// event, but rather, should be treated as variations and have their uncert.
    /// squared-summed in the end of the processing (classification). This method
    /// also saves the LHA ID that was used during generation or rescaling. Ref:
    /// https://arxiv.org/pdf/1510.03865.pdf
    static auto get_pdf_alpha_s_weights(const Variations &shift,                                                 //
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
        if (shift == Variations::PDF_As_Up or shift == Variations::PDF_As_Down)
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

                if (shift == Variations::PDF_As_Up)
                {
                    return 1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
                }

                if (shift == Variations::PDF_As_Down)
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
                         alpha_s_up_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                         LHEWeight_originalXWGTUP;

            // Compute the Alpha_S weight for this event using
            // NNPDF31_nnlo_as_0116 (319300) and divide the new weight by the
            // weight from the PDF the event was produced with.
            alpha_s_down = alpha_s_down_pdf->xfxQ(Generator_id1, Generator_x1, Generator_scalePDF) *
                           alpha_s_down_pdf->xfxQ(Generator_id2, Generator_x2, Generator_scalePDF) /
                           LHEWeight_originalXWGTUP;

            // auto sorted_weights = VecOps::Sort(LHEPdfWeight);
            // auto pdf_shift = (sorted_weights[83] - sorted_weights[15]) / 2.f;
            auto pdf_shift = std::sqrt(sum_shifts_squared) / LHEWeight_originalXWGTUP;
            auto alpha_s_shift = (alpha_s_up - alpha_s_down) / 2.f;

            if (std::isnan(pdf_shift) or std::isnan(alpha_s_shift) or std::isinf(pdf_shift) or
                std::isinf(alpha_s_shift))
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

            if (shift == Variations::PDF_As_Up)
            {
                return 1. + std::sqrt(std::pow(pdf_shift, 2.) + std::pow(alpha_s_shift, 2.));
            }

            if (shift == Variations::PDF_As_Down)
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

    static auto get_fakes_variation_weight(const Shifts::Variations shift,
                                           std::pair<std::size_t, const MUSiCObjects &> muons,
                                           std::pair<std::size_t, const MUSiCObjects &> electrons,
                                           std::pair<std::size_t, const MUSiCObjects &> taus,
                                           std::pair<std::size_t, const MUSiCObjects &> photons,
                                           std::pair<std::size_t, const MUSiCObjects &> bjets,
                                           std::pair<std::size_t, const MUSiCObjects &> jets
                                           //    std::pair<std::size_t, const MUSiCObjects &> met//
                                           ) -> double
    {
        if (shift == Shifts::Variations::Fakes_Up or shift == Shifts::Variations::Fakes_Down)
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

            if (shift == Shifts::Variations::Fakes_Up)
            {
                return 1. + std::sqrt(variation_squared);
            }

            if (shift == Shifts::Variations::Fakes_Down)
            {
                return 1. - std::sqrt(variation_squared);
            }
        }

        return 1.;
    }

    ///////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    /// Set the QCD Scaling weights, using the envelope method. If the sample has
    /// no weights are kept as 1.
    // Ref:
    // https://twiki.cern.ch/twiki/bin/viewauth/CMS/TopSystematics#Factorization_and_renormalizatio
    // Python (Awkward Array) implementation:
    // https://raw.githubusercontent.com/columnflow/columnflow/99a864ef4c6fbb9a80ed21dbe8f0f70d5e3a64cf/columnflow/production/cms/scale.py
    static auto get_qcd_scale_weight(const Variations &shift, const RVec<float> &_LHEScaleWeight) -> double
    {
        if (shift == Variations::QCDScale_Up or shift == Variations::QCDScale_Down)
        {
            RVec<float> LHEScaleWeight;
            for (auto &&weight : _LHEScaleWeight)
            {
                LHEScaleWeight.push_back(weight);
            }

            if (LHEScaleWeight.size() > 0)
            {
                if (not(LHEScaleWeight.size() == 9 or LHEScaleWeight.size() == 8))
                {
                    fmt::print(stderr,
                               fmt::format("ERROR: Unexpected number of QCD scale weights ({}). "
                                           "Expected to be 8 or 9. \nWeights: [{}]\n",
                                           LHEScaleWeight.size(),
                                           fmt::join(LHEScaleWeight, ", ")));
                    exit(-1);
                }

                auto murf_nominal = LHEScaleWeight[4];

                // REFERENCE on how to treat nLHEScaleWeight == 8:
                // https://github.com/rappoccio/QJetMassUproot/blob/3b12e0d16adf4fe2c8a50aac55d6a8a2a360d4d7/cms_utils.py
                if (LHEScaleWeight.size() == 8)
                {
                    murf_nominal = 1.;
                }

                // remove indexes 2 and 6 or 5 (when n = 8) since they corresponds to
                // unphysical values
                if (LHEScaleWeight.size() == 9)
                {
                    LHEScaleWeight = {
                        LHEScaleWeight[0],
                        LHEScaleWeight[1],
                        LHEScaleWeight[3],
                        LHEScaleWeight[5],
                        LHEScaleWeight[7],
                        LHEScaleWeight[8],
                    };
                }
                else if (LHEScaleWeight.size() == 8)
                {
                    LHEScaleWeight = {
                        LHEScaleWeight[0],
                        LHEScaleWeight[1],
                        LHEScaleWeight[3],
                        LHEScaleWeight[4],
                        LHEScaleWeight[6],
                        LHEScaleWeight[7],
                    };
                }

                // The nominal LHEScaleWeight is expected to be 1.
                // All variations will be normalized to the nominal LHEScaleWeight
                // and it is assumed that the nominal weight is already included
                // in the LHEWeight. rescale, just in case, scale all weights to
                // the nominal
                // for (auto &scale_weight : LHEScaleWeight)
                for (std::size_t i = 0; i < LHEScaleWeight.size(); i++)
                {
                    LHEScaleWeight.at(i) /= murf_nominal;
                }

                // fmt::print("-- QCD Scale Up/Down: {} / {} - All: [{}]\n",
                //            VecOps::Max(LHEScaleWeight),
                //            VecOps::Min(LHEScaleWeight),
                //            fmt::join(LHEScaleWeight, ", "));

                if (shift == Variations::QCDScale_Up)
                {
                    return VecOps::Max(LHEScaleWeight);
                }
                if (shift == Variations::QCDScale_Down)
                {
                    return VecOps::Min(LHEScaleWeight);
                }
            }
        }
        return 1.;
    }
};

inline auto format_as(Shifts::Variations v)
{
    return fmt::underlying(v);
}

#endif /*SHIFTS_HPP*/
