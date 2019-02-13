// 
// 
// 

#include "AtlasStampPH.h"

AtlasStampPh::AtlasStampPh(byte address) : AtlasStampTemperatureCompensated(address)
{
	min_value(0.001f);
	max_value(14.000f);
	unit("PH");
}

bool AtlasStampPh::begin()
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

float AtlasStampPh::read()
{
	byte commandResult = _command(ATLAS_READ_COMAND, 1000);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		return _parseResult();
	}
	//TODO: Afinar codigos de respuesta :)
	return -2048.0f;
}

bool AtlasStampPh::readAsync()
{
	return _command_async(ATLAS_READ_COMAND, 1000);
}

float AtlasStampPh::resultAsync()
{
	byte commandResult = _command_result();
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		return _parseResult();
	}
	//TODO: Afinar codigos de respuesta :)
	return -2048.0f;
}

float AtlasStampPh::_parseResult()
{
	//Tenemos los datos en el buffer :)
	//Teoricamente el comando READ debe responder con 
	//una trama del tipo 1,PH,NULL, 1 es el estado, que no
	//se almacena en el buffer y null es el final de comando
	//que tampoco se guarda, asi que ahora el buffer deberia tener un float en string
	//asi que hacemos
	return atof(_getBuffer());
}


bool AtlasStampPh::_stampReady()
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