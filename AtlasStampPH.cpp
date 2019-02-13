// 
// 
// 

#include "AtlasStampPH.h"


AtlasStampPh::AtlasStampPh(uint8_t address) :
	AtlasStampTemperatureCompensated(address, "PH", 2, 0.001f, 14.000f, 1)

{
	//https://www.quora.com/What-is-the-unit-of-measure-for-pH
}

bool const AtlasStampPh::begin()
{
	//Inicialzamos el sensor
	if (_stamp_ready())
	{
		//Force initial reading of stored temperature value
		_get_temperature();
		return true;
	}
	return false;
	
}


bool const AtlasStampPh::_stamp_ready()
{
	bool isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _response_buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stamp_connected())
	{
		// PH EZO  -> '?I,pH,1.1'
		//Comprobamos si es nuestro tipo de sensor :)
		if (_read_buffer(3) == 'p' && _read_buffer(4) == 'H')
		{
			stamp_version[0] = _read_buffer(6);
			stamp_version[1] = _read_buffer(7);
			stamp_version[2] = _read_buffer(8);
			stamp_version[3] = _read_buffer(9);
			stamp_version[4] = '\0';
			isReady = true;
		}
	}
	_ready(isReady);
	return isReady;
}