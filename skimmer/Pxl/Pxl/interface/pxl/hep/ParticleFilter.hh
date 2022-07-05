//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_PARTICLE_FILTER_HH
#define PXL_HEP_PARTICLE_FILTER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <string>

#include "Pxl/Pxl/interface/pxl/core/Filter.hh"
#include "Pxl/Pxl/interface/pxl/hep/Particle.hh"

namespace pxl
{

/** 
 This class provides a pT-based sorting for PXL particles.
 */
class ParticlePtComparator : public ComparatorInterface<Particle>
{
public:
	/// Returns true if pt of \p p1 higher than pt of \p p2.
	virtual bool operator()(const Particle* p1, const Particle* p2)
	{
		return (p1->getPt()>p2->getPt());
	}
};

/** 
 This class provides a pT-eta-name filter criterion for PXL particles.
 Compares equality of name (if set), requires the pt to be higher than 
 \p ptMin, and requires |eta| to be smaller than etaMax.
 */
class ParticlePtEtaNameCriterion : public FilterCriterionInterface<Particle>
{
public:
	/// Construct criterion.
	/// @param name name to match, ignored if empty ("")
	/// @param ptMin minimum transverse momentum
	/// @param etaMax maximum |pseudorapidity|, ignored if <=0
	ParticlePtEtaNameCriterion(const std::string& name, double ptMin = 0.0,
			double etaMax = 0.0) :
		_name(name), _ptMin(ptMin), _etaMax(etaMax)
	{
	}

	/// Returns true if \p pa passes the filter.
	virtual bool operator()(const Particle& pa) const
	{
		if ((_name != "" && pa.getName() != _name) || (_ptMin > 0.0
				&& pa.getPt() < _ptMin) || (_etaMax > 0.0
				&& std::fabs(pa.getEta()) > _etaMax))
			return false;
		return true;
	}

private:
	std::string _name;
	double _ptMin;
	double _etaMax;
};

/** 
 This class provides a pT filter criterion for PXL particles.
 Requires the pt to be higher than  \p ptMin.
 */
class ParticlePtCriterion : public FilterCriterionInterface<Particle>
{
public:
	ParticlePtCriterion(double ptMin = 0.0) :
		_ptMin(ptMin)
	{
	}

	/// Returns true if \p pa passes the filter.
	virtual bool operator()(const Particle& pa) const
	{
		if ((_ptMin > 0.0 && pa.getPt() < _ptMin))
			return false;
		return true;
	}

private:
	double _ptMin;
};
/**
 This typedef provides a filter for PXL particles which sorts
 the particle by transverse momentum. The concrete filter 
 criterion is passed at evaluation time, e.g. the ParticlePtCriterion
 or the ParticlePtEtaNameCriterion. See the example for further
 hints on how to use the filters.
 */
typedef Filter<Particle, ParticlePtComparator> ParticleFilter;

} // namespace pxl

#endif // PXL_HEP_PARTICLE_FILTER_HH
