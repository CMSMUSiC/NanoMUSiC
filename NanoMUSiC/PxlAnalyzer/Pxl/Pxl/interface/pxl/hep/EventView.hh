//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_EVENT_VIEW_hh
#define PXL_HEP_EVENT_VIEW_hh
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include "Pxl/Pxl/interface/pxl/core/ObjectManager.hh"
#include "Pxl/Pxl/interface/pxl/core/weak_ptr.hh"

namespace pxl
{

// pol
/**
 By inheritance from pxl::ObjectManager,
 this class is capable of holding the complete information of one
 multicollision event with decay trees, spatial vertex information,
 four-momenta as well as additional event-related reconstruction data
 in the user records. Physics objects (i.e. instances of the classes pxl::Particle,
 pxl::Vertex or pxl::Collision) as well as other arbitrary pxl::Relative
 derivatives can be aggregated and managed.
 The name 'event view'  arises from the fact that it is
 intended to represent a distinct view of an event (e.g. connecting
 particles to the decay tree according to one out of a
 number of hypotheses, applying different jet energy corrections, etc.).
 To facilitate the development of numerous
 parallel or subsequent event views, as needed for hypothesis evolution,
 for instance, this class features a copy constructor
 which provides a deep copy of the event container with all data members,
 hep objects, and their (redirected) relations.
 This way, PXL provides a flexible generalized event container
 meeting the needs of HEP analyses in channels with ambiguous
 event topologies.
 */
class PXL_DLL_EXPORT EventView : public ObjectManager
{
  public:
    EventView() : ObjectManager()
    {
    }
    /// This copy constructor provides a deep copy of the event container \p original with all data members,
    /// hep objects, and their (redirected) relations.
    EventView(const EventView &original) : ObjectManager(original)
    {
    }
    /// This copy constructor provides a deep copy of the event container \p original with all data members,
    /// hep objects, and their (redirected) relations.
    explicit EventView(const EventView *original) : ObjectManager(original)
    {
    }

    virtual WkPtrBase *createSelfWkPtr()
    {
        return new weak_ptr<EventView>(this);
    }

    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    static const Id &getStaticTypeId();

    virtual void serialize(const OutputStream &out) const
    {
        ObjectManager::serialize(out);
    }

    virtual void deserialize(const InputStream &in)
    {
        ObjectManager::deserialize(in);
    }

    virtual Serializable *clone() const
    {
        return new EventView(*this);
    }

    virtual std::ostream &print(int level = 0, std::ostream &os = std::cout, int pan = 1) const;

  private:
    EventView &operator=(const EventView &original)
    {
        return *this;
    }
};

} // namespace pxl

#endif // PXL_HEP_EVENT_VIEW_hh
