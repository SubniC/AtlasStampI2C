#include "AtlasStampTemperatureCompensated.h"

AtlasStampTemperatureCompensated::AtlasStampTemperatureCompensated(byte address) :
	AtlasStamp(address), _current_temperature(-2048.0)
{

}


char* AtlasStampTemperatureCompensated::info()
{
	sprintf(_infoBuffer, "ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f]",_address, stamp_version, ready(), busy(), min_value(), max_value(), unit(), get_temperature(false),_vcc());
	return _infoBuffer;
}

float AtlasStampTemperatureCompensated::get_temperature(bool force)
{
	//Serial1.println("DEBUG float AtlasStampTemperatureCompensated::temperature(bool force)");
	if (force)
	{
		return _get_temperature();
	}
	else
	{
		return _current_temperature;
	}
}

float AtlasStampTemperatureCompensated::_get_temperature()
{
	//Serial1.println("DEBUG float AtlasStampTemperatureCompensated::_temperature()");

	byte commandResult = _command(ATLAS_TEMPERATURE_READ_COMAND, 300);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		//En el buffer tendremos:
		// ? | T | , | 1 | 9 | . | 5 | null
		//pero no sabemos exactamente cuandos caracteres son la temperatura
		//asi que lo haremos entre el que sabemos que es el primero y NULL
		//recorremos el buffer
		byte byteFromBuffer = 0;
		char tmpBuffer[6] = { 0 };
		//Serial.printf("Bytes in buffer: %d\n", _bytes_in_buffer());
		for (int i = 3; i < _bytes_in_buffer() - 1; i++)
		{
			byteFromBuffer = _readBuffer(i);

			if (NULL_CHARACTER == byteFromBuffer)
			{
				tmpBuffer[i - 3] = '\0';
				break;
			}
			tmpBuffer[i - 3] = byteFromBuffer;
			//Serial.println(char(byteFromBuffer));
		}
		//Despues del bucle debemos tener en tmpTemperature la cadena con el 
		//numero para pasarselo a ATOF
#ifdef ATLAS_DEBUG
		Serial1.printf("AtlasStampTemperatureCompensated::temperature buffer [%s] global buffer [%s]\n", tmpBuffer, _getBuffer());
#endif
		_current_temperature = atof(tmpBuffer);
	}
	//TODO: Afinar codigos de respuesta :)
	return _current_temperature;
}


bool AtlasStampTemperatureCompensated::set_temperature(float temp, float max_divergence)
{
	//Serial1.printf("temp[%4.2f] _current_temperature[%4.2f] max_divergence[%4.2f]\n", temp, _current_temperature, max_divergence);
	if ((!busy()) && (abs(temp - _current_temperature) >= max_divergence))
	{
		return set_temperature(temp);
	}
	return false;
}

bool AtlasStampTemperatureCompensated::set_temperature(float temp)
{
	if(!busy())
	{
		char buffer[7] = { 0 };
		//Guardamos el numero en el buffer
		sprintf(buffer, "T,%4.2f", temp);

		//Generamos el comando
		byte commandResult = _command(buffer, 300);
		
		//Serial1.printf("CMD[%s] RES[%d]\n", buffer, commandResult);
		
		if (commandResult)
		{
			_current_temperature = temp;
			return true;
		}
	}
	return false;
}