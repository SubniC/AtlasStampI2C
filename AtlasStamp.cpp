#include "AtlasStamp.h"

AtlasStamp::AtlasStamp(uint8_t address, char* unit, uint8_t unit_len, float min_value, float max_value, uint8_t max_num_fields_in_response) :
	_address(address),
	_max_response_field_count(max_num_fields_in_response),
	_response_field_count(max_num_fields_in_response),
	_last_result{ (float*)malloc(sizeof(float) * _response_field_count) },
	_is_init(false),
	_is_busy(false),
	_async_comand_ready_by(0),
	_min_value(min_value),
	_max_value(max_value),
	_unit{ (char*)malloc(sizeof(char) * (unit_len+1)) },
	stamp_version{ '0','.','0','\0' },
	is_awake(true)
{
	// Initialize the sensor unit.
	strcpy(_unit, unit);
	_clean_buffer();
}

void AtlasStamp::_resize_response_count(uint8_t count)
{
	if (count == 0) { count = 1; }
	else if (count > _max_response_field_count) { count = _max_response_field_count; }

	if (count == _response_field_count)
	{
#ifdef ATLAS_DEBUG
		Serial.printf("AtlasStamp::_resize_response_count() cant reallocate, already have [%d] fields\n", _response_field_count);
#endif
		return;
	}

#ifdef ATLAS_DEBUG
	Serial.printf("AtlasStamp::_resize_response_count() reallocated space for sensor readings from[%d] to[%d] floats\n", _response_field_count, count);
#endif
	_response_field_count = count;
	_last_result = (float*)realloc(_last_result, sizeof(float) * _response_field_count);
}

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

	Wire.beginTransmission(_address); 	         
	Wire.write(cmd);        			        
	byte wireres = Wire.endTransmission(true);  

	if (wireres == I2C_RESPONSE_OK)
	{
		// Command sent ok, no more communication can happen until the result is read.
		_is_busy = true;
		// Estimated time at which the result should be available.
		_async_comand_ready_by = millis() + t;

		// Reset the buffer and read counter.
		_clean_buffer();

#ifdef ATLAS_DEBUG
		Serial.printf("AtlasStamp::_command_async() [END] T[%d] BUSY[%d] READY_BY[%d] SUCCSED[%d] COMMAND[%s]\n", millis(), _is_busy, _async_comand_ready_by, wireres, cmd);
#endif
		// Any processed command wakes the module, so flag it awake. After a Sleep
		// command this is set true too, but sleep() resets it on return.
		is_awake = true;
		return true;
	}
	else
	{
		return false;
	}
}

// Retrieves the result of an asynchronous command.
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

	// Make sure the module is ready before continuing.
	while (_i2c_response_code == ATLAS_BUSY_RESPONSE) {
		Wire.requestFrom(_address, (uint8_t)MAX_DATA_TO_READ);
		_i2c_response_code = Wire.read();
		if (_i2c_response_code == ATLAS_BUSY_RESPONSE)
		{
			// Drain the buffer before reusing the bus, otherwise I2C may hang.
			_clean_wire();
		}
		delay(CONNECTION_DELAY_MS);
	}

	// Command processed successfully: read the data into the buffer.
	if (_i2c_response_code == ATLAS_SUCCESS_RESPONSE)
	{
		while (Wire.available())
		{
			tmp_char = Wire.read();

			// A NULL character marks the end of the transmission.
			if (tmp_char == NULL_CHARACTER)
			{
				_response_buffer[_i2c_bytes_received] = '\0';
				break;
			}
			else
			{
				_response_buffer[_i2c_bytes_received] = tmp_char;
				_i2c_bytes_received++;
			}
		}
	}
	_clean_wire();
	// Reset the flags.
	_is_busy = false;
	_async_comand_ready_by = 0;

#ifdef ATLAS_DEBUG
	Serial.printf("AtlasStamp::_command_result() [END] T[%d] BUSY[%d] CODE[%d] RESPONSE[%s] TIMEOUT[%d]\n", millis(), _is_busy, _i2c_response_code, _response_buffer, _async_comand_ready_by);
#endif

	// Return the response code. On ATLAS_SUCCESS_RESPONSE the buffer holds the answer.
	return _i2c_response_code;
}


uint8_t AtlasStamp::_command(char* cmd, unsigned long t)
{
#ifdef ATLAS_DEBUG
	Serial.printf("AtlasStamp::_command() [START] CMD[%s] T:[%d]\n", cmd, millis());
#endif

	if (_command_async(cmd, t))
	{
		uint8_t r = 0;
		delay(static_cast<unsigned long>(t/2.0f));
		while (ATLAS_BUSY_RESPONSE == (r = _command_result())) { delay(50); }

		if (ATLAS_SUCCESS_RESPONSE == r)
		{
			return true;
		}
	}
	return false;
}

uint8_t AtlasStamp::read_ascii(char* buffer)
{
	if (ATLAS_SUCCESS_RESPONSE == _command(ATLAS_READ_COMAND, 1000))
	{
		strcpy(buffer, _response_buffer);
		return _i2c_bytes_received;
	}
	return 0;
	
}

uint8_t AtlasStamp::result_ascii_async(char* buffer)
{
	if (ATLAS_SUCCESS_RESPONSE == _command_result())
	{
		strcpy(buffer, _response_buffer);
		return _i2c_bytes_received;
	}
	return 0;

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
	// _response_buffer is expected to hold the sensor reading: either a single float
	// (e.g. 58.7) or a comma-separated list of floats (e.g. 12.5,22.5,1.0,00.2).
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
				current_token = strtok(NULL, ",");
			}
			else
			{
				// Expected another value but none was present: use the error default.
				*(_last_result + i) = -2048.0f;
			}
#ifdef ATLAS_DEBUG
			Serial.printf("Field[%d] value[%4.2f] current_token[%s]\n", i+1, *(_last_result + i), current_token);
#endif
		}
	}
	else
	{
		if (strcmp("No output", _response_buffer) == 0)
		{
			*_last_result = -2048.0f;
		}
		else
		{
			// Single-value sensor: the buffer holds just one float string.
			*_last_result = atof(_response_buffer);
		}

#ifdef ATLAS_DEBUG
		Serial.printf("Field[%d] value[%4.2f]\n", 1, *_last_result);
#endif
	}
	return _last_result;
}

bool AtlasStamp::_stamp_connected()
{
	bool r = false;
	// Already initialized.
	if (_is_init) { return true; }

	// Temporarily set the flag so _command() can return ATLAS_SUCCESS_RESPONSE
	// during the initial contact.
	_is_init = true;

	for (int i = 0; i < MAX_CONNECTION_TRIES; i++)
	{
		// Try to get the device status.
		if (ATLAS_SUCCESS_RESPONSE == _command(ATLAS_INFO_COMAND,300))
		{
			// EZO modules reply with "?I...".
			if (_response_buffer[0] == '?' && _response_buffer[1] == 'I')
			{
				r = true;
				break;
			}
		}
		delay(CONNECTION_DELAY_MS);
	}
	// Clear the flag; the child class sets it once the module info is parsed.
	_is_init = false;
	// r is true if we reached an EZO module at the given address.
	return r;
}

void AtlasStamp::purge()
{
	_is_busy = false;
	_async_comand_ready_by = 0;
}

float AtlasStamp::get_vcc(void)
{
	if (ATLAS_SUCCESS_RESPONSE == _command("Status", 200))
	{
		// Response layout: ?|S|T|A|T|U|S|,|P|,|5|.|0|6|4|NULL
		if (_bytes_in_buffer() >= 13)
		{
			char* res_buff = (char*)(_get_response_buffer() + 10);
			return atof(res_buff);
		}
	}
	return -2048.0f;
}


void AtlasStamp::info(Stream& output)
{
	output.printf("ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] VCC:[%4.4f]", _address, stamp_version, _is_init, _is_busy, _min_value, _max_value, _unit, get_vcc());
}


bool AtlasStamp::led()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("L,?", 150))
	{
		// Buffer holds "?L,1" or "?L,0"; position 3 is the state.
		if (_read_buffer(3) == '1')
		{
			return true;
		}
	}
	return false;
}

bool AtlasStamp::led(bool state)
{
	// "L,1" turns the LED on, "L,0" turns it off.
	sprintf(_command_buffer,"L,%d", state);
	if (ATLAS_SUCCESS_RESPONSE == _command(_command_buffer, 300))
	{
		return true;
	}
	return false;
}

bool const AtlasStamp::sleep(void)
{
	if (ATLAS_SUCCESS_RESPONSE == _command("Sleep", 300))
	{
		is_awake = false;
		return true;
	}
	return false;
}

// Not strictly necessary (any command wakes a sleeping module), but kept to keep
// the API clean: sleep() / sleeping() / wakeup().
bool const AtlasStamp::wakeup(void)
{
	if (is_awake)
	{
		return true;
	}
	return led();
}