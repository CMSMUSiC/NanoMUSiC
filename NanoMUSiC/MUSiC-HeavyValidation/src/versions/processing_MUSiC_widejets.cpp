// NAME: MUSIC WIDE JETS

// SELECT SEED JETS
auto seed_jets = RVec<Math::PtEtaPhiMVector>{};   // jet seeds
auto noseed_jets = RVec<Math::PtEtaPhiMVector>{}; // jets that are no seeds
for (size_t i = 0; i < jets_4vec.size(); i++)
{
    if (jets_4vec.at(i).pt() > 250) // select as seed
    {
        seed_jets.push_back(jets_4vec.at(i));
    }
    else // don't select as seed
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