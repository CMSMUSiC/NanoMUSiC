//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_BASE_EVENT_HH
#define PXL_BASE_EVENT_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include <stdexcept>

#include "Pxl/Pxl/interface/pxl/core/ObjectOwner.hh"
#include "Pxl/Pxl/interface/pxl/core/UserRecord.hh"
#include "Pxl/Pxl/interface/pxl/core/Serializable.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectFactory.hh"

namespace pxl
{

class PXL_DLL_EXPORT Event: public Serializable, public UserRecordHelper
{
public:
	Event() :
		Serializable()
	{
	}

	Event(const Event& event) :
		Serializable(event), UserRecordHelper(event), _objects(event._objects)
	{
	}

	explicit Event(const Event* event) :
		Serializable(*event), UserRecordHelper(*event), _objects(
				event->_objects)
	{
	}

	virtual ~Event()
	{
	}

	virtual const Id& getTypeId() const
	{
		return getStaticTypeId();
	}

	static const Id& getStaticTypeId();

	virtual void serialize(const OutputStream &out) const;
	virtual void deserialize(const InputStream &in);

	/// Creates a deep copy and returns a C++ pointer to the newly-created object.
	virtual Serializable* clone() const
	{
		return new Event(*this);
	}

	/// Creates a new instance of \p objecttype;
	/// objecttype must be a class inheriting from pxl::Relative;
	/// the newly-created instance is owned and will be deleted by the object owner.
	template<class datatype> datatype* create()
	{
		return _objects.create<datatype> ();
	}

	// crateIndexed
	/// Acts like create() and registers the newly-created instance under \p key in the index.
	template<class datatype> datatype* createIndexed(const std::string& key)
	{
		datatype* obj = _objects.create<datatype> ();
		setIndex(key, obj);
		return obj;
	}

	/// Inserts \p obj in the container of the object owner and takes deletion responsability.
	inline void insertObject(Relative* obj)
	{
		_objects.insert(obj);
	}

	/// Inserts \p obj with the \p key in the container of the object owner and takes deletion responsability.
	inline void insertObject(Relative* obj, const std::string& key)
	{
		_objects.insert(obj);
		setIndex(key, obj);
	}

	/// Registers the object \p obj with the \p key in the index and returns true in case of success;
	/// please note that obj must be owned by this object owner and \p key must not be a zero length string.
	inline bool setIndex(const std::string& key, Relative* obj)
	{
		return _objects.setIndexEntry(key, obj);
	}

	/// Provides access to the object owner.
	inline ObjectOwner& getObjectOwner()
	{
		return _objects;
	}

	/// Provides const access to the object owner.
	inline const ObjectOwner& getObjectOwner() const
	{
		return _objects;
	}

	/// Inserts pointer references to all objects which have the type of the template argument
	/// (or inherit from it) into the passed vector, which has to be a vector of pointers to the template
	/// argument class.
	template<class objecttype> inline void getObjectsOfType(std::vector<
			objecttype*>& vec) const
	{
		_objects.getObjectsOfType<objecttype> (vec);
	}

	/// Returns a const reference to the underlying vector with pointers to all contained objects.
	inline const std::vector<Relative*>& getObjects() const
	{
		return _objects.getObjects();
	}

	/// Deletes the object \p obj.
	inline void removeObject(Relative* obj)
	{
		_objects.remove(obj);
	}

	/// Takes the object \p obj from the object owner.
	inline void takeObject(Relative* obj)
	{
		_objects.take(obj);
	}

	/// Clears the object owner and deletes all owned objects.
	inline void clearObjects()
	{
		_objects.clearContainer();
	}

	/// Searches the index for the \p key and returns a dynamically casted
	/// C++ pointer of type \p objecttype* to the corresponding object;
	/// in case key is not found a null pointer is returned.
	template<class objecttype> inline objecttype* findObject(
			const std::string key) const
	{
		return _objects.findObject<objecttype> (key);
	}

	/// Searches the copy history to locate the copy of \p original and
	/// returns a dynamically casted C++ pointer of type \p objecttype* to the corresponding copy;
	/// in case no copy can be traced a null pointer is returned.
	template<class objecttype> inline objecttype* findCopyOf(
			const Relative* original) const
	{
		return _objects.findCopyOf<objecttype> (original);
	}

	/// Provides direct access to the index.
	inline const std::map<std::string, Relative*>& getIndex() const
	{
		return _objects.getIndexEntry();
	}

	/// Removes the index entry with the \p key; please notice: it does not remove the object itself.
	inline void removeIndex(const std::string& key)
	{
		_objects.removeIndexEntry(key);
	}

	/// Clears the index; please notice: it does not remove the objects themself.
	inline void clearIndex()
	{
		_objects.clearIndex();
	}

	virtual std::ostream
	& print(int level = 1, std::ostream& os = std::cout, int pan = 1) const;

private:
	ObjectOwner _objects;
};

} // namespace pxl

#endif // PXL_BASE_EVENT_HH
