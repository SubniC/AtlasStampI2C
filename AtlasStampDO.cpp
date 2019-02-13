// 
// 
// 

#include "AtlasStampDO.h"

AtlasStampDo::AtlasStampDo(byte address) : AtlasStampTemperatureCompensated(address), _current_pressure(-2048.0),_current_salinity(-2048.0)
{
	min_value(0.01f);
	max_value(35.99f);
	unit("mg/L");
}

char* AtlasStampDo::info()
{
	sprintf(_infoBuffer, "ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f] PRESSURE:[%4.2f] SALINITY:[%4.2f]", _address, stamp_version, ready(), busy(), min_value(), max_value(), unit(), get_temperature(false), _vcc(), get_pressure(false),get_salinity(false));
	return _infoBuffer;
}

bool AtlasStampDo::begin()
{
	//Inicialzamos el sensor
	if (_stampReady())
	{
		//Recuperamos la temperatura, presion y salinidad actuales
		_get_temperature();
		_get_pressure();
		_get_salinity();
		return true;
	}
	return false;
}

float AtlasStampDo::read()
{
	byte commandResult = _command(ATLAS_READ_COMAND, 600);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		return _parseResult();
	}
	//TODO: Afinar codigos de respuesta :)
	return -2048.0f;
}

bool AtlasStampDo::readAsync()
{
	return _command_async(ATLAS_READ_COMAND, 600);
}

float AtlasStampDo::resultAsync()
{
	byte commandResult = _command_result();
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		return _parseResult();
	}
	//TODO: Afinar codigos de respuesta :)
	return -2048.0f;
}

float AtlasStampDo::_parseResult()
{
	//Este sensor se supone que solo manda el DO
	//con formato 1,DO,NULL
	//Asi que lo tenemos directamente
	return atof(_getBuffer());
}


bool AtlasStampDo::_stampReady()
{
	bool isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stampConnected())
	{
		// DO EZO  -> '?I,D.O.,1.0' || '?I,DO,1.7' (-> exists in D.O. and DO form)
		if (_readBuffer(3) == 'D' && _readBuffer(4) == 'O')
		{
			stamp_version[0] = _readBuffer(6);
			stamp_version[1] = _readBuffer(7);
			stamp_version[2] = _readBuffer(8);
			stamp_version[3] = _readBuffer(9);
			stamp_version[4] = 0;
			isReady = true;
		}
		else if (_readBuffer(3) == 'D' && _readBuffer(4) == '.' && _readBuffer(5) == 'O' && _readBuffer(6) == '.')
		{
			stamp_version[0] = _readBuffer(8);
			stamp_version[1] = _readBuffer(9);
			stamp_version[2] = _readBuffer(10);
			stamp_version[3] = _readBuffer(11);
			stamp_version[4] = 0;
			isReady = true;
		}
	}
	_ready(isReady);
	return isReady;
}


float AtlasStampDo::get_salinity(bool force)
{
	if (force)
	{
		return _get_salinity();
	}
	else
	{
		return _current_pressure;
	}
}

float AtlasStampDo::_get_salinity()
{
	byte commandResult = _command("S,?", 300);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		//En el buffer tendremos:
		// ? | S | , | 50000 | , | uS | null
		// o
		// ? | S | , | 37.5 | , | ppt | null
		//pero no sabemos exactamente cuandos caracteres son la temperatura
		//asi que lo haremos entre el que sabemos que es el primero y NULL
		//recorremos el buffer
		byte byteFromBuffer = 0;
		char tmpBuffer[12] = { 0 };
		for (int i = 3; i < _bytes_in_buffer() - 1; i++)
		{
			byteFromBuffer = _readBuffer(i);
			//Al encontrar la , hemos terminado :)
			if (',' == byteFromBuffer)
			{
				tmpBuffer[i - 3] = '\0';
				break;
			}
			tmpBuffer[i - 3] = byteFromBuffer;
		}
		//Despues del bucle debemos tener en tmpTemperature la cadena con el 
		//numero para pasarselo a ATOF
		_current_pressure = atof(tmpBuffer);
	}
	//TODO: Afinar codigos de respuesta :)
	return _current_pressure;
}


bool AtlasStampDo::set_salinity(float value, byte unit, float max_divergence)
{
	if (abs(value - _current_salinity) >= max_divergence)
	{
		return set_salinity(value,unit);
	}
	return false;
}

bool AtlasStampDo::set_salinity(float value, byte unit)
{
	char buffer[24] = { 0 };
	if (unit == ATLAS_SALINITY_UNIT_PPT)
	{
		sprintf(buffer, "S,%4.1f,ppt", value);
	}
	else
	{
		sprintf(buffer, "S,%d", (int)value);
	}
	//Generamos el comando
	byte commandResult = _command(buffer, 300);
	if (commandResult)
	{
		return true;
	}
	return false;
}



float AtlasStampDo::get_pressure(bool force)
{
	if (force)
	{
		return _get_pressure();
	}
	else
	{
		return _current_pressure;
	}
}

float AtlasStampDo::_get_pressure()
{
	byte commandResult = _command("P,?", 300);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		//En el buffer tendremos:
		// ? | P | , | 9 | 0 | . | 2 | 5 | null
		//pero no sabemos exactamente cuandos caracteres son la presion
		//asi que lo haremos entre el que sabemos que es el primero y NULL
		//recorremos el buffer
		byte byteFromBuffer = 0;
		char tmpBuffer[7] = { 0 };
		for (int i = 3; i < _bytes_in_buffer() - 1; i++)
		{
			byteFromBuffer = _readBuffer(i);
			if (NULL_CHARACTER == byteFromBuffer)
			{
				tmpBuffer[i - 3] = '\0';
				break;
			}
			tmpBuffer[i - 3] = byteFromBuffer;
		}
		//Despues del bucle debemos tener en tmpPres la cadena con el 
		//numero para pasarselo a ATOF
		_current_pressure = atof(tmpBuffer);
	}
	//TODO: Afinar codigos de respuesta :)
	return _current_pressure;
}

bool AtlasStampDo::set_pressure(float value, float max_divergence)
{
	if (abs(value - _current_pressure) >= max_divergence)
	{
		return set_pressure(value);
	}
	return false;
}

bool AtlasStampDo::set_pressure(float value)
{
	char buffer[7] = { 0 };
	//Guardamos el numero en el buffer
	sprintf(buffer, "P,%4.2f", value);
	//Generamos el comando
	byte commandResult = _command(buffer, 300);
	if (commandResult)
	{
		return true;
	}
	return false;
}