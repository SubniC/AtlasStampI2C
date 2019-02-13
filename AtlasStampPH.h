// AtlasStampPH.h

#ifndef _ATLASSTAMPPH_h
#define _ATLASSTAMPPH_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "AtlasStampTemperatureCompensated.h"

#ifdef ATLAS_DEBUG
	#define ATLAS_DEBUG_PH
#endif

//TODO: Falta emtodo info()
class AtlasStampPh : public AtlasStampTemperatureCompensated
{
public:
	//CONS/DES
	explicit AtlasStampPh(uint8_t);
	
	//BASE
	bool const begin(void);

	//PH
	float* slope(void);


private:
	bool const _stamp_ready(void);
};

#endif

