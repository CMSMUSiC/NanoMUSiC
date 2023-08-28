#ifndef TRIGGER_MATCH_HPP
#define TRIGGER_MATCH_HPP

#include "ObjectFactories/music_objects.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

/// find trigger matching
class TriggerMatch
{
  public:
    std::vector<double> matched_pt;
    std::vector<double> matched_eta;

    TriggerMatch(std::vector<double> &&_matched_pt, std::vector<double> &&_matched_eta)
        : matched_pt(_matched_pt),
          matched_eta(_matched_eta)
    {
    }

    auto get_matched_pt(std::size_t index) const -> double
    {
        return matched_pt.at(index);
    }

    auto get_matched_eta(std::size_t index) const -> double
    {
        return matched_eta.at(index);
    }
};

inline auto make_trigger_matches(const std::unordered_map<std::string, bool> &is_good_trigger_map,
                                 const MUSiCObjects &muons,
                                 const MUSiCObjects &electrons,
                                 const MUSiCObjects &taus,
                                 const MUSiCObjects &photons,
                                 Year year) -> std::unordered_map<std::string, std::optional<TriggerMatch>>
{
    auto matches = std::unordered_map<std::string, std::optional<TriggerMatch>>();

    // Low pT muon trigger
    if (is_good_trigger_map.at("pass_low_pt_muon_trigger") //
        and muons.size() >= 1)
    {
        auto good_muons = VecOps::Filter(muons.p4,
                                         [year](const auto &muon)
                                         {
                                             if (year == Year::Run2017)
                                             {
                                                 return muon.pt() > 29.;
                                             }
                                             return muon.pt() > 26.;
                                         });
        if (good_muons.size() >= 1)
        {
            matches.insert({"pass_low_pt_muon_trigger", TriggerMatch({good_muons[0].pt()}, {good_muons[0].eta()})});
        }
        else
        {
            matches.insert({"pass_low_pt_muon_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_low_pt_muon_trigger", std::nullopt});
    }

    // High pT muon trigger
    if (is_good_trigger_map.at("pass_high_pt_muon_trigger") //
        and muons.size() >= 1)
    {
        auto good_muons = VecOps::Filter(muons.p4,
                                         [](const auto &muon)
                                         {
                                             return muon.pt() > 53.;
                                             // return muon.pt() > 205.;
                                         });
        if (good_muons.size() >= 1)
        {
            matches.insert({"pass_high_pt_muon_trigger", TriggerMatch({good_muons[0].pt()}, {good_muons[0].eta()})});
        }
        else
        {
            matches.insert({"pass_high_pt_muon_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_high_pt_muon_trigger", std::nullopt});
    }

    // double muon trigger
    if (is_good_trigger_map.at("pass_double_muon_trigger") //
        and muons.size() >= 2)
    {
        auto good_muons = VecOps::Filter(muons.p4,
                                         [](const auto &muon)
                                         {
                                             return muon.pt() > 21.;
                                         });
        if (good_muons.size() >= 2)
        {
            matches.insert(
                {"pass_double_muon_trigger",
                 TriggerMatch({good_muons[0].pt(), good_muons[1].pt()}, {good_muons[0].eta(), good_muons[1].eta()})});
        }
        else
        {
            matches.insert({"pass_double_muon_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_double_muon_trigger", std::nullopt});
    }

    // Low pT electron trigger
    if (is_good_trigger_map.at("pass_low_pt_electron_trigger") //
        and electrons.size() >= 1)
    {
        auto good_electrons = VecOps::Filter(electrons.p4,
                                             [year](const auto &electron)
                                             {
                                                 if (year == Year::Run2016APV or year == Year::Run2016)
                                                 {
                                                     return electron.pt() > 35.;
                                                 }
                                                 if (year == Year::Run2017)
                                                 {
                                                     return electron.pt() > 42.;
                                                 }
                                                 return electron.pt() > 40.;
                                             });
        if (good_electrons.size() >= 1)
        {
            matches.insert(
                {"pass_low_pt_electron_trigger", TriggerMatch({good_electrons[0].pt()}, {good_electrons[0].eta()})});
        }
        else
        {
            matches.insert({"pass_low_pt_electron_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_low_pt_electron_trigger", std::nullopt});
    }

    // High pT electron trigger
    if (is_good_trigger_map.at("pass_high_pt_electron_trigger") //
        and electrons.size() >= 1)
    {
        auto good_electrons = VecOps::Filter(electrons.p4,
                                             [](const auto &electron)
                                             {
                                                 return electron.pt() > 120.;
                                             });
        if (good_electrons.size() >= 1)
        {
            matches.insert(
                {"pass_high_pt_electron_trigger", TriggerMatch({good_electrons[0].pt()}, {good_electrons[0].eta()})});
        }
        else
        {
            matches.insert({"pass_high_pt_electron_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_high_pt_electron_trigger", std::nullopt});
    }

    // double electron trigger
    if (is_good_trigger_map.at("pass_double_electron_trigger") //
        and electrons.size() >= 2)
    {
        auto good_electrons = VecOps::Filter(electrons.p4,
                                             [](const auto &electron)
                                             {
                                                 //  if (year == Year::Run2018)
                                                 //  {
                                                 //      return electron.pt() > 32.;
                                                 //  }

                                                 return electron.pt() > 40.;
                                             });
        if (good_electrons.size() >= 2)
        {
            matches.insert({"pass_double_electron_trigger",
                            TriggerMatch({good_electrons[0].pt(), good_electrons[1].pt()},
                                         {good_electrons[0].eta(), good_electrons[1].eta()})});
        }
        else
        {
            matches.insert({"pass_double_electron_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_double_electron_trigger", std::nullopt});
    }

    // photon trigger
    if (is_good_trigger_map.at("pass_photon_trigger") //
        and photons.size() >= 1)
    {
        auto good_photons = VecOps::Filter(photons.p4,
                                           [year](const auto &photon)
                                           {
                                               if (year == Year::Run2016APV or year == Year::Run2016)
                                               {
                                                   return photon.pt() > 200.;
                                               }
                                               return photon.pt() > 225.;
                                           });
        if (good_photons.size() >= 1)
        {
            matches.insert({"pass_photon_trigger", TriggerMatch({good_photons[0].pt()}, {good_photons[0].eta()})});
        }
        else
        {
            matches.insert({"pass_photon_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_photon_trigger", std::nullopt});
    }

    // tau trigger
    if (is_good_trigger_map.at("pass_high_pt_tau_trigger") //
        and taus.size() >= 1)
    {
        auto good_taus = VecOps::Filter(taus.p4,
                                        [year](const auto &tau)
                                        {
                                            if (year == Year::Run2016APV or year == Year::Run2016)
                                            {
                                                return tau.pt() > 160.;
                                            }
                                            return tau.pt() > 200.;
                                        });
        if (good_taus.size() >= 1)
        {
            matches.insert({"pass_high_pt_tau_trigger", TriggerMatch({good_taus[0].pt()}, {good_taus[0].eta()})});
        }
        else
        {
            matches.insert({"pass_high_pt_tau_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_high_pt_tau_trigger", std::nullopt});
    }

    if (is_good_trigger_map.at("pass_double_tau_trigger") //
        and taus.size() >= 2)
    {
        auto good_taus = VecOps::Filter(taus.p4,
                                        [year](const auto &tau)
                                        {
                                            if (year == Year::Run2016APV or year == Year::Run2016)
                                            {
                                                return tau.pt() > 45.;
                                            }

                                            if (year == Year::Run2017)
                                            {
                                                return tau.pt() > 50.;
                                            }
                                            return tau.pt() > 50.;
                                        });
        if (good_taus.size() >= 2)
        {
            matches.insert(
                {"pass_double_tau_trigger",
                 TriggerMatch({good_taus[0].pt(), good_taus[1].pt()}, {good_taus[0].eta(), good_taus[1].eta()})});
        }
        else
        {
            matches.insert({"pass_double_tau_trigger", std::nullopt});
        }
    }
    else
    {
        matches.insert({"pass_double_tau_trigger", std::nullopt});
    }

    return matches;
}

inline auto has_good_match(const std::unordered_map<std::string, std::optional<TriggerMatch>> &trigger_matches,
                           std::size_t n_muons,
                           std::size_t n_electrons,
                           std::size_t n_photons) -> bool
{
    if (n_muons == 1)
    {
        return (trigger_matches.at("pass_low_pt_muon_trigger") //
                or trigger_matches.at("pass_high_pt_muon_trigger"));
    }

    if (n_muons >= 2)
    {
        return (trigger_matches.at("pass_low_pt_muon_trigger")     //
                or trigger_matches.at("pass_high_pt_muon_trigger") //
                or trigger_matches.at("pass_double_muon_trigger"));
    }

    if (n_electrons == 1)
    {
        return (trigger_matches.at("pass_low_pt_electron_trigger") or //
                trigger_matches.at("pass_high_pt_electron_trigger"));
    }

    if (n_electrons >= 2)
    {
        return (trigger_matches.at("pass_low_pt_electron_trigger") or  //
                trigger_matches.at("pass_high_pt_electron_trigger") or //
                trigger_matches.at("pass_double_electron_trigger"));
    }

    if (n_photons >= 1)
    {
        return trigger_matches.at("pass_photon_trigger").has_value();
    }

    return false;
}

#endif // !TRIGGER_MATCH_HPP