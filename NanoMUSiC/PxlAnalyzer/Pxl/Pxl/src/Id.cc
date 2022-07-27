//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#include "Pxl/Pxl/interface/pxl/core/Id.hh"

namespace pxl {

Id::Id()
{
	generate();
}

Id::Id (const InputStream& in)
{
	deserialize (in);
}

Id::Id(Random& rand)
{
	generate(rand);
}

void Id::generate(Random& rand)
{
	for (int i = 0; i < 4; i++)
	{
		Random::uint32 value = rand.randInt();
		char *c = (char *)&value;
		bytes[i*4+0] = c[0];
		bytes[i*4+1] = c[1];
		bytes[i*4+2] = c[2];
		bytes[i*4+3] = c[3];
	}

	/* set version 4 (random)*/
	bytes[7] &= ((1 << 4) - 1);
	bytes[7] |= 4 << 4;

	/* set variant (always DCE 1.1 only) */
	bytes[8] &= ((1 << 7) - 1);
	bytes[8] |= 2 << 6;
}

void Id::generate()
{
	static Random rand;
	generate(rand);
}

Id Id::create()
{
	Id id;
	id.generate();
	return id;
}

bool Id::operator ==(const Id& id) const
{
	for (int i = 0; i < 16; i++)
	{
		if (bytes[i] != id.bytes[i])
			return false;
	}

	return true;
}

bool Id::operator !=(const Id& id) const
{
	for (int i = 0; i < 16; i++)
	{
		if (bytes[i] != id.bytes[i])
			return true;
	}

	return false;
}

void Id::reset()
{
	for (int j = 0; j < 16; j++)
		bytes[j] = 0;
}

bool Id::operator <(const Id& op) const
{
	for (int i = 0; i < 16; i++)
	{
		if (bytes[i] < op.bytes[i])
			return true;
		else if (bytes[i] > op.bytes[i])
			return false;
	}

	return false;
}


void Id::serialize(const OutputStream &out) const
{
	for (size_t i = 0; i < 16; i++)
		out.writeUnsignedChar(bytes[i]);
}

void Id::deserialize(const InputStream &in)
{
	for (size_t i = 0; i < 16; i++)
		in.readUnsignedChar(bytes[i]);
}
	
Id::Id(const char* id)
{
	// reset
	for (int j = 0; j < 16; j++)
		bytes[j] = 0;

	// read 32 char = 16 bytes
	unsigned char first = 0, second = 0;
	int byte = 0;
	const char* source = id;
	unsigned char* target = &first;

	while (*source != 0 && byte < 16)
	{
		// find next a valid character
		if (*source >= '0' && *source <= '9')
		{
			*target = *source -'0';
		}
		else if (*source >= 'a' && *source <= 'f')
		{
			*target = *source -'a' + 10;
		}
		else if (*source >= 'A' && *source <= 'F')
		{
			*target = *source -'A' + 10;
		}
		else
		{
			source++;
			continue;
		}

		// 
		if (target == &first)
			target = &second;
		else
		{
			bytes[byte] = ((first << 4) | second);
			byte++;
			target = &first;
		}

		source++;
	}
}

Id::Id(const std::string& id)
{
	unsigned char first, second;
	int byte = 0;
	unsigned char* target = &first;
	std::string::const_iterator source = id.begin();
	std::string::const_iterator end = id.end();

	while (source != end && byte < 16)
	{
		if (*source >= '0' && *source <= '9')
		{
			*target = *source -'0';
		}
		else if (*source >= 'a' && *source <= 'f')
		{
			*target = *source -'a' + 10;
		}
		else if (*source >= 'A' && *source <= 'F')
		{
			*target = *source -'A' + 10;
		}
		else
		{
			source++;
			continue;
		}

		if (target == &first)
			target = &second;
		else
		{
			bytes[byte] = ((first << 4) | second);
			byte++;
			target = &first;
		}

		source++;
	}
}

std::ostream& operator <<(std::ostream& os, const Id &id)
{
	os << id.toString();
	return os;
}

std::string Id::toString() const
{
	static const char hex[] =
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
			'e', 'f' };

	std::string id;
	id.reserve(36);

	for (int i = 0; i < 4; i++)
	{
		id += hex[bytes[i] >> 4];
		id += hex[bytes[i] & 0x0F];
	}
	id += '-';
	for (int i = 4; i < 6; i++)
	{
		id += hex[bytes[i] >> 4];
		id += hex[bytes[i] & 0x0F];
	}
	id += '-';
	for (int i = 6; i < 8; i++)
	{
		id += hex[bytes[i] >> 4];
		id += hex[bytes[i] & 0x0F];
	}
	id += '-';
	for (int i = 8; i < 10; i++)
	{
		id += hex[bytes[i] >> 4];
		id += hex[bytes[i] & 0x0F];
	}
	id += '-';
	for (int i = 10; i < 16; i++)
	{
		id += hex[bytes[i] >> 4];
		id += hex[bytes[i] & 0x0F];
	}

	return id;
}

} // namespace pxl

