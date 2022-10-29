//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/InputHandler.hpp"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hpp"

namespace pxl
{

/// This method fills the objects from the read-in block into the passed vector. The number of added objects is
/// returned.
int InputHandler::readObjects(std::vector<Serializable *> &objects)
{
    int nObjects = 0;
    while (getChunkReader().getInputStream().good())
    {
        _objectCount++;

        Id id(getChunkReader().getInputStream());

        Serializable *obj = ObjectFactory::instance().create(id);
        if (obj)
        {
            obj->deserialize(getChunkReader().getInputStream());
            objects.push_back(obj);
            nObjects++;
        }
        else
            throw std::runtime_error(
                "InputHandler::readObjects(std::vector<Serializable*>& objects): unknown object in file: " +
                id.toString());
    }
    return nObjects;
}

/// This method fills the objects from the read-in block into the passed pxl::Event. The number of added objects is
/// returned. Caution: Only pxl::Relative derivatives will be restored, e.g. no pxl::Event.
int InputHandler::readObjects(Event *event)
{
    int nObjects = 0;
    while (getChunkReader().getInputStream().good())
    {
        _objectCount++;

        Id id(getChunkReader().getInputStream());

        Relative *obj = dynamic_cast<Relative *>(ObjectFactory::instance().create(id));
        if (obj)
        {
            obj->deserialize(getChunkReader().getInputStream());
            event->insertObject(obj);
            nObjects++;
        }
        else
            throw std::runtime_error("InputHandler::readObjects(pxl::Event* event): unknown object in file: " +
                                     id.toString());
    }
    return nObjects;
}

/// This method fills the objects from the read-in block into the passed pxl::BasicContainer. The number of added
/// objects is returned.
int InputHandler::readObjects(BasicContainer *container)

{
    int nObjects = 0;
    while (getChunkReader().getInputStream().good())
    {
        _objectCount++;

        Id id(getChunkReader().getInputStream());

        Serializable *obj = ObjectFactory::instance().create(id);
        if (obj)
        {
            obj->deserialize(getChunkReader().getInputStream());
            container->insertObject(obj);
            nObjects++;
        }
        else
            throw std::runtime_error(
                "InputHandler::readObjects(std::vector<pxl::Serializable*>& objects): unknown object in file: " +
                id.toString());
    }
    return nObjects;
}

Serializable *InputHandler::readNextObject()
{
    while (!getChunkReader().getInputStream().good())
    {
        if (getChunkReader().getStatus() == ChunkReader::preHeader)
        {
            if (!getChunkReader().next())
            {
                return 0;
            }
        }
        getChunkReader().nextBlock();
        if (getChunkReader().eof())
            return 0;
    }

    _objectCount++;

    Id id(getChunkReader().getInputStream());
    Serializable *obj = ObjectFactory::instance().create(id);
    if (obj)
    {
        obj->deserialize(getChunkReader().getInputStream());
        return obj;
    }
    else
        throw std::runtime_error("InputHandler::readNextObject(): unknown object in file: " + id.toString());
    return obj;
}

Serializable *InputHandler::readPreviousObject()
{
    return seekToObject(_objectCount - 1);
}

Serializable *InputHandler::seekToObject(size_t index)

{
    if (index <= _objectCount)
    {
        ///		bool success =
        seekToFileSection(0);
        // DEACTIVATE for current VISPA version, should be fixed in IO 3.0
        //		if (!success)
        //			throw std::runtime_error("InputHandler::seekToObject: seekToFileSection(0) not successful");
        _objectCount = 0;
    }

    if (index > _objectCount)
    {
        Serializable *obj = 0;
        do
        {
            if (obj != 0)
                delete obj;

            obj = readNextObject();
            if (obj == 0)
                return 0;
        } while (index > _objectCount);
        return obj;
    }

    return 0;
}

bool InputHandler::readEvent(Event *event)
{
    if (getChunkReader().nextBlock() && getChunkReader().getInputStream().good())
    {
        Id id(getChunkReader().getInputStream());
        if (id == event->getStaticTypeId())
        {
            event->deserialize(getChunkReader().getInputStream());
            return true;
        }
    }
    return false;
}

bool InputHandler::readEventIf(Event *event, const std::string &blockInfo, skipMode doSkip)
{
    if (readBlockIf(blockInfo, doSkip))
    {
        Id id(getChunkReader().getInputStream());
        if (id == event->getStaticTypeId())
        {
            event->deserialize(getChunkReader().getInputStream());
            return true;
        }
    }
    return false;
}

bool InputHandler::readBasicContainer(BasicContainer *basicContainer)
{
    if (getChunkReader().nextBlock() && getChunkReader().getInputStream().good())
    {
        Id id(getChunkReader().getInputStream());
        if (id == basicContainer->getStaticTypeId())
        {
            basicContainer->deserialize(getChunkReader().getInputStream());
            return true;
        }
    }
    return false;
}

bool InputHandler::readBasicContainerIf(BasicContainer *basicContainer, const std::string &blockInfo, skipMode doSkip)
{
    if (readBlockIf(blockInfo, doSkip))
    {
        Id id(getChunkReader().getInputStream());
        if (id == basicContainer->getStaticTypeId())
        {
            basicContainer->deserialize(getChunkReader().getInputStream());
            return true;
        }
    }
    return false;
}

bool InputHandler::readInformationChunk(InformationChunk *chunk)
{
    if (getChunkReader().nextBlock() && getChunkReader().getInputStream().good())
    {
        Id id(getChunkReader().getInputStream());
        if (id == chunk->getStaticTypeId())
        {
            chunk->deserialize(getChunkReader().getInputStream());
            return true;
        }
    }
    return false;
}

bool InputHandler::readInformationChunkIf(InformationChunk *event, const std::string &blockInfo, skipMode doSkip)
{
    if (readBlockIf(blockInfo, doSkip))
    {
        Id id(getChunkReader().getInputStream());
        if (id == event->getStaticTypeId())
        {
            event->deserialize(getChunkReader().getInputStream());
            return true;
        }
    }
    return false;
}

int InputHandler::skipFileSections(int n)
{
    int skipped = 0;
    for (; n < 0 && getChunkReader().previous(); ++n)
        --skipped;
    for (; n > 0 && getChunkReader().skip(); --n)
        ++skipped;
    return skipped;
}

bool InputHandler::seekToFileSection(int index)
{
    int count = index - getChunkReader().getSectionCount();
    return (skipFileSections(count) == count);
}

bool InputHandler::readBlockIf(const std::string &blockInfo, skipMode doSkip)
{
    bool success = false;
    while (!success && getChunkReader().getStatus() != 0)
    {
        success = getChunkReader().nextBlock(doSkip, ChunkReader::evaluate, blockInfo);
    }
    return success;
}

/// This method explicitly reads an object of type objecttype. Caution: This method should only be used if the type of
/// the following object is known by hard.
template <class objecttype>
bool InputHandler::readObject(objecttype *obj)

{
    if (getChunkReader().getInputStream().good())
    {
        Id id(getChunkReader().getInputStream());
        if (id != objecttype::getStaticTypeId())
            throw std::runtime_error("InputHandler::readObject(objecttype* obj): unexpected object in file: " +
                                     id.toString());
        obj->deserialize(getChunkReader().getInputStream());
        return true;
    }
    return false;
}

} // namespace pxl
// namespace pxl
