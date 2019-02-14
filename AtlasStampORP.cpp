// 
// 
// 

#include "AtlasStampORP.h"

AtlasStampOrp::AtlasStampOrp(byte address) : 
	AtlasStamp(address, "mV", 2, -1019.9f, 1019.9f, 1)

{
	//https://www.atlas-scientific.com/_files/_datasheets/_circuit/ORP_EZO_datasheet.pdf
}

bool const AtlasStampOrp::begin()
{
	//Inicialzamos el sensor
	return  _stamp_ready();
}

bool const AtlasStampOrp::_stamp_ready()
{
	bool isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stamp_connected())
	{
		// ORP EZO -> '?I,OR,1.0'   (-> wrong in documentation 'OR' instead of 'ORP')
		//Comprobamos si es nuestro tipo de sensor :)
		if (_read_buffer(3) == 'O' && _read_buffer(4) == 'R')
		{
			//No sabemos si llega OR o ORP, de ehcho a mi me llega ORP con el mio
			//asi que nos curamos en salud
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
			isReady = true;
		}
	}
	_ready(isReady);
	return isReady;
}