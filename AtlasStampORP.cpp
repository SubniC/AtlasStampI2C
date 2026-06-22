#include "AtlasStampORP.h"

AtlasStampOrp::AtlasStampOrp(byte address) :
	AtlasStamp(address, "mV", 2, -1019.9f, 1019.9f)
{
}

bool const AtlasStampOrp::begin()
{
	return _stamp_ready();
}

bool const AtlasStampOrp::_stamp_ready()
{
	_is_init = false;
	// The base class checks that an EZO device answers at the address and loads the
	// INFO response into the buffer; here we parse and store the module version.
	if (_stamp_connected())
	{
		// ORP EZO -> '?I,OR,1.0' (the datasheet wrongly shows 'OR' instead of 'ORP').
		if (_read_buffer(3) == 'O' && _read_buffer(4) == 'R')
		{
			// Modules may report either 'OR' or 'ORP', so handle both.
			if (_read_buffer(5) == 'P')
			{
				stamp_version[0] = _read_buffer(7);
				stamp_version[1] = _read_buffer(8);
				stamp_version[2] = _read_buffer(9);
				stamp_version[3] = _read_buffer(10);
				stamp_version[4] = '\0';
			}
			else
			{
				stamp_version[0] = _read_buffer(6);
				stamp_version[1] = _read_buffer(7);
				stamp_version[2] = _read_buffer(8);
				stamp_version[3] = _read_buffer(9);
				stamp_version[4] = '\0';
			}
			_is_init = true;
		}
	}
	return _is_init;
}