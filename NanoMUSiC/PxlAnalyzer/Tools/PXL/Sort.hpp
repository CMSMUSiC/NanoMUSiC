#ifndef pxl_Sort_hh
#define pxl_Sort_hh

#include <algorithm>
#include <vector>

//#include "Tools/PXL/PXL.hpp"
#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"

namespace pxl
{
// inline to avoid stupid compiler errors
inline bool compPart(pxl::Particle *first, pxl::Particle *second)
{
    // there might be something weird going on when doing a direct comparision, so store the values in variables first
    double pt1 = first->getPt();
    double pt2 = second->getPt();
    return pt1 > pt2;
}

inline void sortParticles(std::vector<pxl::Particle *> &vec)
{
    std::sort(vec.begin(), vec.end(), compPart);
}

/**
This class provides a PDG id-pt filter criterion for PXL particles.
Compares equality of PDG id (if set), requires the pt to be higher than
\p ptMin.
*/
class ParticlePDGidPtCriterion : public FilterCriterionInterface<Particle>
{
  public:
    /// Construct criterion.
    /// @param name name to match, ignored if empty ("")
    /// @param ptMin minimum transverse momentum
    /// @param etaMax maximum |pseudorapidity|, ignored if <=0
    ParticlePDGidPtCriterion(const int &id, double ptMin = 0.0) : _id(id), _ptMin(ptMin)
    {
    }

    /// Returns true if \p pa passes the filter.
    virtual bool operator()(const Particle &pa) const
    {
        if ((pa.getPdgNumber() != _id) || (_ptMin > 0.0 && pa.getPt() < _ptMin))
            return false;
        return true;
    }

  private:
    int _id;
    double _ptMin;
};

} // namespace pxl

#endif
