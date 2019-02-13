// AtlasStampTemperatureCompensated.h

#ifndef _ATLASSTAMPTEMPERATURECOMPENSATED_H
#define _ATLASSTAMPTEMPERATURECOMPENSATED_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "AtlasStamp.h"

#define ATLAS_TEMPERATURE_COMAND "T,"
#define ATLAS_TEMPERATURE_READ_COMAND "T,?"


class AtlasStampTemperatureCompensated : public AtlasStamp
{
public:
	explicit AtlasStampTemperatureCompensated (byte);
	char* info(void);
	//TODO: Guardar localmente la ultima temperatura recuperada/fijada
	//que no tengamos que preguntarle al stamp cada vez
	bool set_temperature(float);
	bool set_temperature(float,float);
	float get_temperature(bool=false);
protected:
	float _get_temperature(void);
	float _current_temperature;
};

#endif


