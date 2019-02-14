// 
// 
// 

#include "AtlasStampDO.h"

AtlasStampDo::AtlasStampDo(byte address) :
	AtlasStampTemperatureCompensated(address, "mg/L", 4, 0.01f, 35.99f),
	_current_pressure(-2048.0f),
	_current_salinity(-2048.0f)
{
}

char* const AtlasStampDo::info()
{
	sprintf(_infoBuffer, "ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f] PRESSURE:[%4.2f] SALINITY:[%4.2f]", _address, stamp_version, ready(), busy(), get_min_value(), get_max_value(), get_unit(), get_temperature(), get_vcc(), _current_pressure, _current_salinity);
	return _infoBuffer;
}

bool const AtlasStampDo::begin()
{
	//Inicialzamos el sensor
	if (_stamp_ready())
	{
		//Recuperamos la temperatura, presion y salinidad actuales
		_get_temperature();
		_get_pressure();
		_get_salinity();
		return true;
	}
	return false;
}


bool const AtlasStampDo::_stamp_ready()
{
	bool isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stamp_connected())
	{
		// DO EZO  -> '?I,D.O.,1.0' || '?I,DO,1.7' (-> exists in D.O. and DO form)
		if (_read_buffer(3) == 'D' && _read_buffer(4) == 'O')
		{
			stamp_version[0] = _read_buffer(6);
			stamp_version[1] = _read_buffer(7);
			stamp_version[2] = _read_buffer(8);
			stamp_version[3] = _read_buffer(9);
			stamp_version[4] = 0;
			isReady = true;
		}
		else if (_read_buffer(3) == 'D' && _read_buffer(4) == '.' && _read_buffer(5) == 'O' && _read_buffer(6) == '.')
		{
			stamp_version[0] = _read_buffer(8);
			stamp_version[1] = _read_buffer(9);
			stamp_version[2] = _read_buffer(10);
			stamp_version[3] = _read_buffer(11);
			stamp_version[4] = 0;
			isReady = true;
		}
	}
	_ready(isReady);
	return isReady;
}


float const AtlasStampDo::get_salinity()
{
	return _current_salinity;
}

bool const AtlasStampDo::_get_salinity()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("S,?", 300))
	{
		//En el buffer tendremos:
		// ? | S | , | 50000 | , | uS | null
		// o
		// ? | S | , | 37.5 | , | ppt | null
		//pero no sabemos exactamente cuandos caracteres son la temperatura
		//asi que lo haremos entre el que sabemos que es el primero y NULL
		//recorremos el buffer
		byte byteFromBuffer = 0;
		//char tmpBuffer[12] = { 0 };
		for (int i = 3; i < _bytes_in_buffer(); i++)
		{
			byteFromBuffer = _read_buffer(i);
			//Al encontrar la , hemos terminado :)
			if (',' == byteFromBuffer)
			{
				_command_buffer[i - 3] = '\0';
				break;
			}
			_command_buffer[i - 3] = byteFromBuffer;
		}
		//Despues del bucle debemos tener en tmpTemperature la cadena con el 
		//numero para pasarselo a ATOF
		_current_salinity = atof(_command_buffer);

#ifdef ATLAS_DEBUG_DO
		Serial.printf("DO: _get_salinity() buffer[%s] current float[%4.2f]\n", _command_buffer, _current_salinity);
#endif
		return true;
	}
	return false;
}


bool const AtlasStampDo::set_salinity(float value, byte unit, float max_divergence)
{
	if (abs(value - _current_salinity) >= max_divergence)
	{
		return set_salinity(value,unit);
	}
	return false;
}

bool const AtlasStampDo::set_salinity(float value, byte unit)
{
	if (unit == ATLAS_SALINITY_UNIT_PPT)
	{
		sprintf(_command_buffer, "S,%4.1f,ppt", value);
	}
	else
	{
		sprintf(_command_buffer, "S,%d", (int)value);
	}
	//Generamos el comando
	if (_command(_command_buffer, 300))
	{
		//TODO: Verificar pidiendo el valor?
		_current_salinity = value;
		return true;
	}
	return false;
}



float const AtlasStampDo::get_pressure()
{
	return _current_pressure;
}

bool const AtlasStampDo::_get_pressure()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("P,?", 300))
	{
		//En el buffer tendremos:
		// ? | P | , | 9 | 0 | . | 2 | 5 | null
		char* res_buff = (char*)(_get_response_buffer() + 3);
		_current_pressure = atof(res_buff);

#ifdef ATLAS_DEBUG_DO
		Serial.printf("DO: _get_pressure() buffer[%s] current float[%4.2f]\n", res_buff, _current_pressure);
#endif
		return true;
	}
	return false;
}

bool const AtlasStampDo::set_pressure(float value, float max_divergence)
{
	if (abs(value - _current_pressure) >= max_divergence)
	{
		return set_pressure(value);
	}
	return false;
}

bool const AtlasStampDo::set_pressure(float value)
{
	//Guardamos el numero en el buffer
	sprintf(_command_buffer, "P,%4.2f", value);
	//Generamos el comando
	if (_command(_command_buffer, 300))
	{
		//TODO: Verificar pidiendo el valor?
		_current_pressure = value;
		return true;
	}
	return false;
}