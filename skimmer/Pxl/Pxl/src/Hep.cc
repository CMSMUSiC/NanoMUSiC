//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

namespace pxl
{

static bool _initialized = false;

static ObjectProducerTemplate<AnalysisFork> _AnalysisForkProducer;
static ObjectProducerTemplate<AnalysisProcess> _AnalysisProcessProducer;
static ObjectProducerTemplate<Collision> _CollisionProducer;
static ObjectProducerTemplate<Particle> _ParticleProducer;
static ObjectProducerTemplate<Vertex> _VertexProducer;
static ObjectProducerTemplate<EventView> _EventViewProducer;

void Hep::initialize()
{
	if (_initialized)
		return;

	_AnalysisForkProducer.initialize();
	_AnalysisProcessProducer.initialize();
	_CollisionProducer.initialize();
	_ParticleProducer.initialize();
	_VertexProducer.initialize();
	_EventViewProducer.initialize();

	_initialized = true;
}

void Hep::shutdown()
{
	if (_initialized == false)
		return;

	_AnalysisForkProducer.shutdown();
	_AnalysisProcessProducer.shutdown();
	_CollisionProducer.shutdown();
	_ParticleProducer.shutdown();
	_VertexProducer.shutdown();
	_EventViewProducer.shutdown();

	_initialized = false;
}

} // namespace pxl
