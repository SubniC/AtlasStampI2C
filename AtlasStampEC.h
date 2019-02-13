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

//#define ATLAS_DEBUG_EC


#define ATLAS_EC_RESPONSE_LENGTH 4

class AtlasStampEc : public AtlasStampTemperatureCompensated
{
public:
	explicit AtlasStampEc(byte);
	float* read(void);
	bool readAsync(void);
	float* resultAsync(void);
	bool begin(void);
	char* info(void);
	//TODO: Guardar localmente la ultima K recuperada/fijada
	//que no tengamos que preguntarle al stamp cada vez
	bool set_k(float);
	float get_k(void);

	//Obtiene la ultima medida que se tomo
	//con cada llamada a read o resultAsync se actualiza.
	//TODO: para virtualizar en AtlasStamp
	float* last(void);

private:
	//TODO: si funciona bien esto de poner el _parsedResult en la clase podriamos tener
	//todas als funciones con float* de retorno y asi mover _parsedResult a AtlasStamp y
	//hacer virtuales read, readasync, resultAsync... ya que tendriamos la misma respuesta en todas
	//las funciones, eso si, necesitariamos que cada clase hija de AtlasStamp permitiera saber
	//cuantos elementos tiene en su respuesta
	//TODO2: Lo hemos probado y funciona estupendamente, tenemos que implementar este modelo en las demas clases
	//y virtualizar los metodos de lectura
	float _parsedResult[ATLAS_EC_RESPONSE_LENGTH];
	bool _stampReady();
	void _parseResult();
};


#endif

