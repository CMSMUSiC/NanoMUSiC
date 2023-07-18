#include "GeneratorFilters.hpp"

#include "ROOT/RVec.hxx"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>

namespace GeneratorFilters
{
auto no_filter(const NanoObjects::LHEParticles &lhe_particles, debugger_t &h_debug) -> bool
{
    return true;
}

auto wlnujets_filter(const NanoObjects::LHEParticles &lhe_particles,
                     const float &mass_min,
                     const float &mass_max,
                     const float &pt_min,
                     const float &pt_max,
                     debugger_t &h_debug) -> bool
{
    bool filter_result = false;
    float mass = -999.;
    float pt = -999.;

    // filter Lep+Nu pair
    std::optional<std::size_t> idx_lepton = std::nullopt;
    std::optional<std::size_t> idx_neutrino = std::nullopt;

    for (std::size_t i = 0; i < lhe_particles.nLHEParticles; i++)
    {
        if (not(idx_lepton) and not(idx_neutrino))
        {
            if (std::abs(lhe_particles.pdgId.at(i)) == PDG::Electron::Id //
                or std::abs(lhe_particles.pdgId.at(i)) == PDG::Muon::Id  //
                or std::abs(lhe_particles.pdgId.at(i)) == PDG::Tau::Id)
            {
                idx_lepton = i;
            }
            if (std::abs(lhe_particles.pdgId.at(i)) == PDG::ElectronNeutrino::Id //
                or std::abs(lhe_particles.pdgId.at(i)) == PDG::MuonNeutrino::Id  //
                or std::abs(lhe_particles.pdgId.at(i)) == PDG::TauNeutrino::Id)
            {
                idx_neutrino = i;
            }
        }

        if (idx_lepton and not(idx_neutrino))
        {
            if (std::abs(lhe_particles.pdgId.at(i)) == std::abs(lhe_particles.pdgId.at(*idx_lepton)) + 1)
            {
                idx_neutrino = i;
            }
        }

        if (not(idx_lepton) and idx_neutrino)
        {
            if (std::abs(lhe_particles.pdgId.at(i)) == std::abs(lhe_particles.pdgId.at(*idx_neutrino)) - 1)
            {
                idx_lepton = i;
            }
        }

        if (idx_lepton and idx_neutrino)
        {
            mass = LorentzVectorHelper::mass(lhe_particles.pt[*idx_lepton],
                                             lhe_particles.eta[*idx_lepton],
                                             lhe_particles.phi[*idx_lepton],
                                             PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton]),
                                             lhe_particles.pt[*idx_neutrino],
                                             lhe_particles.eta[*idx_neutrino],
                                             lhe_particles.phi[*idx_neutrino],
                                             PDG::get_mass_by_id(lhe_particles.pdgId[*idx_neutrino]));

            pt = LorentzVectorHelper::pt(lhe_particles.pt[*idx_lepton],
                                         lhe_particles.phi[*idx_lepton],
                                         lhe_particles.pt[*idx_neutrino],
                                         lhe_particles.phi[*idx_neutrino]);

            if ((mass_min - .5 <= mass and mass <= mass_max + .5) and (pt_min - .5 <= pt and pt <= pt_max + .5))
            {
                filter_result = true;
            }
        }
    }

    if (h_debug)
    {
        h_debug->h_total.Fill(mass);
        if (filter_result)
        {
            h_debug->h_pass.Fill(mass);
        }
    }

    return filter_result;
}
auto wlnujets_mass_binned_filter(const NanoObjects::LHEParticles &lhe_particles,
                                 const NanoObjects::GenParticles &gen_particles,
                                 const Year &year,
                                 const float &mass_min,
                                 const float &mass_max,
                                 const float &pt_min,
                                 const float &pt_max,
                                 debugger_t &h_debug) -> bool
{
    bool filter_result = false;

    float mass = -999.;
    float pt = -999.;

    // filter Lep+Nu pair
    std::optional<std::size_t> idx_lepton = std::nullopt;
    std::optional<std::size_t> idx_neutrino = std::nullopt;

    for (std::size_t i = 0; i < gen_particles.nGenParticles; i++)
    {
        auto mother_pdg_id = gen_particles.genPartIdxMother.at(i);
        if (mother_pdg_id >= 0)
        {
            if (not(idx_lepton) and not(idx_neutrino))
            {
                if (std::abs(gen_particles.pdgId.at(i)) == PDG::Electron::Id //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::Muon::Id  //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::Tau::Id)
                {
                    if (std::abs(gen_particles.pdgId.at(gen_particles.genPartIdxMother.at(i))) == 24)
                    {
                        idx_lepton = i;
                    }
                }
                if (std::abs(gen_particles.pdgId.at(i)) == PDG::ElectronNeutrino::Id //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::MuonNeutrino::Id  //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::TauNeutrino::Id)
                {
                    if (std::abs(gen_particles.pdgId.at(gen_particles.genPartIdxMother.at(i))) == 24)
                    {
                        idx_neutrino = i;
                    }
                }
            }

            if (idx_lepton and not(idx_neutrino))
            {
                if (std::abs(gen_particles.pdgId.at(i)) == std::abs(gen_particles.pdgId.at(*idx_lepton)) + 1)
                {
                    if (std::abs(gen_particles.pdgId.at(gen_particles.genPartIdxMother.at(i))) == 24)
                    {
                        idx_neutrino = i;
                    }
                }
            }

            if (not(idx_lepton) and idx_neutrino)
            {
                if (std::abs(gen_particles.pdgId.at(i)) == std::abs(gen_particles.pdgId.at(*idx_neutrino)) - 1)
                {
                    if (std::abs(gen_particles.pdgId.at(gen_particles.genPartIdxMother.at(i))) == 24)
                    {
                        idx_lepton = i;
                    }
                }
            }

            if (idx_lepton and idx_neutrino)
            {
                mass = LorentzVectorHelper::mass(gen_particles.pt[*idx_lepton],
                                                 gen_particles.eta[*idx_lepton],
                                                 gen_particles.phi[*idx_lepton],
                                                 PDG::get_mass_by_id(gen_particles.pdgId[*idx_lepton]),
                                                 gen_particles.pt[*idx_neutrino],
                                                 gen_particles.eta[*idx_neutrino],
                                                 gen_particles.phi[*idx_neutrino],
                                                 PDG::get_mass_by_id(gen_particles.pdgId[*idx_neutrino]));

                pt = LorentzVectorHelper::pt(gen_particles.pt[*idx_lepton],
                                             gen_particles.phi[*idx_lepton],
                                             gen_particles.pt[*idx_neutrino],
                                             gen_particles.phi[*idx_neutrino]);

                if ((pt_min - .5 <= pt and pt <= pt_max + .5) and (mass_min - .5 <= mass and mass <= mass_max + .5))
                {
                    filter_result = true;
                }
            }
        }
    }

    if (h_debug)
    {
        h_debug->h_total.Fill(mass);
        if (filter_result)
        {
            h_debug->h_pass.Fill(mass);
        }
    }

    return filter_result;
}

auto dy_filter(const NanoObjects::LHEParticles &lhe_particles,
               const float &mass_min,
               const float &mass_max,
               const float &pt_min,
               const float &pt_max,
               debugger_t &h_debug) -> bool
{
    bool filter_result = false;
    float mass = -999.;
    float pt = -999.;

    // filter Lep+Lep- pair
    std::optional<std::size_t> idx_lepton_plus = std::nullopt;
    std::optional<std::size_t> idx_lepton_minus = std::nullopt;

    for (std::size_t i = 0; i < lhe_particles.nLHEParticles; i++)
    {
        if (lhe_particles.status.at(i) > 0)
        {
            if (not(idx_lepton_plus) and not(idx_lepton_minus))
            {
                if (lhe_particles.pdgId.at(i) == PDG::Electron::Id //
                    or lhe_particles.pdgId.at(i) == PDG::Muon::Id  //
                    or lhe_particles.pdgId.at(i) == PDG::Tau::Id)
                {
                    idx_lepton_plus = i;
                }
                if (lhe_particles.pdgId.at(i) == -PDG::Electron::Id //
                    or lhe_particles.pdgId.at(i) == -PDG::Muon::Id  //
                    or lhe_particles.pdgId.at(i) == -PDG::Tau::Id)
                {
                    idx_lepton_minus = i;
                }
            }

            if (idx_lepton_plus and not(idx_lepton_minus))
            {
                if (lhe_particles.pdgId.at(i) == -lhe_particles.pdgId.at(*idx_lepton_plus))
                {
                    idx_lepton_minus = i;
                }
            }

            if (not(idx_lepton_plus) and idx_lepton_minus)
            {
                if (lhe_particles.pdgId.at(i) == -lhe_particles.pdgId.at(*idx_lepton_minus))
                {
                    idx_lepton_plus = i;
                }
            }

            if (idx_lepton_plus and idx_lepton_minus)
            {
                mass = LorentzVectorHelper::mass(lhe_particles.pt[*idx_lepton_plus],
                                                 lhe_particles.eta[*idx_lepton_plus],
                                                 lhe_particles.phi[*idx_lepton_plus],
                                                 PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_plus]),
                                                 lhe_particles.pt[*idx_lepton_minus],
                                                 lhe_particles.eta[*idx_lepton_minus],
                                                 lhe_particles.phi[*idx_lepton_minus],
                                                 PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_minus]));

                pt = LorentzVectorHelper::pt(lhe_particles.pt[*idx_lepton_plus],
                                             lhe_particles.phi[*idx_lepton_plus],
                                             lhe_particles.pt[*idx_lepton_minus],
                                             lhe_particles.phi[*idx_lepton_minus]);

                // fmt::print("############################\n");
                // fmt::print("pt: {}\n", pt);
                // fmt::print("mass: {}\n", mass);

                // allow Taus decay to leak to the high mass region
                // this covers the high mass phase-space,
                // since we do no include tau simulation in the POWHEG samples
                float actual_max_mass = mass_max;

                // skip this for now ... will be re-evaluated after presentation at 22/02/2023.
                // if (lhe_particles.pdgId[*idx_lepton_minus] == PDG::Tau::Id)
                // {
                //     actual_max_mass = max_float;
                // }

                if ((mass >= mass_min - .5 and mass <= actual_max_mass + .5) //
                    and (pt >= pt_min - .5 and pt <= pt_max + .5))
                {
                    filter_result = true;
                }
            }
        }
    }

    if (h_debug)
    {
        h_debug->h_total.Fill(mass);
        if (filter_result)
        {
            h_debug->h_pass.Fill(mass);
        }
    }

    // fmt::print("[ Generator Filter ] Didn't pass: DY Mass Cut. Skipping ...\n");
    return filter_result;
}

auto ttbar_filter(const NanoObjects::LHEParticles &lhe_particles,
                  const float &mass_min,
                  const float &mass_max,
                  debugger_t &h_debug) -> bool
{
    auto ttbar_mass = VecOps::InvariantMass(VecOps::Take(lhe_particles.pt, -6),
                                            VecOps::Take(lhe_particles.eta, -6),
                                            VecOps::Take(lhe_particles.phi, -6),
                                            VecOps::Take(lhe_particles.mass, -6));

    bool filter_result = false;
    if (std::max(0.f, mass_min - .5f) <= ttbar_mass and ttbar_mass <= mass_max + .5f)
    {
        // fmt::print("[ Generator Filter ] Pass: TTBar Mass Cut. ...\n");
        filter_result = true;
    }

    if (h_debug)
    {
        h_debug->h_total.Fill(ttbar_mass);
        if (filter_result)
        {
            h_debug->h_pass.Fill(ttbar_mass);
        }
    }

    // fmt::print("[ Generator Filter ] Didn't pass: TTBar Mass Cut. Skipping ...\n");
    return filter_result;
}

auto wg_filter(const NanoObjects::LHEParticles &lhe_particles, const float &pt_max, debugger_t &h_debug) -> bool
{
    bool filter_result = false;
    auto pt_filter = (lhe_particles.pdgId == 22) && (lhe_particles.status == 1);
    if (VecOps::Any(lhe_particles.pt[pt_filter] <= pt_max))
    {
        filter_result = true;
    }

    if (h_debug)
    {
        h_debug->h_total.Fill(lhe_particles.pt[pt_filter].at(0));
        if (filter_result)
        {
            h_debug->h_pass.Fill(lhe_particles.pt[pt_filter].at(0));
        }
    }

    return filter_result;
}

auto ww_2l2v_filter(const NanoObjects::LHEParticles &lhe_particles, const float &mass_max, debugger_t &h_debug) -> bool
{
    bool filter_result = false;
    float mass = -99.;

    // filter Lep+Lep- pair
    std::optional<std::size_t> idx_lepton_plus = std::nullopt;
    std::optional<std::size_t> idx_lepton_minus = std::nullopt;
    for (std::size_t i = 0; i < lhe_particles.nLHEParticles; i++)
    {
        if (lhe_particles.status.at(i) > 0)
        {
            if (not(idx_lepton_plus))
            {
                if (lhe_particles.pdgId.at(i) == PDG::Electron::Id //
                    or lhe_particles.pdgId.at(i) == PDG::Muon::Id  //
                    or lhe_particles.pdgId.at(i) == PDG::Tau::Id)
                {
                    idx_lepton_plus = i;
                }
            }
            if (not(idx_lepton_minus))
            {
                if (lhe_particles.pdgId.at(i) == -PDG::Electron::Id //
                    or lhe_particles.pdgId.at(i) == -PDG::Muon::Id  //
                    or lhe_particles.pdgId.at(i) == -PDG::Tau::Id)
                {
                    idx_lepton_minus = i;
                }
            }

            if (idx_lepton_plus and idx_lepton_minus)
            {
                mass = LorentzVectorHelper::mass(lhe_particles.pt[*idx_lepton_plus],
                                                 lhe_particles.eta[*idx_lepton_plus],
                                                 lhe_particles.phi[*idx_lepton_plus],
                                                 PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_plus]),
                                                 lhe_particles.pt[*idx_lepton_minus],
                                                 lhe_particles.eta[*idx_lepton_minus],
                                                 lhe_particles.phi[*idx_lepton_minus],
                                                 PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_minus]));

                if (mass <= mass_max)
                {
                    filter_result = true;
                }
            }
        }
    }

    if (h_debug)
    {
        h_debug->h_total.Fill(mass);
        if (filter_result)
        {
            h_debug->h_pass.Fill(mass);
        }
    }

    return filter_result;
}

auto gamma_jet_cleanner_filter(const NanoObjects::LHEParticles &lhe_particles, float dr_max, debugger_t &h_debug)
    -> bool
{
    bool filter_result = false;

    float dr = -999.;

    auto gamma_idx = (lhe_particles.pdgId == 22) && (lhe_particles.status == 1);
    if (VecOps::Sum(gamma_idx) >= 1)
    {
        auto jets_idx = (lhe_particles.pdgId != 22) && (lhe_particles.status == 1);

        auto delta_phi = VecOps::DeltaPhi((lhe_particles.phi[gamma_idx]).at(0), lhe_particles.phi[jets_idx]);
        auto delta_eta = ((lhe_particles.eta[gamma_idx]).at(0) - (lhe_particles.eta[jets_idx]));
        auto delta_r = VecOps::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);
        dr = VecOps::Min(delta_r);

        // fmt::print("Gamma IDX: {} - Jet IDX: {} - DeltaR: {}\n", gamma_idx, jets_idx, deltaRs);
    }

    if (dr < dr_max)
    {
        filter_result = true;
    }

    if (h_debug)
    {
        h_debug->h_total.Fill(dr);
        if (filter_result)
        {
            h_debug->h_pass.Fill(dr);
        }
    }

    return filter_result;
}

////////////////////////////////////////////
/// Not needed for now. Samples have to be requested/followed-up.
// auto zg_filter(const NanoObjects::LHEParticles &lhe_particles, const float &pt_max) -> bool
// {
//     if (VecOps::Any(lhe_particles.pt[(lhe_particles.pdgId == 22) && (lhe_particles.status == 1)] <= pt_max))
//     {
//         // fmt::print("[ Generator Filter ] Pass: TTBar Mass Cut. ...\n");
//         return true;
//     }

//     // fmt::print("[ Generator Filter ] Didn't pass: TTBar Mass Cut. Skipping ...\n");
//     return false;
// }

// auto wwto2l2nu_filter(const NanoObjects::LHEParticles &lhe_particles, const float &mass_max, debugger_t &h_debug)
//     -> bool
// {
//     // filter Lep+Lep- pair
//     std::optional<std::size_t> idx_lepton_plus = std::nullopt;
//     std::optional<std::size_t> idx_lepton_minus = std::nullopt;

//     for (std::size_t i = 0; i < lhe_particles.pt.size(); i++)
//     {
//         if (not(idx_lepton_plus) and not(idx_lepton_minus))
//         {
//             if (lhe_particles.pdgId.at(i) == PDG::Electron::Id //
//                 or lhe_particles.pdgId.at(i) == PDG::Muon::Id  //
//                 or lhe_particles.pdgId.at(i) == PDG::Tau::Id)
//             {
//                 idx_lepton_plus = i;
//             }
//             if (lhe_particles.pdgId.at(i) == -PDG::Electron::Id //
//                 or lhe_particles.pdgId.at(i) == -PDG::Muon::Id  //
//                 or lhe_particles.pdgId.at(i) == -PDG::Tau::Id)
//             {
//                 idx_lepton_minus = i;
//             }
//         }

//         if (idx_lepton_plus and not(idx_lepton_minus))
//         {
//             if (lhe_particles.pdgId.at(i) * lhe_particles.pdgId.at(*idx_lepton_plus) < 0)
//             {
//                 idx_lepton_minus = i;
//             }
//         }

//         if (not(idx_lepton_plus) and idx_lepton_minus)
//         {
//             if (lhe_particles.pdgId.at(i) * lhe_particles.pdgId.at(*idx_lepton_minus) < 0)
//             {
//                 idx_lepton_plus = i;
//             }
//         }

//         if (idx_lepton_plus and idx_lepton_minus)
//         {
//             auto mass = LorentzVectorHelper::mass(lhe_particles.pt[*idx_lepton_plus],
//                                                   lhe_particles.eta[*idx_lepton_plus],
//                                                   lhe_particles.phi[*idx_lepton_plus],
//                                                   PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_plus]),
//                                                   lhe_particles.pt[*idx_lepton_minus],
//                                                   lhe_particles.eta[*idx_lepton_minus],
//                                                   lhe_particles.phi[*idx_lepton_minus],
//                                                   PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_minus]));

//             if (mass <= mass_max + .5)
//             {
//                 return true;
//             }
//         }
//     }

//     return false;
// }

} // namespace GeneratorFilters
