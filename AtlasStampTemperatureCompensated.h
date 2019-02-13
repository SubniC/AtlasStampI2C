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
	//CONS/DES
	explicit AtlasStampTemperatureCompensated (uint8_t, char*, uint8_t, float, float, uint8_t = 1);
	
	//BASE
	char* const info(void);

	//Temperature API	
	bool const set_temperature(float);
	bool const set_temperature(float,float);
	float const get_temperature();
protected:
	bool const _get_temperature(void); //Actual read from stamp module
private:
	float _current_temperature; //Local cached temperature value
};

#endif


