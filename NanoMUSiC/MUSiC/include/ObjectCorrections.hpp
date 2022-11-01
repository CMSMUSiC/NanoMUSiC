#ifndef MUSIC_OBJECTCORRECTIONS
#define MUSIC_OBJECTCORRECTIONS

#include "MUSiCEvent.hpp"
#include "NanoObjects.hpp"

// enum Shift
// {
//     Nominal,
//     Up,
//     Down,
//     kTotalShifts, // !!! should always be the last one !!!
// };
// enum Variation
// {
//     Default,
//     JEC,
//     JER,
//     MuonScale,
//     MuonResolution,
//     ElectronScale,
//     ElectronResolution,
//     kTotalVariations, // !!! should always be the last one !!!
// };

auto default_corr = [](const Shift &shift, const NanoObject::NanoObjectCollection &muons,
                       const NanoObject::NanoObjectCollection &electrons,
                       const NanoObject::NanoObjectCollection &photons, const NanoObject::NanoObjectCollection &taus,
                       const NanoObject::NanoObjectCollection &bjets, const NanoObject::NanoObjectCollection &jets,
                       const NanoObject::NanoObject &met) {
    return std::make_tuple(muons, electrons, photons, taus, bjets, jets, met);
};

auto get_correction(const Variation &variation)
{
    switch (variation)
    {
    case Variation::JEC:
        return default_corr;

    case Variation::JER:
        return default_corr;

    case Variation::MuonScale:
        return default_corr;

    case Variation::MuonResolution:
        return default_corr;

    case Variation::ElectronScale:
        return default_corr;

    case Variation::ElectronResolution:
        return default_corr;

    default: // Default (nominal)
        return default_corr;
    }
}

std::optional<NanoObject::NanoAODObjects_t> apply_variation(
    const Variation &variation, const Shift &shift, const bool &is_data, const NanoObject::NanoObjectCollection &muons,
    const NanoObject::NanoObjectCollection &electrons, const NanoObject::NanoObjectCollection &photons,
    const NanoObject::NanoObjectCollection &taus, const NanoObject::NanoObjectCollection &bjets,
    const NanoObject::NanoObjectCollection &jets, const NanoObject::NanoObject &met)
{
    // for now, just pass forward
    if (shift == Shift::Nominal || !is_data)
    {
        auto correction_func = get_correction(variation);
        return correction_func(shift, muons, electrons, photons, taus, bjets, jets, met);
    }
    return std::nullopt;
}

#endif /*MUSIC_OBJECTCORRECTIONS*/