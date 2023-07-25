#ifndef MUSIC_OBJECTS_HPP
#define MUSIC_OBJECTS_HPP

#include <any>
#include <optional>
#include <stdexcept>

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
using namespace ROOT::VecOps;

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
    RVec<double> scale_factor_up;
    RVec<double> scale_factor_down;
    double delta_met_x;
    double delta_met_y;
    RVec<bool> is_fake;

    MUSiCObjects()
        : p4({}),
          scale_factor({}),
          scale_factor_up({}),
          scale_factor_down({}),
          delta_met_x(0.),
          delta_met_y(0.),
          is_fake({})
    {
    }

    MUSiCObjects(const RVec<Math::PtEtaPhiMVector> &_p4,
                 const RVec<double> &_scale_factor,
                 const RVec<double> &_scale_factor_up,
                 const RVec<double> &_scale_factor_down,
                 double _delta_met_x,
                 double _delta_met_y,
                 const RVec<bool> &_is_fake)
        : p4(_p4),
          scale_factor(_scale_factor),
          scale_factor_up(_scale_factor_up),
          scale_factor_down(_scale_factor_down),
          delta_met_x(_delta_met_x),
          delta_met_y(_delta_met_y),
          is_fake(_is_fake)
    {

        if (not(                                          //
                p4.size() == scale_factor.size()          //
                and p4.size() == scale_factor_up.size()   //
                and p4.size() == scale_factor_down.size() //
                and p4.size() == is_fake.size()           //
                ))
        {
            throw std::runtime_error(fmt::format(
                "ERROR: Could not create MUSiCObjects. Input vector have different sizes. \n{} - {} - {} - {}",
                p4.size(),
                scale_factor.size(),
                scale_factor_up.size(),
                scale_factor_down.size(),
                is_fake.size()));
        }

        this->reorder();
    }

    auto get_delta_met_x() const -> double
    {
        return delta_met_x;
    }

    auto get_delta_met_y() const -> double
    {
        return delta_met_y;
    }

    auto take(const RVec<int> &indexes) -> void
    {
        p4 = VecOps::Take(p4, indexes);
        scale_factor = VecOps::Take(scale_factor, indexes);
        scale_factor_up = VecOps::Take(scale_factor_up, indexes);
        scale_factor_down = VecOps::Take(scale_factor_down, indexes);
        is_fake = VecOps::Take(is_fake, indexes);
    }

    auto reorder() -> void
    {
        this->take(VecOps::Argsort(p4,
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
            return CorrectionlibRef_t(correction_ref)->evaluate(std::forward<T>(values));
        }
        return 1.;
    }

    auto size() const -> std::size_t
    {
        return p4.size();
    }

    auto clear(const MUSiCObjects &other, double max_dr = 0.4) -> void
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

        this->take(clear_mask);
    }

    auto clear(MUSiCObjects *other, double max_dr = 0.4) -> void
    {
        clear(*other, max_dr);
    }
};

#endif // !MUSIC_OBJECTS_HPP