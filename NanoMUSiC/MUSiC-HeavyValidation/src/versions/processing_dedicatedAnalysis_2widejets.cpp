// NAME: DIJET WIDE JETS

// ALSO COMMENT THE TRIGGER SUMPT/LEADING JET REQUIREMENT AND LEPTON VETO!!!

// CHECK FOR AT LEAST TWO JETS
if(not(jets_4vec.size() >= 2))
{
    continue;
}

// SELECT 2 LEADING JETS AS SEED JETS
auto seed_jets = RVec<Math::PtEtaPhiMVector>{};   // jet seeds
seed_jets.push_back(jets_4vec.at(0)); // select two leading jets as seeds
seed_jets.push_back(jets_4vec.at(1));
auto noseed_jets = RVec<Math::PtEtaPhiMVector>{}; // jets that are no seeds
if(jets_4vec.size() > 2) // all other jets are noseed jets
{
    for (size_t i = 2; i < jets_4vec.size(); i++)
    {
        noseed_jets.push_back(jets_4vec.at(i));
    }
}

// JET MERGING TO WIDE JETS
auto widejets = seed_jets; // wide jets
if (noseed_jets.size() > 0 and seed_jets.size() > 0) // try to merge if seed and noseed jets were found
{
    for (size_t i = 0; i < noseed_jets.size(); i++) // try to merge for each noseed jet
    {
        auto cur_jet = noseed_jets.at(i);
        auto all_delta_r = RVec<float>{};
        for (size_t j = 0; j < seed_jets.size(); j++) // try to merge to every seed jet
        {
            // calculate distances to wide jets
            all_delta_r.push_back(std::abs(Math::VectorUtil::DeltaR(seed_jets.at(j), cur_jet)));
        }
        // sort after shortest distance
        auto sortidx = VecOps::Argsort(all_delta_r, // sort, smallest element first
                        [](auto p1, auto p2) -> bool
                        {
                            return p1 < p2;
                        });
        if(all_delta_r.at(sortidx.at(0)) < 1.1) // check if smallest deltar is < 1.1
        {
            widejets.at(sortidx.at(0)) += cur_jet; // if so, add the current jet to the closest widejet
        }
    }
}

// 2 WIDEJET DELTA ETA REQUIREMENT
if(not(std::abs(widejets.at(0).eta() - widejets.at(1).eta()) < 1.1)) // delta eta < 1.1 to accept event
{
    continue;
}

// 2 WIDEJET INVARIANT MASS CUT
auto widejetsum = Math::PtEtaPhiMVector(0,0,0,0);
for(size_t i = 0; i < widejets.size(); i++)
{
    widejetsum += widejets.at(i);
}
if(not(widejetsum.mass() > 1530)) // inv mass > 1530 to ensure trigger efficiency
{
    continue;
}

// REORDER WIDEJETS AFTER PT
const auto wjets_reordering_mask = VecOps::Argsort(widejets,
                                                [](auto wjet_1, auto wjet_2) -> bool
                                                {
                                                    return wjet_1.pt() > wjet_2.pt();
                                                });
auto widejets_sorted = VecOps::Take(widejets, wjets_reordering_mask);

// refer to wide jets as jets (formal change for plotting and classification)
njet = widejets_sorted.size();
jets_4vec = widejets_sorted;

// effectively all other jets that are not merged are rejected

// ignore met for all events (simply remove it so it is not there for classifications)
met_4vec = RVec<Math::PtEtaPhiMVector>{};
is_met = false;

// ONLY ANALYZE THE 2jets (2J+0BJ+0MET) class with this code
