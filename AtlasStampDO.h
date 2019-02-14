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

//TODO: Actualizar metodos antiguos


//https://www.atlas-scientific.com/_files/_datasheets/_circuit/DO_EZO_Datasheet_bw.pdf
class AtlasStampDo : public AtlasStampTemperatureCompensated
{
public:
	explicit AtlasStampDo(byte);
	
	//VIRTUAL BASE
	bool const begin(void);
	void info(Stream&);

	//Segun la hoja de caraceristicas la presion se puede omitir
	//si la sonda va a estar a menos de 10m de profundidad
	//pagina 50
	bool const set_pressure(float,float);
	bool const set_pressure(float);
	float const get_pressure();

	//Segun la hoja de caraceristicas la salinida es irrelevante
	//si la conductividad es menor de 2500uS
	//Pagina 49
	bool const set_salinity(float, byte, float);
	bool const set_salinity(float, byte);
	float const get_salinity();

private:
	float _current_salinity;
	float _current_pressure;
	bool const _stamp_ready();

	bool const _get_pressure(void);
	bool const _get_salinity(void);

};


#endif

