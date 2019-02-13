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
	float read(void);
	bool readAsync(void);
	float resultAsync(void);
	bool begin(void);
private:
	bool _stampReady();
	float _parseResult(void);
};

#endif

