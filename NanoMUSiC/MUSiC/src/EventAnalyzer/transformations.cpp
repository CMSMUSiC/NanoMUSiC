#include "EventAnalyzer.hpp"

#include <Math/Vector4D.h>
#include <utility>
#include <variant>

#include "Math/Vector4Dfwd.h"
#include "ROOT/RVec.hxx"

using namespace ROOT::Math;

/// TODO
auto EventAnalyzer::transform_muons() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

auto EventAnalyzer::transform_electrons() -> EventAnalyzer &
{
    if (*this)
    {
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
            if (good_electrons_mask[i] == 1)
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
        electrons.pt_Scale_up = new_pt_nominal_up;
        electrons.pt_Scale_down = new_pt_nominal_down;

        // JER
        electrons.pt_Resolution_up = new_pt_up_nominal;
        electrons.pt_Resolution_down = new_pt_down_nominal;

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
        // met.et_nominal[0] = std::sqrt(met_x * met_x + met_y * met_y);

        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_photons() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_taus() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_bjets_and_jets(JetCorrector &jet_corrections) -> EventAnalyzer &
{
    if (*this)
    {
        // Jets
        jet_transformer(jets, good_jets_mask, jet_corrections);
        // BJets
        jet_transformer(bjets, good_bjets_mask, jet_corrections);

        return *this;
    }
    return *this;
}

/// TODO:
auto EventAnalyzer::transform_met() -> EventAnalyzer &
{
    if (*this)
    {
        return *this;
    }
    return *this;
}