//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <iostream>
#include <string>

#include "Pxl/Pxl/interface/pxl/core/ObjectOwner.hh"

#include "Pxl/Pxl/interface/pxl/hep/AnalysisFork.hh"
#include "Pxl/Pxl/interface/pxl/hep/AnalysisProcess.hh"

namespace pxl {

void AnalysisFork::beginJob(const Serializable* input)
{
	for(ObjectOwnerTypeIterator<AnalysisFork> iter(&getObjectOwner()); 
	iter!=getObjectOwner().end(); ++iter)
		(*iter)->beginJob(input);

	for(ObjectOwnerTypeIterator<AnalysisProcess> iter(&getObjectOwner()); 
	iter!=getObjectOwner().end(); ++iter)
		(*iter)->beginJob(input);
}


void AnalysisFork::beginRun(const Serializable* input)
{
	for(ObjectOwnerTypeIterator<AnalysisFork> iter(&getObjectOwner());
	   iter!=getObjectOwner().end(); ++iter)
		(*iter)->beginRun(input);

	for(ObjectOwnerTypeIterator<AnalysisProcess> iter(&getObjectOwner());
	iter!=getObjectOwner().end(); ++iter)
		(*iter)->beginRun(input);
}

void AnalysisFork::analyseEvent(const Event* event)
{
	for(ObjectOwnerTypeIterator<AnalysisFork> iter(&getObjectOwner());
	   iter!=getObjectOwner().end(); ++iter)
		(*iter)->analyseEvent(event);

	for(ObjectOwnerTypeIterator<AnalysisProcess> iter(&getObjectOwner());
	iter!=getObjectOwner().end(); ++iter)
		(*iter)->analyseEvent(event);
}

void AnalysisFork::finishEvent(const Event* event)
{
	for(ObjectOwnerTypeIterator<AnalysisFork> iter(&getObjectOwner());
	   iter!=getObjectOwner().end(); ++iter)
		(*iter)->finishEvent(event);

	for(ObjectOwnerTypeIterator<AnalysisProcess> iter(&getObjectOwner());
	iter!=getObjectOwner().end(); ++iter)
		(*iter)->finishEvent(event);
}

void AnalysisFork::endRun(const Serializable* input)
{
	for(ObjectOwnerTypeIterator<AnalysisFork> iter(&getObjectOwner());
	   iter!=getObjectOwner().end(); ++iter)
		(*iter)->endRun(input);

	for(ObjectOwnerTypeIterator<AnalysisProcess> iter(&getObjectOwner());
	iter!=getObjectOwner().end(); ++iter)
		(*iter)->endRun(input);
}

void AnalysisFork::endJob(const Serializable* input)
{
	for(ObjectOwnerTypeIterator<AnalysisFork> iter(&getObjectOwner());
	   iter!=getObjectOwner().end(); ++iter)
		(*iter)->endJob(input);

	for(ObjectOwnerTypeIterator<AnalysisProcess> iter(&getObjectOwner());
	iter!=getObjectOwner().end(); ++iter)
		(*iter)->endJob(input);
}

Serializable* AnalysisFork::clone() const
{
	return new AnalysisFork(*this);
}

std::ostream& AnalysisFork::print(int level, std::ostream& os, int pan) const
{
	printPan1st(os, pan) << "AnalysisFork: " << getName() << std::endl;

	if (level>0) printContent(level, os, pan);

	for(ObjectOwner::const_iterator iter = getObjectOwner().begin();
		iter!=getObjectOwner().end(); ++iter)
	{

		if ((*iter)->getMotherRelations().size() == 0)
			(*iter)->printDecayTree(level, os, pan);
	}
	return os;
}

} // namespace pxl

