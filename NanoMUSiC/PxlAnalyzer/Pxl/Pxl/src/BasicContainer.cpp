//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/BasicContainer.hpp"

namespace pxl
{

void BasicContainer::init(const BasicContainer &original)
{
    std::map<Id, Serializable *> copyHistory;

    for (const_iterator iter = original._container.begin(); iter != original._container.end(); ++iter)
    {

        Serializable *pOld = *iter;
        Serializable *pNew = pOld->clone();

        insertObject(pNew);
        copyHistory.insert(std::pair<Id, Serializable *>(pOld->getId(), pNew));
    }

    // redirect index:
    for (map_t::const_iterator iter = original._index.begin(); iter != original._index.end(); ++iter)
    {

        Serializable *pOld = iter->second;

        Serializable *pNew = 0;
        std::map<Id, Serializable *>::const_iterator found = copyHistory.find(pOld->getId());
        if (found != copyHistory.end())
            pNew = found->second;

        if (pNew)
            _index.insert(map_t::const_iterator::value_type(iter->first, pNew));
    }
}

void BasicContainer::clearContainer()
{
    for (iterator iter = _container.begin(); iter != _container.end(); iter++)
    {
        delete (*iter);
    }
    _container.clear();
    _index.clear();
    _uuidSearchMap.clear();
}

void BasicContainer::insertObject(Serializable *value)
{
    _container.push_back(value);
    _uuidSearchMap.insert(std::pair<Id, Serializable *>(value->getId(), value));
}

void BasicContainer::remove(Serializable *value)
{
    for (map_t::const_iterator iter = _index.begin(); iter != _index.end(); iter++)
    {
        if (value == iter->second)
            _index.erase(iter->first);
    }

    _uuidSearchMap.erase(value->getId());

    for (iterator iter = _container.begin(); iter != _container.end(); iter++)
    {
        if (value == (*iter))
        {
            delete *iter;
            _container.erase(iter);
            return;
        }
    }
    throw std::runtime_error("Trying to remove non-existing object from container");
}

size_t BasicContainer::removeObjectsOfType(const Id &typeId)
{
    size_t n = 0;

    BasicContainer::iterator iter = _container.begin();
    while (iter != _container.end())
    {
        if ((*iter)->getTypeId() == typeId)
        {
            n++;
            _uuidSearchMap.erase((*iter)->getId());
            _index.erase((*iter)->getId().toString());
            delete *iter;
            iter = _container.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
    return n;
}

void BasicContainer::take(Serializable *value)
{
    for (map_t::const_iterator iter = _index.begin(); iter != _index.end(); iter++)
    {
        if (value == iter->second)
            _index.erase(iter->first);
    }

    _uuidSearchMap.erase(value->getId());

    for (iterator iter = _container.begin(); iter != _container.end(); iter++)
    {
        if (value == (*iter))
        {
            _container.erase(iter);
            break;
        }
    }
}

bool BasicContainer::has(const Serializable *value) const
{
    return _uuidSearchMap.find(value->getId()) != _uuidSearchMap.end();
}

void BasicContainer::serialize(const OutputStream &out) const
{
    Serializable::serialize(out);
    // write length of vector
    out.writeUnsignedInt(size());
    // serialize container objects
    for (const_iterator iter = begin(); iter != end(); ++iter)
    {
        (*iter)->serialize(out);
    }
    // serialize Container Index
    out.writeUnsignedInt(_index.size());
    for (map_t::const_iterator iter = _index.begin(); iter != _index.end(); ++iter)
    {
        out.writeString(iter->first);
        (iter->second->getId()).serialize(out);
    }
    // serialize user record
    UserRecordHelper::serialize(out);
}

void BasicContainer::deserialize(const InputStream &in)
{
    Serializable::deserialize(in);
    //	std::map<Id, Serializable*> objIdMap;
    unsigned int size = 0;
    in.readUnsignedInt(size);
    // deserialize content
    for (size_t i = 0; i < size; ++i)
    {
        Id typeId(in);
        Serializable *object = dynamic_cast<Serializable *>(ObjectFactory::instance().create(typeId));
        if (object)
        {
            object->deserialize(in);
            insertObject(object);
            // fill temporary map to store id's
            //			objIdMap.insert(std::pair<Id, Serializable*>(object->getId(), object));
        }
        else
        {
            std::string errorMessage =
                "BasicContainer::deserialize(const InputStream &in) : unknown object (TypeID: " + typeId.toString() +
                ") in container!";
            throw std::runtime_error(errorMessage);
        }
    }

    // read index
    in.readUnsignedInt(size);
    for (size_t i = 0; i < size; ++i)
    {
        std::string name;
        in.readString(name);
        Id id(in);
        for (size_t j = 0; j < _container.size(); ++j)
        {
            if (id == _container[j]->getId())
                _index.insert(std::pair<std::string, Serializable *>(name, _container[j]));
        }
        //		_index.insert(std::pair<std::string, Serializable*>(name, objIdMap.find(id)->second));
    }
    // deserialize user record
    UserRecordHelper::deserialize(in);
}

} // namespace pxl
