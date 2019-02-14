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

void AtlasStampDo::info(Stream& output)
{
	output.printf("ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f] PRESSURE:[%4.2f] SALINITY:[%4.2f]\n", _address, stamp_version, ready(), busy(), get_min_value(), get_max_value(), get_unit(), get_temperature(), get_vcc(), _current_pressure, _current_salinity);
}

bool const AtlasStampDo::begin()
{
	//Module inicialization and data sync
	if (_stamp_ready())
	{
		//Recover temperature, presure and salinity parameters from the module
		_load_temperature();
		_load_pressure();
		_load_salinity();
		return true;
	}
	return false;
}


bool const AtlasStampDo::_stamp_ready()
{
	_is_init = false;
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
			_is_init = true;
		}
		else if (_read_buffer(3) == 'D' && _read_buffer(4) == '.' && _read_buffer(5) == 'O' && _read_buffer(6) == '.')
		{
			stamp_version[0] = _read_buffer(8);
			stamp_version[1] = _read_buffer(9);
			stamp_version[2] = _read_buffer(10);
			stamp_version[3] = _read_buffer(11);
			stamp_version[4] = 0;
			_is_init = true;
		}
	}
	return _is_init;
}


float const AtlasStampDo::get_salinity()
{
	return _current_salinity;
}

bool const AtlasStampDo::_load_salinity()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("S,?", 300))
	{
		//En el buffer tendremos:
		// ? | S | , | 50000 | , | uS | null
		// o
		// ? | S | , | 37.5 | , | ppt | null
		byte byteFromBuffer = 0;
		for (int i = 3; i < _bytes_in_buffer(); i++)
		{
			byteFromBuffer = _read_buffer(i);
			if (',' == byteFromBuffer)
			{
				_command_buffer[i - 3] = '\0';
				break;
			}
			_command_buffer[i - 3] = byteFromBuffer;
		}
		_current_salinity = atof(_command_buffer);

#ifdef ATLAS_DEBUG_DO
		Serial.printf("DO: _load_salinity() buffer[%s] current float[%4.2f]\n", _command_buffer, _current_salinity);
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

bool const AtlasStampDo::_load_pressure()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("P,?", 300))
	{
		//En el buffer tendremos:
		// ? | P | , | 9 | 0 | . | 2 | 5 | null
		if (_bytes_in_buffer() >= 6)
		{
			char* res_buff = (char*)(_get_response_buffer() + 3);
			_current_pressure = atof(res_buff);

#ifdef ATLAS_DEBUG_DO
			Serial.printf("DO: _load_pressure() buffer[%s] current float[%4.2f]\n", res_buff, _current_pressure);
#endif
			return true;
		}
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