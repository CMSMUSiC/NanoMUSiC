#ifndef MUSIC_TRIGGER
#define MUSIC_TRIGGER

enum HLTPath
{
    SingleMuonLowPt,
    SingleMuonHighPt,
    SingleElectron,
    DoubleMuon,
    DoubleElectron,
    Tau,
    BJet,
    MET,
    Photon,
    kTotalPaths, // --> should be the last one!
};

struct TriggerBits
{
    // will have size = SIZE
    constexpr static size_t SIZE = sizeof(unsigned int) * 8;
    std::bitset<SIZE> trigger_bits;

    TriggerBits &set(unsigned int path, bool value)
    {
        trigger_bits.set(path, value);
        return *this;
    }

    bool pass(unsigned int path)
    {
        return trigger_bits.test(path);
    }

    bool any()
    {
        return trigger_bits.any();
    }

    auto as_ulong() const
    {
        return trigger_bits.to_ulong();
    }

    auto as_ulonglong() const
    {
        return trigger_bits.to_ullong();
    }

    auto as_uint() const
    {
        return static_cast<unsigned int>(trigger_bits.to_ullong());
    }

    std::string_view as_string() const
    {
        return std::string_view(std::to_string(this->as_ulong()));
    }

    template <typename T1, typename T2>
    static constexpr std::pair<std::vector<float>, std::vector<float>> get_matches(T1 &&obj1_coll, T2 &&obj2_coll)
    {
        auto obj2_coll_size = ranges::distance(obj2_coll.begin(), obj2_coll.end());
        auto matches_distances = std::vector<float>(obj2_coll_size, std::numeric_limits<float>::max());
        auto matches_relative_pT = std::vector<float>(obj2_coll_size, std::numeric_limits<float>::max());
        for (const auto &obj1 : obj1_coll)
        {
            for (const auto &idx_obj2 : views::enumerate(obj2_coll))
            {
                auto [idx, obj2] = idx_obj2;
                matches_distances.at(idx) = std::min(NanoObject::DeltaR(obj1, obj2), matches_distances.at(idx));
                matches_relative_pT.at(idx) = std::min(NanoObject::RelativePt(obj1, obj2), matches_relative_pT.at(idx));
            }
        }
        return std::make_pair(matches_distances, matches_relative_pT);
    }

    template <typename T1, typename T2>
    static constexpr std::tuple<bool, float, float, float> trigger_matcher(T1 trigger_objs, T2 nano_objs, Year &year)
    {
        bool _has_match = false;
        float _trigger_sf_nominal = 1.0;
        float _trigger_sf_up = 1.0;
        float _trigger_sf_down = 1.0;
        auto [matches_distances, matches_rel_pT] = TriggerBits::get_matches(trigger_objs, nano_objs);
        if (MUSiCTools::MinElem(matches_distances).value_or(std::numeric_limits<float>::max()) <
            ObjConfig::Muons[year].MaxDeltaRTriggerMatch)
        {
            _has_match = true;
            //////////////////////////////////////////////////////////////
            // FIX ME: Here it should evaluate the trigger SF.
            //////////////////////////////////////////////////////////////
        }
        return std::make_tuple(_has_match, _trigger_sf_nominal, _trigger_sf_up, _trigger_sf_down);
    }

    // Run2017 configurations
    // hltL3crIsoL1sMu22Or25L1f0L2f10QL3f27QL3trkIsoFiltered0p07 -> HLT_IsoMu27 - bit: 8
    // hltL3crIsoL1sSingleMu22L1f0L2f10QL3f24QL3trkIsoFiltered0p07 -> HLT_IsoMu24 - bit: 8
    // hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q -> HLT_Mu50 - bit: 1024
    // hltL3fL1sMu22Or25L1f0L2f10QL3Filtered100Q -> HLT_OldMu100 - bit: 2048
    // hltL3fL1sMu25f0TkFiltered100Q -> HLT_OldMu100 - bit: 2048

    // Single Muon - Low pT
    static constexpr bool SingleMuonLowPtBits(const int &trigger_bit, const Year &year)
    {
        switch (year)
        {
        case Year::Run2016APV:
            return (trigger_bit & 8);
        case Year::Run2016:
            return (trigger_bit & 8);
        case Year::Run2017:
            return (trigger_bit & 8);
        case Year::Run2018:
            return (trigger_bit & 8);
        default:
            throw std::runtime_error("Year (" + std::to_string(year) +
                                     ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
        }
    };

    // Single Muon - High pT
    static constexpr bool SingleMuonHighPtBits(const int &trigger_bit, const Year &year)
    {
        switch (year)
        {
        case Year::Run2016APV:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        case Year::Run2016:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        case Year::Run2017:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        case Year::Run2018:
            return ((trigger_bit & 1024) || (trigger_bit & 2048));
        default:
            throw std::runtime_error("Year (" + std::to_string(year) +
                                     ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
        }
    };

    static constexpr bool check_bit(const int &trigger_bit, const HLTPath &path, const Year &year)
    {
        switch (path)
        {
        case HLTPath::SingleMuonLowPt:
            return SingleMuonLowPtBits(trigger_bit, year);

        case HLTPath::SingleMuonHighPt:
            return SingleMuonHighPtBits(trigger_bit, year);

        default:
            throw std::runtime_error("Year (" + std::to_string(year) +
                                     ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
        }
    }
};

#endif /*MUSIC_TRIGGER*/
