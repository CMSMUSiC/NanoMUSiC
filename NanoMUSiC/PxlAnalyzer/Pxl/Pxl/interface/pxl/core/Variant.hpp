//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_VARIANT_HH
#define PXL_BASE_VARIANT_HH

#include "Pxl/Pxl/interface/pxl/core/Basic3Vector.hpp"
#include "Pxl/Pxl/interface/pxl/core/Id.hpp"
#include "Pxl/Pxl/interface/pxl/core/LorentzVector.hpp"
#include "Pxl/Pxl/interface/pxl/core/Serializable.hpp"
#include "Pxl/Pxl/interface/pxl/core/functions.hpp"
#include "Pxl/Pxl/interface/pxl/core/macros.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <typeinfo>

#define VARIANT_ADD_TYPE_DECL_POD(NAME, TYPE, VALUE, FIELD)                                                            \
    bool is##NAME() const                                                                                              \
    {                                                                                                                  \
        return (type == TYPE);                                                                                         \
    }                                                                                                                  \
    operator VALUE() const                                                                                             \
    {                                                                                                                  \
        return to##NAME();                                                                                             \
    }                                                                                                                  \
    VALUE &as##NAME()                                                                                                  \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return data._u_##FIELD;                                                                                        \
    }                                                                                                                  \
    const VALUE &as##NAME() const                                                                                      \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return data._u_##FIELD;                                                                                        \
    }                                                                                                                  \
    static Variant from##NAME(const VALUE &a)                                                                          \
    {                                                                                                                  \
        return Variant(a);                                                                                             \
    }                                                                                                                  \
    VALUE to##NAME() const;                                                                                            \
    Variant &operator=(const VALUE &a)                                                                                 \
    {                                                                                                                  \
        clear();                                                                                                       \
        type = TYPE;                                                                                                   \
        data._u_##FIELD = a;                                                                                           \
        return *this;                                                                                                  \
    }                                                                                                                  \
    bool operator!=(const VALUE &a) const                                                                              \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return data._u_##FIELD != a;                                                                                   \
    }                                                                                                                  \
    bool operator==(const VALUE &a) const                                                                              \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return data._u_##FIELD == a;                                                                                   \
    }                                                                                                                  \
    Variant(const VALUE &a)                                                                                            \
    {                                                                                                                  \
        data._u_##FIELD = a;                                                                                           \
        type = TYPE;                                                                                                   \
    }

#define VARIANT_ADD_TYPE_DECL_PTR_BASE(NAME, TYPE, VALUE, FIELD)                                                       \
    bool is##NAME() const                                                                                              \
    {                                                                                                                  \
        return (type == TYPE);                                                                                         \
    }                                                                                                                  \
    VALUE &as##NAME()                                                                                                  \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return *data._u_##FIELD;                                                                                       \
    }                                                                                                                  \
    const VALUE &as##NAME() const                                                                                      \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return *data._u_##FIELD;                                                                                       \
    }                                                                                                                  \
    static Variant from##NAME(const VALUE &a)                                                                          \
    {                                                                                                                  \
        return Variant(a);                                                                                             \
    }

#define VARIANT_ADD_TYPE_DECL_PTR(NAME, TYPE, VALUE, FIELD)                                                            \
    bool operator!=(const VALUE &a) const                                                                              \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return *data._u_##FIELD != a;                                                                                  \
    }                                                                                                                  \
    bool operator==(const VALUE &a) const                                                                              \
    {                                                                                                                  \
        check(TYPE);                                                                                                   \
        return *data._u_##FIELD == a;                                                                                  \
    }                                                                                                                  \
    VARIANT_ADD_TYPE_DECL_PTR_BASE(NAME, TYPE, VALUE, FIELD)                                                           \
    Variant &operator=(const VALUE &a)                                                                                 \
    {                                                                                                                  \
        if (type != TYPE)                                                                                              \
        {                                                                                                              \
            clear();                                                                                                   \
            data._u_##FIELD = new VALUE;                                                                               \
        }                                                                                                              \
        type = TYPE;                                                                                                   \
        (*data._u_##FIELD) = a;                                                                                        \
        return *this;                                                                                                  \
    }                                                                                                                  \
    Variant(const VALUE &a)                                                                                            \
    {                                                                                                                  \
        data._u_##FIELD = new VALUE(a);                                                                                \
        type = TYPE;                                                                                                   \
    }

namespace pxl
{

class PXL_DLL_EXPORT Variant
{
  public:
    enum Type
    {
        TYPE_NONE = 0,
        TYPE_BOOL,
        TYPE_CHAR,
        TYPE_UCHAR,
        TYPE_INT16,
        TYPE_UINT16,
        TYPE_INT32,
        TYPE_UINT32,
        TYPE_INT64,
        TYPE_UINT64,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_STRING,
        TYPE_SERIALIZABLE,
        TYPE_BASIC3VECTOR,
        TYPE_LORENTZVECTOR,
        TYPE_VECTOR
    };

    class bad_conversion : public std::exception
    {
        std::string msg;

      public:
        const char *what() const throw()
        {
            return msg.c_str();
        }
        bad_conversion(Type f, Type t)
        {
            msg = "Variant: bad conversion from '";
            msg += Variant::getTypeName(f);
            msg += "' to '";
            msg += Variant::getTypeName(t);
            msg += "'";
        }
        ~bad_conversion() throw()
        {
        }
    };

    Variant();
    ~Variant();

    Variant(const Variant &a);

    const std::type_info &getTypeInfo() const;

    const char *getTypeName() const
    {
        return getTypeName(type);
    }

    static Type toType(const std::string &name);

    static const char *getTypeName(Type type);

    template <class T>
    T to() const
    {
        throw bad_conversion(type, TYPE_NONE);
    }

    Type getType() const
    {
        return type;
    }

    bool operator==(const Variant &a) const;

    bool operator!=(const Variant &a) const;

    bool operator>(const Variant &a) const;

    bool operator>=(const Variant &a) const;

    bool operator<(const Variant &a) const;

    bool operator<=(const Variant &a) const;

    Variant &operator=(const Variant &a)
    {
        copy(a);
        return *this;
    }

    bool isValid()
    {
        return (type != TYPE_NONE);
    }

    VARIANT_ADD_TYPE_DECL_POD(Bool, TYPE_BOOL, bool, bool)

    VARIANT_ADD_TYPE_DECL_POD(Char, TYPE_CHAR, char, char)

    VARIANT_ADD_TYPE_DECL_POD(UChar, TYPE_UCHAR, unsigned char, uchar)

    VARIANT_ADD_TYPE_DECL_POD(Int16, TYPE_INT16, int16_t, int16)

    VARIANT_ADD_TYPE_DECL_POD(UInt16, TYPE_UINT16, uint16_t, uint16)

    VARIANT_ADD_TYPE_DECL_POD(Int32, TYPE_INT32, int32_t, int32)

    VARIANT_ADD_TYPE_DECL_POD(UInt32, TYPE_UINT32, uint32_t, uint32)

    VARIANT_ADD_TYPE_DECL_POD(Int64, TYPE_INT64, int64_t, int64)

    VARIANT_ADD_TYPE_DECL_POD(UInt64, TYPE_UINT64, uint64_t, uint64)

    VARIANT_ADD_TYPE_DECL_POD(Float, TYPE_FLOAT, float, float)

    VARIANT_ADD_TYPE_DECL_POD(Double, TYPE_DOUBLE, double, double)

    VARIANT_ADD_TYPE_DECL_PTR(Basic3Vector, TYPE_BASIC3VECTOR, Basic3Vector, Basic3Vector)

    VARIANT_ADD_TYPE_DECL_PTR(LorentzVector, TYPE_LORENTZVECTOR, LorentzVector, LorentzVector)

    VARIANT_ADD_TYPE_DECL_PTR(String, TYPE_STRING, std::string, string)
    Variant(const char *s);
    std::string toString() const;
    static Variant fromString(const std::string &str, Type type);
    operator std::string() const
    {
        return toString();
    }
    bool operator!=(const char *a) const
    {
        check(TYPE_STRING);
        return data._u_string->compare(a) != 0;
    }

    // serializable
    VARIANT_ADD_TYPE_DECL_PTR_BASE(Serializable, TYPE_SERIALIZABLE, Serializable, Serializable)
    Variant(const Serializable *a);
    Variant(const Serializable &a);
    Variant &operator=(const Serializable &a)
    {
        clear();
        type = TYPE_SERIALIZABLE;
        data._u_Serializable = a.clone();
        return *this;
    }

    Variant &operator=(const Serializable *a)
    {
        clear();
        type = TYPE_SERIALIZABLE;
        data._u_Serializable = a->clone();
        return *this;
    }

    template <class T>
    T cast()
    {
        check(TYPE_SERIALIZABLE);
        return dynamic_cast<T>(data._u_Serializable);
    }

    template <class T>
    const T cast() const
    {
        check(TYPE_SERIALIZABLE);
        return dynamic_cast<T>(data._u_Serializable);
    }

    bool operator!=(const Serializable *a) const
    {
        check(TYPE_SERIALIZABLE);
        return data._u_Serializable != a;
    }
    bool operator==(const Serializable *a) const
    {
        check(TYPE_SERIALIZABLE);
        return data._u_Serializable == a;
    }

    // std::vector
    Variant(const std::vector<Variant> &a);
    bool isVector() const;
    operator std::vector<Variant> &();
    operator const std::vector<Variant> &() const;
    std::vector<Variant> &asVector();
    const std::vector<Variant> &asVector() const;
    static Variant fromVector(const std::vector<Variant> &s);
    std::vector<Variant> toVector() const;
    Variant &operator=(const std::vector<Variant> &a);
    bool operator==(const std::vector<Variant> &a) const;
    bool operator!=(const std::vector<Variant> &a) const;
    Variant &operator[](size_t i);
    const Variant &operator[](size_t i) const;
    void resize(size_t i);

    // io
    void serialize(const OutputStream &out) const;
    void deserialize(const InputStream &in);
    void clear();

  protected:
    Type type;

    typedef std::vector<Variant> vector_t;
    typedef std::map<std::string, Variant> map_t;
    union {
        bool _u_bool;
        char _u_char;
        unsigned char _u_uchar;
        int16_t _u_int16;
        uint16_t _u_uint16;
        int32_t _u_int32;
        uint32_t _u_uint32;
        int64_t _u_int64;
        uint64_t _u_uint64;
        double _u_double;
        float _u_float;
        Serializable *_u_Serializable;
        Basic3Vector *_u_Basic3Vector;
        LorentzVector *_u_LorentzVector;
        vector_t *_u_vector;
        std::string *_u_string;
    } data;

  private:
    void copy(const Variant &a);
    void check(const Type t) const;
    void check(const Type t);
};

#define VARIANT_TO_DECL(NAME, VALUE)                                                                                   \
    template <>                                                                                                        \
    inline VALUE Variant::to<VALUE>() const                                                                            \
    {                                                                                                                  \
        return to##NAME();                                                                                             \
    }

VARIANT_TO_DECL(Bool, bool)
VARIANT_TO_DECL(Char, char)
VARIANT_TO_DECL(UChar, unsigned char)
VARIANT_TO_DECL(Int16, int16_t)
VARIANT_TO_DECL(UInt16, uint16_t)
VARIANT_TO_DECL(Int32, int32_t)
VARIANT_TO_DECL(UInt32, uint32_t)
VARIANT_TO_DECL(Int64, int64_t)
VARIANT_TO_DECL(UInt64, uint64_t)
VARIANT_TO_DECL(Float, float)
VARIANT_TO_DECL(String, std::string)
VARIANT_TO_DECL(Double, double)
VARIANT_TO_DECL(Vector, std::vector<Variant>)

PXL_DLL_EXPORT std::ostream &operator<<(std::ostream &os, const Variant &v);

} // namespace pxl

#endif // PXL_BASE_VARIANT_HH
