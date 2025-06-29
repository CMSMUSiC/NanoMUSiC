#ifndef MUSIC_OBJECTS_HPP
#define MUSIC_OBJECTS_HPP

#include <any>
#include <concepts>
#include <cstdlib>
#include <map>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <vdt/exp.h>

#include "Math/GenVector/VectorUtil.h"
#include "fmt/format.h"

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "Shifts.hpp"

using namespace ROOT;
using namespace ROOT::Math;
// using namespace ROOT::ROOT::VecOps;

// correctionlib
// More info: https://twiki.cern.ch/twiki/bin/viewauth/CMS/BTagCalibration
// More info: https://github.com/cms-nanoAOD/correctionlib
// More info: https://cms-nanoaod.github.io/correctionlib/index.html
// Instructions:
// https://indico.cern.ch/event/1096988/contributions/4615134/attachments/2346047/4000529/Nov21_btaggingSFjsons.pdf
#include "correction.h"
using CorrectionlibRef_t = correction::Correction::Ref;

inline auto get_uniform_vector_size(const std::unordered_map<Shifts::Variations, RVec<double>> &map)
    -> std::optional<std::size_t>
{
    std::size_t expected_size = map.empty() ? 0 : map.cbegin()->second.size();

    for (const auto &[key, vec] : map)
    {
        if (vec.size() != expected_size)
        {
            return std::nullopt;
        }
    }

    return expected_size;
}

inline std::string map_to_string(const std::unordered_map<Shifts::Variations, RVec<double>> &map)
{
    if (map.empty())
    {
        return "{}";
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{ ";

    bool first_entry = true;
    for (const auto &[key, vec] : map)
    {
        if (!first_entry)
        {
            oss << ", ";
        }
        first_entry = false;

        oss << "  " << Shifts::variation_to_string(key) << ": [";

        bool first_element = true;
        for (const auto &value : vec)
        {
            if (!first_element)
            {
                oss << ", ";
            }
            first_element = false;
            oss << value;
        }

        oss << "]";
    }

    oss << " }";
    return oss.str();
}

class MUSiCObjects
{
  public:
    RVec<Math::PtEtaPhiMVector> p4;
    std::unordered_map<Shifts::Variations, RVec<double>> scale_factor;
    RVec<double> delta_met_x;
    RVec<double> delta_met_y;
    RVec<bool> is_fake;

    MUSiCObjects()
        : p4({}),
          scale_factor({}),
          delta_met_x({}),
          delta_met_y({}),
          is_fake({})
    {
    }

    MUSiCObjects(const RVec<Math::PtEtaPhiMVector> &_p4,
                 std::unordered_map<Shifts::Variations, RVec<double>> &_scale_factor,
                 const RVec<double> &_delta_met_x,
                 const RVec<double> &_delta_met_y,
                 const RVec<bool> &_is_fake)
        : p4(_p4),
          scale_factor(_scale_factor),
          delta_met_x(_delta_met_x),
          delta_met_y(_delta_met_y),
          is_fake(_is_fake)
    {
        auto scale_factor_size = get_uniform_vector_size(_scale_factor);

        if (not(scale_factor_size))
        {
            throw std::runtime_error(
                fmt::format("Could not create MUSiCObjects. Scale factors have different sizes. {}\n",
                            map_to_string(_scale_factor)));
        }

        if (not(                                    //
                p4.size() == *scale_factor_size     //
                and p4.size() == delta_met_x.size() //
                and p4.size() == delta_met_y.size() //
                and p4.size() == is_fake.size()     //
                ))
        {
            throw std::runtime_error(
                fmt::format("Could not create MUSiCObjects. Input vectors have different sizes. {} - {} - {} - {} - "
                            "{}\n",
                            p4.size(),
                            scale_factor_size ? *scale_factor_size : -1,
                            delta_met_x.size(),
                            delta_met_y.size(),
                            is_fake.size()));
        }

        if (not(std::is_sorted(this->p4.cbegin(),
                               this->p4.cend(),
                               [](auto p1, auto p2) -> bool
                               {
                                   return p1.pt() > p2.pt();
                               })))
        {
            this->reorder();
        }
    }

    auto get_delta_met_x() const -> double
    {
        return ROOT::VecOps::Sum(delta_met_x);
    }

    auto get_delta_met_y() const -> double
    {
        return ROOT::VecOps::Sum(delta_met_y);
    }

    auto indexes() const -> RVec<int>
    {
        RVec<int> v(this->size());
        std::iota(v.begin(), v.end(), 0); // Fills the vector with values from 0 to N-1
        return v;
    };

    auto take_inplace(const RVec<int> &indexes) -> void
    {
        auto _scale_factor = scale_factor;
        for (const auto &[variation, vec] : _scale_factor)
        {
            _scale_factor[variation] = ROOT::VecOps::Take(vec, indexes);
        }
        scale_factor = _scale_factor;

        delta_met_x = ROOT::VecOps::Take(delta_met_x, indexes);
        delta_met_y = ROOT::VecOps::Take(delta_met_y, indexes);
        is_fake = ROOT::VecOps::Take(is_fake, indexes);
        p4 = ROOT::VecOps::Take(p4, indexes);
    }

    auto take_as_copy(const RVec<int> &indexes) -> MUSiCObjects
    {
        auto _scale_factor = scale_factor;
        for (const auto &[variation, vec] : _scale_factor)
        {
            _scale_factor[variation] = ROOT::VecOps::Take(vec, indexes);
        }

        return MUSiCObjects(ROOT::VecOps::Take(p4, indexes),          //
                            _scale_factor,                            //
                            ROOT::VecOps::Take(delta_met_x, indexes), //
                            ROOT::VecOps::Take(delta_met_y, indexes), //
                            ROOT::VecOps::Take(is_fake, indexes));
    }

    auto reorder() -> void
    {
        this->take_inplace(ROOT::VecOps::Argsort(p4,
                                                 [](auto p1, auto p2) -> bool
                                                 {
                                                     return p1.pt() > p2.pt();
                                                 }));
    }

    // get scale factors from correctionlib
    template <class T = const std::vector<std::variant<int, double, std::string>>>
    static auto get_scale_factor(const CorrectionlibRef_t &correction_ref, bool is_data, T &&values) -> double
    {
        if (not(is_data))
        {
            try
            {
                return CorrectionlibRef_t(correction_ref)->evaluate(std::forward<T>(values));
            }
            catch (const std::exception &e)
            {
                // Catch any other unexpected exceptions
                auto inputs = std::vector<std::string>{};
                for (auto &&var : correction_ref->inputs())
                {
                    inputs.push_back(fmt::format("({} as {})", var.name(), var.typeStr()));
                }
                throw std::runtime_error(
                    fmt::format("Caught an exception when trying to evaluate a scale factor from "
                                "correctionlib. Exception: {}. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                                e.what(),
                                correction_ref->name(),
                                fmt::join(inputs, " - ")));
            }
            // Catch any other unexpected exceptions
            catch (...)
            {
                auto inputs = std::vector<std::string>{};
                for (auto &&var : correction_ref->inputs())
                {
                    inputs.push_back(fmt::format("({} as {})", var.name(), var.typeStr()));
                }
                throw std::runtime_error(
                    fmt::format("Caught an unkown exception when trying to evaluate a scale factor from "
                                "correctionlib. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                                correction_ref->name(),
                                fmt::join(inputs, " - ")));
            }
        }
        return 1.;
    }

    auto size() const -> std::size_t
    {
        return p4.size();
    }

    auto clear_mask(const MUSiCObjects &other, double max_dr = 0.4, bool debug = false) -> RVec<int>
    {
        auto clear_mask = RVec<int>();

        for (std::size_t i = 0; i < this->size(); i++)
        {
            bool has_overlap = false;
            for (std::size_t j = 0; j < other.size(); j++)
            {
                if (VectorUtil::DeltaR(this->p4[i], other.p4[j]) < max_dr)
                {
                    has_overlap = true;
                    break;
                }
            }

            if (not(has_overlap))
            {
                clear_mask.push_back(i);
            }
        }

        if (debug)
        {
            fmt::print("Cleanning mask: [{}]\n", fmt::join(clear_mask, ", "));
        }

        return clear_mask;
    }

    auto static transverse_mass(const Math::PtEtaPhiMVector &vec) -> double
    {
        return std::sqrt(std::pow(vec.Et(), 2) - std::pow(vec.px(), 2) - std::pow(vec.py(), 2));
    }

    static auto get_scale_factor(const Shifts::Variations &shift,
                                 std::pair<std::size_t, const MUSiCObjects &> muons,
                                 std::pair<std::size_t, const MUSiCObjects &> electrons,
                                 std::pair<std::size_t, const MUSiCObjects &> taus,
                                 std::pair<std::size_t, const MUSiCObjects &> photons,
                                 std::pair<std::size_t, const MUSiCObjects &> bjets,
                                 std::pair<std::size_t, const MUSiCObjects &> jets,
                                 std::pair<std::size_t, const MUSiCObjects &> met) -> double
    {
        auto agg = [&](std::pair<std::size_t, const MUSiCObjects &> music_objects) -> double
        {
            auto [n, this_obj] = music_objects;

            // no objects were selected
            if (this_obj.scale_factor.empty())
            {
                return 1.;
            }

            if (this_obj.scale_factor.contains(shift))
            {
                return std::reduce(this_obj.scale_factor.at(shift).begin(),
                                   this_obj.scale_factor.at(shift).begin() + n,
                                   1.,
                                   std::multiplies<double>());
            }
            return std::reduce(this_obj.scale_factor.at(Shifts::Variations::Nominal).begin(),
                               this_obj.scale_factor.at(Shifts::Variations::Nominal).begin() + n,
                               1.,
                               std::multiplies<double>());
        };

        return agg(muons) * agg(electrons) * agg(photons) * agg(taus) * agg(bjets) * agg(jets) * agg(met);
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
                    2) +

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

    static auto push_sf_inplace(std::unordered_map<Shifts::Variations, RVec<double>> &scale_factors,
                                Shifts::Variations shift,
                                double sf) -> void
    {
        if (not(scale_factors.contains(shift)))
        {
            scale_factors[shift] = {};
        }
        scale_factors[shift].push_back(sf);
    }
};

#endif // !MUSIC_OBJECTS_HPP
