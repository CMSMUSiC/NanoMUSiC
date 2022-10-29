//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_FILE_FACTORY_HH
#define PXL_FILE_FACTORY_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <map>

#include "Pxl/Pxl/interface/pxl/core/File.hpp"
#include "Pxl/Pxl/interface/pxl/core/Id.hpp"

namespace pxl
{

class FileProducerInterface;

class PXL_DLL_EXPORT FileFactory
{
  private:
    FileFactory();

    std::map<std::string, const FileProducerInterface *> _Producers;

  public:
    static FileFactory &instance();

    FileImpl *create(const std::string &id);

    void registerProducer(const std::string &id, const FileProducerInterface *producer);
    void unregisterProducer(const FileProducerInterface *producer);

    bool hasSchema(const std::string &schema);
};

class FileProducerInterface
{
  public:
    virtual ~FileProducerInterface()
    {
    }

    virtual FileImpl *create() const = 0;
};

template <class T>
class FileProducerTemplate : public FileProducerInterface
{
  public:
    void initialize(const std::string &schema)
    {
        FileFactory::instance().registerProducer(schema, this);
    }

    void shutdown()
    {
        FileFactory::instance().unregisterProducer(this);
    }

    FileImpl *create() const
    {
        return new T();
    }
};

} // namespace pxl

#endif // PXL_IO_OBJECT_FACTORY_HH
