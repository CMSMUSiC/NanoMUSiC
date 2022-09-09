//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"
#include "Pxl/Pxl/interface/pxl/core/logging.hh"

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::ObjectFactory"

namespace pxl
{

ObjectFactory::ObjectFactory()
{
}

ObjectFactory &ObjectFactory::instance()
{
    static ObjectFactory f;
    return f;
}

Serializable *ObjectFactory::create(const Id &id)
{
    std::map<Id, const ObjectProducerInterface *>::iterator result;
    result = _Producers.find(id);
    if (result == _Producers.end())
        return 0;
    else
        return (*result).second->create();
}

void ObjectFactory::registerProducer(const Id &id, const ObjectProducerInterface *producer)
{
    PXL_LOG_INFO << "register object producer for " << id;
    _Producers[id] = producer;
}

void ObjectFactory::unregisterProducer(const ObjectProducerInterface *producer)
{
    std::map<Id, const ObjectProducerInterface *>::iterator i;
    for (i = _Producers.begin(); i != _Producers.end(); i++)
    {
        if (i->second == producer)
        {
            PXL_LOG_INFO << "unregister object producer for " << i->first;
            _Producers.erase(i);
            return;
        }
    }
}

} // namespace pxl
