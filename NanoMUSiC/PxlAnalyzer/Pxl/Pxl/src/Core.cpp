/*
 * init.cpp
 *
 *  Created on: 14.07.2010
 *      Author: gmueller
 */

#include "Pxl/Pxl/interface/pxl/core.hpp"

#include "LocalFileImpl.hpp"
#include "Pxl/Pxl/interface/pxl/core/FileFactory.hpp"
#include "StdFileImpl.hpp"

#ifdef PXL_ENABLE_SFTP
#include "sFTPFileImpl.hpp"
#endif
#ifdef PXL_ENABLE_DCAP
#include "dCapFileImpl.hpp"
#endif

namespace pxl
{

static bool _initialized = false;

static ObjectProducerTemplate<BasicContainer> _BasicContainerProducer;
static ObjectProducerTemplate<BasicMatrix> _BasicMatrixProducer;
static ObjectProducerTemplate<BasicNVector> _BasicNVectorProducer;
static ObjectProducerTemplate<Event> _EventProducer;
static ObjectProducerTemplate<InformationChunk> _InformationChunkProducer;
static ObjectProducerTemplate<Object> _ObjectProducer;
static ObjectProducerTemplate<ObjectManager> _ObjectManagerProducer;
static FileProducerTemplate<LocalFileImpl> _LocalFileProducer;
static FileProducerTemplate<StdFileImpl> _StdFileProducer;
#ifdef PXL_ENABLE_SFTP
static FileProducerTemplate<sFTPFileImpl> _sFTPFileProducer;
#endif
#ifdef PXL_ENABLE_DCAP
static FileProducerTemplate<dCapFileImpl> _dCapFileProducer;
#endif
void Core::initialize()
{
    if (_initialized)
        return;

    _BasicContainerProducer.initialize();
    _BasicMatrixProducer.initialize();
    _BasicNVectorProducer.initialize();
    _EventProducer.initialize();
    _InformationChunkProducer.initialize();
    _ObjectProducer.initialize();
    _ObjectManagerProducer.initialize();
    _LocalFileProducer.initialize("file");
    _StdFileProducer.initialize("std");
#ifdef PXL_ENABLE_SFTP
    _sFTPFileProducer.initialize("ssh");
#endif
#ifdef PXL_ENABLE_DCAP
    _dCapFileProducer.initialize("dcap");
#endif
    _initialized = true;
}

void Core::shutdown()
{
    if (_initialized == false)
        return;

    _BasicMatrixProducer.shutdown();
    _BasicContainerProducer.shutdown();
    _BasicNVectorProducer.shutdown();
    _EventProducer.shutdown();
    _InformationChunkProducer.shutdown();
    _ObjectProducer.shutdown();
    _ObjectManagerProducer.shutdown();
    _LocalFileProducer.shutdown();
#ifdef PXL_ENABLE_SFTP
    _sFTPFileProducer.shutdown();
#endif
#ifdef PXL_ENABLE_DCAP
    _dCapFileProducer.shutdown();
#endif
    _initialized = false;
}

} // namespace pxl
