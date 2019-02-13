#include "AtlasStamp.h"


AtlasStamp::AtlasStamp(uint8_t address, char* unit, uint8_t unit_len, float min_value, float max_value, uint8_t num_fields_in_response) : 
	_address(address),
	_response_field_count(num_fields_in_response),
	_last_result{ (float*)malloc(sizeof(float) * _response_field_count) },
	_is_init(false),
	_is_busy(false),
	_async_comand_ready_by(0),
	_min_value(min_value),
	_max_value(max_value),
	_unit{ (char*)malloc(sizeof(char) * unit_len) },
	stamp_version{ '0','.','0','\0' }
{
	//Inicialize the sensor unit
	memcpy(_unit, unit, unit_len);
	_clean_buffer();
}

uint8_t AtlasStamp::raw_command(char* cmd, unsigned long timeout)
{
	return _raw_command(cmd, timeout);
}

void AtlasStamp::_clean_buffer()
{
	for (int i = 0; i < MAX_DATA_TO_READ; i++)
	{
		_response_buffer[i] = 0;
	}
	_i2c_bytes_received = 0;
}

/// <summary>
/// DEPRECATED
/// </summary>
void AtlasStamp::_clean_wire()
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
		Serial.printf("AtlasStamp::_command_async() [END] T:[%d] BUSY [%d] TIMEOUT [%d] RESPONSE [%d] COMMAND [%s]\n", millis(), _is_busy, _async_comand_ready_by, wireres, cmd);
#endif

		return true;
	}
	else
	{
		return false;
	}


}

//Obtiene el resultado de un comando asincrono
uint8_t AtlasStamp::_command_result()
{
	uint8_t tmp_char = 0;
	uint8_t _i2c_response_code = 254;

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

	_clean_buffer();								//Limpiamos el buffer

	//Nos aseguramos de que esta listo antes de seguir :)
	while (_i2c_response_code == ATLAS_BUSY_RESPONSE) {
		Wire.requestFrom(_address, (uint8_t)MAX_DATA_TO_READ);
		_i2c_response_code = Wire.read();   
		if (_i2c_response_code == ATLAS_BUSY_RESPONSE)
		{
			//Esto es necesario si no queremos que se cuelgue el i2c
			//Tenemos que limpiar los datos del buffer antes de volver a usar el bus
			_clean_wire();
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
				_response_buffer[_i2c_bytes_received] = '\0';
				_i2c_bytes_received++;
				//Terminamos
				break;
			}
			else
			{
				//Guardamos ese jugoso caracter en nuestro array
				_response_buffer[_i2c_bytes_received] = tmp_char;        //load this byte into our array.
				_i2c_bytes_received++;
				//TODO: Controlar aquí el buffer overflow!
			}
		}
	}
	_clean_wire();
	//Volvemos a poner los flags en su sitio
	_is_busy = false;
	_async_comand_ready_by = 0;

#ifdef ATLAS_DEBUG
	Serial.printf("AtlasStamp::_command_result() [END] T:[%d] RESPONSE [%d] BUSY [%d] TIMEOUT [%d]\n", millis(), _i2c_response_code, _is_busy, _async_comand_ready_by);
#endif

	//Devolvemos el codigo de respuesta :)
	//Si es 1 tendremos _response_buffer cargado con la respuesta al comando
	return _i2c_response_code;
}


uint8_t AtlasStamp::_command(char* cmd, unsigned long t)
{

#ifdef ATLAS_DEBUG
	Serial.printf("AtlasStamp::_command() [START] T:[%d]\n", millis());
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

uint8_t AtlasStamp::_raw_command(char* cmd, unsigned long t)
{
	uint8_t tmp_char = 0;
	uint8_t _i2c_response_code = 254;
	
	_clean_buffer();								//Limpiamos el buffer
	_clean_wire();

	Wire.beginTransmission(_address); 	                //call the circuit by its ID number.
	Wire.write(cmd);        			        //transmit the command that was sent through the serial port.
	uint8_t responseStamp = Wire.endTransmission(true);          	                //end the I2C data transmission.

	//Failsafe para que nos e cuelgue
	if (responseStamp != 0)
	{
#ifdef ATLAS_DEBUG
		Serial.printf("AtlasStamp::_raw_command() I2C response error: %d\n", responseStamp);
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
			_clean_wire();
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
				_response_buffer[_i2c_bytes_received] = '\0';
				_i2c_bytes_received++;
				//Terminamos
				break;
			}
			else 
			{
				//Guardamos ese jugoso caracter en nuestro array
				_response_buffer[_i2c_bytes_received] = tmp_char;        //load this byte into our array.
				_i2c_bytes_received++;
				//TODO: Controlar aquí el buffer overflow!
			}
		}
	}
	//Esto es necesario si no queremos que se cuelgue el i2c
	//Tenemos que limpiar los datos del buffer antes de volver a usar el bus
	_clean_wire();
#ifdef ATLAS_DEBUG
	Serial.printf("AtlasStamp::_raw_command() [END] T[%d]  BYTESREC [%d] CODE [%d] RESPONSE [%s]\n", millis(), _i2c_bytes_received, _i2c_response_code, _response_buffer);
	Serial.printf("AtlasStamp::_raw_command() [END] BUSY[%d] CMD[%s] BYTESREC[%d] CODE[%d] RESPONSE[%s]\n", busy(), cmd, _i2c_bytes_received, _i2c_response_code, _response_buffer);
#endif
	//Devolvemos el codigo de respuesta :)
	//Si es 1 tendremos _response_buffer cargado con la respuesta al comando
	return _i2c_response_code;
}

void AtlasStamp::raw_response(char* localBuffer)
{
	strcpy(localBuffer, _response_buffer);
}

char AtlasStamp::_read_buffer(byte pos)
{
	if (pos >= 0 && pos <= MAX_DATA_TO_READ)
	{
		return _response_buffer[pos];
	}
	return 0;
}

uint8_t AtlasStamp::_bytes_in_buffer()
{
	return _i2c_bytes_received;
}

uint8_t AtlasStamp::address()
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

float* const AtlasStamp::read()
{
	if (ATLAS_SUCCESS_RESPONSE == _command(ATLAS_READ_COMAND, 1000))
	{
		return _parse_sensor_read();
	}
	return nullptr;
}

bool const AtlasStamp::read_async()
{
	return _command_async(ATLAS_READ_COMAND, 1000);
}

float* const AtlasStamp::result_async()
{
	if (ATLAS_SUCCESS_RESPONSE == _command_result())
	{
		return _parse_sensor_read();
	}
	return nullptr;
}

float* const AtlasStamp::_parse_sensor_read(void)
{
	//Cuando llamamos a esta fucnion deberiamos tener en el _response_buffer una cadena 
	//representando la medida del sensor, esta dependera, pudiendo ser, un float (58.7)
	// o una lista de floats separada por comas (12.5,22.5,1.0,00.2)
#ifdef ATLAS_DEBUG
	Serial.printf("AtlasStamp::_parse_sensor_read() T[%lu] sensor fields [%d] response buffer [%s]\n", millis(), _response_field_count, _response_buffer);
#endif

	if (_response_field_count > 1)
	{
		char *current_token;
		current_token = strtok(_response_buffer,",");
		for (int i = 0; i < _response_field_count; i++)
		{
			if (current_token != NULL)
			{
				*(_last_result+i) = atof(current_token);
				//Get next value if previous was not null
				current_token = strtok(NULL, ",");
			}
			else
			{
				//The sensor is suposed to have multiple values, but
				//thats not true so set the value to default error
				*(_last_result + i) = -2048.0;
			}
#ifdef ATLAS_DEBUG
			Serial.printf("Field[%d] value[%4.2f] current_token[%s]\n", i+1, *(_last_result + i), current_token);
#endif
		}
	}
	else
	{
		//Is a simple sensor only a float string in the buffer, just set it.
		*_last_result = atof(_response_buffer);
#ifdef ATLAS_DEBUG
		Serial.printf("Field[%d] value[%4.2f]\n", 1, *_last_result);
#endif
	}
	return _last_result;
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

char* AtlasStamp::_get_response_buffer()
{
	return _response_buffer;
}


bool AtlasStamp::_stamp_connected()
{
	//If we are already inicialized return true
	if (_is_init) { return true; }

	uint8_t stampConnected = 255;
	uint8_t info_response = 255;
	for (int i = 0; i < MAX_CONNECTION_TRIES; i++)
	{
		Wire.beginTransmission(_address);      // just do a short connection attempt without command to scan i2c for devices
		stampConnected = Wire.endTransmission(true);
#ifdef ATLAS_DEBUG
		Serial.printf("AtlasStamp::_stampConnected() _addres:[0x%02x] result:[%d]\n", _address, stampConnected);
#endif // ATLAS_DEBUG

		//stampResponse=0 significa que tenemos device
		if (0 == stampConnected)
		{
			//Ahora sabemos que hay un dispositivo I2C en la direccion _address
			//Vamos a intentar ejecutar un comando INFO para ver si es compatible con
			//esta clase
			info_response = _raw_command(ATLAS_INFO_COMAND, 300);

#ifdef ATLAS_DEBUG
			Serial.printf("AtlasStamp::_stampConnected() CMD[%s] _addres:[0x%02x] result2:[%d]\n", ATLAS_INFO_COMAND, _address, info_response);
			Serial.printf("AtlasStamp::_stampConnected() _buffer:[%s]\n", _response_buffer);
#endif // ATLAS_DEBUG

			//Es un sensor EZO, devolvemos TRUE y seguimos :)
			if (_response_buffer[0] == '?' && _response_buffer[1] == 'I')
			{
				//Esto sigue teniendo el _response_buffer
				return true;
			}
		}
		delay(CONNECTION_DELAY_MS);
	}
	return false;
}

char* AtlasStamp::get_unit()
{
	return _unit;
}

void AtlasStamp::purge()
{
	_is_busy = false;
	_async_comand_ready_by = 0;
}

float AtlasStamp::get_vcc(void)
{
	float returnVal = -2048.0f;
	char tmpItem[8] = { 0 };
	if (ATLAS_SUCCESS_RESPONSE == _command("Status", 200))
	{
		//Una vez qui tenemos en el bufer algo asi (las | separan, no cuentan como caracter en el buffer)
		//?|S|T|A|T|U|S|,|P|,|5|.|0|6|4|NULL
		//TODO: http://stackoverflow.com/questions/32822988/get-the-last-token-of-a-string-in-c
		if (_read_buffer(9) == ',' && _i2c_bytes_received > 10)
		{
			for (int i = 10; i < _i2c_bytes_received; i++)
			{
				if (_read_buffer(i) != NULL_CHARACTER)
				{
					tmpItem[i - 10] = _read_buffer(i);
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

float AtlasStamp::get_min_value()
{
	return _min_value;
}

float AtlasStamp::get_max_value()
{
	return _max_value;
}

//void AtlasStamp::unit(char* unit)
//{
//	_unit = unit;
//}
//
//void AtlasStamp::min_value(float value)
//{
//	_min_value = value;
//}
//
//void AtlasStamp::max_value(float value)
//{
//	_max_value = value;
//}

char* const AtlasStamp::info()
{
	sprintf(_infoBuffer,"ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] VCC:[%4.4f]", _address, stamp_version, _is_init, _is_busy, _min_value, _max_value, _unit, get_vcc());
	return _infoBuffer;
}


bool AtlasStamp::led()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("L,?", 150))
	{
		//Una vez qui tenemos en el bufer algo asi (las | separan, no cuentan como caracter en el buffer)
		// ? | L | , | 1 | null
		// ? | L | , | 0 | null
		//Comporbamos que la posicion 3 del buffer sea un 0 o un 1
		if (_read_buffer(3) == '1')
		{
			return true;
		}
	}
	return false;
}

bool AtlasStamp::led(bool state)
{
	//El comando de fijar el led es:
	// L,1 para activar
	// ó 
	// L,0 para desactivar
	sprintf(_command_buffer,"L,%d", state);
	//Send the command and wait for confirmation
	if (ATLAS_SUCCESS_RESPONSE == _command(_command_buffer, 300))
	{
		return true;
	}
	return false;
}