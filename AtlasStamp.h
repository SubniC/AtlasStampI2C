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
	//Constructor and destructor
	explicit AtlasStamp(uint8_t, char*, uint8_t, float, float, uint8_t=1);
	//virtual ~AtlasStamp() {};

	//Atlas API
	//bool calibrate(void);
	//bool factoryReset(void);
	float get_vcc(void);

	virtual char* const info(void);

	bool led();
	bool led(bool);
	//bool is_led_on();
	//bool protocolLock();
	//bool protocolLock(bool);
	//bool sleep(void);
	//bool wakeup(void);
	//char* status(void);
	uint8_t address(); //Get the device address
	bool ready(); //True if the sensor is inicialized
	
	//Async reading synchronization
	bool busy(); //True if is taking and async read
	bool available(); //True if we have an async result ready

	//Helpers methods
	float get_max_value(); //Returns the max value of the sensor
	float get_min_value(); //Returns the min value of the sensor
	char* get_unit(); //Returns unit of the sensor as char array
	void purge(); //Cleans the internal object buffers and state to READY

	//DEBUG METHOS SHOULD NOT BE USED IN PRODUCCTION CODE
	uint8_t raw_command(char*, unsigned long);
	void raw_response(char*);

	//Virtual methods, not using pure virtual here to optimize program memory with pic32 compiler
	//more here: http://chipkit.net/efficient-cplus-plus/
	//http://www.learncpp.com/cpp-tutorial/126-pure-virtual-functions-abstract-base-classes-and-interface-classes/
	
	virtual bool const begin(void) { while (1); }
	
	float* const read(void);
	bool const readAsync(void);
	float* const resultAsync(void);

	inline uint8_t const response_count(void) const { return _response_field_count; }

protected:

	float* const _parseResult(void);
	virtual bool const _stampReady(void) { while (1); };

	char _infoBuffer[256] = { 0 };
	uint8_t _address;
	char stamp_version[5];
	void _ready(bool); //Fija el valor de _is_init
	char* _getBuffer();
	char _readBuffer(uint8_t);
	void _clean_buffer(void);
	uint8_t _bytes_in_buffer(void);
	uint8_t _command(char *, unsigned long);
	uint8_t _raw_command(char *, unsigned long);
	bool _stampConnected(void);
	
	//void max_value(float);
	//void min_value(float);
	//void unit(char*);
	/*
	Commandos asincronos
	*/
	bool _command_async(char *, unsigned long);
	uint8_t _command_result();

private:
	char _buffer[BUFFER_SIZE];


	float* _last_result;
	uint8_t _response_field_count;
	char* _unit;


	uint8_t _i2c_bytes_received;
	bool _is_init;
	bool _is_busy;
	float _max_value;
	float _min_value;
	unsigned long _async_comand_ready_by;
	void _cleanWire(void);

};

#endif


