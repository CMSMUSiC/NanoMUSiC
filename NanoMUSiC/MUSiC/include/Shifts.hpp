#ifndef SHIFTS_HPP
#define SHIFTS_HPP

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <fmt/core.h>
template <typename EnumType>
requires std::is_enum_v<EnumType>
struct fmt::formatter<EnumType> : fmt::formatter<std::underlying_type_t<EnumType>>
{
    // Forwards the formatting by casting the enum to it's underlying type
    auto format(const EnumType &enumValue, format_context &ctx) const
    {
        return fmt::formatter<std::underlying_type_t<EnumType>>::format(
            static_cast<std::underlying_type_t<EnumType>>(enumValue), ctx);
    }
};

#include "ObjectFactories/music_objects.hpp"
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

// Nominal,
//
// // Constant
// PU_Up,
// PU_Down,
// Fakes_Up,
// PDF_As_Up,
// ScaleFactor_Up,
// PreFiring_Up,
// PreFiring_Down,
// QCDScale_Up,
// QCDScale_Down,
//
// // Differential
// ElectronResolution_Up,
// ElectronResolution_Down,
// ElectronScale_Up,
// ElectronScale_Down,
// PhotonResolution_Up,
// PhotonResolution_Down,
// PhotonScale_Up,
// PhotonScale_Down,
// JetResolution_Up,
// JetResolution_Down,
// JetScale_Up,
// JetScale_Down,
// UnclusteredEnergy_Up,
// UnclusteredEnergy_Down,
// TauEnergy_Up,
// TauEnergy_Down,
//
// // always the last one
// kTotalVariations,
//
// // backup
// Fakes_Down,
// PDF_As_Down,
// ScaleFactor_Down,
// MuonResolution_Up,
// MuonResolution_Down,
// MuonScale_Up,
// MuonScale_Down,
#define LIST_OF_SHIFTS                                                                                                 \
    X(Nominal)                                                                                                         \
    X(PU_Up)                                                                                                           \
    X(PU_Down)                                                                                                         \
    X(Fakes_Up)                                                                                                        \
    X(PDF_As_Up)                                                                                                       \
    X(ScaleFactor_Up)                                                                                                  \
    X(PreFiring_Up)                                                                                                    \
    X(PreFiring_Down)                                                                                                  \
    X(QCDScale_Up)                                                                                                     \
    X(QCDScale_Down)                                                                                                   \
    X(ElectronResolution_Up)                                                                                           \
    X(ElectronResolution_Down)                                                                                         \
    X(ElectronScale_Up)                                                                                                \
    X(ElectronScale_Down)                                                                                              \
    X(PhotonResolution_Up)                                                                                             \
    X(PhotonResolution_Down)                                                                                           \
    X(PhotonScale_Up)                                                                                                  \
    X(PhotonScale_Down)                                                                                                \
    X(JetResolution_Up)                                                                                                \
    X(JetResolution_Down)                                                                                              \
    X(JetScale_Up)                                                                                                     \
    X(JetScale_Down)                                                                                                   \
    X(UnclusteredEnergy_Up)                                                                                            \
    X(UnclusteredEnergy_Down)                                                                                          \
    X(TauEnergy_Up)                                                                                                    \
    X(TauEnergy_Down)                                                                                                  \
    X(kTotalVariations)                                                                                                \
    X(Fakes_Down)                                                                                                      \
    X(PDF_As_Down)                                                                                                     \
    X(ScaleFactor_Down)                                                                                                \
    X(MuonResolution_Up)                                                                                               \
    X(MuonResolution_Down)                                                                                             \
    X(MuonScale_Up)                                                                                                    \
    X(MuonScale_Down)

class Shifts
{
  public:
    enum class Variations
    {
#define X(name) name,
        LIST_OF_SHIFTS
#undef X
    };

    template <typename T>
    static auto variation_to_string(T _var) -> std::string
    {
        auto var = static_cast<Variations>(_var);
        switch (var)
        {
#define X(name)                                                                                                        \
    case Variations::name:                                                                                             \
        return #name;
            LIST_OF_SHIFTS
#undef X
        default:
            fmt::print(stderr, "ERROR: Could not convert variation ({}) to string.", static_cast<unsigned int>(var));
            std::exit(EXIT_FAILURE);
        }
    }

    static auto string_to_variation(const std::string &variation) -> Variations
    {
#define X(name)                                                                                                        \
    if (variation == #name)                                                                                            \
    {                                                                                                                  \
        return Variations::name;                                                                                       \
    }
        LIST_OF_SHIFTS
#undef X

        fmt::print(stderr, "ERROR: Could not convert string ({}) to variation.", variation);
        std::exit(EXIT_FAILURE);
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
                                       2) //
                              + std::pow(std::reduce(this_electrons.scale_factor_shift.begin(),
                                                     this_electrons.scale_factor_shift.begin() + n_electrons,
                                                     0.,
                                                     std::plus<double>()),
                                         2) //
                              + std::pow(std::reduce(this_taus.scale_factor_shift.begin(),
                                                     this_taus.scale_factor_shift.begin() + n_taus,
                                                     0.,
                                                     std::plus<double>()),
                                         2) //
                              + std::pow(std::reduce(this_photons.scale_factor_shift.begin(),
                                                     this_photons.scale_factor_shift.begin() + n_photons,
                                                     0.,
                                                     std::plus<double>()),
                                         2) //
                              + std::pow(std::reduce(this_bjets.scale_factor_shift.begin(),
                                                     this_bjets.scale_factor_shift.begin() + n_bjets,
                                                     0.,
                                                     std::plus<double>()),
                                         2) //
                              + std::pow(std::reduce(this_jets.scale_factor_shift.begin(),
                                                     this_jets.scale_factor_shift.begin() + n_jets,
                                                     0.,
                                                     std::plus<double>()),
                                         2) //
                              + std::pow(std::reduce(this_met.scale_factor_shift.begin(),
                                                     this_met.scale_factor_shift.begin() + n_met,
                                                     0.,
                                                     std::plus<double>()),
                                         2) //
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
                                           std::pair<std::size_t, const MUSiCObjects &> jets) -> double
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
                    2) +
                std::pow(std::reduce(this_electrons.is_fake.cbegin(),
                                     this_electrons.is_fake.cbegin() + n_electrons,
                                     0.,
                                     std::plus<double>()) *
                             0.5,
                         2) +

                std::pow(std::reduce(
                             this_taus.is_fake.cbegin(), this_taus.is_fake.cbegin() + n_taus, 0., std::plus<double>()) *
                             0.5,
                         2) +

                std::pow(std::reduce(this_photons.is_fake.cbegin(),
                                     this_photons.is_fake.cbegin() + n_photons,
                                     0.,
                                     std::plus<double>()) *
                             0.5,
                         2) +
                std::pow(
                    std::reduce(
                        this_bjets.is_fake.cbegin(), this_bjets.is_fake.cbegin() + n_bjets, 0., std::plus<double>()) *
                        0.5,
                    2) +

                std::pow(std::reduce(
                             this_jets.is_fake.cbegin(), this_jets.is_fake.cbegin() + n_jets, 0., std::plus<double>()) *
                             0.5,
                         2);

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
                               fmt::runtime("ERROR: Unexpected number of QCD scale weights ({}). "
                                            "Expected to be 8 or 9. \nWeights: [{}]\n"),
                               LHEScaleWeight.size(),
                               fmt::join(LHEScaleWeight, ", "));
                    std::exit(EXIT_FAILURE);
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
                //            ROOT::VecOps::Max(LHEScaleWeight),
                //            ROOT::VecOps::Min(LHEScaleWeight),
                //            fmt::join(LHEScaleWeight, ", "));

                if (shift == Variations::QCDScale_Up)
                {
                    return ROOT::VecOps::Max(LHEScaleWeight);
                }
                if (shift == Variations::QCDScale_Down)
                {
                    return ROOT::VecOps::Min(LHEScaleWeight);
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
