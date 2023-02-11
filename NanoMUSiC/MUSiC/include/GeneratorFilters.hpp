#ifndef GENERATOR_FILTERS
#include <optional>

#include <functional>
#include <map>
#include <string>
#include <variant>

#include <fmt/format.h>

#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "NanoObjects.hpp"

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

namespace GeneratorFilters
{

// - GenPart_mass - Vector < Float_t >: Description: Mass stored for all particles with the exception of quarks (except
// top), leptons/neutrinos, photons with mass < 1 GeV, gluons, pi0(111), pi+(211), D0(421), and D+(411). For these
// particles, you can lookup the value from PDG.
//
// - GenPart_genPartIdxMother - Vector < Int_t >: Description: index of the mother particle
//
// - GenPart_pdgId - Vector < Int_t >
//
// - GenPart_status - Vector < Int_t > Description: Particle status  1=stable
//
// - GenPart_statusFlags - Vector < Int_t >:
// Description: gen status flags stored bitwise, bits are:
// 0 : isPrompt
// 1 : isDecayedLeptonHadron
// 2 : isTauDecayProduct
// 3 : isPromptTauDecayProduct
// 4 : isDirectTauDecayProduct
// 5 : isDirectPromptTauDecayProduct
// 6 : isDirectHadronDecayProduct
// 7 : isHardProcess
// 8 : fromHardProcess
// 9 : isHardProcessTauDecayProduct
// 10 : isDirectHardProcessTauDecayProduc
// 11 : fromHardProcessBeforeFS
// 12 : isFirstCopy
// 13 : isLastCopy
// 14 : isLastCopyBeforeFS

using Inputs_t = std::vector<std::variant<std::string, float, double, bool, int, NanoObjects::GenParticles>>;

template <typename T>
auto get_arg(const Inputs_t &args, std::size_t index, const std::string &error_message = "") -> T
{
    try
    {
        return std::get<T>(args.at(index));
    }
    catch (const std::exception &e)
    {
        fmt::print("Error trying to load arguments for a Generator Filter.\n [ {} ] {}", error_message, e.what());
        exit(-1);
    }
}

inline auto dy_mass_filter(const NanoObjects::GenParticles &gen_particles, const float &mass_min, const float &mass_max)
    -> bool
{
    // auto gen_particles = get_arg<NanoObjects::GenParticles>(args, 0, "NanoObjets");

    // auto mass_min = get_arg<float>(args, 1, "mass_min");
    // auto mass_max = get_arg<float>(args, 2, "mass_max");

    // filter by Z or gamma mass
    auto is_Z = gen_particles.pdgId == PDG::Z::Id;
    auto is_Gamma = gen_particles.pdgId == PDG::Photon::Id;
    auto mass_lower_bound = gen_particles.mass >= mass_min;
    auto mass_upper_bound = gen_particles.mass <= mass_max;

    auto filtered_masses = gen_particles.mass[(is_Z || is_Gamma) && mass_lower_bound && mass_upper_bound];
    auto filtered_ids = gen_particles.pdgId[(is_Z || is_Gamma) && mass_lower_bound && mass_upper_bound];

    if (filtered_masses.size() > 0)
    {
        return true;
    }
    // fmt::print("------------------------\n");
    // fmt::print("pdgIds: {}\n", gen_particles.pdgId);
    // fmt::print("mass: {}\n", gen_particles.mass);
    // fmt::print("filtered_masses: {}\n", filtered_masses);
    // fmt::print("filtered_ids: {}\n", filtered_ids);

    // filter by Z/gamma or Lep+Lep- pair
    // std::optional<std::size_t> idx_Z_or_gamma = std::nullopt;
    std::optional<std::size_t> idx_lepton_plus = std::nullopt;
    std::optional<std::size_t> idx_lepton_minus = std::nullopt;

    for (std::size_t i = 0; i < gen_particles.nGenParticles; i++)
    {
        // if (not idx_Z_or_gamma)
        // {
        // }
        if (not(idx_lepton_plus) and not(idx_lepton_minus))
        {
            if (gen_particles.pdgId.at(i) == PDG::Electron::Id //
                or gen_particles.pdgId.at(i) == PDG::Muon::Id  //
                or gen_particles.pdgId.at(i) == PDG::Tau::Id)
            {
                idx_lepton_plus = i;
            }
            if (gen_particles.pdgId.at(i) == -PDG::Electron::Id //
                or gen_particles.pdgId.at(i) == -PDG::Muon::Id  //
                or gen_particles.pdgId.at(i) == -PDG::Tau::Id)
            {
                idx_lepton_minus = i;
            }
        }

        if (idx_lepton_plus and not(idx_lepton_minus))
        {
            if (gen_particles.pdgId.at(i) == -gen_particles.pdgId.at(*idx_lepton_plus))
            {
                idx_lepton_minus = i;
            }
        }

        if (not(idx_lepton_plus) and idx_lepton_minus)
        {
            if (gen_particles.pdgId.at(i) == -gen_particles.pdgId.at(*idx_lepton_minus))
            {
                idx_lepton_plus = i;
            }
        }
        if (idx_lepton_plus and idx_lepton_minus)
        {
            // fmt::print(
            //     "idx_lepton: {} - idx_anti_lepton: {}\n", idx_lepton.value_or(-999), idx_anti_lepton.value_or(-999));
            // fmt::print("pdgIds: {}\n", gen_particles.pdgId);

            auto mass = LorentzVectorHelper::mass(gen_particles.pt[*idx_lepton_plus],
                                                  gen_particles.eta[*idx_lepton_plus],
                                                  gen_particles.phi[*idx_lepton_plus],
                                                  PDG::get_mass_by_id(gen_particles.pdgId[*idx_lepton_plus]),
                                                  gen_particles.pt[*idx_lepton_minus],
                                                  gen_particles.eta[*idx_lepton_minus],
                                                  gen_particles.phi[*idx_lepton_minus],
                                                  PDG::get_mass_by_id(gen_particles.pdgId[*idx_lepton_minus]));

            // fmt::print("--> mass: {}\n", mass);

            if (mass >= mass_min - .5 and mass <= mass_max + .5)
            {
                // fmt::print("#############################\n");
                // fmt::print("--> GOOD mass: {}\n", mass);
                // fmt::print("gen_particles.pdgId: {}\n", gen_particles.pdgId);
                // fmt::print("idx plus: {}\n", *idx_lepton_plus);
                // fmt::print("idx minus: {}\n", *idx_lepton_minus);
                // fmt::print("#############################\n");
                return true;
            }
        }
    }

    // fmt::print("------------------------\n");
    // fmt::print("------------------------\n");
    // fmt::print("[ Generator Filter ] Didn't pass: DY Mass Cut. Skipping ...\n");
    return false;
}

inline auto dy_pt_filter(const Inputs_t &args) -> bool
{
    return true;
}

const std::map<std::string, std::function<bool(const NanoObjects::GenParticles &)>> filters = {
    {"DYJetsToLL_M-50_13TeV_AM"s,
     [](const NanoObjects::GenParticles &gen_particles) -> bool {
         return dy_mass_filter(gen_particles, 50., 120.);
     }}, //
};

} // namespace GeneratorFilters

#define GENERATOR_FILTERS
#endif // GENERATOR_FILTERS