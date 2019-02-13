#include "AtlasStamp.h"


AtlasStamp::AtlasStamp(byte address)
{
	_address = address;
	_is_init = false;
	_is_busy = false;
	_async_comand_ready_by = 0;
	_min_value = 0;
	_max_value = 0;
	_unit = "UNK";
	stamp_version[0] = '0';
	stamp_version[1] = '.';
	stamp_version[2] = '0';
	stamp_version[3] = '\0';
	_cleanBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <brief>
/// 	OJO ESTA FUNCION ESTA PENSADA PARA DEPURACION, NO PAR ASU USO GENERAL UTILIZAR LAS
/// 	FUNCIONES PROPIAS DE CADA CLASE HIJA PARA EL DESPLIEGUE DE LOS SENSORES!
/// </brief>
///
/// <remarks>	Mdps, 20/11/2017. </remarks>
///
/// <param name="cmd">	  	[in,out] If non-null, the command. </param>
/// <param name="timeout">	The timeout. </param>
///
/// <returns>	A byte. </returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
byte AtlasStamp::raw_command(char* cmd, unsigned long timeout)
{
	return _raw_command(cmd, timeout);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <brief>	Clean buffer </brief>
///
/// <remarks>	Mdps, 20/11/2017. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////

void AtlasStamp::_cleanBuffer()
{
	for (int i = 0; i < MAX_DATA_TO_READ; i++)
	{
		_buffer[i] = 0;
	}
	_i2c_bytes_received = 0;
}

/// <summary>
/// DEPRECATED
/// </summary>
void AtlasStamp::_cleanWire()
{
	while (Wire.available()) { Wire.read(); }
}

/// <summary>
/// Inicia un comando asincrono
/// </summary>
/// <param name="cmd">Comando a enviar</param>
/// <param name="t">Tiempo que requiere elk Atlas para procesarlo</param>
/// <returns>True en caso de exito, False en caso contrario</returns>
bool AtlasStamp::_command_async(char* cmd, unsigned long t)
{
	
	if (!_is_init)
	{
		return false;
	}
	else if (_is_busy)
	{
		return false;
	}

	Wire.beginTransmission(_address); 	                //call the circuit by its ID number.
	Wire.write(cmd);        			        //transmit the command that was sent through the serial port.
	byte wireres = Wire.endTransmission(true);          	                //end the I2C data transmission.

	if (wireres == 0)
	{
		//Estamos ocupados procesando esto, no se pueden ejecutar comandos
		_is_busy = true;
		//Calculamos el punto en el que se supone que tendremos una repuesta del sensor
		_async_comand_ready_by = millis() + t;

#ifdef ATLAS_DEBUG
		Serial1.printf("AtlasStamp::_command_async() [END] T:[%d] BUSY [%d] TIMEOUT [%d] RESPONSE [%d] COMMAND [%s]\n", millis(), _is_busy, _async_comand_ready_by, wireres, cmd);
#endif

		return true;
	}
	else
	{
		return false;
	}


}

//Obtiene el resultado de un comando asincrono
byte AtlasStamp::_command_result()
{
	byte tmp_char = 0;
	byte _i2c_response_code = 254;

	if (!_is_init)
	{
		return ATLAS_ERROR_RESPONSE;
	}
	else if (!_is_busy)
	{
		return ATLAS_NODATA_RESPONSE;
	}
	else if (!available())
	{
		return ATLAS_BUSY_RESPONSE;
	}

	_cleanBuffer();								//Limpiamos el buffer

	//Nos aseguramos de que esta listo antes de seguir :)
	while (_i2c_response_code == ATLAS_BUSY_RESPONSE) {
		Wire.requestFrom(_address, (byte)MAX_DATA_TO_READ);
		_i2c_response_code = Wire.read();   
		if (_i2c_response_code == ATLAS_BUSY_RESPONSE)
		{
			//Esto es necesario si no queremos que se cuelgue el i2c
			//Tenemos que limpiar los datos del buffer antes de volver a usar el bus
			_cleanWire();
		}
		delay(CONNECTION_DELAY_MS);
	}

	//OK, ha procesado el comando y la respuesta es correcta :)
	//vamos a recuperar los datos
	if (_i2c_response_code == ATLAS_SUCCESS_RESPONSE)
	{
		//Mientras tengamos datos cargamos el buffer
		while (Wire.available())
		{
			//Obtenemos el primer byte
			tmp_char = Wire.read();

			//Si el caracter es NULL es el final de la transmision
			if (tmp_char == NULL_CHARACTER)
			{
				//Añadimos el final de carro al buffer
				_buffer[_i2c_bytes_received] = '\0';
				_i2c_bytes_received++;
				//Terminamos
				break;
			}
			else
			{
				//Guardamos ese jugoso caracter en nuestro array
				_buffer[_i2c_bytes_received] = tmp_char;        //load this byte into our array.
				_i2c_bytes_received++;
				//TODO: Controlar aquí el buffer overflow!
			}
		}
	}
	_cleanWire();
	//Volvemos a poner los flags en su sitio
	_is_busy = false;
	_async_comand_ready_by = 0;

#ifdef ATLAS_DEBUG
	Serial1.printf("AtlasStamp::_command_result() [END] T:[%d] RESPONSE [%d] BUSY [%d] TIMEOUT [%d]\n", millis(), _i2c_response_code, _is_busy, _async_comand_ready_by);
#endif

	//Devolvemos el codigo de respuesta :)
	//Si es 1 tendremos _buffer cargado con la respuesta al comando
	return _i2c_response_code;
}


byte AtlasStamp::_command(char* cmd, unsigned long t)
{

#ifdef ATLAS_DEBUG
	Serial1.printf("AtlasStamp::_command() [START] T:[%d]\n", millis());
#endif
	//Si el STAMP no esta listo,
	//devolvemos NULO
	if (!_is_init)
	{
		return ATLAS_ERROR_RESPONSE;
	}
	else if (_is_busy)
	{
		return ATLAS_BUSY_RESPONSE;
	}

	return _raw_command(cmd, t);
}

byte AtlasStamp::_raw_command(char* cmd, unsigned long t)
{
	byte tmp_char = 0;
	byte _i2c_response_code = 254;
	
	_cleanBuffer();								//Limpiamos el buffer
	_cleanWire();

	Wire.beginTransmission(_address); 	                //call the circuit by its ID number.
	Wire.write(cmd);        			        //transmit the command that was sent through the serial port.
	byte responseStamp = Wire.endTransmission(true);          	                //end the I2C data transmission.

	//Failsafe para que nos e cuelgue
	if (responseStamp != 0)
	{
#ifdef ATLAS_DEBUG
		Serial1.printf("AtlasStamp::_raw_command() I2C response error: %d\n", responseStamp);
#endif
		return responseStamp;
	}

	//Esperamos la mitad del tiempo que nos piden :) 
	//Cuandoe se delay pasa empezamos a atosigar al STAMP cada 100ms
	delay(t); 

	while (_i2c_response_code == ATLAS_BUSY_RESPONSE) {
		//TODO: devuelve el numero de bytes leidos
		//podriamos verificar que tenemos alguno
		Wire.requestFrom((uint8_t)_address, (uint8_t)MAX_DATA_TO_READ); 	  //call the circuit and request 48 bytes (this is more then we need).		
		_i2c_response_code = Wire.read();      //the first byte is the response code, we read this separately.
		if (_i2c_response_code == ATLAS_BUSY_RESPONSE)
		{
			//Esto es necesario si no queremos que se cuelgue el i2c
			//Tenemos que limpiar los datos del buffer antes de volver a usar el bus
			_cleanWire();
		}
		delay(50);
	}
	//OK, ha procesado el comando y la respuesta es correcta :)
	//vamos a recuperar los datos
	if (_i2c_response_code == ATLAS_SUCCESS_RESPONSE)
	{
		//Mientras tengamos datos cargamos el buffer
		while (Wire.available())
		{
			//Obtenemos el primer byte
			tmp_char = Wire.read();
			//Si el caracter es NULL es el final de la transmision
			if (tmp_char == NULL_CHARACTER)
			{			
				//Terminamos la transmision
				//Añadimos el final de carro al buffer
				_buffer[_i2c_bytes_received] = '\0';
				_i2c_bytes_received++;
				//Terminamos
				break;
			}
			else 
			{
				//Guardamos ese jugoso caracter en nuestro array
				_buffer[_i2c_bytes_received] = tmp_char;        //load this byte into our array.
				_i2c_bytes_received++;
				//TODO: Controlar aquí el buffer overflow!
			}
		}
	}
	//Esto es necesario si no queremos que se cuelgue el i2c
	//Tenemos que limpiar los datos del buffer antes de volver a usar el bus
	_cleanWire();
#ifdef ATLAS_DEBUG
	Serial1.printf("AtlasStamp::_raw_command() [END] T[%d]  BYTESREC [%d] CODE [%d] RESPONSE [%s]\n", millis(), _i2c_bytes_received, _i2c_response_code, _buffer);
	Serial1.printf("AtlasStamp::_raw_command() [END] BUSY[%d] CMD[%s] BYTESREC[%d] CODE[%d] RESPONSE[%s]\n", busy(), cmd, _i2c_bytes_received, _i2c_response_code, _buffer);
#endif
	//Devolvemos el codigo de respuesta :)
	//Si es 1 tendremos _buffer cargado con la respuesta al comando
	return _i2c_response_code;
}

void AtlasStamp::raw_response(char* localBuffer)
{
	strcpy(localBuffer, _buffer);
	//_cleanBuffer();
}

char AtlasStamp::_readBuffer(byte pos)
{
	if (pos >= 0 && pos <= MAX_DATA_TO_READ)
	{
		return _buffer[pos];
	}
	return 0;
}

byte AtlasStamp::_bytes_in_buffer()
{
	return _i2c_bytes_received;
}

byte AtlasStamp::address()
{
	return _address;
}

bool AtlasStamp::ready()
{
	return _is_init;
}

bool AtlasStamp::busy()
{
	return _is_busy;
}

void AtlasStamp::_ready(bool isReady)
{
	_is_init = isReady;
}

bool AtlasStamp::available()
{
	//Si esta ocupado y ademas hemos hecho timeout
	//devolvemos true, tenemos comando disponible y es momento de obtener el resultado :)
	if (_is_busy && (_async_comand_ready_by < millis()))
	{
		return true;
	}
	return false;
}

char* AtlasStamp::_getBuffer()
{
	return _buffer;
}


bool AtlasStamp::_stampConnected()
{
	//Si ay estamos conectados decimos que si :)
	if (_is_init) { return true; }

	byte stampConnected = 255;
	byte info_response = 255;
	for (int i = 0; i < MAX_CONNECTION_TRIES; i++)
	{
		Wire.beginTransmission(_address);      // just do a short connection attempt without command to scan i2c for devices
		stampConnected = Wire.endTransmission(true);
#ifdef ATLAS_DEBUG
		Serial1.printf("AtlasStamp::_stampConnected() _addres:[0x%02x] result:[%d]\n", _address, stampConnected);
#endif // ATLAS_DEBUG

		//stampResponse=0 significa que tenemos device
		if (0 == stampConnected)
		{
			//Ahora sabemos que hay un dispositivo I2C en la direccion _address
			//Vamos a intentar ejecutar un comando INFO para ver si es compatible con
			//esta clase
			info_response = _raw_command(ATLAS_INFO_COMAND, 300);

#ifdef ATLAS_DEBUG
			Serial1.printf("AtlasStamp::_stampConnected() CMD[%s] _addres:[0x%02x] result2:[%d]\n", ATLAS_INFO_COMAND, _address, info_response);
			Serial1.printf("AtlasStamp::_stampConnected() _buffer:[%s]\n", _buffer);
#endif // ATLAS_DEBUG

			//Es un sensor EZO, devolvemos TRUE y seguimos :)
			if (_buffer[0] == '?' && _buffer[1] == 'I')
			{
				//Esto sigue teniendo el _buffer
				return true;
			}
		}
		delay(CONNECTION_DELAY_MS);
	}
	return false;
}

char* AtlasStamp::unit()
{
	return _unit;
}

void AtlasStamp::purge()
{
	_is_busy = false;
	_async_comand_ready_by = 0;
}

float AtlasStamp::_vcc(void)
{
	byte commandResult = _command("Status", 200);
	float returnVal = -2048.0f;
	char tmpItem[8] = { 0 };
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		//Una vez qui tenemos en el bufer algo asi (las | separan, no cuentan como caracter en el buffer)
		//?|S|T|A|T|U|S|,|P|,|5|.|0|6|4|NULL
		//TODO: http://stackoverflow.com/questions/32822988/get-the-last-token-of-a-string-in-c
		if (_readBuffer(9) == ',' && _bytes_in_buffer() > 10)
		{
			for (int i = 10; i < _bytes_in_buffer(); i++)
			{
				if (_readBuffer(i) != NULL_CHARACTER)
				{
					tmpItem[i - 10] = _readBuffer(i);
				}
				else
				{
					tmpItem[i - 10] = '\0';
				}
			}
			returnVal = atof(tmpItem);
		}
	}
	return returnVal;
}

float AtlasStamp::min_value()
{
	return _min_value;
}

float AtlasStamp::max_value()
{
	return _max_value;
}

void AtlasStamp::unit(char* unit)
{
	_unit = unit;
}

void AtlasStamp::min_value(float value)
{
	_min_value = value;
}

void AtlasStamp::max_value(float value)
{
	_max_value = value;
}

char* AtlasStamp::info()
{
	sprintf(_infoBuffer,"ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] VCC:[%4.4f]", _address, stamp_version, _is_init, _is_busy, _min_value, _max_value, _unit, _vcc());
	return _infoBuffer;
}


bool AtlasStamp::led()
{
	byte commandResult = _command("L,?", 150);
	bool tmpState = false;
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		//Una vez qui tenemos en el bufer algo asi (las | separan, no cuentan como caracter en el buffer)
		// ? | L | , | 1 | null
		// ? | L | , | 0 | null
		//Comporbamos que la posicion 3 del buffer sea un 0 o un 1
		if (_readBuffer(3) == '1')
		{
			tmpState = true;
		}
	}
	//TODO: Afinar codigos de respuesta :)
	//commandResult puede ser varias cosas
	return tmpState;
}

bool AtlasStamp::led(bool state)
{
	//El comando de fijar el led es:
	// L,1 para activar
	// ó 
	// L,0 para desactivar
	char tmpCmd[4] = "L,0";

	//Si queremos activarlo
	if (state)
	{
		tmpCmd[2] = '1';
	}

	//Lanzamos el coamndo y esperamos la confirmación
	byte commandResult = _command(tmpCmd, 300);
	if (ATLAS_SUCCESS_RESPONSE == commandResult)
	{
		return true;
	}
	return false;
}