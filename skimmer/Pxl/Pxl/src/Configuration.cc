//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------


#include "Pxl/Pxl/interface/pxl/core/Configuration.hh"


namespace pxl
{


Configuration &Configuration::instance()
{
	static Configuration configuration;
	return configuration;
}


void Configuration::addItem(const std::string &section, const std::string &key, const Variant &item)
{
	mapIterator iter = _data.find(section);
	if (iter == _data.end())
	{
		Configuration::multimapType newSection;
		iter = _data.insert(iter, std::pair<std::string, Configuration::multimapType>(section,newSection) );
	}
	iter->second.insert(std::pair<std::string, Variant>(key, item));
}


const Variant &Configuration::getItem(const std::string &section, const std::string &key)
{
	return (*_data[section].find(key)).second;
}


Configuration::multimapType& Configuration::getSection(const std::string &section)
{
	return _data[section];
}


} // namespace
