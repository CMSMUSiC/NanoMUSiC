//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_RELATIVE_HH
#define PXL_BASE_RELATIVE_HH

#include <iostream>
#include <stdexcept>
#include <string>

#include "Pxl/Pxl/interface/pxl/core/Id.hpp"
#include "Pxl/Pxl/interface/pxl/core/Relations.hpp"
#include "Pxl/Pxl/interface/pxl/core/Serializable.hpp"
#include "Pxl/Pxl/interface/pxl/core/SoftRelations.hpp"
#include "Pxl/Pxl/interface/pxl/core/WkPtrBase.hpp"

namespace pxl
{

class ObjectOwner;

/**
 This base class provides common functionalities for all derived PXL objects,
 such as mother-daughter relations, weak pointer concept and related service routines.
 It has a C++ pointer to the pxl::ObjectOwner it is aggregated in in order to avoid
 mother/daughter relations to outside objects to be established.
 */
class PXL_DLL_EXPORT Relative : public Serializable
{
  public:
    /// Destructor, ensures safe deletion of all hard relations.
    virtual ~Relative();

    /// Returns the PXL unique object-id (UUID)
    inline Id id() const
    {
        return getId();
    }

    static const Id &getStaticTypeId()
    {
        static const Id id("5dee644d-906f-4d8e-aecc-d9a644293260");
        return id;
    }

    /// Returns the unique ID of this class
    virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    virtual void serialize(const OutputStream &out) const
    {
        Serializable::serialize(out);

        _softRelations.serialize(out);

        out.writeString(_name);

        // write out for historic reasons (layout), to be deprecated in pxl 4.0
        out.writeBool(false);
    }

    virtual void deserialize(const InputStream &in)
    {
        Serializable::deserialize(in);

        _softRelations.deserialize(in);

        in.readString(_name);

        // read for historic reasons (layout), to be deprecated in pxl 4.0
        bool hasLayout;
        in.readBool(hasLayout);
    }

    /// Returns a C++ pointer to the pxl::ObjectOwner it is owned by.
    inline ObjectOwner *owner() const
    {
        return _refObjectOwner;
    }

    /// Creates a deep copy and returns a C++ pointer to the newly-created object.
    virtual Serializable *clone() const
    {
        return new Relative(*this);
    }

    /// Grants const access to the Relations instance managing mother relations
    const Relations &getMotherRelations() const
    {
        return _motherRelations;
    }
    /// Grants const access to the pxl::Relations instance managing daughter relations
    const Relations &getDaughterRelations() const
    {
        return _daughterRelations;
    }

    /// Grants const access to the pxl::Relations instance managing flat relations
    const Relations &getFlatRelations() const
    {
        return _flatRelations;
    }

    /// Returns the first entry of the mother relations.
    /// In case the collection is empty, 0 is returned.
    Relative *getMother() const
    {
        return _motherRelations.getFirst();
    }

    /// Returns the first entry of the daughter relations.
    /// In case the collection is empty, 0 is returned.
    Relative *getDaughter() const
    {
        return _daughterRelations.getFirst();
    }

    /// Returns all daughter relations.
    const std::set<Relative *> &getDaughters() const
    {
        return _daughterRelations.getContainer();
    }

    /// Returns all mother relations.
    const std::set<Relative *> &getMothers() const
    {
        return _motherRelations.getContainer();
    }

    /// Returns number of daughters
    size_t numberOfDaughters() const
    {
        return _daughterRelations.size();
    }

    /// Returns number of mothers
    size_t numberOfMothers() const
    {
        return _motherRelations.size();
    }

    /// Establishes a mother relation to the \p target object; please notice, that
    /// only relations between objects owned by the same object owner will be established.
    void linkMother(Relative *target);
    /// Establishes a daughter relation to the \p target object; please notice, that
    /// only relations between objects owned by the same object owner will be established.
    void linkDaughter(Relative *target);
    /// Establishes a flat relation to the \p target object; please notice, that
    /// only relations between objects owned by the same object owner will be established.
    void linkFlat(Relative *target);

    /// Removes an existing daughter relation to the \p target object.
    void unlinkMother(Relative *target);
    /// Removes an existing daughter relation to the \p target object.
    void unlinkDaughter(Relative *target);
    /// Removes an existing daughter relation to the \p target object.
    void unlinkFlat(Relative *target);

    /// Removes all existing mother relations.
    void unlinkMothers();
    /// Removes all existing daughter relations.
    void unlinkDaughters();
    /// Removes all existing flat relations.
    void unlinkFlat();

    /// Create a soft relation with name \p type to the Relative \p relative
    void linkSoft(Relative *relative, const std::string &type);

    /// Remove a soft relation with name \p type to the Relative \p relative
    void unlinkSoft(Relative *relative, const std::string &type);

    /// Get access to the soft relations.
    const SoftRelations &getSoftRelations() const
    {
        return _softRelations;
    }

    /// Get access to the soft relations.
    SoftRelations &getSoftRelations()
    {
        return _softRelations;
    }

    /// Returns the name.
    inline const std::string &getName() const
    {
        return _name;
    }

    /// Sets the name to the contents of \p v.
    inline void setName(const std::string &v)
    {
        _name = v;
    }

    /// Recursively invokes its own and the print() methods of all daughter objects.
    /// @param level verbosity level
    /// @param os output _stream, default is std::cout
    /// @param pan print indention
    /// @return output _stream
    std::ostream &printDecayTree(int level = 0, std::ostream &os = std::cout, int pan = 1) const;

    /// Prints out object state information on various verbosity levels.
    /// @param level verbosity level
    /// @param os output _stream, default is std::cout
    /// @param pan print indention
    /// @return output _stream
    virtual std::ostream &print(int level = 1, std::ostream &os = std::cout, int pan = 0) const;

    /// Creates a weak pointer to itself and returns a pxl::WkPtrBase* to the newly-created weak pointer instance.
    virtual WkPtrBase *createSelfWkPtr()
    {
        throw std::runtime_error(
            "pxl::ObjectBase::createSelfWkPtr(): ATTENTION! Inheriting class must reimplement this virtual method.");
        return 0;
    }

  protected:
    /// Default constructor.
    Relative() : Serializable(), _refWkPtrSpec(0), _refObjectOwner(0), _name("default")
    {
    }

    /// Copy constructor. Relations are not copied.
    Relative(const Relative &original) : Serializable(), _refWkPtrSpec(0), _refObjectOwner(0), _name(original._name)
    {
    }

    /// Copy constructor. Relations are not copied.
    explicit Relative(const Relative *original)
        : Serializable(), _refWkPtrSpec(0), _refObjectOwner(0), _name(original->_name)
    {
    }

    /// Service method for printing relations tree
    std::ostream &printPan1st(std::ostream &os, int pan) const;

    /// Service method for printing relations tree
    std::ostream &printPan(std::ostream &os, int pan) const;

  private:
    WkPtrBase *_refWkPtrSpec;     /// reference to a weak pointer
    ObjectOwner *_refObjectOwner; /// reference to our object owner

    Relations _motherRelations;   /// mother relations, managed by object owner
    Relations _daughterRelations; /// daughter relations, managed by object owner
    Relations _flatRelations;     /// flat relations, managed by object owner

    SoftRelations _softRelations; /// soft relations

    std::string _name; /// arbitrary name of this object

    friend class WkPtrBase;
    friend class ObjectOwner;

    /// No assignment of Relative derivatives is allowed, the assignment
    /// operator is private
    Relative &operator=(const Relative &original)
    {
        return *this;
    }
};

} // namespace pxl
// namespace pxl

// operators
PXL_DLL_EXPORT std::ostream &operator<<(std::ostream &cxxx, const pxl::Relative &obj);

#endif // PXL_BASE_OBJECTBASE_HH
