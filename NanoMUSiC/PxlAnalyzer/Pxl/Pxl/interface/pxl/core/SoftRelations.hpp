//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_SOFTRELATIONS_HH
#define PXL_BASE_SOFTRELATIONS_HH

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core/Id.hpp"
#include "Pxl/Pxl/interface/pxl/core/Serializable.hpp"
#include "Pxl/Pxl/interface/pxl/core/Stream.hpp"

namespace pxl
{
class ObjectOwner;
class ObjectManager;
class BasicContainer;

/**
 This class provides the storage of relations to any Serializable derivatives, by means
 of the unique Id (UUID) of each object. Each relation has a certain name.
 In contrast to the standard Relations class, relations between objects with different
 (or no) object owners can be established.
 The relations are stored as pairs of names (string) and UUIDs (Id). More than one soft relation
 with the same name is allowed to be established. Since a relation is not stored as a pointer,
 it can only be retrieved indirectly. This also means that soft relations exist beyond the
 lifetime of the related objects, unless they are explicitly removed.
 An object of type SoftRelation can be added to any class which is supposed to provide such relations,
 e.g. all derivatives of the Relative such as the Particle.
*/
class PXL_DLL_EXPORT SoftRelations
{
  public:
    typedef std::multimap<std::string, Id>::const_iterator const_iterator;
    typedef std::multimap<std::string, Id>::iterator iterator;

    /// Writes the current content to the passed stream.
    void serialize(const OutputStream &out) const;

    /// Replaces the current content with the content of passed stream, which is read in.
    void deserialize(const InputStream &in);

    /// Returns the first Serializable pointer which is contained in the passed object owner \p owner
    /// and which is contained in these soft relations. If the name of the soft relation \p name
    /// is also passed, the first Serializable which has the soft relation of the according name is returned.
    /// Returns 0 in case no match is found.
    Serializable *getFirst(const ObjectOwner &owner, const std::string &name = "") const;

    /// Returns the first Serializable pointer which is contained in the passed BasicContainer \p container
    /// and which is contained in these soft relations. If the name of the soft relation \p name
    /// is also passed, the first Serializable which has the soft relation of the according name is returned.
    /// Returns 0 in case no match is found.
    Serializable *getFirst(const BasicContainer &container, const std::string &name = "") const;

    /// Fills all Serializables which are both contained in the object owner \p owner as well as in these soft
    /// relations with the passed name \p name into the passed vector of Serializable pointers.
    /// Returns the number of filled Serializables.
    int getSoftRelatives(std::vector<Serializable *> &vec, const ObjectOwner &owner, const std::string &name) const;

    /// Fills all Serializables which are both contained in the object owner \p owner as well as in these soft
    /// relations into the passed vector of Serializable pointers.
    /// Returns the number of filled Serializables.
    int getSoftRelatives(std::vector<Serializable *> &vec, const ObjectOwner &owner) const;

    /// Fills all Serializables which are both contained in the object owner of object manager \p manager as well as
    /// in these soft relations with the passed name \p name into the passed vector of Serializable pointers.
    /// Returns the number of filled Serializables.
    int getSoftRelatives(std::vector<Serializable *> &vec, const ObjectManager &manager, const std::string &name) const;

    /// Fills all Serializables which are both contained in the object owner of object manager \p manager as well as
    /// in these soft relations into the passed vector of Serializable pointers.
    /// Returns the number of filled Serializables.
    int getSoftRelatives(std::vector<Serializable *> &vec, const ObjectManager &manager) const;

    /// Fills all Serializables which are both contained in the BasicContainer \p container as well as in these soft
    /// relations with the passed name \p name into the passed vector of Serializable pointers.
    /// Returns the number of filled Serializables.
    int getSoftRelatives(std::vector<Serializable *> &vec, const BasicContainer &container,
                         const std::string &name) const;

    /// Fills all Serializables which are both contained in the BasicContainer \p container as well as in these soft
    /// relations into the passed vector of Serializable pointers.
    /// Returns the number of filled Serializables.
    int getSoftRelatives(std::vector<Serializable *> &vec, const BasicContainer &container) const;

    /// Fills all Serializables which are both contained in the  object owner \p owner as well as in these soft
    /// relations with the passed name \p name into the passed vector of objectname pointers.
    /// Only objects which match the template type (or derivatives) are filled.
    /// Returns the number of filled objects.
    template <class objecttype>
    int getSoftRelativesOfType(std::vector<objecttype *> &vec, const ObjectOwner &owner, const std::string &name) const;

    /// Fills all Serializables which are both contained in the object owner \p owner as well as in these soft
    /// relations into the passed vector of objecttype pointers.
    /// Only objects which match the template name (or derivatives) are filled.
    /// Returns the number of filled objects.
    template <class objecttype>
    int getSoftRelativesOfType(std::vector<objecttype *> &vec, const ObjectOwner &owner) const;

    /// Fills all Serializables which are both contained in the  basic container \p container as well as in these soft
    /// relations with the passed name \p name into the passed vector of objecttype pointers.
    /// Only objects which match the template type (or derivatives) are filled.
    /// Returns the number of filled objects.
    template <class objecttype>
    int getSoftRelativesOfType(std::vector<objecttype *> &vec, const BasicContainer &owner,
                               const std::string &name) const;

    /// Fills all Serializables which are both contained in the basic container \p container as well as in these soft
    /// relations into the passed vector of objecttype pointers.
    /// Only objects which match the template type (or derivatives) are filled.
    /// Returns the number of filled objects.
    template <class objecttype>
    int getSoftRelativesOfType(std::vector<objecttype *> &vec, const BasicContainer &owner) const;

    /// Removes all Serializables from the passed vecttor which are NOT contained in these soft
    /// relations with the passed name \p name.
    /// Returns the number of remaining Serializables.
    int keepSoftRelatives(std::vector<Serializable *> &vec, const std::string &name) const;

    /// Removes all Serializables from the passed vecttor which are NOT contained in these soft
    /// relations.
    /// Returns the number of remaining Serializables.
    int keepSoftRelatives(std::vector<Serializable *> &vec) const;

    /// Returns true if the passed Serializable \p relative is contained in these soft relations.
    bool has(const Serializable *relative) const;

    /// Returns true if the passed Serializable \p relative is contained in these soft relations
    /// with the name \p name.
    bool has(const Serializable *relative, const std::string &name) const;

    /// Returns true if the passed UUID \p id is contained in these soft relations
    bool has(const Id &id) const;

    /// Returns true if the passed UUID \p id is contained in these soft relations
    /// with the name \p name.
    bool has(const Id &id, const std::string &name) const;

    /// Returns true if a soft relation with name \p name exists.
    bool hasName(const std::string &name) const;

    /// Returns the number of soft relations with name \p name.
    int count(const std::string &name) const;

    /// Establishes a soft relation to the passed Serializable \p relative and
    /// with the name \p name.
    void set(const Serializable *relative, const std::string &name);

    /// Removes all existing soft relations with name \p name
    /// to the passed Serializable \p relative.
    void remove(const Serializable *relative, const std::string &name);

    /// Removes all existing soft relations to the passed Serializable \p relative.
    void remove(const Serializable *relative);

    /// Returns the number of soft relations.
    size_t size() const
    {
        return _relationsMap.size();
    }

    /// Deletes all contained relations.
    void clearContainer()
    {
        _relationsMap.clear();
    }

    /// Gives const access to the underlying container.
    const std::multimap<std::string, Id> &getContainer() const
    {
        return _relationsMap;
    }

    /// Returns a const begin-iterator to the underlying container.
    const_iterator begin() const
    {
        return _relationsMap.begin();
    }

    /// Returns a begin-iterator to the underlying container.
    iterator begin()
    {
        return _relationsMap.begin();
    }

    /// Returns a const end-iterator to the underlying container.
    const_iterator end() const
    {
        return _relationsMap.end();
    }

    /// Returns an end-iterator to the underlying container.
    iterator end()
    {
        return _relationsMap.end();
    }

    /// Prints information about this object to the passed stream.
    std::ostream &print(int level = 0, std::ostream &os = std::cout, int pan = 1) const;

    const std::string toString() const
    {
        std::ostringstream ss;
        this->print(0, ss);
        return ss.str();
    }

  private:
    std::multimap<std::string, Id> _relationsMap; /// Map containing the soft relations (name, UUID)

    // Helper functions are needed to allow templates and forward
    // declarations here
    Serializable *getByIdHelper(const ObjectOwner &owner, const_iterator iter) const;
    Serializable *getByIdHelper(const BasicContainer &owner, const_iterator iter) const;
};

//
// Template methods have to be specified in the header!
//

template <class objecttype>
int SoftRelations::getSoftRelativesOfType(std::vector<objecttype *> &vec, const ObjectOwner &owner,
                                          const std::string &name) const
{
    int size = vec.size();
    std::pair<const_iterator, const_iterator> iterators = _relationsMap.equal_range(name);
    for (const_iterator iter = iterators.first; iter != iterators.second; ++iter)
    {
        Serializable *relative = getByIdHelper(owner, iter);
        if (relative != 0)
        {
            objecttype *object = dynamic_cast<objecttype *>(relative);
            if (object != 0)
                vec.push_back(object);
        }
    }
    return vec.size() - size;
}

template <class objecttype>
int SoftRelations::getSoftRelativesOfType(std::vector<objecttype *> &vec, const ObjectOwner &owner) const
{
    int size = vec.size();
    for (const_iterator iter = _relationsMap.begin(); iter != _relationsMap.end(); ++iter)
    {
        Serializable *relative = getByIdHelper(owner, iter);
        if (relative != 0)
        {
            objecttype *object = dynamic_cast<objecttype *>(relative);
            if (object != 0)
                vec.push_back(object);
        }
    }
    return vec.size() - size;
}

template <class objecttype>
int SoftRelations::getSoftRelativesOfType(std::vector<objecttype *> &vec, const BasicContainer &owner,
                                          const std::string &name) const
{
    int size = vec.size();
    std::pair<const_iterator, const_iterator> iterators = _relationsMap.equal_range(name);
    for (const_iterator iter = iterators.first; iter != iterators.second; ++iter)
    {
        Serializable *relative = getByIdHelper(owner, iter);
        if (relative != 0)
        {
            objecttype *object = dynamic_cast<objecttype *>(relative);
            if (object != 0)
                vec.push_back(object);
        }
    }
    return vec.size() - size;
}

template <class objecttype>
int SoftRelations::getSoftRelativesOfType(std::vector<objecttype *> &vec, const BasicContainer &owner) const
{
    int size = vec.size();
    for (const_iterator iter = _relationsMap.begin(); iter != _relationsMap.end(); ++iter)
    {
        Serializable *relative = getByIdHelper(owner, iter);
        if (relative != 0)
        {
            objecttype *object = dynamic_cast<objecttype *>(relative);
            if (object != 0)
                vec.push_back(object);
        }
    }
    return vec.size() - size;
}

} // namespace pxl

#endif /*PXL_BASE_SOFTRELATIONS_HH*/
