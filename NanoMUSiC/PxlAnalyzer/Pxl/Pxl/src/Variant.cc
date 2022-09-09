//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "SafeInt.hh"

#include "Pxl/Pxl/interface/pxl/core/Tokenizer.hh"
#include "Pxl/Pxl/interface/pxl/core/Variant.hh"

#include <algorithm>

namespace pxl
{

Variant::Variant() : type(TYPE_NONE)
{
}

Variant::~Variant()
{
    clear();
}

Variant::Variant(const Variant &a) : type(TYPE_NONE)
{
    copy(a);
}

Variant::Variant(const char *s)
{
    data._u_string = new std::string(s);
    type = TYPE_STRING;
}

Variant::Variant(const Serializable &a)
{
    data._u_Serializable = a.clone();
    type = TYPE_SERIALIZABLE;
}

Variant::Variant(const Serializable *a)
{
    data._u_Serializable = a->clone();
    type = TYPE_SERIALIZABLE;
}

Variant::Variant(const std::vector<Variant> &a)
{
    data._u_vector = new std::vector<Variant>(a);
    type = TYPE_VECTOR;
}

void Variant::clear()
{
    if (type == TYPE_STRING)
    {
        safe_delete(data._u_string);
    }
    else if (type == TYPE_BASIC3VECTOR)
    {
        safe_delete(data._u_Basic3Vector);
    }
    else if (type == TYPE_LORENTZVECTOR)
    {
        safe_delete(data._u_LorentzVector);
    }
    else if (type == TYPE_SERIALIZABLE)
    {
        safe_delete(data._u_Serializable);
    }
    else if (type == TYPE_VECTOR)
    {
        safe_delete(data._u_vector);
    }
    type = TYPE_NONE;
}

void Variant::check(const Type t) const
{
    if (type != t)
        throw bad_conversion(type, t);
}

void Variant::check(const Type t)
{
    if (type == TYPE_NONE)
    {
        memset(&data, 0, sizeof(data));
        switch (t)
        {
        case TYPE_STRING:
            data._u_string = new std::string;
            break;
        case TYPE_BASIC3VECTOR:
            data._u_Basic3Vector = new Basic3Vector;
            break;
        case TYPE_LORENTZVECTOR:
            data._u_LorentzVector = new LorentzVector;
            break;
        case TYPE_VECTOR:
            data._u_vector = new vector_t;
            break;
        default:
            break;
        }
        type = t;
    }
    else if (type != t)
    {
        throw bad_conversion(type, t);
    }
}

const std::type_info &Variant::getTypeInfo() const
{
    if (type == TYPE_BOOL)
    {
        const std::type_info &ti = typeid(data._u_bool);
        return ti;
    }
    else if (type == TYPE_CHAR)
    {
        const std::type_info &ti = typeid(data._u_char);
        return ti;
    }
    else if (type == TYPE_UCHAR)
    {
        const std::type_info &ti = typeid(data._u_uchar);
        return ti;
    }
    else if (type == TYPE_INT16)
    {
        const std::type_info &ti = typeid(data._u_int16);
        return ti;
    }
    else if (type == TYPE_UINT16)
    {
        const std::type_info &ti = typeid(data._u_uint16);
        return ti;
    }
    else if (type == TYPE_INT32)
    {
        const std::type_info &ti = typeid(data._u_int32);
        return ti;
    }
    else if (type == TYPE_UINT32)
    {
        const std::type_info &ti = typeid(data._u_uint32);
        return ti;
    }
    else if (type == TYPE_INT64)
    {
        const std::type_info &ti = typeid(data._u_int64);
        return ti;
    }
    else if (type == TYPE_UINT64)
    {
        const std::type_info &ti = typeid(data._u_uint64);
        return ti;
    }
    else if (type == TYPE_FLOAT)
    {
        const std::type_info &ti = typeid(data._u_float);
        return ti;
    }
    else if (type == TYPE_DOUBLE)
    {
        const std::type_info &ti = typeid(data._u_double);
        return ti;
    }
    else if (type == TYPE_STRING)
    {
        const std::type_info &ti = typeid(*data._u_string);
        return ti;
    }
    else if (type == TYPE_SERIALIZABLE)
    {
        const std::type_info &ti = typeid(data._u_Serializable);
        return ti;
    }
    else if (type == TYPE_BASIC3VECTOR)
    {
        const std::type_info &ti = typeid(data._u_Basic3Vector);
        return ti;
    }
    else if (type == TYPE_LORENTZVECTOR)
    {
        const std::type_info &ti = typeid(data._u_LorentzVector);
        return ti;
    }
    else if (type == TYPE_VECTOR)
    {
        const std::type_info &ti = typeid(*data._u_vector);
        return ti;
    }
    else
    {
        const std::type_info &ti = typeid(0);
        return ti;
    }
}

const char *Variant::getTypeName(Type type)
{
    if (type == TYPE_NONE)
    {
        return "none";
    }
    else if (type == TYPE_BOOL)
    {
        return "bool";
    }
    else if (type == TYPE_CHAR)
    {
        return "char";
    }
    else if (type == TYPE_UCHAR)
    {
        return "uchar";
    }
    else if (type == TYPE_INT16)
    {
        return "int16";
    }
    else if (type == TYPE_UINT16)
    {
        return "uint16";
    }
    else if (type == TYPE_INT32)
    {
        return "int32";
    }
    else if (type == TYPE_UINT32)
    {
        return "uint32";
    }
    else if (type == TYPE_INT64)
    {
        return "int64";
    }
    else if (type == TYPE_UINT64)
    {
        return "uint64";
    }
    else if (type == TYPE_FLOAT)
    {
        return "float";
    }
    else if (type == TYPE_DOUBLE)
    {
        return "double";
    }
    else if (type == TYPE_SERIALIZABLE)
    {
        return "Serializable";
    }
    else if (type == TYPE_BASIC3VECTOR)
    {
        return "Basic3Vector";
    }
    else if (type == TYPE_LORENTZVECTOR)
    {
        return "LorentzVector";
    }
    else if (type == TYPE_STRING)
    {
        return "string";
    }
    else if (type == TYPE_VECTOR)
    {
        return "vector";
    }
    else
    {
        return "unknown";
    }
}

Variant::Type Variant::toType(const std::string &name)
{
    if (name == "none")
    {
        return TYPE_NONE;
    }
    else if (name == "bool")
    {
        return TYPE_BOOL;
    }
    else if (name == "char")
    {
        return TYPE_CHAR;
    }
    else if (name == "uchar")
    {
        return TYPE_UCHAR;
    }
    else if (name == "int16")
    {
        return TYPE_INT16;
    }
    else if (name == "uint16")
    {
        return TYPE_UINT16;
    }
    else if (name == "int32")
    {
        return TYPE_INT32;
    }
    else if (name == "uint32")
    {
        return TYPE_UINT32;
    }
    else if (name == "int64")
    {
        return TYPE_INT64;
    }
    else if (name == "uint64")
    {
        return TYPE_UINT64;
    }
    else if (name == "float")
    {
        return TYPE_FLOAT;
    }
    else if (name == "double")
    {
        return TYPE_DOUBLE;
    }
    else if (name == "Serializable")
    {
        return TYPE_SERIALIZABLE;
    }
    else if (name == "Basic3Vector")
    {
        return TYPE_BASIC3VECTOR;
    }
    else if (name == "LorentzVector")
    {
        return TYPE_LORENTZVECTOR;
    }
    else if (name == "string")
    {
        return TYPE_STRING;
    }
    else if (name == "vector")
    {
        return TYPE_VECTOR;
    }
    else
    {
        return TYPE_NONE;
    }
}

bool Variant::operator==(const Variant &a) const
{
    if (type != a.type)
    {
        std::string msg = "cannot apply compare operator for differing types: ";
        msg += this->getTypeName(type);
        msg += ", ";
        msg += a.getTypeName(a.type);
        throw std::runtime_error("Variant: " + msg);
    }

    if (type == TYPE_BOOL)
    {
        return (data._u_bool == a.data._u_bool);
    }
    else if (type == TYPE_CHAR)
    {
        return (data._u_char == a.data._u_char);
    }
    else if (type == TYPE_UCHAR)
    {
        return (data._u_uchar == a.data._u_uchar);
    }
    else if (type == TYPE_INT16)
    {
        return (data._u_int16 == a.data._u_int16);
    }
    else if (type == TYPE_UINT16)
    {
        return (data._u_uint16 == a.data._u_uint16);
    }
    else if (type == TYPE_INT32)
    {
        return (data._u_int32 == a.data._u_int32);
    }
    else if (type == TYPE_UINT32)
    {
        return (data._u_uint32 == a.data._u_uint32);
    }
    else if (type == TYPE_INT64)
    {
        return (data._u_int64 == a.data._u_int64);
    }
    else if (type == TYPE_UINT64)
    {
        return (data._u_uint64 == a.data._u_uint64);
    }
    else if (type == TYPE_FLOAT)
    {
        return (data._u_float == a.data._u_float);
    }
    else if (type == TYPE_DOUBLE)
    {
        return (data._u_double == a.data._u_double);
    }
    else if (type == TYPE_SERIALIZABLE)
    {
        return (data._u_Serializable == a.data._u_Serializable);
    }
    else if (type == TYPE_BASIC3VECTOR)
    {
        return ((*data._u_Basic3Vector) == (*a.data._u_Basic3Vector));
    }
    else if (type == TYPE_STRING)
    {
        return (*data._u_string == *a.data._u_string);
    }
    else if (type == TYPE_VECTOR)
    {
        return (*data._u_vector == *a.data._u_vector);
    }
    else
    {
        std::string msg = "compare operator not implemented for type ";
        msg += this->getTypeName(type);
        throw std::runtime_error("Variant: " + msg);
    }
}

bool Variant::operator!=(const Variant &a) const
{
    return !((*this) == a);
}

bool Variant::operator>(const Variant &a) const
{
    if (type != a.type)
    {
        std::string msg = "cannot apply compare operator for differing types: ";
        msg += this->getTypeName(type);
        msg += ", ";
        msg += a.getTypeName(a.type);
        throw std::runtime_error("Variant: " + msg);
    }

    if (type == TYPE_CHAR)
    {
        return (data._u_char > a.data._u_char);
    }
    else if (type == TYPE_UCHAR)
    {
        return (data._u_uchar > a.data._u_uchar);
    }
    else if (type == TYPE_STRING)
    {
        return (*data._u_string > *a.data._u_string);
    }
    else if (type == TYPE_INT16)
    {
        return (data._u_int16 > a.data._u_int16);
    }
    else if (type == TYPE_UINT16)
    {
        return (data._u_uint16 > a.data._u_uint16);
    }
    else if (type == TYPE_INT32)
    {
        return (data._u_int32 > a.data._u_int32);
    }
    else if (type == TYPE_UINT32)
    {
        return (data._u_uint32 > a.data._u_uint32);
    }
    else if (type == TYPE_INT64)
    {
        return (data._u_int64 > a.data._u_int64);
    }
    else if (type == TYPE_UINT64)
    {
        return (data._u_uint64 > a.data._u_uint64);
    }
    else if (type == TYPE_FLOAT)
    {
        return (data._u_float > a.data._u_float);
    }
    else if (type == TYPE_DOUBLE)
    {
        return (data._u_double > a.data._u_double);
    }
    else
    {
        std::string msg = "compare operator not implemented for type ";
        msg += this->getTypeName(type);
        throw std::runtime_error("Variant: " + msg);
    }
}

bool Variant::operator>=(const Variant &a) const
{
    if (type != a.type)
    {
        std::string msg = "cannot apply compare operator for differing types: ";
        msg += this->getTypeName(type);
        msg += ", ";
        msg += a.getTypeName(a.type);
        throw std::runtime_error("Variant: " + msg);
    }

    if (type == TYPE_CHAR)
    {
        return (data._u_char >= a.data._u_char);
    }
    else if (type == TYPE_UCHAR)
    {
        return (data._u_uchar >= a.data._u_uchar);
    }
    else if (type == TYPE_STRING)
    {
        return (*data._u_string >= *a.data._u_string);
    }
    else if (type == TYPE_INT16)
    {
        return (data._u_int16 >= a.data._u_int16);
    }
    else if (type == TYPE_UINT16)
    {
        return (data._u_uint16 >= a.data._u_uint16);
    }
    else if (type == TYPE_INT32)
    {
        return (data._u_int32 >= a.data._u_int32);
    }
    else if (type == TYPE_UINT32)
    {
        return (data._u_uint32 >= a.data._u_uint32);
    }
    else if (type == TYPE_INT64)
    {
        return (data._u_int64 >= a.data._u_int64);
    }
    else if (type == TYPE_UINT64)
    {
        return (data._u_uint64 >= a.data._u_uint64);
    }
    else if (type == TYPE_FLOAT)
    {
        return (data._u_float >= a.data._u_float);
    }
    else if (type == TYPE_DOUBLE)
    {
        return (data._u_double >= a.data._u_double);
    }
    else
    {
        std::string msg = "compare operator not implemented for type ";
        msg += this->getTypeName(type);
        throw std::runtime_error("Variant: " + msg);
    }
}

bool Variant::operator<(const Variant &a) const
{
    return !((*this) >= a);
}

bool Variant::operator<=(const Variant &a) const
{
    return !((*this) > a);
}

std::string Variant::toString() const
{
    if (type == TYPE_STRING)
        return *data._u_string;

    std::stringstream sstr;
    if (type == TYPE_BOOL)
    {
        sstr << data._u_bool;
    }
    else if (type == TYPE_CHAR)
    {
        sstr << data._u_char;
    }
    else if (type == TYPE_UCHAR)
    {
        sstr << data._u_uchar;
    }
    else if (type == TYPE_INT16)
    {
        sstr << data._u_int16;
    }
    else if (type == TYPE_UINT16)
    {
        sstr << data._u_uint16;
    }
    else if (type == TYPE_INT32)
    {
        sstr << data._u_int32;
    }
    else if (type == TYPE_UINT32)
    {
        sstr << data._u_uint32;
    }
    else if (type == TYPE_INT64)
    {
        sstr << data._u_int64;
    }
    else if (type == TYPE_UINT64)
    {
        sstr << data._u_uint64;
    }
    else if (type == TYPE_FLOAT)
    {
        sstr << data._u_float;
    }
    else if (type == TYPE_DOUBLE)
    {
        sstr << data._u_double;
    }
    else if (type == TYPE_BASIC3VECTOR)
    {
        sstr << data._u_Basic3Vector->getX() << " " << data._u_Basic3Vector->getY() << " "
             << data._u_Basic3Vector->getZ();
    }
    else if (type == TYPE_LORENTZVECTOR)
    {
        sstr << data._u_LorentzVector->getX() << " " << data._u_LorentzVector->getY() << " "
             << data._u_LorentzVector->getZ() << " " << data._u_LorentzVector->getE();
    }
    else if (type == TYPE_VECTOR)
    {
        sstr << *data._u_vector;
    }

    return sstr.str();
}

Variant Variant::fromString(const std::string &str, Type type)
{
    std::stringstream sstr(str);
    switch (type)
    {
    case TYPE_BOOL: {
        std::string upperstr(str);
        std::transform(upperstr.begin(), upperstr.end(), upperstr.begin(), (int (*)(int))toupper);
        if (upperstr == "YES")
            return Variant(true);
        else if (upperstr == "NO")
            return Variant(false);
        if (upperstr == "TRUE")
            return Variant(true);
        else if (upperstr == "FALSE")
            return Variant(false);
        if (upperstr == "1")
            return Variant(true);
        else if (upperstr == "0")
            return Variant(false);
        throw bad_conversion(type, TYPE_BOOL);
    }
    case TYPE_CHAR: {
        char c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_UCHAR: {
        unsigned char c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_INT16: {
        int16_t c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_UINT16: {
        uint16_t c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_INT32: {
        int32_t c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_UINT32: {
        uint32_t c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_INT64: {
        int64_t c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_UINT64: {
        uint64_t c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_FLOAT: {
        float c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_DOUBLE: {
        double c;
        sstr >> c;
        return Variant(c);
    }
    case TYPE_BASIC3VECTOR: {
        double d;
        Basic3Vector v;
        sstr >> d;
        v.setX(d);
        sstr >> d;
        v.setY(d);
        sstr >> d;
        v.setZ(d);
        return Variant(v);
    }
    case TYPE_LORENTZVECTOR: {
        double d;
        LorentzVector v;
        sstr >> d;
        v.setX(d);
        sstr >> d;
        v.setY(d);
        sstr >> d;
        v.setZ(d);
        sstr >> d;
        v.setE(d);
        return Variant(v);
    }
    case TYPE_STRING: {
        return Variant(str);
    }
    case TYPE_VECTOR: {
        std::vector<Variant> stringVectorValue;
        Tokenizer tok;
        tok.setText(str);
        tok.setCharType('[', Tokenizer::WHITESPACE);
        tok.setCharType(']', Tokenizer::WHITESPACE);
        tok.setCharType('(', Tokenizer::WHITESPACE);
        tok.setCharType(')', Tokenizer::WHITESPACE);
        tok.setCharType('\"', Tokenizer::LITERAL);
        tok.setCharType('\'', Tokenizer::LITERAL);
        tok.setCharType(',', Tokenizer::DELIM);
        while (tok.hasNext())
        {
            std::string s = tok.next();
            stringVectorValue.push_back(s);
        }
        return Variant(stringVectorValue);
    }
    default:
        throw std::runtime_error("pxl::Variant::fromString: unknown type");
    }
}

void Variant::serialize(const OutputStream &out) const
{
    out.write((unsigned char)type);
    switch (type)
    {
    case TYPE_BOOL:
        out.writeBool(data._u_bool);
        break;
    case TYPE_CHAR:
        out.write(data._u_char);
        break;
    case TYPE_UCHAR:
        out.write(data._u_uchar);
        break;
    case TYPE_INT16:
        out.write(data._u_int16);
        break;
    case TYPE_UINT16:
        out.write(data._u_uint16);
        break;
    case TYPE_INT32:
        out.write(data._u_int32);
        break;
    case TYPE_UINT32:
        out.write(data._u_uint32);
        break;
    case TYPE_INT64:
        out.write(data._u_int64);
        break;
    case TYPE_UINT64:
        out.write(data._u_uint64);
        break;
    case TYPE_FLOAT:
        out.writeFloat(data._u_float);
        break;
    case TYPE_DOUBLE:
        out.writeDouble(data._u_double);
        break;
    case TYPE_STRING:
        out.writeString(*data._u_string);
        break;
    case TYPE_BASIC3VECTOR:
        data._u_Basic3Vector->serialize(out);
        break;
    case TYPE_LORENTZVECTOR:
        data._u_LorentzVector->serialize(out);
        break;
    case TYPE_VECTOR: {
        uint32_t s = data._u_vector->size();
        out.write(s);
        for (size_t i = 0; i < s; i++)
            data._u_vector->at(i).serialize(out);
        break;
    }
    default:
        break;
    }
}

void Variant::deserialize(const InputStream &in)
{
    unsigned char t;
    in.read(t);
    check(static_cast<Type>(t));

    switch (type)
    {
    case TYPE_BOOL:
        in.readBool(data._u_bool);
        break;
    case TYPE_CHAR:
        in.read(data._u_char);
        break;
    case TYPE_UCHAR:
        in.read(data._u_uchar);
        break;
    case TYPE_INT16:
        in.read(data._u_int16);
        break;
    case TYPE_UINT16:
        in.read(data._u_uint16);
        break;
    case TYPE_INT32:
        in.read(data._u_int32);
        break;
    case TYPE_UINT32:
        in.read(data._u_uint32);
        break;
    case TYPE_INT64:
        in.read(data._u_int64);
        break;
    case TYPE_UINT64:
        in.read(data._u_uint64);
        break;
    case TYPE_FLOAT:
        in.readFloat(data._u_float);
        break;
    case TYPE_DOUBLE:
        in.readDouble(data._u_double);
        break;
    case TYPE_STRING:
        in.readString(*data._u_string);
        break;
    case TYPE_BASIC3VECTOR:
        data._u_Basic3Vector->deserialize(in);
        break;
    case TYPE_LORENTZVECTOR:
        data._u_LorentzVector->deserialize(in);
        break;
    case TYPE_VECTOR: {
        uint32_t s = 0;
        in.read(s);
        data._u_vector->resize(s);
        for (size_t i = 0; i < s; i++)
            data._u_vector->at(i).deserialize(in);
        break;
    }
    default:
        break;
    }
}

void Variant::copy(const Variant &a)
{
    Type t = a.type;
    if (t == TYPE_BOOL)
    {
        operator=(a.data._u_bool);
    }
    else if (t == TYPE_CHAR)
    {
        operator=(a.data._u_char);
    }
    else if (t == TYPE_UCHAR)
    {
        operator=(a.data._u_uchar);
    }
    else if (t == TYPE_INT16)
    {
        operator=(a.data._u_int16);
    }
    else if (t == TYPE_UINT16)
    {
        operator=(a.data._u_uint16);
    }
    else if (t == TYPE_INT32)
    {
        operator=(a.data._u_int32);
    }
    else if (t == TYPE_UINT32)
    {
        operator=(a.data._u_uint32);
    }
    else if (t == TYPE_INT64)
    {
        operator=(a.data._u_int64);
    }
    else if (t == TYPE_UINT64)
    {
        operator=(a.data._u_uint64);
    }
    else if (t == TYPE_FLOAT)
    {
        operator=(a.data._u_float);
    }
    else if (t == TYPE_DOUBLE)
    {
        operator=(a.data._u_double);
    }
    else if (t == TYPE_STRING)
    {
        operator=(*a.data._u_string);
    }
    else if (t == TYPE_SERIALIZABLE)
    {
        operator=(a.data._u_Serializable);
    }
    else if (t == TYPE_BASIC3VECTOR)
    {
        operator=(*a.data._u_Basic3Vector);
    }
    else if (t == TYPE_LORENTZVECTOR)
    {
        operator=(*a.data._u_LorentzVector);
    }
    else if (t == TYPE_VECTOR)
    {
        operator=(*a.data._u_vector);
    }
    else
    {
        type = TYPE_NONE;
    }
}

bool Variant::toBool() const
{
    switch (type)
    {
    case TYPE_BOOL:
        return data._u_bool;
        break;
    case TYPE_CHAR:
        return data._u_char != 0;
        break;
    case TYPE_UCHAR:
        return data._u_uchar != 0;
        break;
    case TYPE_INT16:
        return data._u_int16 != 0;
        break;
    case TYPE_UINT16:
        return data._u_uint16 != 0;
        break;
    case TYPE_INT32:
        return data._u_int32 != 0;
        break;
    case TYPE_UINT32:
        return data._u_uint32 != 0;
        break;
    case TYPE_INT64:
        return data._u_int64 != 0;
        break;
    case TYPE_UINT64:
        return data._u_uint64 != 0;
        break;
    case TYPE_SERIALIZABLE:
        return data._u_Serializable != 0;
        break;
    case TYPE_STRING: {
        std::string upperstr(*data._u_string);
        std::transform(upperstr.begin(), upperstr.end(), upperstr.begin(), (int (*)(int))toupper);
        if (upperstr == "YES")
            return true;
        else if (upperstr == "NO")
            return false;
        if (upperstr == "TRUE")
            return true;
        else if (upperstr == "FALSE")
            return false;
        if (upperstr == "1")
            return true;
        else if (upperstr == "0")
            return false;
        else
            throw bad_conversion(type, TYPE_BOOL);
    }
    break;
    case TYPE_VECTOR:
        return data._u_vector->size() != 0;
        break;
    case TYPE_FLOAT:
    case TYPE_DOUBLE:
    case TYPE_BASIC3VECTOR:
    case TYPE_LORENTZVECTOR:
    case TYPE_NONE:
        throw bad_conversion(type, TYPE_BOOL);
        break;
    }
    return false;
}

#define INT_CASE(from_var, from_type, to_type, to)                                                                     \
    case Variant::from_type: {                                                                                         \
        SafeInt<to> l(data._u_##from_var);                                                                             \
        return l;                                                                                                      \
    }                                                                                                                  \
    break;

#define INT_FUNCTION(to_type, fun, to)                                                                                 \
    to Variant::fun() const                                                                                            \
    {                                                                                                                  \
        switch (type)                                                                                                  \
        {                                                                                                              \
        case Variant::TYPE_BOOL:                                                                                       \
            return data._u_bool ? 1 : 0;                                                                               \
            break;                                                                                                     \
            INT_CASE(char, TYPE_CHAR, to_type, to)                                                                     \
            INT_CASE(uchar, TYPE_UCHAR, to_type, to)                                                                   \
            INT_CASE(int16, TYPE_INT16, to_type, to)                                                                   \
            INT_CASE(uint16, TYPE_UINT16, to_type, to)                                                                 \
            INT_CASE(int32, TYPE_INT32, to_type, to)                                                                   \
            INT_CASE(uint32, TYPE_UINT32, to_type, to)                                                                 \
            INT_CASE(int64, TYPE_INT64, to_type, to)                                                                   \
            INT_CASE(uint64, TYPE_UINT64, to_type, to)                                                                 \
            INT_CASE(float, TYPE_FLOAT, to_type, to)                                                                   \
            INT_CASE(double, TYPE_DOUBLE, to_type, to)                                                                 \
        case Variant::TYPE_STRING: {                                                                                   \
            SafeInt<to> l(atol(data._u_string->c_str()));                                                              \
            return l;                                                                                                  \
        }                                                                                                              \
        break;                                                                                                         \
        case Variant::TYPE_SERIALIZABLE:                                                                               \
        case Variant::TYPE_BASIC3VECTOR:                                                                               \
        case Variant::TYPE_LORENTZVECTOR:                                                                              \
        case Variant::TYPE_VECTOR:                                                                                     \
        case Variant::TYPE_NONE:                                                                                       \
            throw bad_conversion(type, to_type);                                                                       \
            break;                                                                                                     \
        }                                                                                                              \
        return 0;                                                                                                      \
    }

INT_FUNCTION(TYPE_CHAR, toChar, char)
INT_FUNCTION(TYPE_UCHAR, toUChar, unsigned char)
INT_FUNCTION(TYPE_INT16, toInt16, int16_t)
INT_FUNCTION(TYPE_UINT16, toUInt16, uint16_t)
INT_FUNCTION(TYPE_INT32, toInt32, int32_t)
INT_FUNCTION(TYPE_UINT32, toUInt32, uint32_t)
INT_FUNCTION(TYPE_INT64, toInt64, int64_t)
INT_FUNCTION(TYPE_UINT64, toUInt64, uint64_t)

PXL_DLL_EXPORT std::ostream &operator<<(std::ostream &os, const Variant &v)
{
    switch (v.getType())
    {
    case Variant::TYPE_BOOL:
        os << v.asBool();
        break;
    case Variant::TYPE_CHAR:
        os << v.asChar();
        break;
    case Variant::TYPE_UCHAR:
        os << v.asUChar();
        break;
    case Variant::TYPE_INT16:
        os << v.asInt16();
        break;
    case Variant::TYPE_UINT16:
        os << v.asUInt16();
        break;
    case Variant::TYPE_INT32:
        os << v.asInt32();
        break;
    case Variant::TYPE_UINT32:
        os << v.asUInt32();
        break;
    case Variant::TYPE_INT64:
        os << v.asInt64();
        break;
    case Variant::TYPE_UINT64:
        os << v.asUInt64();
        break;
    case Variant::TYPE_FLOAT:
        os << v.asFloat();
        break;
    case Variant::TYPE_DOUBLE:
        os << v.asDouble();
        break;
    case Variant::TYPE_STRING:
        os << v.asString();
        break;
    case Variant::TYPE_SERIALIZABLE:
        v.asSerializable().print(0, os);
        break;
    case Variant::TYPE_BASIC3VECTOR:
        os << v.asBasic3Vector();
        break;
    case Variant::TYPE_VECTOR: {
        const std::vector<Variant> &vec = v.asVector();
        os << "(";
        for (size_t i = 0; i < vec.size(); i++)
        {
            if (i != 0)
                os << ", ";
            os << vec[i];
        }
        os << ")";
        break;
    }
    default:
        break;
    }
    return os;
}

// std::vector
bool Variant::isVector() const
{
    return (type == TYPE_VECTOR);
}

Variant::operator std::vector<Variant> &()
{
    check(TYPE_VECTOR);
    return *data._u_vector;
}

Variant::operator const std::vector<Variant> &() const
{
    check(TYPE_VECTOR);
    return *data._u_vector;
}

std::vector<Variant> &Variant::asVector()
{
    check(TYPE_VECTOR);
    return *data._u_vector;
}

const std::vector<Variant> &Variant::asVector() const
{
    check(TYPE_VECTOR);
    return *data._u_vector;
}

Variant Variant::fromVector(const std::vector<Variant> &s)
{
    return Variant(s);
}

Variant &Variant::operator=(const std::vector<Variant> &a)
{
    if (type != TYPE_VECTOR)
    {
        clear();
    }
    data._u_vector = new std::vector<Variant>(a);
    type = TYPE_VECTOR;
    return *this;
}

bool Variant::operator==(const std::vector<Variant> &a) const
{
    if (type != TYPE_VECTOR)
        return false;
    return *data._u_vector == a;
}

bool Variant::operator!=(const std::vector<Variant> &a) const
{
    check(TYPE_VECTOR);
    return *data._u_vector != a;
}

Variant &Variant::operator[](size_t i)
{
    check(TYPE_VECTOR);
    return (*data._u_vector)[i];
}

const Variant &Variant::operator[](size_t i) const
{
    check(TYPE_VECTOR);
    return (*data._u_vector)[i];
}

void Variant::resize(size_t i)
{
    check(TYPE_VECTOR);
    return data._u_vector->resize(i);
}

float Variant::toFloat() const
{
    if (type == TYPE_CHAR)
    {
        return static_cast<float>(data._u_char);
    }
    else if (type == TYPE_UCHAR)
    {
        return static_cast<float>(data._u_uchar);
    }
    else if (type == TYPE_INT16)
    {
        return static_cast<float>(data._u_int16);
    }
    else if (type == TYPE_UINT16)
    {
        return static_cast<float>(data._u_uint16);
    }
    else if (type == TYPE_INT32)
    {
        return static_cast<float>(data._u_int32);
    }
    else if (type == TYPE_UINT32)
    {
        return static_cast<float>(data._u_uint32);
    }
    else if (type == TYPE_INT64)
    {
        return static_cast<float>(data._u_int64);
    }
    else if (type == TYPE_UINT64)
    {
        return static_cast<float>(data._u_uint64);
    }
    else if (type == TYPE_FLOAT)
    {
        return static_cast<float>(data._u_float);
    }
    else if (type == TYPE_DOUBLE)
    {
        return static_cast<float>(data._u_double);
    }
    else if (type == TYPE_STRING)
    {
        return static_cast<float>(std::atof(data._u_string->c_str()));
    }
    else if (type == TYPE_BOOL)
    {
        return data._u_bool ? 1.0f : 0.0f;
    }
    else
    {
        return 0.0;
    }
}

double Variant::toDouble() const
{
    if (type == TYPE_CHAR)
    {
        return static_cast<double>(data._u_char);
    }
    else if (type == TYPE_UCHAR)
    {
        return static_cast<double>(data._u_uchar);
    }
    else if (type == TYPE_INT16)
    {
        return static_cast<double>(data._u_int16);
    }
    else if (type == TYPE_UINT16)
    {
        return static_cast<double>(data._u_uint16);
    }
    else if (type == TYPE_INT32)
    {
        return static_cast<double>(data._u_int32);
    }
    else if (type == TYPE_UINT32)
    {
        return static_cast<double>(data._u_uint32);
    }
    else if (type == TYPE_INT64)
    {
        return static_cast<double>(data._u_int64);
    }
    else if (type == TYPE_UINT64)
    {
        return static_cast<double>(data._u_uint64);
    }
    else if (type == TYPE_FLOAT)
    {
        return static_cast<double>(data._u_float);
    }
    else if (type == TYPE_DOUBLE)
    {
        return static_cast<double>(data._u_double);
    }
    else if (type == TYPE_STRING)
    {
        return std::atof(data._u_string->c_str());
    }
    else if (type == TYPE_BOOL)
    {
        return data._u_bool ? 1.0 : 0.0;
    }
    else
    {
        return 0.0;
    }
}

std::vector<Variant> Variant::toVector() const
{
    if (type == TYPE_VECTOR)
    {
        return *data._u_vector;
    }
    else
    {
        throw bad_conversion(type, TYPE_VECTOR);
    }
}

} // namespace pxl
