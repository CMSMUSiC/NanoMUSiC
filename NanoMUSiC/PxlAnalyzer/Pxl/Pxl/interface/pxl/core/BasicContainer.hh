//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_BASICCONTAINER_HH
#define PXL_BASE_BASICCONTAINER_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <map>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core/UserRecord.hh"

#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"
#include "Pxl/Pxl/interface/pxl/core/Serializable.hh"

namespace pxl
{

/**
 A Container for all objects that inherit from pxl::Serializable. This should be a rudiment version of the
 pxl::ObjectOwner.
 */
class PXL_DLL_EXPORT BasicContainer : public Serializable, public UserRecordHelper
{
  public:
    /// The Index for the BasicContainer
    typedef std::map<std::string, Serializable *> map_t;

    BasicContainer() : Serializable()
    {
    }

    BasicContainer(const BasicContainer &basicContainer)
        : Serializable(basicContainer), UserRecordHelper(basicContainer), _container(), _index(), _uuidSearchMap()
    {
        this->init(basicContainer);
    }

    explicit BasicContainer(const BasicContainer *basicContainer)
        : Serializable(*basicContainer), UserRecordHelper(*basicContainer), _container(), _index(), _uuidSearchMap()
    {
        this->init(*basicContainer);
    }

    BasicContainer &operator=(const BasicContainer &original)
    {
        if (this != &original)
        {
            clearContainer();
            this->init(original);
        }
        return *this;
    }

    void init(const BasicContainer &basicContainer);

    /// This destructor deletes all contained objects.
    virtual ~BasicContainer()
    {
        this->clearContainer();
    }

    /// Creates a deep copy and returns a C++ pointer to the newly-created object.
    virtual Serializable *clone() const
    {
        return new BasicContainer(*this);
    }

    /// Allows read access to the contained STL vector of
    /// Relative pointers to, e.g., use STL algorithms.
    const std::vector<Serializable *> &getObjects() const
    {
        return _container;
    }

    /// Creates a new instance of \p objecttype;
    /// objecttype must be a class inheriting from pxl::Serializable;
    /// the newly-created instance is owned and will be deleted by this object owner.
    template <class objecttype> objecttype *create()
    {
        objecttype *pitem = new objecttype;
        _container.push_back(static_cast<Serializable *>(pitem));
        _uuidSearchMap.insert(std::pair<Id, Serializable *>(pitem->getId(), pitem));
        return pitem;
    }

    /// Creates a copy of \p original by invoking the
    /// copy constructor of \p objecttype; / \p objecttype must be a class
    /// inheriting from pxl::Serializable; / the newly-created instance
    /// is owned and will be deleted by this object owner.
    template <class objecttype> objecttype *create(const objecttype *original)
    {
        objecttype *pitem = new objecttype(*original);
        _container.push_back(static_cast<Serializable *>(pitem));
        _uuidSearchMap.insert(std::pair<Id, Serializable *>(pitem->getId(), pitem));
        return pitem;
    }

    /// Inserts \p value in the container of this object owner
    /// and takes deletion responsability.
    void insertObject(Serializable *value);

    /// Deletes \p value.
    void remove(Serializable *value);

    /// Deletes all objects of \p objecttype; returns number of deleted
    /// objects
    size_t removeObjectsOfType(const Id &typeId);

    template <class objecttype> size_t removeObjectsOfType()
    {
        return removeObjectsOfType(objecttype::getStaticTypeId());
    }

    /// Takes \p value from the container.
    void take(Serializable *value);

    /// Returns true if \p value is owned by this object owner.
    bool has(const Serializable *value) const;

    /// Clears the object owner and deletes all owned objects.
    void clearContainer();

    /// Typedef for standard const_iterator.
    typedef std::vector<Serializable *>::const_iterator const_iterator;
    typedef std::vector<Serializable *>::iterator iterator;

    /// This returns the const iterator to the first element of the contained vector.
    inline const_iterator begin() const
    {
        return _container.begin();
    }

    /// This returns the iterator to the first element of the contained vector.
    inline iterator begin()
    {
        return _container.begin();
    }

    /// This returns the const iterator to the end of the contained vector.
    inline const_iterator end() const
    {
        return _container.end();
    }

    /// This returns the iterator to the end of the contained vector.
    inline iterator end()
    {
        return _container.end();
    }

    /// Returns the number of elements the BasicContainer holds.
    inline size_t size() const
    {
        return _container.size();
    }

    /// This method searches the index for the \p key and returns a
    /// dynamically casted / C++ pointer of type \p objecttype* to the
    /// corresponding object; / in case the key is not found a null pointer is
    /// returned.
    template <class objecttype> inline objecttype *findObject(const std::string &key) const // goes via Index & casts

    {
        map_t::const_iterator it = _index.find(key);
        if (it != _index.end())
            return dynamic_cast<objecttype *>(it->second);
        return 0;
    }

    inline Serializable *findObject(const std::string &key) const
    {
        map_t::const_iterator it = _index.find(key);
        if (it != _index.end())
            return it->second;
        return 0;
    }

    /// Returns a Serializable pointer for a contained object with the passed ID.
    /// In case the Serializable is not in the container, 0 is returned.
    Serializable *getById(Id id) const
    {
        std::map<Id, Serializable *>::const_iterator found = _uuidSearchMap.find(id);
        if (found != _uuidSearchMap.end())
            return found->second;
        return 0;
    }

    /// Fills into the passed vector weak pointers to the objects of the
    /// type specified by the template argument.
    template <class objecttype> size_t getObjectsOfType(std::vector<objecttype *> &vec) const
    {
        size_t size = vec.size();
        for (BasicContainer::const_iterator iter = begin(); iter != end(); ++iter)
        {
            objecttype *obj = dynamic_cast<objecttype *>(*iter);
            if (obj != 0)
                vec.push_back(obj);
        }
        return vec.size() - size;
    }

    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    static const Id &getStaticTypeId()
    {
        static const Id id("38ebda57-df6f-a577-b811-0bba49745b09");
        return id;
    }

    virtual void serialize(const OutputStream &out) const;

    virtual void deserialize(const InputStream &in);

    //////////////////////////////////////////////
    // User Record
    //////////////////////////////////////////////

    /// This method provides direct read access to the index.
    inline const map_t &getIndexEntry() const
    {
        return _index;
    }

    inline void removeIndexEntry(const std::string &key)
    {
        _index.erase(key);
    }

    /// This method clears the index; please notice: it does not remove the objects themselves.
    inline void clearIndex()
    {
        _index.clear();
    }

  private:
    std::vector<Serializable *> _container;
    map_t _index;
    std::map<Id, Serializable *> _uuidSearchMap;
};

} // namespace pxl

#endif // PXL_BASE_BASICCONTAINER_HH
