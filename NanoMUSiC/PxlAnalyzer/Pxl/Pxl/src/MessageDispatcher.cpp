//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/MessageDispatcher.hpp"
#include "Pxl/Pxl/interface/pxl/core/logging.hpp"

#include <algorithm>

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::MessageDispatcher"

namespace pxl
{

MessageHandler::~MessageHandler()
{
    MessageDispatcher::instance().unsubscribe(this);
}

MessageDispatcher::MessageDispatcher() : lastId(0)
{
}

MessageDispatcher::~MessageDispatcher()
{
}

MessageDispatcher &MessageDispatcher::instance()
{
    static MessageDispatcher f;
    return f;
}

size_t MessageDispatcher::publish(std::string mid, const Variant &parameter)
{
    size_t id = getId(mid);
    publish(id, parameter);
    return id;
}

void MessageDispatcher::publish(size_t messageid, const Variant &parameter)
{
    subscriptions_map_t::iterator iMap = subscriptions.find(messageid);
    if (iMap == subscriptions.end())
        return;

    handler_vector_t::iterator iVec = (*iMap).second.begin();
    handler_vector_t::iterator iEnd = (*iMap).second.end();
    for (; iVec != iEnd; iVec++)
    {
        (*iVec)->handleMessage(messageid, parameter);
    }
}

size_t MessageDispatcher::subscribe(std::string mid, MessageHandler *handler)
{
    size_t id = getId(mid);
    subscribe(id, handler);
    return id;
}

void MessageDispatcher::subscribe(size_t messageid, MessageHandler *handler)
{
    handler_vector_t &v = subscriptions[messageid];
    if (std::find(v.begin(), v.end(), handler) == v.end())
    {
        PXL_LOG_INFO << "New subsciber to: " << messageid;
        v.push_back(handler);
    }
}

size_t MessageDispatcher::unsubscribe(std::string mid, MessageHandler *handler)
{
    size_t id = getId(mid);
    unsubscribe(id, handler);
    return id;
}

void MessageDispatcher::unsubscribe(size_t messageid, MessageHandler *handler)
{
    PXL_LOG_INFO << "Request unsubscribe";
    handler_vector_t &v = subscriptions[messageid];
    v.erase(std::remove(v.begin(), v.end(), handler), v.end());
}

void MessageDispatcher::unsubscribe(MessageHandler *handler)
{
    subscriptions_map_t::iterator i = subscriptions.begin();
    subscriptions_map_t::iterator end = subscriptions.end();
    for (; i != end; i++)
    {
        handler_vector_t &v = i->second;
        v.erase(std::remove(v.begin(), v.end(), handler), v.end());
    }
}

size_t MessageDispatcher::getId(const std::string &name)
{
    std::map<std::string, size_t>::iterator i = ids.find(name);
    if (i != ids.end())
    {
        return i->second;
    }
    else
    {
        lastId++;
        PXL_LOG_INFO << "New message id: '" << name << "' -> " << lastId;
        ids[name] = lastId;
        return lastId;
    }
}

std::string MessageDispatcher::getName(const size_t &mid)
{
    static std::string emptyString = "";

    for (std::map<std::string, size_t>::iterator i = ids.begin(); i != ids.end(); ++i)
    {
        if (i->second == mid)
        {
            return i->first;
        }
    }

    return emptyString;
}

const std::map<std::string, size_t> MessageDispatcher::getIds() const
{
    return ids;
}

} // namespace pxl
