#include "AtlasStampPH.h"


AtlasStampPh::AtlasStampPh(uint8_t address) :
	AtlasStampTemperatureCompensated(address, "PH", 2, 0.001f, 14.000f)
{
}

bool const AtlasStampPh::begin()
{
	if (_stamp_ready())
	{
		// Force an initial read of the stored temperature value.
		_load_temperature();
		return true;
	}
	return false;
}


bool const AtlasStampPh::_stamp_ready()
{
	_is_init = false;
	// The base class checks that an EZO device answers at the address and loads the
	// INFO response into the buffer; here we parse and store the module version.
	if (_stamp_connected())
	{
		// PH EZO -> '?I,pH,1.1'
		if (_read_buffer(3) == 'p' && _read_buffer(4) == 'H')
		{
			stamp_version[0] = _read_buffer(6);
			stamp_version[1] = _read_buffer(7);
			stamp_version[2] = _read_buffer(8);
			stamp_version[3] = _read_buffer(9);
			stamp_version[4] = '\0';
			_is_init = true;
		}
	}
	return _is_init;
}