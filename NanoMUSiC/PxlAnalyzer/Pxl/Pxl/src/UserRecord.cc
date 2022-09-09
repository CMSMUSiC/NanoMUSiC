//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/UserRecord.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"
#include "Pxl/Pxl/interface/pxl/core/logging.hh"

#undef PXL_LOG_MODULE_NAME
#define PXL_LOG_MODULE_NAME "pxl::UserRecords"

namespace pxl
{

void UserRecords::serialize(const OutputStream &out) const
{
    out.writeUnsignedInt(getContainer()->size());
    for (const_iterator iter = getContainer()->begin(); iter != getContainer()->end(); ++iter)
    {
        out.writeString(iter->first);
        Variant::Type type = iter->second.getType();

        char cType = ' ';
        switch (type)
        {
        case Variant::TYPE_BOOL:
            cType = 'b';
            out.writeChar(cType);
            out.writeBool(iter->second.asBool());
            break;
        case Variant::TYPE_CHAR:
            cType = 'c';
            out.writeChar(cType);
            out.writeChar(iter->second.asChar());
            break;
        case Variant::TYPE_UCHAR:
            cType = 'C';
            out.writeChar(cType);
            out.writeUnsignedChar(iter->second.asUChar());
            break;
        case Variant::TYPE_INT32:
            cType = 'i';
            out.writeChar(cType);
            out.write(iter->second.asInt32());
            break;
        case Variant::TYPE_UINT32:
            cType = 'I';
            out.writeChar(cType);
            out.write(iter->second.asUInt32());
            break;
        case Variant::TYPE_INT16:
            cType = 'o';
            out.writeChar(cType);
            out.write(iter->second.asInt16());
            break;
        case Variant::TYPE_UINT16:
            cType = 'O';
            out.writeChar(cType);
            out.write(iter->second.asUInt16());
            break;
        case Variant::TYPE_INT64:
            cType = 'm';
            out.writeChar(cType);
            out.write(iter->second.asInt64());
            break;
        case Variant::TYPE_UINT64:
            cType = 'M';
            out.writeChar(cType);
            out.write(iter->second.asUInt64());
            break;
        case Variant::TYPE_DOUBLE:
            cType = 'd';
            out.writeChar(cType);
            out.writeDouble(iter->second.asDouble());
            break;
        case Variant::TYPE_FLOAT:
            cType = 'f';
            out.writeChar(cType);
            out.writeFloat(iter->second.asFloat());
            break;
        case Variant::TYPE_STRING:
            cType = 's';
            out.writeChar(cType);
            out.writeString(iter->second.asString());
            break;
        case Variant::TYPE_SERIALIZABLE:
            cType = 'S';
            out.writeChar(cType);
            iter->second.asSerializable().serialize(out);
            break;
        case Variant::TYPE_BASIC3VECTOR: {
            cType = 'V';
            out.writeChar(cType);
            const Basic3Vector &v = iter->second.asBasic3Vector();
            out.writeDouble(v.getX());
            out.writeDouble(v.getY());
            out.writeDouble(v.getZ());
            break;
        }
        case Variant::TYPE_LORENTZVECTOR: {
            cType = 'Z';
            out.writeChar(cType);
            const LorentzVector &L = iter->second.asLorentzVector();
            out.writeDouble(L.getX());
            out.writeDouble(L.getY());
            out.writeDouble(L.getZ());
            out.writeDouble(L.getT());
            break;
        }
        case Variant::TYPE_VECTOR: {
            cType = 'v';
            out.writeChar(cType);
            iter->second.serialize(out);
            break;
        }

        default:
            out.writeChar(cType);
            PXL_LOG_WARNING << "Type not handled in pxl::Variant I/O.";
            break;
        }
    }
}

void UserRecords::deserialize(const InputStream &in)
{
    unsigned int size = 0;
    in.readUnsignedInt(size);
    for (unsigned int j = 0; j < size; ++j)
    {
        std::string name;
        in.readString(name);
        char cType;
        in.readChar(cType);

        iterator insertPos = setContainer()->end();

        // FIXME: temporary solution here - could also use static lookup-map,
        // but leave this unchanged until decided if to switch to new UR implementation.
        switch (cType)
        {
        case 'b': {
            bool b;
            in.readBool(b);
            setFast(insertPos, name, b);
            break;
        }
        case 'c': {
            char c;
            in.readChar(c);
            setFast(insertPos, name, c);
            break;
        }
        case 'C': {
            unsigned char c;
            in.readUnsignedChar(c);
            setFast(insertPos, name, c);
            break;
        }

        case 'l':
        case 'i': {
            int32_t ii;
            in.read(ii);
            setFast(insertPos, name, ii);
            break;
        }
        case 'L':
        case 'I': {
            uint32_t ui;
            in.read(ui);
            setFast(insertPos, name, ui);
            break;
        }
        case 'o': {
            short s;
            in.readShort(s);
            setFast(insertPos, name, s);
            break;
        }
        case 'O': {
            unsigned short us;
            in.readUnsignedShort(us);
            setFast(insertPos, name, us);
            break;
        }
        case 'm': {
            int64_t l;
            in.read(l);
            setFast(insertPos, name, l);
            break;
        }
        case 'M': {
            uint64_t ul;
            in.read(ul);
            setFast(insertPos, name, ul);
            break;
        }
        case 'd': {
            double d;
            in.readDouble(d);
            setFast(insertPos, name, d);
            break;
        }
        case 'f': {
            float f;
            in.readFloat(f);
            setFast(insertPos, name, f);
            break;
        }
        case 's': {
            std::string ss;
            in.readString(ss);
            setFast(insertPos, name, ss);
            break;
        }
        case 'S': {
            Id id(in);
            Serializable *obj = ObjectFactory::instance().create(id);
            obj->deserialize(in);
            setFast(insertPos, name, obj);
            delete obj;
            break;
        }
        case 'V': {
            Basic3Vector obj;
            double d;
            in.readDouble(d);
            obj.setX(d);
            in.readDouble(d);
            obj.setY(d);
            in.readDouble(d);
            obj.setZ(d);
            setFast(insertPos, name, obj);
            break;
        }
        case 'Z': {
            LorentzVector obj;
            double d;
            in.readDouble(d);
            obj.setX(d);
            in.readDouble(d);
            obj.setY(d);
            in.readDouble(d);
            obj.setZ(d);
            in.readDouble(d);
            obj.setT(d);
            setFast(insertPos, name, obj);
            break;
        }
        case 'v': {
            Variant obj;
            obj.deserialize(in);
            setFast(insertPos, name, obj);
            break;
        }

        default:
            PXL_LOG_WARNING << "Type " << cType << " not handled in pxl::Variant I/O.";
            break;
        }
    }
}

std::ostream &UserRecords::print(int level, std::ostream &os, int pan) const
{
    os << "UserRecord size " << size() << "\n";
    for (const_iterator iter = getContainer()->begin(); iter != getContainer()->end(); ++iter)
    {
        os << "-->";

        os << " ('" << iter->first << "', " << iter->second << "),";
        os << " type: " << iter->second.getTypeName() << std::endl;
    }

    return os;
}

} // namespace pxl
