#ifndef MUSIC_OBJECTCORRECTIONS
#define MUSIC_OBJECTCORRECTIONS

#include "NanoObjects.hpp"
#include "Outputs.hpp"

auto default_corr = [](const Shift &shift, const NanoObjects::NanoObjectCollection &muons,
                       const NanoObjects::NanoObjectCollection &electrons, const NanoObjects::NanoObjectCollection &photons,
                       const NanoObjects::NanoObjectCollection &taus, const NanoObjects::NanoObjectCollection &bjets,
                       const NanoObjects::NanoObjectCollection &jets, const NanoObjects::NanoObject &met) {
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

std::optional<NanoObjects::NanoAODObjects_t> apply_variation(
    const Variation &variation, const Shift &shift, const bool &is_data, const NanoObjects::NanoObjectCollection &muons,
    const NanoObjects::NanoObjectCollection &electrons, const NanoObjects::NanoObjectCollection &photons,
    const NanoObjects::NanoObjectCollection &taus, const NanoObjects::NanoObjectCollection &bjets,
    const NanoObjects::NanoObjectCollection &jets, const NanoObjects::NanoObject &met)
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