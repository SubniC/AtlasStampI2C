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

class AtlasStampDo : public AtlasStampTemperatureCompensated
{
public:
	explicit AtlasStampDo(byte);
	float read(void);
	bool readAsync(void);
	float resultAsync(void);
	bool begin(void);
	char* info(void);
	//Segun la hoja de caraceristicas la presion se puede omitir
	//si la sonda va a estar a menos de 10m de profundidad
	//pagina 50
	bool set_pressure(float,float);
	bool set_pressure(float);
	float get_pressure(bool=false);

	//Segun la hoja de caraceristicas la salinidas es irrelevante
	//si la conductividad es menor de 2500uS
	//Pagina 49
	bool set_salinity(float, byte, float);
	bool set_salinity(float, byte);
	float get_salinity(bool=false);

private:
	float _current_salinity;
	float _current_pressure;
	bool _stampReady();
	float _parseResult(void);

	float _get_pressure(void);
	float _get_salinity(void);

};


#endif

