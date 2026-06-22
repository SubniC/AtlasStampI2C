// AtlasStampDO.h

#ifndef _ATLASSTAMPDO_h
#define _ATLASSTAMPDO_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "AtlasStampTemperatureCompensated.h"

#ifdef ATLAS_DEBUG
#define ATLAS_DEBUG_DO
#endif

#define ATLAS_SALINITY_UNIT_US 1
#define ATLAS_SALINITY_UNIT_PPT 2

// Datasheet: DO_EZO_Datasheet (atlas-scientific.com)
class AtlasStampDo : public AtlasStampTemperatureCompensated
{
public:
	explicit AtlasStampDo(byte);

	bool const begin(void);
	void info(Stream&);

	// Per the datasheet, pressure can be omitted for probes shallower than 10 m.
	bool const set_pressure(float,float);
	bool const set_pressure(float);
	float const get_pressure();

	// Per the datasheet, salinity is irrelevant when conductivity is below 2500 uS.
	bool const set_salinity(float, byte, float);
	bool const set_salinity(float, byte);
	float const get_salinity();

private:
	float _current_salinity;
	float _current_pressure;
	bool const _stamp_ready();

	bool const _load_pressure(void);
	bool const _load_salinity(void);

};


#endif

