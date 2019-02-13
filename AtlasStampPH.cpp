// 
// 
// 

#include "AtlasStampPH.h"
//AtlasStamp(address, unit, unit_len, min_value, max_value, num_fields_in_response)
//
AtlasStampPh::AtlasStampPh(uint8_t address) :
	AtlasStampTemperatureCompensated(address, "PH", 2, 0.001f, 14.000f, 1)

{
}

bool const AtlasStampPh::begin()
{
	//Inicialzamos el sensor
	if (_stampReady())
	{
		//Force initial reading of stored temperature value
		get_temperature(true);
		return true;
	}
	return false;
	
}


bool const AtlasStampPh::_stampReady()
{
	bool isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stampConnected())
	{
		// PH EZO  -> '?I,pH,1.1'
		//Comprobamos si es nuestro tipo de sensor :)
		if (_readBuffer(3) == 'p' && _readBuffer(4) == 'H')
		{
			stamp_version[0] = _readBuffer(6);
			stamp_version[1] = _readBuffer(7);
			stamp_version[2] = _readBuffer(8);
			stamp_version[3] = _readBuffer(9);
			stamp_version[4] = '\0';
			isReady = true;
		}
	}
	_ready(isReady);
	return isReady;
}