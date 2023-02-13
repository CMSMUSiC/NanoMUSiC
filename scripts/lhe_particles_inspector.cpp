#include "ROOT/RVec.hxx"
#include "TTree.h"
#include "fmt/format.h"

#include <optional>

using namespace ROOT::VecOps;

namespace LorentzVectorHelper
{

auto mass(const float &pt1,
          const float &eta1,
          const float &phi1,
          const float &mass1,
          const float &pt2,
          const float &eta2,
          const float &phi2,
          const float &mass2) -> float
{
    // Conversion from (pt, eta, phi, mass) to (x, y, z, e) coordinate system
    const auto x1 = pt1 * std::cos(phi1);
    const auto y1 = pt1 * std::sin(phi1);
    const auto z1 = pt1 * std::sinh(eta1);
    const auto e1 = std::sqrt(x1 * x1 + y1 * y1 + z1 * z1 + mass1 * mass1);

    const auto x2 = pt2 * std::cos(phi2);
    const auto y2 = pt2 * std::sin(phi2);
    const auto z2 = pt2 * std::sinh(eta2);
    const auto e2 = std::sqrt(x2 * x2 + y2 * y2 + z2 * z2 + mass2 * mass2);

    // Addition of particle four-vector elements
    const auto e = e1 + e2;
    const auto x = x1 + x2;
    const auto y = y1 + y2;
    const auto z = z1 + z2;

    return std::sqrt(e * e - x * x - y * y - z * z);
}
} // namespace LorentzVectorHelper

namespace PDG
{
namespace Electron
{
constexpr int Id = 11;
constexpr float Mass = 0.51099895 / 1000.0;
} // namespace Electron

namespace Muon
{
constexpr int Id = 13;
constexpr float Mass = 105.6583755 / 1000.0;
} // namespace Muon

namespace Tau
{
constexpr int Id = 15;
constexpr float Mass = 1776.86 / 1000.;
} // namespace Tau

namespace Bottom
{
constexpr int Id = 5;
constexpr float Mass = 4.18;
} // namespace Bottom
namespace Top
{
constexpr int Id = 6;
constexpr float Mass = 172.76;
} // namespace Top

namespace Gluon
{
constexpr int Id = 21;
constexpr float Mass = 0.;
} // namespace Gluon

namespace Photon
{
constexpr int Id = 22;
constexpr float Mass = 0;
} // namespace Photon

namespace Z
{
constexpr int Id = 23;
constexpr float Mass = 91.1876;
} // namespace Z

namespace W
{
constexpr int Id = 24;
constexpr float Mass = 80.379; // NOT the CDF result
} // namespace W

inline auto get_mass_by_id(const int &id) -> float
{
    if (std::abs(id) == Electron::Id)
    {
        return Electron::Mass;
    }
    if (std::abs(id) == Muon::Id)
    {
        return Muon::Mass;
    }
    if (std::abs(id) == Tau::Id)
    {
        return Tau::Mass;
    }

    if (std::abs(id) == Gluon::Id)
    {
        return Gluon::Mass;
    }
    if (std::abs(id) == Photon::Id)
    {
        return Photon::Mass;
    }
    if (std::abs(id) == Z::Id)
    {
        return Z::Mass;
    }
    if (std::abs(id) == W::Id)
    {
        return W::Mass;
    }

    throw(std::runtime_error(fmt::format("Invalid PDG Id ({}).", id)));
}

} // namespace PDG

void lhe_particles_inspector()
{
    // TFile *_file0 = TFile::Open(
    //     "root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL16NanoAODAPVv9/"
    //     "WGJets_MonoPhoton_PtG-40to130_TuneCP5_13TeV-madgraph-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v11-v2/"
    //     "40000/E54F6BA5-D0B0-4844-9475-94F4AB1ABE72.root");

    // TFile *_file0 = TFile::Open(
    //     "root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL18NanoAODv9/"
    //     "WGToLNuG_01J_5f_PtG_130_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v16_L1v1-v1/"
    //     "130000/69972AEE-D53B-FF40-9925-3424B94F337A.root");

    TFile *_file0 = TFile::Open(
        "root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL18NanoAODv9/WWTo2L2Nu_TuneCP5_13TeV-powheg-pythia8/"
        "NANOAODSIM/106X_upgrade2018_realistic_v16_L1v1-v2/260000/C9232127-81AC-AE46-8B94-6D3A16ACF5AA.root");

    auto t = _file0->Get<TTree>("Events");

    auto df = ROOT::RDataFrame(
        *t, {"LHEPart_pdgId", "LHEPart_status", "LHEPart_pt", "LHEPart_eta", "LHEPart_phi", "LHEPart_mass"});
    ULong64_t total = 0;
    ULong64_t accepted = 0;

    fmt::print("Starting ...\n");
    df.Foreach(
        [&](ROOT::RVec<int> pdgId,
            ROOT::RVec<int> status,
            ROOT::RVec<float> pt,
            ROOT::RVec<float> eta,
            ROOT::RVec<float> phi,
            ROOT::RVec<float> mass) {
            total++;
            // std::cout << "------------------------" << std::endl;
            // std::cout << pdgId << std::endl;
            // std::cout << status << std::endl;
            // std::cout << pt << std::endl;

            auto filter = [&](const float &mass_max) -> bool {
                // filter Lep+Lep- pair
                std::optional<std::size_t> idx_lepton_plus = std::nullopt;
                std::optional<std::size_t> idx_lepton_minus = std::nullopt;

                for (std::size_t i = 0; i < pt.size(); i++)
                {
                    if (not(idx_lepton_plus) and not(idx_lepton_minus))
                    {
                        if (pdgId.at(i) == PDG::Electron::Id //
                            or pdgId.at(i) == PDG::Muon::Id  //
                            or pdgId.at(i) == PDG::Tau::Id)
                        {
                            idx_lepton_plus = i;
                        }
                        if (pdgId.at(i) == -PDG::Electron::Id //
                            or pdgId.at(i) == -PDG::Muon::Id  //
                            or pdgId.at(i) == -PDG::Tau::Id)
                        {
                            idx_lepton_minus = i;
                        }
                    }

                    if (idx_lepton_plus and not(idx_lepton_minus))
                    {
                        if (pdgId.at(i) * pdgId.at(*idx_lepton_plus) < 0)
                        {
                            idx_lepton_minus = i;
                        }
                    }

                    if (not(idx_lepton_plus) and idx_lepton_minus)
                    {
                        if (pdgId.at(i) * pdgId.at(*idx_lepton_minus) < 0)
                        {
                            idx_lepton_plus = i;
                        }
                    }

                    if (idx_lepton_plus and idx_lepton_minus)
                    {
                        auto mass = LorentzVectorHelper::mass(pt[*idx_lepton_plus],
                                                              eta[*idx_lepton_plus],
                                                              phi[*idx_lepton_plus],
                                                              PDG::get_mass_by_id(pdgId[*idx_lepton_plus]),
                                                              pt[*idx_lepton_minus],
                                                              eta[*idx_lepton_minus],
                                                              phi[*idx_lepton_minus],
                                                              PDG::get_mass_by_id(pdgId[*idx_lepton_minus]));

                        // fmt::print("############################\n");
                        // fmt::print("pt: {}\n", pt);
                        // fmt::print("mass: {}\n", mass);

                        if (mass <= mass_max + .5)
                        {
                            return true;
                        }
                    }
                }

                // fmt::print("[ Generator Filter ] Didn't pass: DY Mass Cut. Skipping ...\n");
                return false;
            };

            if (filter(500.))
            {
                accepted++;
            }
        },
        {"LHEPart_pdgId", "LHEPart_status", "LHEPart_pt", "LHEPart_eta", "LHEPart_phi", "LHEPart_mass"});

    fmt::print("Filter result: {}\n", static_cast<float>(accepted) / static_cast<float>(total));

    std::cout << _file0->GetName() << std::endl;
}