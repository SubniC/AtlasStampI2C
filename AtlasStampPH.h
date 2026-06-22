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

class AtlasStampPh : public AtlasStampTemperatureCompensated
{
public:
	explicit AtlasStampPh(uint8_t);

	bool const begin(void);

private:
	bool const _stamp_ready(void);
};

#endif

