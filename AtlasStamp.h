// AtlasStamp.h

#ifndef _ATLASSTAMP_h
#define _ATLASSTAMP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>  

//#define ATLAS_DEBUG

#define ATLAS_SENSOR_PH 1
#define ATLAS_SENSOR_ORP 2
#define ATLAS_SENSOR_DO 3
#define ATLAS_SENSOR_EC 4
#define ATLAS_SENSOR_RTD 5

#define MAX_DATA_TO_READ 32
#define BUFFER_SIZE MAX_DATA_TO_READ+1
#define ATLAS_ERROR_RESPONSE 0
#define ATLAS_SUCCESS_RESPONSE 1
#define ATLAS_FAIL_RESPONSE 2
#define ATLAS_BUSY_RESPONSE 254
#define ATLAS_NODATA_RESPONSE 255
#define NULL_CHARACTER 0

#define MAX_CONNECTION_TRIES 3
#define CONNECTION_DELAY_MS 100

// example:
// PH EZO  -> '?I,pH,1.1'
// ORP EZO -> '?I,OR,1.0'   (-> wrong in documentation 'OR' instead of 'ORP')
// DO EZO  -> '?I,D.O.,1.0' || '?I,DO,1.7' (-> exists in D.O. and DO form)
// EC EZO  -> '?I,EC,1.0 '

// Legazy PH  -> 'P,V5.0,5/13'
// Legazy ORP -> 'O,V4.4,2/13'
// Legazy DO  -> 'D,V5.0,1/13'
// Legazy EC  -> 'E,V3.1,5/13'

#define ATLAS_INFO_COMAND "I"
#define ATLAS_READ_COMAND "R"

//TODO: para el parseo de las respuestas separadas por comas http://www.cplusplus.com/reference/cstdio/sscanf/
//TODO: metodos virtuales puros y eficiencia en sistemas embebidos http://chipkit.net/efficient-cplus-plus/

/**
 * @class	AtlasStamp
 *
 * @brief	
 *
 * @author	Mdps
 * @date	20/01/2017
 */
class AtlasStamp
{
public:
	//Constructor y destructor
	explicit AtlasStamp(byte);


	//Metodos de la api de ATLAS
	//bool calibrate(void);
	//bool factoryReset(void);
	char* info(void);
	bool led(); //Esta encendido el led?
	bool led(bool);
	//bool protocolLock();
	//bool protocolLock(bool);
	//bool sleep(void);
	//bool wakeup(void);
	//char* status(void);
	byte address();
	bool ready(); //Indica si esta inicializado el sensor
	
	//Metodos para sincronizacion en lecturas asincronas
	bool busy(); //Inidica si esta en una lectura asincrona
	bool available(); //Indica si tenemos resultado de una lectura asincrona

	//Metodos de apoyo
	float max_value();
	float min_value();
	char* unit();

	/// <summary>
	/// Limpia el sensor, eliminado el estado BUSY si es que existe
	/// </summary>
	void purge();

	byte raw_command(char*, unsigned long);
	void raw_response(char*);

	//METODO VIRTUALES PURAS
	//http://www.learncpp.com/cpp-tutorial/126-pure-virtual-functions-abstract-base-classes-and-interface-classes/
	//Virtualidad pura no es optima con el compilador de pic32 http ://chipkit.net/efficient-cplus-plus/
	//
	//virtual float* read(void)=0;
	//virtual bool readAsync(void)=0;
	//virtual float* resultAsync(void)=0;
	//virtual ~AtlasStamp(){};

private:
	char _buffer[BUFFER_SIZE];
	byte _i2c_bytes_received;
	bool _is_init;
	bool _is_busy;
	float _max_value;
	float _min_value;
	char* _unit;
	unsigned long _async_comand_ready_by;
	void _cleanWire(void);
protected:
	//TODO: Hacer limpieza de metodos, esta clase va a pasar a ser abstracta asi que podemos tener
	//mas metodos protected y menos funciones de asignacion y de lectura
	char _infoBuffer[256] = { 0 };
	uint8_t _address;
	char stamp_version[5];
	void _ready(bool); //Fija el valor de _is_init
	char* _getBuffer();
	char _readBuffer(byte);
	void _cleanBuffer(void);
	byte _bytes_in_buffer(void);
	byte _command(char *, unsigned long);
	byte _raw_command(char *, unsigned long);
	bool _stampConnected(void);
	float _vcc(void);
	void max_value(float);
	void min_value(float);
	void unit(char*);
	/*
	Commandos asincronos
	*/
	bool _command_async(char *, unsigned long);
	byte _command_result();
};

#endif


