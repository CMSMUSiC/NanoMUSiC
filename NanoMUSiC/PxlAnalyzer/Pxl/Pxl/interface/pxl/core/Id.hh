//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_ID_HH
#define PXL_BASE_ID_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include "Pxl/Pxl/interface/pxl/core/Random.hh"
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <iostream>

#include "Pxl/Pxl/interface/pxl/core/Stream.hh"

namespace pxl
{

/**
 This class contains an implementation of a unique
identification (or UUID) for various objects used in PXL, e.g. Object Id
 or Type Id. A new Id can be created with the static method create().
 */

class PXL_DLL_EXPORT Id
{
  private:
    unsigned char bytes[16]; /// Storage for the actual 16-digit ID.

  public:
    /// Constructor, creates a new UUID.
    Id();

    /// Constructor, reads a UUID from the InputStream \p in..
    Id(const InputStream &in);

    /// Constructor from a char array \p id.
    explicit Id(const char *id);

    /// Constructor from a string \p id.
    explicit Id(const std::string &id);

    /// Constructor with explicit passing of a Random object \p rand.
    Id(Random &rand);

    /// Geneate an ID with explicit passing of a Random object \p rand.
    void generate(Random &rand);

    /// Generate an ID.
    void generate();

    /// Sets the ID to 0 (i.e. all components to 0).
    void reset();

    /// Create a new UUID. Standard way of creating a new Id.
    static Id create();

    bool operator==(const Id &id) const;
    bool operator!=(const Id &id) const;

    /// Less-than-operator, provides ordering of IDs.
    bool operator<(const Id &op) const;

    /// Returns a string representation of the ID.
    std::string toString() const;

    /// Write this Id to the OutputStream \p out
    void serialize(const OutputStream &out) const;

    /// Fill the content of this Id from the InputStream \p in
    void deserialize(const InputStream &in);
};

PXL_DLL_EXPORT std::ostream &operator<<(std::ostream &os, const Id &id);

} // namespace pxl

#endif // PXL_BASE_ID_HH
