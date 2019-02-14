#include "AtlasStampTemperatureCompensated.h"

AtlasStampTemperatureCompensated::AtlasStampTemperatureCompensated(uint8_t address, char* unit, uint8_t unit_len, float min_value, float max_value, uint8_t max_num_fields_in_response) :
	AtlasStamp(address, unit, unit_len, min_value, max_value, max_num_fields_in_response), _current_temperature(-2048.0)
{

}


void AtlasStampTemperatureCompensated::info(Stream& output)
{
	output.printf("ADDRESS:[0x%02x] VERSION:[%s] READY:[%d] BUSY:[%d] MIN:[%4.3f] MAX:[%4.3f] UNIT:[%s] TMP:[%4.2f] VCC:[%4.4f]\n",_address, stamp_version, ready(), busy(), get_min_value(), get_max_value(), get_unit(), _current_temperature, get_vcc());
}

float const AtlasStampTemperatureCompensated::get_temperature()
{
	return _current_temperature;
}

bool const AtlasStampTemperatureCompensated::_load_temperature()
{	
	if (ATLAS_SUCCESS_RESPONSE == _command(ATLAS_TEMPERATURE_READ_COMAND, 300))
	{
		//En el buffer tendremos:
		// ? | T | , | 1 | 9 | . | 5 | null
		if (_bytes_in_buffer() >= 5)
		{
			char* res_buff = (char*)(_get_response_buffer() + 3);
			_current_temperature = atof(res_buff);

#ifdef ATLAS_DEBUG
			Serial.printf("AtlasStampTemperatureCompensated::temperature buffer [%s] current float [%4.2f]\n", res_buff, _current_temperature);
#endif
			return true;
		}
	}
	return false;
}


bool const  AtlasStampTemperatureCompensated::set_temperature(float temp, float max_divergence)
{
	//Serial.printf("temp[%4.2f] _current_temperature[%4.2f] max_divergence[%4.2f]\n", temp, _current_temperature, max_divergence);
	if ((!busy()) && (abs(temp - _current_temperature) >= max_divergence))
	{
		return set_temperature(temp);
	}
	return false;
}

bool const AtlasStampTemperatureCompensated::set_temperature(float temp)
{
	sprintf(_command_buffer, "T,%4.2f", temp);	
	if (ATLAS_SUCCESS_RESPONSE == _command(_command_buffer, 300))
	{
		//TODO: Query the temperature to validate the change?
		_current_temperature = temp;
		return true;
	}
	return false;
}