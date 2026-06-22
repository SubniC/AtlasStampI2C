// AtlasStampEC.h

#ifndef _ATLASSTAMPEC_h
#define _ATLASSTAMPEC_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "AtlasStampTemperatureCompensated.h"

#ifdef ATLAS_DEBUG
#define ATLAS_DEBUG_EC
#endif

// Datasheet: ec_EZO_datasheet (atlas-scientific.com)
class AtlasStampEc : public AtlasStampTemperatureCompensated
{
public:
	enum Parameters{
		EC  = 1 << 0,
		TDS = 1 << 1,
		S   = 1 << 2,
		SG  = 1 << 3,
	};

	explicit AtlasStampEc(byte);

	bool const begin(void);
	void info(Stream&);
	bool set_k(float);
	float get_k(void);
	
	bool const set_output_parameter(Parameters, bool);
	bool const get_output_parameter(Parameters);


private:
	uint8_t _parameter_state = 0;

	float _current_k;

	bool _load_k(void);

	bool const _load_parameters();

	bool const _stamp_ready();

	inline char* parameter_to_char(Parameters p) __attribute__((always_inline))
	{
		switch(p)
		{
			case Parameters::EC:
				return "EC";
				break;

			case Parameters::TDS:
				return "TDS";
				break;

			case Parameters::S:
				return "S";
				break;

			case Parameters::SG:
				return "SG";
				break;

			default:
				return nullptr;
		}

	}
};


#endif

