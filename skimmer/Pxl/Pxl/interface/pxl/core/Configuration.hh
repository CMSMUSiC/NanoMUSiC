 //-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_CORE_CONFIGURATION
#define PXL_CORE_CONFIGURATION

#include <map>
#include <string>

#include "Pxl/Pxl/interface/pxl/core/Variant.hh"
#include "Pxl/Pxl/interface/pxl/core/macros.hh"


namespace pxl
{

class PXL_DLL_EXPORT Configuration
{
	private:
		std::map< std::string, std::multimap<std::string, Variant> > _data;

	public:

		typedef std::pair<std::string, Variant> itemType;

		typedef std::multimap<std::string, Variant> multimapType;
		typedef multimapType::const_iterator multiMapConstIterator;
		typedef multimapType::iterator multimapIterator;

		typedef std::map<std::string, multimapType> mapType;
		typedef mapType::const_iterator mapConstIterator;
		typedef mapType::iterator mapIterator;
		Configuration()
		{
		
		};
		
		static Configuration &instance();

		/// add key, item to section, create section if necessary
		void addItem(const std::string &section, const std::string &key, const Variant &item);

		/// Returns first item with key in section
		const Variant &getItem(const std::string &section, const std::string &key);

		/// Returns multimap for the section
		multimapType& getSection(const std::string &section);
};
} // namespace

#endif //PXL_CORE_CONFIGURATION
