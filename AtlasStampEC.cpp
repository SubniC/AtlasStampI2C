// 
// 
// 

#include "AtlasStampEC.h"

//TODO: Implementar configuracion d eparametros en la trama
//https://www.atlas-scientific.com/_files/_datasheets/_circuit/EC_EZO_Datasheet.pdf
//necesitamos unas fucniones tipo get_fields() y set_fields() que fije los campos que recuperartemos del sensor
//luego el numero de posiciones de la respuesta variara
//TODO: segun he leido por ahi seria recomendable cambiar el array por un vector

AtlasStampEc::AtlasStampEc(byte address) : AtlasStampTemperatureCompensated(address)
{
	min_value(0.07f);
	max_value(500000.0f);
	unit("uS/cm");
	_parsedResult[0] = -2048.0;
	_parsedResult[1] = -2048.0;
	_parsedResult[2] = -2048.0;
	_parsedResult[3] = -2048.0;
}

char* AtlasStampEc::info()
{
	sprintf(_infoBuffer, "ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f] K:[%4.2f]", _address, stamp_version, ready(), busy(), min_value(), max_value(), unit(), get_temperature(false), _vcc(), get_k());
	return _infoBuffer;
}

bool AtlasStampEc::begin()
{
	//Inicialzamos el sensor
	if (_stampReady())
	{
		//Fijamos la recuperamos la temperatura actual
		_get_temperature();
		return true;
	}
	return false;
}

float* AtlasStampEc::last()
{
	return _parsedResult;
}

float* AtlasStampEc::read()
{
	//TODO: Esta funcion esta a  medio implementar
	byte commandResult = _command(ATLAS_READ_COMAND, 1000);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		_parseResult();
	}
	//Si todo ha ido bien, llegados a este punto tendremos en readResult un array
	//con 4 floats que son los datos obtenidos de la medida del sensor
	return _parsedResult;
}

//TODO: se podria mover a la clase base
bool AtlasStampEc::readAsync()
{
	return _command_async(ATLAS_READ_COMAND, 1000);
}

float* AtlasStampEc::resultAsync()
{
	byte commandResult = _command_result();
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		_parseResult();
	}

	return _parsedResult;
}

void AtlasStampEc::_parseResult()
{
	char tmpItem[16] = { 0 };
	byte tmpIndex = 0;
	byte resultIndex = 0;
	//Esta trama de respuesta tiene varios parametros
	//Ahora tenemos en el buffer:
	// EC,TDS,SAL,SG
	//  Siendo:
	//  EC electrical conductivity
	//	TDS Total dissolved solids
	//	SAL Salinity
	//	SG Specific gravity of sea water
#ifdef ATLAS_DEBUG_EC
	Serial.printf("AtlasStampEc::_parseResult() BUFFER[%d] = [%s]\n",_bytes_in_buffer(),_getBuffer());
#endif
	byte byteFromBuffer = 0;
	for (int i = 0; i < _bytes_in_buffer(); i++)
	{
		//Obtenemos el byte de la posición I
		byteFromBuffer = _readBuffer(i);	
		//Si el byte es una coma
		if (byteFromBuffer == ',' || (NULL_CHARACTER == byteFromBuffer || '\0' == byteFromBuffer))
		{
			//Es que hemos encontrado el final de un datos (un float como string)
			//Añadimos en la posicion correspondiente el caracrer de final de cadena
			tmpItem[tmpIndex] = '\0';
#ifdef ATLAS_DEBUG_EC
			Serial.printf("AtlasStampEc::_parseResult() tmpItem = [%s] (char[%d])\n", tmpItem, tmpIndex);
#endif
			//Transformamos el array de char a un float
			_parsedResult[resultIndex] = atof(tmpItem);

#ifdef ATLAS_DEBUG_EC
			Serial.printf("AtlasStampEc::_parseResult() _parsedResult[%d]: [%f] (float)\n",resultIndex,_parsedResult[resultIndex]);
#endif
			//Incrementamos el contador de resultados :)
			resultIndex++;
			//Ponemos a 0 el indice del buffer temporal de floats
			tmpIndex = 0;
			//Nos saltamos el resto de la iteracion, ya tenemos lo que queriamos
			if (byteFromBuffer == ',')
			{
				continue;
			}
			else
			{
				break;
			}
		}
		else
		{
			//Si noe s ni una coma ni un final de cadena, estamos recogiendo datos
			//asi que lo almacenamos en el buffer temporal
			tmpItem[tmpIndex] = byteFromBuffer;
			tmpIndex++;
		}
	}
#ifdef ATLAS_DEBUG_EC
	Serial.printf("AtlasStampEc::_parseResult() %d [%f] [%f] [%f] [%f]\n", &_parsedResult, _parsedResult[0], _parsedResult[1], _parsedResult[2], _parsedResult[3]);
#endif
}


bool AtlasStampEc::_stampReady()
{
	boolean isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stampConnected())
	{
		// EC EZO  -> '?I,EC,1.0 '
		//Comprobamos si es nuestro tipo de sensor :)
		if (_readBuffer(3) == 'E' && _readBuffer(4) == 'C')
		{
			stamp_version[0] = _readBuffer(6);
			stamp_version[1] = _readBuffer(7);
			stamp_version[2] = _readBuffer(8);
			stamp_version[3] = _readBuffer(9);
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