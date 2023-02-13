#include "GeneratorFilters.hpp"

namespace GeneratorFilters
{
auto no_filter(const NanoObjects::LHEParticles &lhe_particles) -> bool
{
    return true;
}

auto dy_filter(const NanoObjects::LHEParticles &lhe_particles,
               const float &mass_min,
               const float &mass_max,
               const float &pt_min,
               const float &pt_max) -> bool
{
    // filter Lep+Lep- pair
    std::optional<std::size_t> idx_lepton_plus = std::nullopt;
    std::optional<std::size_t> idx_lepton_minus = std::nullopt;

    for (std::size_t i = 0; i < lhe_particles.nLHEParticles; i++)
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
            auto mass = LorentzVectorHelper::mass(lhe_particles.pt[*idx_lepton_plus],
                                                  lhe_particles.eta[*idx_lepton_plus],
                                                  lhe_particles.phi[*idx_lepton_plus],
                                                  PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_plus]),
                                                  lhe_particles.pt[*idx_lepton_minus],
                                                  lhe_particles.eta[*idx_lepton_minus],
                                                  lhe_particles.phi[*idx_lepton_minus],
                                                  PDG::get_mass_by_id(lhe_particles.pdgId[*idx_lepton_minus]));

            auto pt = LorentzVectorHelper::pt(lhe_particles.pt[*idx_lepton_plus],
                                              lhe_particles.phi[*idx_lepton_plus],
                                              lhe_particles.pt[*idx_lepton_minus],
                                              lhe_particles.phi[*idx_lepton_minus]);

            // fmt::print("############################\n");
            // fmt::print("pt: {}\n", pt);
            // fmt::print("mass: {}\n", mass);

            if ((mass >= mass_min - .5 and mass <= mass_max + .5) and (pt >= pt_min - .5 and mass <= pt_max + .5))
            {
                return true;
            }
        }
    }

    // fmt::print("[ Generator Filter ] Didn't pass: DY Mass Cut. Skipping ...\n");
    return false;
}

} // namespace GeneratorFilters
