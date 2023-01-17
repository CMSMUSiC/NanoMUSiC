//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <iostream>

#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hpp"
#include "Pxl/Pxl/interface/pxl/core/ObjectOwner.hpp"
#include "Pxl/Pxl/interface/pxl/core/Relations.hpp"
#include "Pxl/Pxl/interface/pxl/core/Relative.hpp"
#include "Pxl/Pxl/interface/pxl/core/logging.hpp"

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::ObjectOwner"

namespace pxl
{

void ObjectOwner::init(const ObjectOwner &original)
{
    // copy objects: loop in STL style
    for (const_iterator iter = original._container.begin(); iter != original._container.end(); iter++)
    {

        Relative *pOld = *iter;
        Relative *pNew = dynamic_cast<Relative *>(pOld->clone());

        insert(pNew);

        std::map<Id, Relative *>::iterator insertPos = _copyHistory.lower_bound(pOld->id());
        if (insertPos == _copyHistory.end() || insertPos->first != pOld->id())
            _copyHistory.insert(insertPos, std::map<Id, Relative *>::iterator::value_type(pOld->id(), pNew));
        else
            insertPos->second = pNew;
    }

    // FIXME: possibly inefficient, might be done all in one loop
    // redirect relations: loop in PTL style
    for (const_iterator iter = original._container.begin(); iter != original._container.end(); iter++)
    {
        Relative *pOld = *iter;
        Relative *pNew = 0;
        std::map<Id, Relative *>::const_iterator found = _copyHistory.find(pOld->id());
        if (found != _copyHistory.end())
            pNew = found->second;

        // mother relations
        for (Relations::const_iterator iter = pOld->getMotherRelations().begin();
             iter != pOld->getMotherRelations().end(); ++iter)
        {
            Relative *pOldRel = *iter;
            Relative *pNewRel = 0;
            std::map<Id, Relative *>::const_iterator foundRel = _copyHistory.find(pOldRel->id());
            if (foundRel != _copyHistory.end())
                pNewRel = foundRel->second;

            if (pOldRel)
            {
                if (pNewRel)
                    pNew->linkMother(pNewRel);
                else
                    PXL_LOG_WARNING << "ObjectOwner::init(const ObjectOwner&): WARNING: some original objects had "
                                       "relations to objects of other owners.";
            }
            else
                PXL_LOG_WARNING << "ObjectOwner::init(const ObjectOwner&): WARNING: some originally related objects no "
                                   "longer exist.";
        }

        // daughter relations
        // have been set automatically above
    }

    // redirect index:
    for (std::map<std::string, Relative *>::const_iterator iter = original._index.begin();
         iter != original._index.end(); ++iter)
    {

        Relative *pOld = iter->second;

        Relative *pNew = 0;
        std::map<Id, Relative *>::const_iterator found = _copyHistory.find(pOld->id());
        if (found != _copyHistory.end())
            pNew = found->second;

        if (pNew)
            _index.insert(std::map<std::string, Relative *>::const_iterator::value_type(iter->first, pNew));
        else
            PXL_LOG_WARNING << "pxl::ObjectOwner::ObjectOwner(...): WARNING: some original indices pointed to objects "
                               "of other owners.";
    }
}

void ObjectOwner::clearContainer()
{
    for (iterator iter = _container.begin(); iter != _container.end(); iter++)
    {
        (*iter)->_refObjectOwner = 0;
        delete (*iter);
    }
    _container.clear();
    _copyHistory.clear();
    _index.clear();
    _uuidSearchMap.clear();
}

void ObjectOwner::insert(Relative *item)
{
    if (item->_refObjectOwner == this)
        return;
    if (item->_refObjectOwner)
        throw std::runtime_error("Error in ObjectOwner::set: Object already has another object owner.");
    item->_refObjectOwner = this;
    _container.push_back(item);
    _uuidSearchMap.insert(std::pair<Id, Relative *>(item->getId(), item));
}

void ObjectOwner::remove(Relative *item)
{
    if (item->_refObjectOwner != this)
    {
        PXL_LOG_WARNING << "Trying to remove Relative from object owner which has different object owner.";
        return;
    }

    // search & remove possible indices (multiple occurrences possible!)
    for (std::map<std::string, Relative *>::const_iterator iter = _index.begin(); iter != _index.end(); iter++)
    {
        if (item == iter->second)
            _index.erase(iter->first);
    }

    // search & remove possible copy history
    for (std::map<Id, Relative *>::const_iterator iter = _copyHistory.begin(); iter != _copyHistory.end(); iter++)
    {

        // FIXME: inefficient
        if (item == iter->second)
        {
            _copyHistory.erase(iter->first);
            break; // multiple occurrences *not* possible!
        }
    }

    _uuidSearchMap.erase(item->getId());

    item->_refObjectOwner = 0;
    for (iterator iter = _container.begin(); iter != _container.end(); iter++)
    {

        if (item == (*iter))
        {
            delete *iter;
            _container.erase(iter);
            break;
        }
    }
}

void ObjectOwner::take(Relative *item)
{
    if (item->_refObjectOwner != this)
    {
        PXL_LOG_WARNING << "Trying to take Relative from object owner which has different object owner.";
        return;
    }

    // search & remove possible indices (multiple occurrences possible!)
    for (std::map<std::string, Relative *>::const_iterator iter = _index.begin(); iter != _index.end(); iter++)
    {
        if (item == iter->second)
            _index.erase(iter->first);
    }

    // search & remove possible copy history
    for (std::map<Id, Relative *>::const_iterator iter = _copyHistory.begin(); iter != _copyHistory.end(); iter++)
    {
        if (item == iter->second)
        {
            _copyHistory.erase(iter->first);
            break; // multiple occurrences *not* possible!
        }
    }

    _uuidSearchMap.erase(item->getId());

    item->_refObjectOwner = 0;
    for (iterator iter = _container.begin(); iter != _container.end(); iter++)
    {

        if (item == (*iter))
        {
            _container.erase(iter);
            break;
        }
    }
}

bool ObjectOwner::has(const Relative *item) const
{
    return item->_refObjectOwner == this;
}

bool ObjectOwner::setIndexEntry(const std::string &key, Relative *obj, bool overwrite)
{
    if (!key.length() || !has(obj))
    {
        if (!key.length())
            PXL_LOG_ERROR << "Error in setting index: key has zero length!";
        else
            PXL_LOG_ERROR << "Error in setting index: Object does not belong to ObjectOwner.";
        return false;
    }

    std::map<std::string, Relative *>::iterator insertPos = _index.lower_bound(key);
    if (insertPos == _index.end() || insertPos->first != key)
        _index.insert(insertPos, std::map<std::string, Relative *>::iterator::value_type(key, obj));
    else
    {
        if (overwrite)
            insertPos->second = obj;
        else
        {
            PXL_LOG_WARNING << "Warning in setting Index: Key " << key
                            << " already present and bool 'overwrite' set to false.";
            return false;
        }
    }
    return true;
}

void ObjectOwner::serialize(const OutputStream &out) const
{
    // contents of vector
    out.writeUnsignedInt(size());
    for (const_iterator iter = begin(); iter != end(); ++iter)
    {
        (*iter)->serialize(out);

        // serialize relations explicitly
        (*iter)->getMotherRelations().serialize(out);
        (*iter)->getDaughterRelations().serialize(out);
        (*iter)->getFlatRelations().serialize(out);
    }

    // index
    out.writeUnsignedInt(_index.size());
    for (std::map<std::string, Relative *>::const_iterator iter = _index.begin(); iter != _index.end(); ++iter)
    {
        out.writeString(iter->first);
        (iter->second->id()).serialize(out);
    }
}

void ObjectOwner::deserialize(const InputStream &in)
{
    /* Algorithm:
     * a) deserialize all objects. those deserialize themselves, and their relations
     * as the related objects' uuids.
     * b) create temprorary id-object map within the same loop.
     * (no need (!) for orphan relations in this new algorithm)
     * c) recreate relations (fetching objects from map)
     * d) fill index (fetching objects from map)
     * [e) shall the CopyHistory be serialised/deserialised? this is hard since
     * one would need to have all ObjectOwners in memory, ie the whole event, and do some
     * explicit stuff afterwards.]
     * no error handling at the moment
     */

    std::map<Id, Relative *> objIdMap;
    std::multimap<Relative *, Id> daughterRelationsMap;
    std::multimap<Relative *, Id> motherRelationsMap;
    std::multimap<Relative *, Id> flatRelationsMap;

    unsigned int size = 0;
    in.readUnsignedInt(size);
    for (unsigned int i = 0; i < size; ++i)
    {
        Id typeId(in);
        // Contained object must be a Relative derivative
        Relative *object = dynamic_cast<Relative *>(ObjectFactory::instance().create(typeId));
        object->deserialize(in);
        insert(object);
        objIdMap.insert(std::pair<Id, Relative *>(object->id(), object));

        int msize = 0;
        in.readInt(msize);
        for (int j = 0; j < msize; ++j)
        {
            Id id(in);
            motherRelationsMap.insert(std::pair<Relative *, Id>(object, id));
        }

        int dsize = 0;
        in.readInt(dsize);
        for (int j = 0; j < dsize; ++j)
        {
            Id id(in);
            daughterRelationsMap.insert(std::pair<Relative *, Id>(object, id));
        }
        int fsize = 0;
        in.readInt(fsize);
        for (int j = 0; j < fsize; ++j)
        {
            Id id(in);
            flatRelationsMap.insert(std::pair<Relative *, Id>(object, id));
        }
    }

    for (std::multimap<Relative *, Id>::const_iterator iter = daughterRelationsMap.begin();
         iter != daughterRelationsMap.end(); ++iter)
    {
        Relative *target = objIdMap.find(iter->second)->second;
        iter->first->linkDaughter(target);
    }

    for (std::multimap<Relative *, Id>::const_iterator iter = flatRelationsMap.begin(); iter != flatRelationsMap.end();
         ++iter)
    {
        Relative *target = objIdMap.find(iter->second)->second;
        iter->first->linkFlat(target);
    }

    in.readUnsignedInt(size);

    for (unsigned int i = 0; i < size; ++i)
    {
        std::string name;
        in.readString(name);
        Id id(in);
        _index.insert(std::pair<std::string, Relative *>(name, objIdMap.find(id)->second));
    }
}

} // namespace pxl
