//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_CORE_MESASSAGE_DISPATCHER_HH
#define PXL_CORE_MESASSAGE_DISPATCHER_HH

#include "Pxl/Pxl/interface/pxl/core/Variant.hh"
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <map>
#include <vector>

namespace pxl
{

/**
 React on published messages
 */
class PXL_DLL_EXPORT MessageHandler
{
  public:
    virtual ~MessageHandler();
    virtual void handleMessage(size_t mid, const Variant &parameter) = 0;
};

/**
 Allows to exchange messages.
 */
class PXL_DLL_EXPORT MessageDispatcher
{
  public:
    MessageDispatcher();
    ~MessageDispatcher();

    static MessageDispatcher &instance();

    size_t publish(std::string mid, const Variant &parameter);
    void publish(size_t mid, const Variant &parameter);

    size_t subscribe(std::string mid, MessageHandler *handler);
    void subscribe(size_t mid, MessageHandler *handler);

    size_t unsubscribe(std::string mid, MessageHandler *handler);
    void unsubscribe(size_t mid, MessageHandler *handler);
    void unsubscribe(MessageHandler *handler);

    size_t getId(const std::string &name);
    std::string getName(const size_t &mid);
    const std::map<std::string, size_t> getIds() const;

  private:
    typedef std::vector<MessageHandler *> handler_vector_t;
    typedef std::map<size_t, handler_vector_t> subscriptions_map_t;
    subscriptions_map_t subscriptions;
    std::map<std::string, size_t> ids;
    size_t lastId;
};

} // namespace pxl

#endif // PXL_CORE_MESASSAGE_DISPATCHER_HH
