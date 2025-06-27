#ifndef SHIFTS_HPP
#define SHIFTS_HPP

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fmt/core.h>
#include <string>

#include <fmt/format.h>
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

#include "ROOT/RVec.hxx"
using namespace ROOT;

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
    X(PreFiring_Up)                                                                                                    \
    X(PreFiring_Down)                                                                                                  \
    X(QCDScale_Up)                                                                                                     \
    X(QCDScale_Down)                                                                                                   \
    X(MuonReco_Up)                                                                                                     \
    X(MuonReco_Down)                                                                                                   \
    X(MuonId_Up)                                                                                                       \
    X(MuonId_Down)                                                                                                     \
    X(MuonIso_Up)                                                                                                      \
    X(MuonIso_Down)                                                                                                    \
    X(ElectronReco_Up)                                                                                                 \
    X(ElectronReco_Down)                                                                                               \
    X(ElectronId_Up)                                                                                                   \
    X(ElectronId_Down)                                                                                                 \
    X(ElectronDiffResolution_Up)                                                                                       \
    X(ElectronDiffResolution_Down)                                                                                     \
    X(ElectronDiffScale_Up)                                                                                            \
    X(ElectronDiffScale_Down)                                                                                          \
    X(PhotonId_Up)                                                                                                     \
    X(PhotonId_Down)                                                                                                   \
    X(PhotonVeto_Up)                                                                                                   \
    X(PhotonVeto_Down)                                                                                                 \
    X(PhotonDiffResolution_Up)                                                                                         \
    X(PhotonDiffResolution_Down)                                                                                       \
    X(PhotonDiffScale_Up)                                                                                              \
    X(PhotonDiffScale_Down)                                                                                            \
    X(TauVsE_Up)                                                                                                       \
    X(TauVsE_Down)                                                                                                     \
    X(TauVsMu_Up)                                                                                                      \
    X(TauVsMu_Down)                                                                                                    \
    X(TauVsJet_Up)                                                                                                     \
    X(TauVsJet_Down)                                                                                                   \
    X(TauDiffEnergy_Up)                                                                                                \
    X(TauDiffEnergy_Down)                                                                                              \
    X(JetBTag_Up)                                                                                                      \
    X(JetBTag_Down)                                                                                                    \
    X(JetDiffResolution_Up)                                                                                            \
    X(JetDiffResolution_Down)                                                                                          \
    X(JetDiffScale_Up)                                                                                                 \
    X(JetDiffScale_Down)                                                                                               \
    X(METDiffUnclusteredEnergy_Up)                                                                                     \
    X(METDiffUnclusteredEnergy_Down)                                                                                   \
    X(kTotalVariations)                                                                                                \
    X(Fakes_Down)                                                                                                      \
    X(PDF_As_Down)                                                                                                     \
    X(MuonDiffResolution_Up)                                                                                           \
    X(MuonDiffResolution_Down)                                                                                         \
    X(MuonDiffScale_Up)                                                                                                \
    X(MuonDiffScale_Down)

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
            throw std::runtime_error(
                fmt::format("Could not convert variation ({}) to string.", static_cast<unsigned int>(var)));
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

        throw std::runtime_error(fmt::format("Could not convert string ({}) to variation.", variation));
    }

  private:
    const std::vector<Variations> m_constant_shifts;
    const std::vector<Variations> m_differential_shifts;

  public:
    Shifts(bool is_data)
        : m_constant_shifts(is_data ? std::vector<Variations>{Variations::Nominal}
                                    : std::vector<Variations>{Variations::Nominal,  //
                                                              Variations::PU_Up,    //
                                                              Variations::PU_Down,  //
                                                              Variations::Fakes_Up, //
                                                              //    Variations::Fakes_Down,        //
                                                              Variations::PDF_As_Up, //
                                                              //    Variations::PDF_As_Down,      //
                                                              Variations::PreFiring_Up,   //
                                                              Variations::PreFiring_Down, //
                                                              Variations::QCDScale_Up,
                                                              Variations::QCDScale_Down,
                                                              Variations::MuonReco_Up,
                                                              Variations::MuonReco_Down,
                                                              Variations::MuonId_Up,
                                                              Variations::MuonId_Down,
                                                              Variations::MuonIso_Up,
                                                              Variations::MuonIso_Down,
                                                              Variations::ElectronReco_Up,
                                                              Variations::ElectronReco_Down,
                                                              Variations::ElectronId_Up,
                                                              Variations::ElectronId_Down,
                                                              Variations::PhotonId_Up,
                                                              Variations::PhotonId_Down,
                                                              Variations::PhotonVeto_Up,
                                                              Variations::PhotonVeto_Down,
                                                              Variations::TauVsE_Up,
                                                              Variations::TauVsE_Down,
                                                              Variations::TauVsMu_Up,
                                                              Variations::TauVsMu_Down,
                                                              Variations::TauVsJet_Up,
                                                              Variations::TauVsJet_Down,
                                                              Variations::JetBTag_Up,
                                                              Variations::JetBTag_Down}),

          m_differential_shifts(is_data ? std::vector<Variations>{Variations::Nominal}
                                        : std::vector<Variations>{
                                              Variations::Nominal, //
                                                                   //    Variations::MuonResolution_Up,       //
                                                                   //    Variations::MuonResolution_Down,     //
                                                                   //    Variations::MuonScale_Up,            //
                                                                   //    Variations::MuonScale_Down,          //
                                              Variations::ElectronDiffResolution_Up,     //
                                              Variations::ElectronDiffResolution_Down,   //
                                              Variations::ElectronDiffScale_Up,          //
                                              Variations::ElectronDiffScale_Down,        //
                                              Variations::PhotonDiffResolution_Up,       //
                                              Variations::PhotonDiffResolution_Down,     //
                                              Variations::PhotonDiffScale_Up,            //
                                              Variations::PhotonDiffScale_Down,          //
                                              Variations::JetDiffResolution_Up,          //
                                              Variations::JetDiffResolution_Down,        //
                                              Variations::JetDiffScale_Up,               //
                                              Variations::JetDiffScale_Down,             //
                                              Variations::METDiffUnclusteredEnergy_Up,   //
                                              Variations::METDiffUnclusteredEnergy_Down, //
                                              Variations::TauDiffEnergy_Up,              //
                                              Variations::TauDiffEnergy_Down             //
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
            throw std::runtime_error(
                fmt::format("Could not resolve shift. Differential ({}) "
                            "and Constant ({}) can not be both variations.",
                            diff_shift,
                            const_shift));
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
                    throw std::runtime_error( fmt::format(fmt::runtime("Unexpected number of QCD scale weights ({}). "
                                             "Expected to be 8 or 9. \nWeights: [{}]\n"),
                                LHEScaleWeight.size(),
                                fmt::join(LHEScaleWeight, ", ")) );
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

    static auto is_diff(Variations shift) -> bool
    {
        return variation_to_string(shift).find("Diff") != std::string::npos;
    }
    static auto is_MET_diff(Variations shift) -> bool
    {
        return variation_to_string(shift).find("METDiff") != std::string::npos;
    }
};

inline auto format_as(Shifts::Variations v)
{
    return fmt::underlying(v);
}

#endif /*SHIFTS_HPP*/
