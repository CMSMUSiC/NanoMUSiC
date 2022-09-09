//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_USERRECORD_HH
#define PXL_BASE_USERRECORD_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Pxl/Pxl/interface/pxl/core/Id.hh"
#include "Pxl/Pxl/interface/pxl/core/Stream.hh"
#include "Pxl/Pxl/interface/pxl/core/Variant.hh"

namespace pxl
{
/**
 This class is intented to aggregate information complementary to data members in form
 of string-variant pairs.
 All PXL physics objects own user records and provide methods for quick
 access to individual user record entries.
 */
class PXL_DLL_EXPORT UserRecords
{

  private:
    class DataSocket
    {
      public:
        DataSocket() : _references(1)
        {
        }
        DataSocket(const DataSocket &original) : _references(1), _data(original._data)
        {
        }
        DataSocket(const DataSocket *original) : _references(1), _data(original->_data)
        {
        }
        virtual ~DataSocket()
        {
        }

        // for deep copies
        virtual DataSocket *clone() const
        {
            return new DataSocket(this);
        }

        // methods to grant data access
        inline std::map<std::string, Variant> *getData()
        {
            return &_data;
        }
        inline void setData(const std::map<std::string, Variant> *object)
        {
            _data = *object;
        }

        unsigned int _references;
        std::map<std::string, Variant> _data;

    }; // class Datasocket

  public:
    typedef std::map<std::string, Variant> map_type;
    typedef map_type::const_iterator const_iterator;
    typedef map_type::iterator iterator;

    UserRecords()
    {
        _dataSocket = new DataSocket;
    }
    UserRecords(const UserRecords &original)
    {
        _dataSocket = original._dataSocket;
        _dataSocket->_references++;
    }
    explicit UserRecords(const UserRecords *original)
    {
        _dataSocket = original->_dataSocket;
        _dataSocket->_references++;
    }
    ~UserRecords()
    {
        dropDataSocket();
    }

    void serialize(const OutputStream &out) const;
    void deserialize(const InputStream &in);

    /// This assignment operator acts directly on the aggregated data.
    inline UserRecords &operator=(const UserRecords &original)
    {
        dropDataSocket();
        _dataSocket = original._dataSocket;
        _dataSocket->_references++;
        return *this;
    }

    /// Inserts (or replaces) the user record indetified by \p key.
    void set(const std::string &key, const Variant &item)
    {
        findOrAlloc(key) = item;
    }

    /// Searches and returns the user record item indetified by \p key; a pxl::Exception is thrown in case the key is
    /// not found.
    const Variant &get(const std::string &key) const
    {
        const Variant *value = find(key);
        if (!value)
            throw std::runtime_error("pxl::UserRecord::get(...): key '" + key + "' not found");
        return *value;
    }

    /// find the user record entry identified by key. return 0 when no entry is found.
    Variant *find(const std::string &key)
    {
        iterator found = setContainer()->find(key);
        if (found == setContainer()->end())
            return 0;
        return &found->second;
    }

    const Variant *find(const std::string &key) const
    {
        const_iterator found = getContainer()->find(key);
        if (found == getContainer()->end())
            return 0;
        return &found->second;
    }

    /// Checks if the user record entry identified by key is present.
    bool has(const std::string &key) const
    {
        const_iterator found = getContainer()->find(key);
        if (found == getContainer()->end())
            return false;
        return true;
    }

    /// Checks if user record entry identified by \p key is present.
    /// If yes, its value is put into the passed \p item.
    template <typename datatype> bool get(const std::string &key, datatype &item) const
    {
        const Variant *value = find(key);
        if (!value)
            return false;
        item = value->to<datatype>();
        return true;
    }

    /// Checks if a user record entry identified by \p key is present,
    /// and changes it to the passed value in case it is present. If not, an exception is thrown.
    template <typename datatype> void change(const std::string &key, datatype item)
    {
        iterator found = setContainer()->find(key);
        if (found == getContainer()->end())
            throw std::runtime_error("pxl::UserRecord::change(...): UserRecord entry '" + key + "' not found");

        Variant &value = found->second;
        if (value.getTypeInfo() != typeid(datatype))
            throw std::runtime_error("pxl::UserRecord::change(...): UserRecord entry '" + key + "' of wrong type");

        value = item;
    }

    inline void clear()
    {
        setContainer()->clear();
    }

    inline void erase(const std::string &key)
    {
        size_t s = setContainer()->erase(key);
        if (s == 0)
            throw std::runtime_error("Cannot erase unknown key: " + key);
    }

    /// Grants read access to the aggregated data.
    inline const std::map<std::string, Variant> *getContainer() const
    {
        return _dataSocket->getData();
    }

    inline const_iterator begin() const
    {
        return getContainer()->begin();
    }

    inline const_iterator end() const
    {
        return getContainer()->end();
    }

    inline size_t size() const
    {
        return getContainer()->size();
    }

    std::ostream &print(int level = 0, std::ostream &os = std::cout, int pan = 0) const;
    const std::string toString() const
    {
        std::ostringstream ss;
        this->print(0, ss);
        return ss.str();
    }

  private:
    DataSocket *_dataSocket;

    /// Grants write access to the aggregated data;
    /// if necessary, the copy-on-write mechanism performs a deep copy of the aggregated data first.
    inline std::map<std::string, Variant> *setContainer()
    {
        if (_dataSocket->_references > 1)
        {
            _dataSocket->_references--;
            _dataSocket = new DataSocket(*_dataSocket);
        }
        return _dataSocket->getData();
    }

    inline void dropDataSocket()
    {
        if (_dataSocket->_references-- == 1)
            delete _dataSocket;
    }

    Variant &findOrAlloc(const std::string &key)
    {
        iterator insertPos = setContainer()->lower_bound(key);
        if (insertPos == getContainer()->end() || insertPos->first != key)
            return setContainer()->insert(insertPos, make_pair(key, Variant()))->second;
        else
            return insertPos->second;
    }

    Variant &setFast(iterator insertPos, const std::string &key, const Variant &item)
    {
        Variant &v = setContainer()->insert(insertPos, make_pair(key, Variant()))->second;
        v = item;
        return v;
    }
};

class PXL_DLL_EXPORT UserRecordHelper
{
  public:
    const UserRecords &getUserRecords() const
    {
        return _userRecords;
    }

    void setUserRecords(const UserRecords &value)
    {
        _userRecords = value;
    }

    UserRecords &getUserRecords()
    {
        return _userRecords;
    }

    inline void setUserRecord(const std::string &key, const Variant &value)
    {
        _userRecords.set(key, value);
    }

    void eraseUserRecord(const std::string &key)
    {
        _userRecords.erase(key);
    }

    const Variant &getUserRecord(const std::string &key) const

    {
        return _userRecords.get(key);
    }

    template <typename datatype> bool getUserRecord(const std::string &key, datatype &item) const
    {
        return _userRecords.template get<datatype>(key, item);
    }

    inline bool hasUserRecord(const std::string &key) const
    {
        return _userRecords.has(key);
    }

    void serialize(const OutputStream &out) const
    {
        _userRecords.serialize(out);
    }

    void deserialize(const InputStream &in)
    {
        _userRecords.deserialize(in);
    }

  private:
    UserRecords _userRecords;
};

} // namespace pxl

#endif // PXL_BASE_USERRECORD_HH
