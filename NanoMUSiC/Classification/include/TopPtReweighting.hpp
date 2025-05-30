#include "ROOT/RVec.hxx"
#include "TMath.h"

using ROOT::RVec;

inline auto top_pt_reweighting(bool is_data,
                               const RVec<float> &GenPart_pt,
                               const RVec<int> &GenPart_pdgId,
                               const RVec<int> &GenPart_statusFlags) -> double
{
    if (is_data)
    {
        return 1.;
    }

    // Status flags for fromHardProcess and isLastCopy
    // constexpr int fromHardProcess = 1 << 7; // bit 7
    constexpr int isLastCopy = 1 << 13; // bit 13

    float top_pt = -1.0;
    float antitop_pt = -1.0;
    bool foundTop = false;
    bool foundAntitop = false;

    // Search for top and antitop quarks
    for (std::size_t i = 0; i < GenPart_pt.size(); i++)
    {
        // Check if particle is from hard process and is last copy
        // bool isFromHardProcess = (GenPart_statusFlags[i] & fromHardProcess) != 0;
        bool isLastCopyFlag = (GenPart_statusFlags[i] & isLastCopy) != 0;

        // if (isFromHardProcess && isLastCopyFlag)
        if (isLastCopyFlag)
        {
            if (GenPart_pdgId[i] == 6 and not(foundTop))
            {
                // Found top quark
                top_pt = GenPart_pt[i];
                foundTop = true;
            }
            if (GenPart_pdgId[i] == -6 and not(foundAntitop))
            {
                // Found antitop quark
                antitop_pt = GenPart_pt[i];
                foundAntitop = true;
            }
        }

        // Early exit if both found
        if (foundTop && foundAntitop)
        {
            break;
        }
    }

    // Apply reweighting based on what was found
    if (foundTop && foundAntitop)
    {
        // Both tops found - apply full ttbar reweighting
        double weight_top = TMath::Exp(0.0615 - 0.0005 * top_pt);
        double weight_antitop = TMath::Exp(0.0615 - 0.0005 * antitop_pt);
        return TMath::Sqrt(weight_top * weight_antitop);
    }
    else if (foundTop && not(foundAntitop))
    {
        // Only top found - apply single-top correction
        double weight_top = TMath::Exp(0.0615 - 0.0005 * top_pt);
        return TMath::Sqrt(weight_top);
    }
    else if (not(foundTop) && foundAntitop)
    {
        // Only antitop found - apply single-antitop correction
        double weight_antitop = TMath::Exp(0.0615 - 0.0005 * antitop_pt);
        return TMath::Sqrt(weight_antitop);
    }
    else
    {
        // Neither found - return neutral weight
        return 1.0;
    }
}
