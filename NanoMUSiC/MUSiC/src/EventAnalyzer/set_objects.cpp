#include "EventAnalyzer.hpp"

// builder interface
auto EventAnalyzer::set_event_info(NanoObjects::EventInfo &&_event_info) -> EventAnalyzer &
{
    if (*this)
    {
        event_info = _event_info;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_generator_info(NanoObjects::GeneratorInfo &&_generator_info) -> EventAnalyzer &
{
    if (*this)
    {
        generator_info = _generator_info;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_lhe_info(NanoObjects::LHEInfo &&_lhe_info) -> EventAnalyzer &
{
    if (*this)
    {
        lhe_info = _lhe_info;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_gen_particles(NanoObjects::GenParticles &&_gen_particles) -> EventAnalyzer &
{
    if (*this)
    {
        gen_particles = _gen_particles;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_lhe_particles(NanoObjects::LHEParticles &&_lhe_particles) -> EventAnalyzer &
{
    if (*this)
    {
        lhe_particles = _lhe_particles;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_gen_jets(NanoObjects::GenJets &&_gen_jets) -> EventAnalyzer &
{
    if (*this)
    {
        gen_jets = _gen_jets;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_muons(NanoObjects::Muons &&_muons, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        muons = _muons;
        muons.good_muons_mask["nominal"] = mask;
        muons.good_low_pt_muons_mask["nominal"] = mask;
        muons.good_high_pt_muons_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_muons(NanoObjects::Muons &&_muons) -> EventAnalyzer &
{
    return set_muons(std::move(_muons), RVec<int>(_muons.size, 1));
}

auto EventAnalyzer::set_electrons(NanoObjects::Electrons &&_electrons, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        electrons = _electrons;
        electrons.good_electrons_mask["nominal"] = mask;
        electrons.good_low_pt_electrons_mask["nominal"] = mask;
        electrons.good_high_pt_electrons_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_electrons(NanoObjects::Electrons &&_electrons) -> EventAnalyzer &
{
    return set_electrons(std::move(_electrons), RVec<int>(_electrons.size, 1));
}

auto EventAnalyzer::set_photons(NanoObjects::Photons &&_photons, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        photons = _photons;
        photons.good_photons_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_photons(NanoObjects::Photons &&_photons) -> EventAnalyzer &
{
    return set_photons(std::move(_photons), RVec<int>(_photons.size, 1));
}

auto EventAnalyzer::set_taus(NanoObjects::Taus &&_taus, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        taus = _taus;
        taus.good_taus_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_taus(NanoObjects::Taus &&_taus) -> EventAnalyzer &
{
    return set_taus(std::move(_taus), RVec<int>(_taus.size, 1));
}

auto EventAnalyzer::set_bjets(NanoObjects::BJets &&_bjets, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        bjets = _bjets;
        bjets.good_jets_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_bjets(NanoObjects::BJets &&_bjets) -> EventAnalyzer &
{
    return set_bjets(std::move(_bjets), RVec<int>(_bjets.size, 1));
}

auto EventAnalyzer::set_jets(NanoObjects::Jets &&_jets, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        jets = _jets;
        jets.good_jets_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_jets(NanoObjects::Jets &&_jets) -> EventAnalyzer &
{
    return set_jets(std::move(_jets), RVec<int>(_jets.size, 1));
}

auto EventAnalyzer::set_met(NanoObjects::MET &&_met, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        met = _met;
        met.good_met_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_met(NanoObjects::MET &&_met) -> EventAnalyzer &
{
    return set_met(std::move(_met), RVec<int>(_met.size, 1));
}

auto EventAnalyzer::set_trgobjs(NanoObjects::TrgObjs &&_trgobjs, RVec<int> &&mask) -> EventAnalyzer &
{
    if (*this)
    {
        trgobjs = _trgobjs;
        trgobjs.good_trgobjs_mask["nominal"] = mask;
        return *this;
    }
    return *this;
}

auto EventAnalyzer::set_trgobjs(NanoObjects::TrgObjs &&_trgobjs) -> EventAnalyzer &
{
    return set_trgobjs(std::move(_trgobjs), RVec<int>(_trgobjs.size, 1));
}
