//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_OBJECT_OWNER
#define PXL_BASE_OBJECT_OWNER
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <algorithm>
#include <map>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core/Relative.hpp"
#include "Pxl/Pxl/interface/pxl/core/weak_ptr.hpp"

namespace pxl
{

class ObjectOwner;

// - - - - - - - - - - - - - - - - - - - - - - - - - - -
/// For STL-style iteration on selective class: iterator class template;
/// this iterator behaves like a normal STL iterator but ignores all objects
/// that cannot be interpreted as type objecttype (tested by dynamic casts).
/// Use in STL-style, except that either begin gets the objecttype class as template argument,
/// or a constructor of the TypeIterator is used.
template <class objecttype>
class ObjectOwnerTypeIterator
{
    typedef std::vector<Relative *>::const_iterator const_iterator;
    typedef std::vector<Relative *>::iterator iterator;

  public:
    /// Copy constructor.
    ObjectOwnerTypeIterator(const ObjectOwnerTypeIterator &other);

    /// Constructor from ObjectOwner instance.
    ObjectOwnerTypeIterator(const ObjectOwner &container);

    /// Constructor from ObjectOwner instance.
    ObjectOwnerTypeIterator(const ObjectOwner *container);

    const ObjectOwnerTypeIterator<objecttype> operator++(int);

    const ObjectOwnerTypeIterator<objecttype> &operator++();

    inline objecttype *operator*();

    inline bool operator==(const_iterator iter);

    inline bool operator!=(const_iterator iter);

  private:
    const ObjectOwner *_containerRef;
    const_iterator _iter;
};

/**
 This class is an active container for Relative derivatives, such as
 Vertex, Particle or EventView;
 it has the ownership and deletion responsability for the contained objects.
 The method template create() can be used to create derivatives
 of Relative within object owners. The method insert() can be
 used to manually add objects, has() tests object ownerships, and
 remove() explicitely removes objects from the owner and deletes them.
 The copy constructor of the class ObjectOwner also produces copies of
 the contained objects, and re-establishes corresponding mother-daughter relations
 amongst the copied objects. For the convenience of a quick and targeted object access, the
 newly created owner carries a so-called copy history for mapping original and
 copied objects. This information is used by the findCopyOf() method:
 provided a reference to the original object, this method returns a pointer to the copied object.
 A further, powerful tool for targeted object access is the so-called index, which
 allows to map objects to unique string identifiers, the key. The method findObject()
 can be used to directly access objects by their keys or object-ids.
 The ObjectOwner extends the functionality of the contained STL vector. It provides a selective iterator, the class
 template ObjectOwner::TypeIterator, that ignores all objects other than the specialized data type.
 */
class PXL_DLL_EXPORT ObjectOwner
{
  public:
    ObjectOwner() : _container(), _copyHistory(), _index(), _uuidSearchMap()
    {
    }
    /// This copy constructor performs a deep copy of object
    /// owner \p original and all contained objects with (redirected) relations.
    /// A copy history keeps track of originals and copies
    /// and the findCopyOf() method allows quick access to the copies.
    ObjectOwner(const ObjectOwner &original) : _container(), _copyHistory(), _index(), _uuidSearchMap()
    {
        this->init(original);
    }

    /// This copy constructor performs a deep copy of object
    /// owner \p original and all contained objects with (redirected) relations.
    /// A copy history keeps track of originals and copies
    /// and the findCopyOf() method allows quick access to the copies.
    explicit ObjectOwner(const ObjectOwner *original) : _container(), _copyHistory(), _index(), _uuidSearchMap()
    {
        this->init(*original);
    }
    /// This destructor deletes all contained objects.
    virtual ~ObjectOwner()
    {
        this->clearContainer();
    }

    ObjectOwner &operator=(const ObjectOwner &original)
    {
        if (this != &original)
        {
            this->clearContainer();
            this->init(original);
        }
        return *this;
    }

    virtual void serialize(const OutputStream &out) const;

    virtual void deserialize(const InputStream &in);

    /// Creates a new instance of \p objecttype;
    /// objecttype must be a class inheriting from Relative;
    /// the newly-created instance is owned and will be deleted by this object owner.
    template <class objecttype>
    objecttype *create()
    {
        objecttype *pitem = new objecttype;
        pitem->_refObjectOwner = this;
        _container.push_back(static_cast<Relative *>(pitem));
        _uuidSearchMap.insert(std::pair<Id, Relative *>(pitem->getId(), pitem));
        return pitem;
    }

    /// Creates a copy of \p original by invoking the copy constructor of \p objecttype;
    /// \p objecttype must be a class inheriting from Relative;
    /// the newly-created instance is owned and will be deleted by this object owner.
    template <class objecttype>
    objecttype *create(const objecttype *original)
    {
        objecttype *pitem = new objecttype(*original);
        pitem->_refObjectOwner = this;
        _container.push_back(static_cast<Relative *>(pitem));
        _uuidSearchMap.insert(std::pair<Id, Relative *>(pitem->getId(), pitem));
        return pitem;
    }

    /// Inserts \p value in the container of this object owner and takes deletion responsibility.
    void set(Relative *value)
    {
        insert(value);
    }

    /// Inserts \p value in the container of this object owner and takes deletion responsibility.
    void insert(Relative *value);

    /// Deletes \p value.
    void remove(Relative *value);

    /// Takes \p value from the object owner, meaning that
    /// the object owner loses the ownership of \p value.
    void take(Relative *value);

    /// Returns true if \p value is owned by this object owner.
    bool has(const Relative *value) const;

    /// Clears the object owner and deletes all owned objects.
    void clearContainer();

    /// Searches the index for the \p key and returns a dynamically casted
    /// C++ pointer of type \p objecttype* to the corresponding object;
    /// in case key is not found, a null pointer is returned.
    template <class objecttype>
    inline objecttype *findObject(const std::string &key) const // goes via Index & casts

    {
        std::map<std::string, Relative *>::const_iterator it = _index.find(key);
        if (it != _index.end())
            return dynamic_cast<objecttype *>(it->second);
        return 0;
    }

    /// Searches the index for the \p key and returns a dynamically casted
    /// and returns a Relative pointer to the corresponding object.
    /// In case \p key is not found, a null pointer is returned.
    inline Relative *findObject(const std::string &key) const
    {
        std::map<std::string, Relative *>::const_iterator it = _index.find(key);
        if (it != _index.end())
            return it->second;
        return 0;
    }

    /// Returns a Relative pointer for a contained object with the passed ID.
    /// In case the Relative is not in the container, 0 is returned.
    Relative *getById(Id id) const
    {
        std::map<Id, Relative *>::const_iterator found = _uuidSearchMap.find(id);
        if (found != _uuidSearchMap.end())
            return found->second;
        return 0;
    }

    /// Searches the copy history to locate the copy of \p original and
    /// returns a dynamically casted C++ pointer of type \p objecttype* to the corresponding copy;
    /// in case no copy can be traced a null pointer is returned.
    template <class objecttype>
    objecttype *findCopyOf(const Relative *original) const // goes via CopyHistory & casts

    {
        std::map<Id, Relative *>::const_iterator it = _copyHistory.find(original->id());
        if (it != _copyHistory.end())
            return dynamic_cast<objecttype *>(it->second);
        return 0;
    }

    /// Provides direct access to the copy history (created by the copy constructor).
    inline const std::map<Id, Relative *> &getCopyHistory() const
    {
        return _copyHistory;
    }
    /// Clears the copy history  (created by the copy constructor).
    inline void clearCopyHistory()
    {
        _copyHistory.clear();
    }

    /// Registers the object \p obj with the \p key in the index and returns true in case of success;
    /// in case the key string is present, by default an error message is given. A bool can be set which allows
    /// overwriting. Please notice that \p obj must be owned by this object owner and \p key must not be a zero length
    /// string.
    bool setIndexEntry(const std::string &key, Relative *obj, bool overwrite = false);

    /// Provides direct read access to the index.
    inline const std::map<std::string, Relative *> &getIndexEntry() const
    {
        return _index;
    }
    /// Removes the index entry with the \p key; please notice: it does not remove the object itself.
    inline void removeIndexEntry(const std::string &key)
    {
        _index.erase(key);
    }
    /// Clears the index; please notice: it does not remove the objects themselves.
    inline void clearIndex()
    {
        _index.clear();
    }

    /// Allows read access to the contained STL vector of Relative pointers to, e.g., use STL algorithms.
    const std::vector<Relative *> &getObjects() const
    {
        return _container;
    }

    /// Typedef for standard const_iterator.
    typedef std::vector<Relative *>::const_iterator const_iterator;
    typedef std::vector<Relative *>::iterator iterator;

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

    /// Returns the number of elements the ObjectOwner holds.
    inline size_t size() const
    {
        return _container.size();
    }

    /// Fills into the passed vector weak pointers to the objects of the type specified by the template argument.
    template <class objecttype>
    size_t getObjectsOfType(std::vector<objecttype *> &vec) const
    {
        size_t size = vec.size();
        for (ObjectOwner::const_iterator iter = begin(); iter != end(); ++iter)
        {
            objecttype *obj = dynamic_cast<objecttype *>(*iter);
            if (obj != 0)
                vec.push_back(obj);
        }
        return vec.size() - size;
    }

    /// This templated method provides an STL-style begin()-method to
    /// initialise the TypeIterator.
    template <class objecttype>
    const ObjectOwnerTypeIterator<objecttype> begin() const
    {
        ObjectOwnerTypeIterator<objecttype> it(this);
        return it;
    }

    /// Sorts the container according to the passed function.
    /// See the STL sort documentation for more details and examples.
    void sort(int (*comp)(Relative *, Relative *))
    {
        std::sort(_container.begin(), _container.end(), comp);
    }

  private:
    void init(const ObjectOwner &original);

    std::vector<Relative *> _container;
    std::map<Id, Relative *> _copyHistory;
    std::map<std::string, Relative *> _index;
    std::map<Id, Relative *> _uuidSearchMap;
};

/// Copy constructor.
template <class objecttype>
ObjectOwnerTypeIterator<objecttype>::ObjectOwnerTypeIterator(const ObjectOwnerTypeIterator &other)
    : _containerRef(other._containerRef), _iter(other._iter)
{
}

/// Constructor from ObjectOwner instance.
template <class objecttype>
ObjectOwnerTypeIterator<objecttype>::ObjectOwnerTypeIterator(const ObjectOwner &container)
    : _containerRef(&container), _iter(container.begin())
{
    if (_iter != _containerRef->end() && dynamic_cast<objecttype *>(*_iter) == 0)
        (*this)++;
}

/// Constructor from ObjectOwner instance.
template <class objecttype>
ObjectOwnerTypeIterator<objecttype>::ObjectOwnerTypeIterator(const ObjectOwner *container)
    : _containerRef(container), _iter(container->begin())
{
    if (_iter != _containerRef->end() && dynamic_cast<objecttype *>(*_iter) == 0)
        (*this)++;
}

template <class objecttype>
const ObjectOwnerTypeIterator<objecttype> ObjectOwnerTypeIterator<objecttype>::operator++(int)
{
    ObjectOwnerTypeIterator orig = *this;
    if (_iter != _containerRef->end())
        do
            _iter++;
        while (_iter != _containerRef->end() && dynamic_cast<objecttype *>(*_iter) == 0);
    return orig;
}

template <class objecttype>
const ObjectOwnerTypeIterator<objecttype> &ObjectOwnerTypeIterator<objecttype>::operator++()
{
    if (_iter != _containerRef->end())
        do
            _iter++;
        while (_iter != _containerRef->end() && dynamic_cast<objecttype *>(*_iter) == 0);
    return *this;
}

template <class objecttype>
inline objecttype *ObjectOwnerTypeIterator<objecttype>::operator*()
{
    return _iter == _containerRef->end() ? 0 : dynamic_cast<objecttype *>(*_iter);
}

template <class objecttype>
inline bool ObjectOwnerTypeIterator<objecttype>::operator==(const_iterator iter)
{
    return (_iter == iter);
}

template <class objecttype>
inline bool ObjectOwnerTypeIterator<objecttype>::operator!=(const_iterator iter)
{
    return (_iter != iter);
}

} // namespace pxl

#endif // PXL_BASE_OBJECT_OWNER
