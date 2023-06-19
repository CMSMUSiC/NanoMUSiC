#ifndef MAKE_PHOTONS_HPP
#define MAKE_PHOTONS_HPP

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

#include "Configs.hpp"
#include "music_objects.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

namespace ObjectFactories
{

/////////////////////////////////////////////////////////////////////////////////////////
/// Photons ID SFs, in the correctionlib JSONs, are implemented in: UL-Photon-ID-SF
/// inputs: year (string), variation (string), WorkingPoint (string), eta_SC (real), pt (real)
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint: Loose, Medium, Tight, wp80, wp90
/// - eta: [-inf, inf)
/// - pt [20., inf)
///
/// Low pT
/// RECO: From Twiki [0]: "The scale factor to reconstruct a supercluster with H/E<0.5 is assumed to be 100%."
/// ID: Tight
/// ISO: No recomendations (already incorporated).
///
/// [0] - https://twiki.cern.ch/twiki/bin/view/CMS/EgammaRunIIRecommendations#E_gamma_RECO
///
/// Photons PixelSeed SFs, in the correctionlib JSONs, are implemented in:  UL-Photon-PixVeto-SF
/// These are the Photon Pixel Veto Scale Factors (nominal, up or down) for 2018 Ultra Legacy dataset.
/// - year: 2016preVFP, 2016postVFP, 2017, 2018
/// - variation: sf/sfup/sfdown (sfup = sf + syst, sfdown = sf - syst)
/// - WorkingPoint (SFs available for the cut-based and MVA IDs): Loose, MVA, Medium, Tight
/// - HasPixBin: For each working point of choice, they are dependent on the photon pseudorapidity and R9: Possible
/// bin choices: ['EBInc','EBHighR9','EBLowR9','EEInc','EEHighR9','EELowR9']

inline auto make_photons(const RVec<float> &Photon_pt,  //
                         const RVec<float> &Photon_eta, //
                         const RVec<float> &Photon_phi, //
                         const RVec<bool> &Photon_isScEtaEB,
                         const RVec<bool> &Photon_isScEtaEE,
                         const RVec<Int_t> &Photon_cutBased, //
                         const RVec<bool> &Photon_pixelSeed, //
                         float met_px,                       //
                         float met_py,                       //
                         std::string _year) -> RVec<Math::PtEtaPhiMVector>
{
    auto year = get_runyear(_year);
    auto photons = RVec<Math::PtEtaPhiMVector>{};

    for (std::size_t i = 0; i < Photon_pt.size(); i++)
    {
        bool is_good_photon = (Photon_pt.at(i) >= ObjConfig::Photons[year].MinPt)               //
                              && (Photon_isScEtaEB.at(i))                                       //
                              && (not Photon_isScEtaEE.at(i))                                   // only EB photons
                              && (Photon_cutBased.at(i) >= ObjConfig::Photons[year].cutBasedId) //
                              && (Photon_pixelSeed.at(i) == false);

        if (is_good_photon)
        {
            photons.push_back(Math::PtEtaPhiMVector(Photon_pt[i], Photon_eta[i], Photon_phi[i], PDG::Photon::Mass));
        }
    }
    const auto photon_reordering_mask = VecOps::Argsort(photons,
                                                        [](auto photon_1, auto photon_2) -> bool
                                                        {
                                                            return photon_1.pt() > photon_2.pt();
                                                        });

    return VecOps::Take(photons, photon_reordering_mask);
}

} // namespace ObjectFactories

#endif // !MAKE_PHOTONS_HPP
