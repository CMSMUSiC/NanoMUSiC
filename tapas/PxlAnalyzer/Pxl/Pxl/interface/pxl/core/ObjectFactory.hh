//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_IO_OBJECT_FACTORY_HH
#define PXL_IO_OBJECT_FACTORY_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <map>

#include "Pxl/Pxl/interface/pxl/core/Serializable.hh"
#include "Pxl/Pxl/interface/pxl/core/Id.hh"

// io
/**
 This class serves the PXL I/O scheme by managing
 the relation of classes to UUIDs.
 */

namespace pxl
{

class ObjectProducerInterface;

class PXL_DLL_EXPORT ObjectFactory
{
private:

	ObjectFactory();

	std::map<Id, const ObjectProducerInterface *> _Producers;

public:

	static ObjectFactory& instance();

	Serializable *create(const Id& id);

	void
			registerProducer(const Id& id,
					const ObjectProducerInterface* producer);
	void unregisterProducer(const ObjectProducerInterface* producer);
};

class ObjectProducerInterface
{
public:
	virtual ~ObjectProducerInterface()
	{
	}

	virtual Serializable *create() const = 0;
};

template<class T>
class ObjectProducerTemplate: public ObjectProducerInterface
{
public:

	void initialize()
	{
		ObjectFactory::instance().registerProducer(T::getStaticTypeId(), this);
	}

	void shutdown()
	{
		ObjectFactory::instance().unregisterProducer(this);
	}

	virtual Serializable *create() const
	{
		return new T();
	}
};

} // namespace pxl

#endif // PXL_IO_OBJECT_FACTORY_HH
