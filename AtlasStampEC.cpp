// 
// 
// 

#include "AtlasStampEC.h"

//TODO: Implementar configuracion d eparametros en la trama
//https://www.atlas-scientific.com/_files/_datasheets/_circuit/EC_EZO_Datasheet.pdf
//necesitamos unas fucniones tipo get_fields() y set_fields() que fije los campos que recuperartemos del sensor
//luego el numero de posiciones de la respuesta variara
//TODO: segun he leido por ahi seria recomendable cambiar el array por un vector

AtlasStampEc::AtlasStampEc(byte address) : 
	AtlasStampTemperatureCompensated(address, "uS/cm", 5, 0.07f, 500000.0f, 4)
{
}



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
		}
		else
		{
			_parameter_state &= ~type;
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
#ifdef ATLAS_DEBUG_EC
			Serial.println("EC: All output disabled for EC module...\n");
#endif
		}
		else
		{
			char *current_token;
			current_token = strtok(res_buff, ",");
			while (current_token != NULL)
			{
				if (strcmp("EC", current_token) == 0)
				{
					_parameter_state |= Parameters::EC;
#ifdef ATLAS_DEBUG_EC
					Serial.println("EC: EC output enabled");
#endif
				}
				else if (strcmp("TDS", current_token) == 0)
				{
					_parameter_state |= Parameters::TDS;
#ifdef ATLAS_DEBUG_EC
					Serial.println("EC: TDS output enabled");
#endif
				}
				else if(strcmp("S", current_token) == 0)
				{
					_parameter_state |= Parameters::S;
#ifdef ATLAS_DEBUG_EC
					Serial.println("EC: S output enabled");
#endif
				}
				else if(strcmp("SG", current_token) == 0)
				{
					_parameter_state |= Parameters::SG;
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
		}
		return true;
	}
	return false;
}

char* const AtlasStampEc::info()
{
	sprintf(_infoBuffer, "ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f] K:[%4.2f]", _address, stamp_version, ready(), busy(), get_min_value(), get_max_value(), get_unit(), get_temperature(false), get_vcc(), get_k());
	return _infoBuffer;
}

bool const AtlasStampEc::begin()
{
	//Inicialize sensor information
	if (_stamp_ready())
	{
		//Load module output parameters
		_load_parameters();

		//Load the module current temperature
		get_temperature(true);
		return true;
	}
	return false;
}

//void AtlasStampEc::_parseResult()
//{
//	char tmpItem[16] = { 0 };
//	byte tmpIndex = 0;
//	byte resultIndex = 0;
//	//Esta trama de respuesta tiene varios parametros
//	//Ahora tenemos en el buffer:
//	// EC,TDS,SAL,SG
//	//  Siendo:
//	//  EC electrical conductivity
//	//	TDS Total dissolved solids
//	//	SAL Salinity
//	//	SG Specific gravity of sea water
//#ifdef ATLAS_DEBUG_EC
//	Serial.printf("AtlasStampEc::_parseResult() BUFFER[%d] = [%s]\n",_bytes_in_buffer(),_get_response_buffer());
//#endif
//	byte byteFromBuffer = 0;
//	for (int i = 0; i < _bytes_in_buffer(); i++)
//	{
//		//Obtenemos el byte de la posición I
//		byteFromBuffer = _read_buffer(i);	
//		//Si el byte es una coma
//		if (byteFromBuffer == ',' || (NULL_CHARACTER == byteFromBuffer || '\0' == byteFromBuffer))
//		{
//			//Es que hemos encontrado el final de un dato (un float como string)
//			//Añadimos en la posicion correspondiente el caracrer de final de cadena
//			tmpItem[tmpIndex] = '\0';
//#ifdef ATLAS_DEBUG_EC
//			Serial.printf("AtlasStampEc::_parseResult() tmpItem = [%s] (char[%d])\n", tmpItem, tmpIndex);
//#endif
//			//Transformamos el array de char a un float
//			_parsedResult[resultIndex] = atof(tmpItem);
//
//#ifdef ATLAS_DEBUG_EC
//			Serial.printf("AtlasStampEc::_parseResult() _parsedResult[%d]: [%f] (float)\n",resultIndex,_parsedResult[resultIndex]);
//#endif
//			//Incrementamos el contador de resultados :)
//			resultIndex++;
//			//Ponemos a 0 el indice del buffer temporal de floats
//			tmpIndex = 0;
//			//Nos saltamos el resto de la iteracion, ya tenemos lo que queriamos
//			if (byteFromBuffer == ',')
//			{
//				continue;
//			}
//			else
//			{
//				break;
//			}
//		}
//		else
//		{
//			//Si noe s ni una coma ni un final de cadena, estamos recogiendo datos
//			//asi que lo almacenamos en el buffer temporal
//			tmpItem[tmpIndex] = byteFromBuffer;
//			tmpIndex++;
//		}
//	}
//#ifdef ATLAS_DEBUG_EC
//	Serial.printf("AtlasStampEc::_parseResult() %d [%f] [%f] [%f] [%f]\n", &_parsedResult, _parsedResult[0], _parsedResult[1], _parsedResult[2], _parsedResult[3]);
//#endif
//}


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