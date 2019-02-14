// 
// 
// 

#include "AtlasStampEC.h"

AtlasStampEc::AtlasStampEc(byte address) : 
	AtlasStampTemperatureCompensated(address, "uS/cm", 5, 0.07f, 500000.0f, 4)
{
}


//https://www.learncpp.com/cpp-tutorial/3-8a-bit-flags-and-bit-masks/
bool const AtlasStampEc::set_output_parameter(Parameters type, bool value)
{
	if (static_cast<bool>(_parameter_state & type) == value)
	{
#ifdef ATLAS_DEBUG_EC
		Serial.printf("EC: The output parameter [%s] is already in the desired state [%d]\n", parameter_to_char(type), value);
#endif
		//The output parameter is already in the desired state
		return true;
	}

	sprintf(_command_buffer, "O,%s,%d", parameter_to_char(type), value);
	if (ATLAS_SUCCESS_RESPONSE == _command(_command_buffer, 300))
	{
		//No output for this command
		if (value)
		{
			_parameter_state |= type;
			_resize_response_count(response_count()+1);
		}
		else
		{
			_parameter_state &= ~type;
			_resize_response_count(response_count() - 1);
		}

#ifdef ATLAS_DEBUG_EC
		Serial.printf("EC: Output parameter [%s] changed state to [%d]\n", parameter_to_char(type), value);
#endif

		return true;
	}

	return false;
}


bool const AtlasStampEc::get_output_parameter(Parameters type)
{
	//Needs the module to be inicialized
	return _parameter_state & type;
}

bool const AtlasStampEc::_load_parameters()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("O,?", 300))
	{
		char* res_buff = (char*)(_get_response_buffer() + 3);
		if (strcmp(res_buff, "No output") == 0)
		{
			_parameter_state = 0;
			_resize_response_count(0);
#ifdef ATLAS_DEBUG_EC
			Serial.println("EC: All output disabled for EC module...\n");
#endif
		}
		else
		{
			//http://www.cplusplus.com/reference/cstring/strtok/?kw=strtok
			char *current_token;
			current_token = strtok(res_buff, ",");
			uint8_t _new_count = 0;
			while (current_token != NULL)
			{
				if (strcmp("EC", current_token) == 0)
				{
					_parameter_state |= Parameters::EC;
					_new_count++;
#ifdef ATLAS_DEBUG_EC
					Serial.println("EC: EC output enabled");
#endif
				}
				else if (strcmp("TDS", current_token) == 0)
				{
					_parameter_state |= Parameters::TDS;
					_new_count++;
#ifdef ATLAS_DEBUG_EC
					Serial.println("EC: TDS output enabled");
#endif
				}
				else if(strcmp("S", current_token) == 0)
				{
					_parameter_state |= Parameters::S;
					_new_count++;

#ifdef ATLAS_DEBUG_EC
					Serial.println("EC: S output enabled");
#endif
				}
				else if(strcmp("SG", current_token) == 0)
				{
					_parameter_state |= Parameters::SG;
					_new_count++;
#ifdef ATLAS_DEBUG_EC
					Serial.println("EC: SG output enabled");
#endif
				}
#ifdef ATLAS_DEBUG_EC
				else
				{
					Serial.printf("Unknown parameter in available output for EC module value [%s]\n", current_token);
				}
#endif
				current_token = strtok(NULL, ",");
			}
			_resize_response_count(_new_count);
		}
		return true;
	}
	return false;
}

char* const AtlasStampEc::info()
{
	sprintf(_infoBuffer, "ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f] K:[%4.2f]", _address, stamp_version, ready(), busy(), get_min_value(), get_max_value(), get_unit(), get_temperature(), get_vcc(), get_k());
	return _infoBuffer;
}

bool const AtlasStampEc::begin()
{
	//Inicialize sensor information
	if (_stamp_ready())
	{
		//Load module output parameters
		_load_parameters();

		//Load the module current temperature value
		_get_temperature();
		return true;
	}
	return false;
}


//TODO: revisar de aquí hacia abajo
bool const AtlasStampEc::_stamp_ready()
{
	bool isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _response_buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stamp_connected())
	{
		// EC EZO  -> '?I,EC,1.0 '
		//Comprobamos si es nuestro tipo de sensor :)
		if (_read_buffer(3) == 'E' && _read_buffer(4) == 'C')
		{
			stamp_version[0] = _read_buffer(6);
			stamp_version[1] = _read_buffer(7);
			stamp_version[2] = _read_buffer(8);
			stamp_version[3] = _read_buffer(9);
			stamp_version[4] = 0;
			isReady = true;
		}
	}
	_ready(isReady);
	return isReady;
}

float AtlasStampEc::get_k()
{
	float tmpk = -2048.0;
	byte commandResult = _command("K,?", 300);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		//En el buffer tendremos:
		// ? | K | , | 0 | . | 6 | 6 | null
		//pero no sabemos exactamente cuandos caracteres son la k
		//asi que lo haremos entre el que sabemos que es el primero y NULL
		//recorremos el buffer
		byte byteFromBuffer = 0;
		char tmpBuffer[7] = { 0 };
		for (int i = 3; i < _bytes_in_buffer() - 1; i++)
		{
			byteFromBuffer = _read_buffer(i);
			if (NULL_CHARACTER == byteFromBuffer)
			{
				tmpBuffer[i - 3] = '\0';
				break;
			}
			tmpBuffer[i - 3] = byteFromBuffer;
		}
		//Despues del bucle debemos tener en tmpPres la cadena con el 
		//numero para pasarselo a ATOF
		tmpk = atof(tmpBuffer);
	}
	//TODO: Afinar codigos de respuesta :)
	return tmpk;
}

bool AtlasStampEc::set_k(float value)
{
	char buffer[7] = { 0 };
	//Guardamos el numero en el buffer
	sprintf(buffer, "K,%4.2f", value);
	//Generamos el comando
	byte commandResult = _command(buffer, 300);
	if (commandResult)
	{
		return true;
	}
	return false;
}