// 
// 
// 

#include "AtlasStampEC.h"

AtlasStampEc::AtlasStampEc(byte address) :
	AtlasStampTemperatureCompensated(address, "uS/cm", 5, 0.07f, 500000.0f, 4),
	_current_k(-2048.0f)
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

void AtlasStampEc::info(Stream& output)
{
	output.printf("ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f] K:[%4.2f]\n", _address, stamp_version, ready(), busy(), get_min_value(), get_max_value(), get_unit(), get_temperature(), get_vcc(), get_k());
}

bool const AtlasStampEc::begin()
{
	//Inicialize sensor information
	if (_stamp_ready())
	{
		//Load module output parameters
		_load_parameters();

		//Load the module current temperature value
		_load_temperature();

		//Load the module current K
		_load_k();

		return true;
	}
	return false;
}


bool const AtlasStampEc::_stamp_ready()
{
	_is_init = false;
	if (_stamp_connected())
	{
		// EC EZO  -> '?I,EC,1.0 '
		//Is the type of sensor that is supposed to be?
		if (_read_buffer(3) == 'E' && _read_buffer(4) == 'C')
		{
			stamp_version[0] = _read_buffer(6);
			stamp_version[1] = _read_buffer(7);
			stamp_version[2] = _read_buffer(8);
			stamp_version[3] = _read_buffer(9);
			stamp_version[4] = 0;
			_is_init = true;
		}
	}
	return _is_init;
}

float AtlasStampEc::get_k()
{
	return _current_k;
}

bool AtlasStampEc::_load_k()
{
	if (ATLAS_SUCCESS_RESPONSE == _command("K,?", 300))
	{
		//En el buffer tendremos:
		// ? | K | , | 0 | . | 6 | 6 | null
		char* res_buff = (char*)(_get_response_buffer() + 3);
		_current_k = atof(res_buff);
		return true;
	}
	return false;
}

bool AtlasStampEc::set_k(float value)
{
	if (_current_k == value)
	{
#ifdef ATLAS_DEBUG_EC
		Serial.println("EC: cant set K already same value [%4.2f]\n", _current_k);
#endif
		return true;
	}

	sprintf(_command_buffer, "K,%4.2f", value);
	if (ATLAS_SUCCESS_RESPONSE == _command(_command_buffer, 300))
	{
		_current_k = value;
		return true;
	}
	return false;
}