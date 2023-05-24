#include "EventAnalyzer.hpp"

#include <Math/Vector4D.h>
#include <stdexcept>
#include <utility>
#include <variant>

#include "Math/Vector4Dfwd.h"
#include "ROOT/RVec.hxx"

using namespace ROOT::Math;

/// TODO
auto EventAnalyzer::transform_muons(const std::string_view &variation) -> EventAnalyzer &
{
    if (*this)
    {
        if (variation == "nominal")
        {
            return *this;
        }
        if (variation == "scale_up")
        {
            return *this;
        }
        if (variation == "scale_down")
        {
            return *this;
        }
        if (variation == "resolution_up")
        {
            return *this;
        }
        if (variation == "resolution_down")
        {
            return *this;
        }
        throw std::runtime_error(fmt::format("ERROR: Bad variation request: {}."));
    }
    return *this;
}

///////////////////////////////////////////////////////////////
/// References:
/// https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2#Applying_the_Energy_Scale_and_sm
///
auto EventAnalyzer::transform_electrons(const std::string_view &variation) -> EventAnalyzer &
{
    if (*this)
    {
        if (variation == "nominal")
        {
            return *this;
        }
        if (variation == "scale_up")
        {
            return *this;
        }
        if (variation == "scale_down")
        {
            return *this;
        }
        if (variation == "resolution_up")
        {
            return *this;
        }
        if (variation == "resolution_down")
        {
            return *this;
        }
        throw std::runtime_error(fmt::format("ERROR: Bad variation request: {}."));
        // Scale: Up - Resolution: Nominal
        auto dX_up_nominal = RVec<float>(electrons.size);
        auto dY_up_nominal = RVec<float>(electrons.size);
        auto new_pt_up_nominal = electrons.pt;

        //  Scale: Up - Resolution: Nominal
        auto dX_down_nominal = RVec<float>(electrons.size);
        auto dY_down_nominal = RVec<float>(electrons.size);
        auto new_pt_down_nominal = electrons.pt;

        // Scale: Nominal - Resolution: Up
        auto dX_nominal_up = RVec<float>(electrons.size);
        auto dY_nominal_up = RVec<float>(electrons.size);
        auto new_pt_nominal_up = electrons.pt;

        //  Scale: Nominal - Resolution: Down
        auto dX_nominal_down = RVec<float>(electrons.size);
        auto dY_nominal_down = RVec<float>(electrons.size);
        auto new_pt_nominal_down = electrons.pt;

        auto energies = ROOT::VecOps::Map(
            VecOps::Construct<Math::PtEtaPhiMVector>(
                electrons.pt, electrons.eta, electrons.phi, ROOT::RVec<float>(electrons.size, PDG::Electron::Mass)),
            [&](const Math::PtEtaPhiMVector &this_electrons) -> float
            {
                return this_electrons.energy();
            });

        for (std::size_t i = 0; i < electrons.size; i++)
        {
            if (electrons.good_electrons_mask["nominal"][i] == 1)
            {
                // Scale: Up - Resolution: Nominal
                new_pt_up_nominal[i] = electrons.pt[i] * (1.f - electrons.dEscaleUp[i] / energies[i]);
                dX_up_nominal[i] = (new_pt_up_nominal[i] - electrons.pt[i]) * std::cos(electrons.phi[i]);
                dY_up_nominal[i] = (new_pt_up_nominal[i] - electrons.pt[i]) * std::sin(electrons.phi[i]);

                // Scale: Down - Resolution: Nominal
                new_pt_down_nominal[i] = electrons.pt[i] * (1.f - electrons.dEscaleDown[i] / energies[i]);
                dX_down_nominal[i] = (new_pt_down_nominal[i] - electrons.pt[i]) * std::cos(electrons.phi[i]);
                dY_down_nominal[i] = (new_pt_down_nominal[i] - electrons.pt[i]) * std::sin(electrons.phi[i]);

                // Scale: Nominal - Resolution: Up
                new_pt_nominal_up[i] = electrons.pt[i] * (1.f - electrons.dEsigmaUp[i] / energies[i]);
                dX_nominal_up[i] = (new_pt_nominal_up[i] - electrons.pt[i]) * std::cos(electrons.phi[i]);
                dY_nominal_up[i] = (new_pt_nominal_up[i] - electrons.pt[i]) * std::sin(electrons.phi[i]);

                // Scale: Nominal - Resolution: Down
                new_pt_nominal_down[i] = electrons.pt[i] * (1.f - electrons.dEsigmaDown[i] / energies[i]);
                dX_nominal_down[i] = (new_pt_nominal_down[i] - electrons.pt[i]) * std::cos(electrons.phi[i]);
                dY_nominal_down[i] = (new_pt_nominal_down[i] - electrons.pt[i]) * std::sin(electrons.phi[i]);
            }
        }

        // Scale
        electrons.pt_Scale_up = new_pt_up_nominal;
        electrons.pt_Scale_down = new_pt_down_nominal;

        // Resolution
        electrons.pt_Resolution_up = new_pt_nominal_up;
        electrons.pt_Resolution_down = new_pt_nominal_down;

        // MET
        auto met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_up_nominal);
        auto met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_up_nominal);
        met.Electron_Scale_up[0] = std::sqrt(met_x * met_x + met_y * met_y);

        met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_down_nominal);
        met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_down_nominal);
        met.Electron_Scale_down[0] = std::sqrt(met_x * met_x + met_y * met_y);

        met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_nominal_up);
        met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_nominal_up);
        met.Electron_Resolution_up[0] = std::sqrt(met_x * met_x + met_y * met_y);

        met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_nominal_down);
        met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_nominal_down);
        met.Electron_Resolution_down[0] = std::sqrt(met_x * met_x + met_y * met_y);

        // met_x = (met.pt[0] * std::cos(met.phi[0])) - VecOps::Sum(dX_nominal);
        // met_y = (met.pt[0] * std::sin(met.phi[0])) - VecOps::Sum(dY_nominal);
        // met.pt[0] = std::sqrt(met_x * met_x + met_y * met_y);

        return *this;
    }
    return *this;
}

///////////////////////////////////////////////////////////////
/// TODO:
/// References:
/// https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2#Applying_the_Energy_Scale_and_sm
auto EventAnalyzer::transform_photons(const std::string_view &variation) -> EventAnalyzer &
{
    if (*this)
    {
        if (variation == "nominal")
        {
            return *this;
        }
        if (variation == "scale_up")
        {
            return *this;
        }
        if (variation == "scale_down")
        {
            return *this;
        }
        if (variation == "resolution_up")
        {
            return *this;
        }
        if (variation == "resolution_down")
        {
            return *this;
        }
        throw std::runtime_error(fmt::format("ERROR: Bad variation request: {}."));
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_taus(const std::string_view &variation) -> EventAnalyzer &
{
    if (*this)
    {
        if (variation == "nominal")
        {
            return *this;
        }
        if (variation == "scale_up")
        {
            return *this;
        }
        if (variation == "scale_down")
        {
            return *this;
        }
        if (variation == "resolution_up")
        {
            return *this;
        }
        if (variation == "resolution_down")
        {
            return *this;
        }
        throw std::runtime_error(fmt::format("ERROR: Bad variation request: {}."));
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_bjets_and_jets(const std::string_view &variation, JetCorrector &jet_corrections)
    -> EventAnalyzer &
{
    if (*this)
    {
        if (variation == "nominal")
        {
            return *this;
        }
        if (variation == "scale_up")
        {
            return *this;
        }
        if (variation == "scale_down")
        {
            return *this;
        }
        if (variation == "resolution_up")
        {
            return *this;
        }
        if (variation == "resolution_down")
        {
            return *this;
        }
        throw std::runtime_error(fmt::format("ERROR: Bad variation request: {}."));
        // Jets
        jet_transformer(variation, jets, jets.good_jets_mask["nominal"], jet_corrections);
        // BJets
        jet_transformer(variation, bjets, bjets.good_jets_mask["nominal"], jet_corrections);

        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_met(const std::string_view &variation) -> EventAnalyzer &
{
    if (*this)
    {
        if (variation == "nominal")
        {
            return *this;
        }
        if (variation == "unclustered_energy_up")
        {
            return *this;
        }
        if (variation == "unclustered_energy_down")
        {
            return *this;
        }
        throw std::runtime_error(fmt::format("ERROR: Bad variation request: {}."));
        return *this;
    }
    return *this;
}