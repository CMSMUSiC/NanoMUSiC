#ifndef MUSIC_OBJECTS_HPP
#define MUSIC_OBJECTS_HPP

#include <any>
#include <cstdlib>
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
    RVec<Math::PtEtaPhiMVector> p4;
    RVec<double> scale_factor;
    RVec<double> scale_factor_shift;
    double delta_met_x;
    double delta_met_y;
    RVec<bool> is_fake;

    MUSiCObjects()
        : p4({}),
          scale_factor({}),
          scale_factor_shift({}),
          delta_met_x(0.),
          delta_met_y(0.),
          is_fake({})
    {
    }

    MUSiCObjects(const RVec<Math::PtEtaPhiMVector> &_p4,
                 const RVec<double> &_scale_factor,
                 const RVec<double> &_scale_factor_shift,
                 double _delta_met_x,
                 double _delta_met_y,
                 const RVec<bool> &_is_fake)
        : p4(_p4),
          scale_factor(_scale_factor),
          scale_factor_shift(_scale_factor_shift),
          delta_met_x(_delta_met_x),
          delta_met_y(_delta_met_y),
          is_fake(_is_fake)
    {

        if (not(                                           //
                p4.size() == scale_factor.size()           //
                and p4.size() == scale_factor_shift.size() //
                and p4.size() == is_fake.size()            //
                ))
        {
            fmt::print(stderr,
                       "ERROR: Could not create MUSiCObjects. Input vectors have different sizes. \n{} - {} - {} - {}",
                       p4.size(),
                       scale_factor.size(),
                       scale_factor_shift.size(),
                       is_fake.size());
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
        return delta_met_x;
    }

    auto get_delta_met_y() const -> double
    {
        return delta_met_y;
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
        for (std::size_t i = 0; i < this->size(); i++)
        {
            if (std::find(indexes.cbegin(), indexes.cend(), static_cast<int>(i)) == indexes.cend())
            {
                delta_met_x -= this->p4[i].px();
                delta_met_y -= this->p4[i].py();
            }
        }
        is_fake = ROOT::VecOps::Take(is_fake, indexes);
        p4 = ROOT::VecOps::Take(p4, indexes);
    }


    auto take_as_copy(const RVec<int> &indexes) -> MUSiCObjects
    {
        auto new_delta_met_x = delta_met_x;
        auto new_delta_met_y = delta_met_y;
        for (std::size_t i = 0; i < this->size(); i++)
        {
            if (std::find(indexes.cbegin(), indexes.cend(), static_cast<int>(i)) == indexes.cend())
            {
                new_delta_met_x -= this->p4[i].px();
                new_delta_met_y -= this->p4[i].py();
            }
        }

        return MUSiCObjects(ROOT::VecOps::Take(p4, indexes),                 //
                            ROOT::VecOps::Take(scale_factor, indexes),       //
                            ROOT::VecOps::Take(scale_factor_shift, indexes), //
                            new_delta_met_x,                                 //
                            new_delta_met_y,                                 //
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
