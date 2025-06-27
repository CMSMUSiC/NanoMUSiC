#ifndef MAKE_MEP_HPP
#define MAKE_MEP_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "Shifts.hpp"
#include "music_objects.hpp"
#include <cstdlib>
#include <fmt/core.h>

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

namespace ObjectFactories
{

inline auto get_unclustered_energy_shift(const Shifts::Variations shift, const double MET_MetUnclustEnUpDelta) -> double
{
    if (shift == Shifts::Variations::METDiffUnclusteredEnergy_Up)
    {
        return MET_MetUnclustEnUpDelta;
    }

    if (shift == Shifts::Variations::METDiffUnclusteredEnergy_Down)
    {
        return -MET_MetUnclustEnUpDelta;
    }

    return 0.;
}

struct METXYCorr
{
    double pt;
    double phi;

    METXYCorr(const CorrectionlibRef_t &met_xy_corr_pt_data,
              const CorrectionlibRef_t &met_xy_corr_phi_data,
              const CorrectionlibRef_t &met_xy_corr_pt_mc,
              const CorrectionlibRef_t &met_xy_corr_phi_mc,
              float original_pt,
              float original_phi,
              float npvs,
              float run,
              bool is_data)
    {
        const auto met_xy_corr_pt = met_xy_corr_pt_mc;
        const auto met_xy_corr_phi = met_xy_corr_phi_mc;
        if (is_data)
        {
            const auto met_xy_corr_pt = met_xy_corr_pt_data;
            const auto met_xy_corr_phi = met_xy_corr_phi_data;
        }

        try
        {
            pt = met_xy_corr_pt->evaluate({original_pt, original_phi, npvs, run});
        }
        catch (const std::exception &e)
        {
            // Catch any other unexpected exceptions
            auto inputs = std::vector<std::string>{};
            for (auto &&var : met_xy_corr_pt->inputs())
            {
                inputs.push_back(fmt::format("({} as {})", var.name(), var.typeStr()));
            }
            throw std::runtime_error(
                fmt::format("Caught an exception when trying to evaluate a scale factor from "
                            "correctionlib. Exception: {}. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                            e.what(),
                            met_xy_corr_pt->name(),
                            fmt::join(inputs, " - ")));
        }
        // Catch any other unexpected exceptions
        catch (...)
        {
            auto inputs = std::vector<std::string>{};
            for (auto &&var : met_xy_corr_pt->inputs())
            {
                inputs.push_back(fmt::format("({} as {})", var.name(), var.typeStr()));
            }
            throw std::runtime_error(
                fmt::format("Caught an unkown exception when trying to evaluate a scale factor from "
                            "correctionlib. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                            met_xy_corr_pt->name(),
                            fmt::join(inputs, " - ")));
        }

        try
        {
            phi = met_xy_corr_phi->evaluate({original_pt, original_phi, npvs, run});
        }
        catch (const std::exception &e)
        {
            // Catch any other unexpected exceptions
            auto inputs = std::vector<std::string>{};
            for (auto &&var : met_xy_corr_phi->inputs())
            {
                inputs.push_back(fmt::format("({} as {})", var.name(), var.typeStr()));
            }
            throw std::runtime_error( fmt::format(
                       "Caught an exception when trying to evaluate a scale factor from "
                       "correctionlib. Exception: {}. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                       e.what(),
                       met_xy_corr_phi->name(),
                       fmt::join(inputs, " - ")) );
        }
        // Catch any other unexpected exceptions
        catch (...)
        {
            auto inputs = std::vector<std::string>{};
            for (auto &&var : met_xy_corr_phi->inputs())
            {
                inputs.push_back(fmt::format("({} as {})", var.name(), var.typeStr()));
            }
            throw std::runtime_error(
                fmt::format("Caught an unkown exception when trying to evaluate a scale factor from "
                            "correctionlib. Correctionlib Ref: {}. Expected inputs: [{}].\n",
                            met_xy_corr_phi->name(),
                            fmt::join(inputs, " - ")));
        }
    }
};

inline auto make_met(const double MET_pt,                            //
                     const double MET_phi,                           //
                     const double MET_MetUnclustEnUpDeltaX,          //
                     const double MET_MetUnclustEnUpDeltaY,          //
                     const double delta_met_px_from_muons,           //
                     const double delta_met_py_from_muons,           //
                     const double delta_met_px_from_electrons,       //
                     const double delta_met_py_from_electrons,       //
                     const double delta_met_px_from_taus,            //
                     const double delta_met_py_from_taus,            //
                     const double delta_met_px_from_photons,         //
                     const double delta_met_py_from_photons,         //
                     const double delta_met_px_from_jets,            //
                     const double delta_met_py_from_jets,            //
                     const double delta_met_px_from_bjets,           //
                     const double delta_met_py_from_bjets,           //
                     const CorrectionlibRef_t &met_xy_corr_pt_data,  //
                     const CorrectionlibRef_t &met_xy_corr_phi_data, //
                     const CorrectionlibRef_t &met_xy_corr_pt_mc,    //
                     const CorrectionlibRef_t &met_xy_corr_phi_mc,   //
                     const int npvs,                                 //
                     unsigned int run,                               //
                     bool is_data,                                   //
                     const std::string &_year,                       //
                     const Shifts::Variations shift) -> std::pair<MUSiCObjects, bool>
{

    auto year = get_runyear(_year);
    auto met_p4 = RVec<Math::PtEtaPhiMVector>{};
    auto scale_factors = std::unordered_map<Shifts::Variations, RVec<double>>{};
    auto scale_factor_shift = RVec<double>{};
    auto delta_met_x = RVec<double>{};
    auto delta_met_y = RVec<double>{};
    auto is_fake = RVec<bool>{};

    bool is_fake_met = false;
    if (MET_pt > 6500.)
    {
        is_fake_met = true;
    }

    if (not(is_fake_met))
    {
        auto corrected_xy_met = METXYCorr(met_xy_corr_pt_data,
                                          met_xy_corr_phi_data,
                                          met_xy_corr_pt_mc,
                                          met_xy_corr_phi_mc,
                                          MET_pt,
                                          MET_phi,
                                          static_cast<float>(npvs),
                                          static_cast<float>(run),
                                          is_data);

        auto met_px = corrected_xy_met.pt * std::cos(corrected_xy_met.phi) //
                      - delta_met_px_from_muons                            //
                      - delta_met_px_from_electrons                        //
                      - delta_met_px_from_taus                             //
                      - delta_met_px_from_photons                          //
                      - delta_met_px_from_jets                             //
                      - delta_met_px_from_bjets                            //
                      + get_unclustered_energy_shift(shift, MET_MetUnclustEnUpDeltaX);

        auto met_py = corrected_xy_met.pt * std::sin(corrected_xy_met.phi) //
                      - delta_met_py_from_muons                            //
                      - delta_met_py_from_electrons                        //
                      - delta_met_py_from_taus                             //
                      - delta_met_py_from_photons                          //
                      - delta_met_py_from_jets                             //
                      - delta_met_py_from_bjets                            //
                      + get_unclustered_energy_shift(shift, MET_MetUnclustEnUpDeltaY);

        auto this_met = Math::PxPyPzMVector(met_px, met_py, 0., 0.);

        bool is_good_met = this_met.pt() >= ObjConfig::MET[year].MediumPt;

        if (is_good_met)
        {

            if (Shifts::is_MET_diff(shift))
            {
                MUSiCObjects::push_sf_inplace(scale_factors, shift, 1.);
            }
            else
            {
                MUSiCObjects::push_sf_inplace(scale_factors, Shifts::Variations::Nominal, 1.);
            }

            delta_met_x.push_back(0.);
            delta_met_y.push_back(0.);
            met_p4.push_back(Math::PtEtaPhiMVector(this_met));

            is_fake.push_back(false);
        }
    }

    return {MUSiCObjects(met_p4,        //
                         scale_factors, //
                         delta_met_x,   //
                         delta_met_y,   //
                         is_fake),
            is_fake_met};
}

} // namespace ObjectFactories

#endif // !MAKE_MEP_HPP
