#ifndef MUSIC_OBJECTS_HPP
#define MUSIC_OBJECTS_HPP

#include <any>
#include <cstdlib>
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

class MUSiCObjects
{
  public:
    class IdScore
    {
      public:
        static constexpr unsigned int Loose = 1;
        static constexpr unsigned int Medium = 2 | 1;
        static constexpr unsigned int Tight = 4 | 2 | 1;
        static constexpr unsigned int VTight = 8 | 4 | 2 | 1;

        unsigned int num_loose = 0;
        unsigned int num_medium = 0;
        unsigned int num_tight = 0;
        unsigned int num_vtight = 0;

        auto to_string() const -> std::string
        {
            return fmt::format(
                "Loose: {} - Medium: {} - Tight: {} - Very Tight: {}", num_loose, num_medium, num_tight, num_vtight);
        }

        auto operator+(const IdScore &other) const -> IdScore
        {
            return IdScore{.num_loose = num_loose + other.num_loose,
                           .num_medium = num_medium + other.num_medium,
                           .num_tight = num_tight + other.num_tight,
                           .num_vtight = num_vtight + other.num_vtight};
        }

        static auto is_loose(unsigned int score) -> bool
        {
            return (score & Loose) == Loose;
        }

        static auto is_medium(unsigned int score) -> bool
        {
            return (score & Medium) == Medium;
        }

        static auto is_tight(unsigned int score) -> bool
        {
            return (score & Tight) == Tight;
        }

        static auto is_vtight(unsigned int score) -> bool
        {
            return (score & VTight) == VTight;
        }

        static auto accum_score(const RVec<unsigned int> &scores, const unsigned int num_obj) -> IdScore
        {
            return std::accumulate(scores.cbegin(),
                                   scores.cbegin() + num_obj,
                                   IdScore{},
                                   [](IdScore accum, auto s) -> IdScore
                                   {
                                       if ((s & Loose) == Loose)
                                       {
                                           accum.num_loose += 1;
                                       }
                                       if ((s & Medium) == Medium)
                                       {
                                           accum.num_medium += 1;
                                       }
                                       if ((s & Tight) == Tight)
                                       {
                                           accum.num_tight += 1;
                                       }
                                       if ((s & VTight) == VTight)
                                       {
                                           accum.num_vtight += 1;
                                       }

                                       return accum;
                                   });
        };
    };

    RVec<Math::PtEtaPhiMVector> p4;
    RVec<double> scale_factor;
    RVec<double> scale_factor_shift;
    RVec<double> delta_met_x;
    RVec<double> delta_met_y;
    RVec<bool> is_fake;
    RVec<unsigned int> id_score;

    MUSiCObjects()
        : p4({}),
          scale_factor({}),
          scale_factor_shift({}),
          delta_met_x({}),
          delta_met_y({}),
          is_fake({}),
          id_score({})
    {
    }

    MUSiCObjects(const RVec<Math::PtEtaPhiMVector> &_p4,
                 const RVec<double> &_scale_factor,
                 const RVec<double> &_scale_factor_shift,
                 const RVec<double> &_delta_met_x,
                 const RVec<double> &_delta_met_y,
                 const RVec<bool> &_is_fake,
                 const RVec<unsigned int> &_id_score)
        : p4(_p4),
          scale_factor(_scale_factor),
          scale_factor_shift(_scale_factor_shift),
          delta_met_x(_delta_met_x),
          delta_met_y(_delta_met_y),
          is_fake(_is_fake),
          id_score(_id_score)
    {
        if (not(                                           //
                p4.size() == scale_factor.size()           //
                and p4.size() == scale_factor_shift.size() //
                and p4.size() == delta_met_x.size()        //
                and p4.size() == delta_met_y.size()        //
                and p4.size() == is_fake.size()            //
                and p4.size() == id_score.size()           //
                ))
        {
            fmt::print(stderr,
                       "ERROR: Could not create MUSiCObjects. Input vectors have different sizes. {} - {} - {} - {} - "
                       "{} - {} - {}\n",
                       p4.size(),
                       scale_factor.size(),
                       scale_factor_shift.size(),
                       delta_met_x.size(),
                       delta_met_y.size(),
                       is_fake.size(),
                       id_score.size());
            std::exit(EXIT_FAILURE);
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
        scale_factor = ROOT::VecOps::Take(scale_factor, indexes);
        scale_factor_shift = ROOT::VecOps::Take(scale_factor_shift, indexes);
        delta_met_x = ROOT::VecOps::Take(delta_met_x, indexes);
        delta_met_y = ROOT::VecOps::Take(delta_met_y, indexes);
        is_fake = ROOT::VecOps::Take(is_fake, indexes);
        p4 = ROOT::VecOps::Take(p4, indexes);
        id_score = ROOT::VecOps::Take(id_score, indexes);
    }

    auto take_as_copy(const RVec<int> &indexes) -> MUSiCObjects
    {
        return MUSiCObjects(ROOT::VecOps::Take(p4, indexes),                 //
                            ROOT::VecOps::Take(scale_factor, indexes),       //
                            ROOT::VecOps::Take(scale_factor_shift, indexes), //
                            ROOT::VecOps::Take(delta_met_x, indexes),
                            ROOT::VecOps::Take(delta_met_y, indexes),
                            ROOT::VecOps::Take(is_fake, indexes),
                            ROOT::VecOps::Take(id_score, indexes));
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
                fmt::print(stderr,
                           "ERROR: Caught an exception when trying to evaluate a scale factor from "
                           "correctionlib. Exception: {}. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                           e.what(),
                           correction_ref->name(),
                           fmt::join(inputs, " - "));
                std::exit(EXIT_FAILURE);
            }
            // Catch any other unexpected exceptions
            catch (...)
            {
                auto inputs = std::vector<std::string>{};
                for (auto &&var : correction_ref->inputs())
                {
                    inputs.push_back(fmt::format("({} as {})", var.name(), var.typeStr()));
                }
                fmt::print(stderr,
                           "ERROR: Caught an unkown exception when trying to evaluate a scale factor from "
                           "correctionlib. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                           correction_ref->name(),
                           fmt::join(inputs, " - "));
                std::exit(EXIT_FAILURE);
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

    // auto clear(MUSiCObjects *other, double max_dr = 0.4) -> void
    // {
    //     clear(*other, max_dr);
    // }

    auto static transverse_mass(const Math::PtEtaPhiMVector &vec) -> double
    {
        return std::sqrt(std::pow(vec.e(), 2) - std::pow(vec.px(), 2) - std::pow(vec.py(), 2));
    }
};

#endif // !MUSIC_OBJECTS_HPP
