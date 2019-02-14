// AtlasStamp.h

#ifndef _ATLASSTAMP_h
#define _ATLASSTAMP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>  

#define ATLAS_DEBUG

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


//TODO: terminar de implementar API de atlas
//TODO: Revisar char* para que todos sean C style strings
//TODO: _command() y raw_command() se peuden unificar?
//TODO: Cambio dinamico de _num_fields_response
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
	//bool protocolLock();
	//bool protocolLock(bool);
	
	//bool const sleep(uint32_t duration); //podemos implementarlo con timeout, 0 para siempre?
	bool const sleep(void);
	//bool const wakeup(void); //No parece necesario
	inline bool const sleeping(void) const __attribute__((always_inline))
	{
		return !is_awake;
	}
	


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
	//uint8_t raw_command(char*, unsigned long);
	//void raw_response(char*);

	//Virtual methods, not using pure virtual here to optimize program memory with pic32 compiler
	//more here: http://chipkit.net/efficient-cplus-plus/
	//http://www.learncpp.com/cpp-tutorial/126-pure-virtual-functions-abstract-base-classes-and-interface-classes/
	//TODO: Esto podria tener una implementacion basica como la que hay en ORP, asi no 
	//tendriamos que implementarlo en la base.
	virtual bool const begin(void) { while (1); }
	
	float* const read(void);
	bool const read_async(void);
	float* const result_async(void);

	inline uint8_t const response_count(void) const { return _response_field_count; }

protected:

	//TODO: Esto no es igual en todas las clases? solo para sacar la version? no se podria hacer en la conexion?
	//y eliminamos esta fucnion de las clases derivadas? ademas eso permitira crear objetos "base" AtlasStamp compatibles
	//con cualquier modulo o no?
	virtual bool const _stamp_ready(void) { while (1); };
	
	float* const _parse_sensor_read(void);
	
	char _command_buffer[32];

	//TODO: no es necesario, hacer otra cosa
	char _infoBuffer[256]; //Necesario?
	void _ready(bool); //Fija el valor de _is_init, NECESARIO?

	char* _get_response_buffer();
	char _read_buffer(uint8_t);
	void _clean_buffer(void);
	uint8_t _bytes_in_buffer(void);

	uint8_t _command(char *, unsigned long);
	uint8_t _raw_command(char *, unsigned long);
	
	bool _stamp_connected(void);

	uint8_t _address;
	char stamp_version[5];

	void _resize_response_count(uint8_t = 1);
	/*
	Commandos asincronos
	*/
	bool _command_async(char *, unsigned long);
	uint8_t _command_result();

private:
	char _response_buffer[BUFFER_SIZE];

	bool is_awake;

	float* _last_result;
	char* _unit;


	uint8_t _i2c_bytes_received;
	bool _is_init;
	bool _is_busy;
	float _max_value;
	float _min_value;
	unsigned long _async_comand_ready_by;
	
	uint8_t _response_field_count; //TODO: no tocar directamente solo por medio del constructor o la funcion _resize_response_count()
	uint8_t _max_response_field_count; //Maximo numero de campos de respuesta que puede tener el sensor

	void _clean_wire(void);
};

#endif


