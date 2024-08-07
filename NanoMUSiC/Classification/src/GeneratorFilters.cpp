#include "GeneratorFilters.hpp"

#include "ROOT/RVec.hxx"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace GeneratorFilters
{
auto get_filter(const std::string &filter_name) -> Filter_t
{
    auto it = filters.find(filter_name);
    if (it != filters.end())
    {
        return it->second;
    }
    else
    {
        fmt::print(stderr, "ERROR: Could not find generator filter: {}", filter_name);
        std::exit(EXIT_FAILURE);
    }
}

auto no_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, debugger_t &h_debug) -> bool
{
    return true;
}

auto wlnujets_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                     float mass_min,
                     float mass_max,
                     float pt_min,
                     float pt_max,
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
                break;
            }
        }
    }

    if (h_debug)
    {
        h_debug->fill(pt, filter_result);
    }

    return filter_result;
}

auto wlnujets_mass_binned_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                                 const NanoAODGenInfo::GenParticles &gen_particles,
                                 const Year &year,
                                 float mass_min,
                                 float mass_max,
                                 float pt_min,
                                 float pt_max,
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
        auto mother_idx = gen_particles.genPartIdxMother.at(i);
        if (mother_idx >= 0)
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
                        continue;
                    }
                }
                if (std::abs(gen_particles.pdgId.at(i)) == PDG::ElectronNeutrino::Id //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::MuonNeutrino::Id  //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::TauNeutrino::Id)
                {
                    if (std::abs(gen_particles.pdgId.at(gen_particles.genPartIdxMother.at(i))) == 24)
                    {
                        idx_neutrino = i;
                        continue;
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
                    break;
                }
            }
        }
    }

    if (h_debug)
    {
        h_debug->fill(pt, filter_result);
    }

    return filter_result;
}

auto wlnujets_mass_binned_sherpa_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                                        const NanoAODGenInfo::GenParticles &gen_particles,
                                        const Year &year,
                                        float mass_min,
                                        float mass_max,
                                        float pt_min,
                                        float pt_max,
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
        auto mother_idx = gen_particles.genPartIdxMother.at(i);
        if (mother_idx < 0)
        {
            if (not(idx_lepton) and not(idx_neutrino))
            {
                if (std::abs(gen_particles.pdgId.at(i)) == PDG::Electron::Id //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::Muon::Id  //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::Tau::Id)
                {
                    idx_lepton = i;
                    continue;
                }
                if (std::abs(gen_particles.pdgId.at(i)) == PDG::ElectronNeutrino::Id //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::MuonNeutrino::Id  //
                    or std::abs(gen_particles.pdgId.at(i)) == PDG::TauNeutrino::Id)
                {
                    idx_neutrino = i;
                    continue;
                }
            }

            if (idx_lepton and not(idx_neutrino))
            {
                if (std::abs(gen_particles.pdgId.at(i)) == std::abs(gen_particles.pdgId.at(*idx_lepton)) + 1)
                {
                    idx_neutrino = i;
                    continue;
                }
            }

            if (not(idx_lepton) and idx_neutrino)
            {
                if (std::abs(gen_particles.pdgId.at(i)) == std::abs(gen_particles.pdgId.at(*idx_neutrino)) - 1)
                {
                    idx_lepton = i;
                    continue;
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
                    break;
                }
            }
        }
    }

    if (h_debug)
    {
        h_debug->fill(pt, filter_result);
    }

    return filter_result;
}

auto dy_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
               float mass_min,
               float mass_max,
               float pt_min,
               float pt_max,
               FilterTaus filter_taus,
               debugger_t &h_debug) -> bool
{
    auto do_filter_taus = true;
    if (filter_taus == FilterTaus::DoNotFilterTaus)
    {
        do_filter_taus = false;
    }

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
                if (std::abs(lhe_particles.pdgId[*idx_lepton_minus]) == PDG::Tau::Id and not(do_filter_taus))
                {
                    mass_max = std::numeric_limits<double>::max();
                }
                else
                {
                    mass_max = mass_max + 0.5;
                }

                if ((mass_min - .5 <= mass and mass <= mass_max + .5) //
                    and (pt_min - .5 <= pt and pt <= pt_max + .5))
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

auto ttbar_filter(const NanoAODGenInfo::LHEParticles &lhe_particles,
                  float mass_min,
                  float mass_max,
                  debugger_t &h_debug) -> bool
{
    auto ttbar_mass = ROOT::VecOps::InvariantMass(ROOT::VecOps::Take(lhe_particles.pt, -6),
                                                  ROOT::VecOps::Take(lhe_particles.eta, -6),
                                                  ROOT::VecOps::Take(lhe_particles.phi, -6),
                                                  ROOT::VecOps::Take(lhe_particles.mass, -6));

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

auto wg_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, float pt_max, debugger_t &h_debug) -> bool
{
    bool filter_result = false;
    auto pt_filter = (lhe_particles.pdgId == 22) && (lhe_particles.status == 1);
    if (ROOT::VecOps::Any(lhe_particles.pt[pt_filter] <= pt_max))
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

auto ww_2l2v_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, float mass_max, debugger_t &h_debug) -> bool
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

auto gamma_jet_cleanner_filter(const NanoAODGenInfo::LHEParticles &lhe_particles, float dr_max, debugger_t &h_debug)
    -> bool
{
    bool filter_result = false;

    float dr = -999.;

    auto gamma_idx = (lhe_particles.pdgId == 22) && (lhe_particles.status == 1);
    if (ROOT::VecOps::Sum(gamma_idx) >= 1)
    {
        auto jets_idx = (lhe_particles.pdgId != 22) && (lhe_particles.status == 1);

        auto delta_phi = ROOT::VecOps::DeltaPhi((lhe_particles.phi[gamma_idx]).at(0), lhe_particles.phi[jets_idx]);
        auto delta_eta = ((lhe_particles.eta[gamma_idx]).at(0) - (lhe_particles.eta[jets_idx]));
        auto delta_r = ROOT::VecOps::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);

        dr = ROOT::VecOps::Min(delta_r);
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

} // namespace GeneratorFilters
