// AtlasStampORP.h

#ifndef _ATLASSTAMPORP_h
#define _ATLASSTAMPORP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "AtlasStamp.h"

#ifdef ATLAS_DEBUG
#define ATLAS_DEBUG_ORP
#endif

class AtlasStampOrp : public AtlasStamp
{
public:
	explicit AtlasStampOrp(byte);

	bool const begin(void);


private:
	bool const _stamp_ready();
};

#endif

