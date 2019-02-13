// 
// 
// 

#include "AtlasStampORP.h"

AtlasStampOrp::AtlasStampOrp(byte address) : AtlasStamp(address)
{
	min_value(-1019.9f);
	max_value(1019.9f);
	unit("mV");
}

bool AtlasStampOrp::begin()
{
	//Inicialzamos el sensor
	return  _stampReady();
}

float AtlasStampOrp::read()
{
	byte commandResult = _command(ATLAS_READ_COMAND, 1000);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		return _parseResult();
	}
	//TODO: Afinar codigos de respuesta :)
	return -2048.0f;
}

//TODO: se podria mover a la clase base
bool AtlasStampOrp::readAsync()
{
	return _command_async(ATLAS_READ_COMAND, 1000);
}

float AtlasStampOrp::resultAsync()
{
	byte commandResult = _command_result();
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		return _parseResult();
	}
	//TODO: Afinar codigos de respuesta :)
	return -2048.0f;
}

float AtlasStampOrp::_parseResult()
{
	//Tenemos los datos en el buffer :)
	//Teoricamente el comando READ debe responder con 
	//una trama del tipo 1,ORP,NULL, 1 es el estado, que no
	//se almacena en el buffer y null es el final de comando
	//que tampoco se guarda, asi que ahora el buffer deberia tener un float en string
	//asi que hacemos
	return atof(_getBuffer());
}


bool AtlasStampOrp::_stampReady()
{
	bool isReady = false;
	//El padre controla que este coenctado un dispositivo en la direccion
	//y que sea un EZO, ya de paso carga _buffer con los datos del comando
	//INFO asi que sacamos y asignamos la version del sensor :)
	if (_stampConnected())
	{
		// ORP EZO -> '?I,OR,1.0'   (-> wrong in documentation 'OR' instead of 'ORP')
		//Comprobamos si es nuestro tipo de sensor :)
		if (_readBuffer(3) == 'O' && _readBuffer(4) == 'R')
		{
			//No sabemos si llega OR o ORP, de ehcho a mi me llega ORP con el mio
			//asi que nos curamos en salud
			if (_readBuffer(5) == 'P')
			{
				stamp_version[0] = _readBuffer(7);
				stamp_version[1] = _readBuffer(8);
				stamp_version[2] = _readBuffer(9);
				stamp_version[3] = _readBuffer(10);
				stamp_version[4] = '\0';
			}
			else
			{
				stamp_version[0] = _readBuffer(6);
				stamp_version[1] = _readBuffer(7);
				stamp_version[2] = _readBuffer(8);
				stamp_version[3] = _readBuffer(9);
				stamp_version[4] = '\0';
			}
			isReady = true;
		}
	}
	_ready(isReady);
	return isReady;
}