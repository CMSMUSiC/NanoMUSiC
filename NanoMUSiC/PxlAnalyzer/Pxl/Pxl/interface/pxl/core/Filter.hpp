//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_FILTER_HH
#define PXL_BASE_FILTER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <algorithm>
#include <map>

#include "Pxl/Pxl/interface/pxl/core/ObjectOwner.hpp"

namespace pxl
{

/**
 This class template provides a generic interface for a comparison
 object which can be used in a filter to sort the objects of type
 \p comparetype.
 */
template <class comparetype>
class ComparatorInterface
{
  public:
    virtual bool operator()(const comparetype *, const comparetype *) = 0;
    virtual ~ComparatorInterface()
    {
    }
};

/**
 This class template provides a generic interface for a filter
 criterion which can be used in a filter to filter out objects
 of type \p comparetype.
 */
template <class objecttype>
class FilterCriterionInterface
{
  public:
    virtual bool operator()(const objecttype &) const = 0;
    virtual ~FilterCriterionInterface()
    {
    }
};

/**
 This class template provides a sorted filter for PXL objects;
 it handles objects of type \p objecttype sorted by the criterion \p compare.
 The user can create own filters by instantiating this class with an object type
 and a criterion.
 Then, the apply method can be used with a specific, user-defined filter criterion.
 */
template <class objecttype, class compare>
class Filter
{
  public:
    virtual ~Filter()
    {
    }

    /// This method applies the filter by running over the \p objects container and fills
    /// the passed vector with pointers to the objects passing the filter criterion
    /// \p criterion.
    /// Returns the number of filled objects.
    virtual size_t apply(const ObjectOwner &objects, std::vector<objecttype *> &fillVector,
                         const FilterCriterionInterface<objecttype> &criterion)
    {
        size_t size = fillVector.size();

        // fill map:
        for (ObjectOwnerTypeIterator<objecttype> iter(objects); iter != objects.end(); ++iter)
        {
            if (!criterion(**iter))
                continue;

            fillVector.push_back(*iter);
        }
        //<typename std::vector<objecttype*>::iterator, compare>
        compare comp;
        std::sort(fillVector.begin(), fillVector.end(), comp);
        return fillVector.size() - size;
    }
};

} // namespace pxl

#endif // PXL_BASE_FILTER_HH
