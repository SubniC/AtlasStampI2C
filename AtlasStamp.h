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
#define I2C_RESPONSE_OK 0


#define MAX_CONNECTION_TRIES 3
#define CONNECTION_DELAY_MS 250

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

//http://gcc.gnu.org/onlinedocs/gcc-4.9.4/gcc/Inline.html
//TODO: terminar de implementar API de atlas
//TODO: destructores
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

	virtual void info(Stream&);

	bool led();
	bool led(bool);
	//bool protocolLock();
	//bool protocolLock(bool);
	
	bool const sleep(void);
	bool const wakeup(void);
	inline bool const sleeping(void) const __attribute__((always_inline)){ return !is_awake; }
	
	//Get the device address
	inline  uint8_t const address() const __attribute__((always_inline)) { return _address; }
	//True if the sensor is inicialized
	inline  bool const ready() const __attribute__((always_inline)) { return _is_init; }
	//Async reading synchronization
	//True if is taking and async read
	inline bool const busy() const __attribute__((always_inline)) { return _is_busy; }

	//True if we have an async result ready
	inline bool available() const __attribute__((always_inline))
	{
		if (_is_busy && (_async_comand_ready_by < millis()))
		{
			return true;
		}
		return false;
	}

	//Returns unit of the sensor as char array
	char* const get_unit() const
	{
		return _unit;
	}
	//Returns the min value of the sensor
	float const get_min_value() const
	{
		return _min_value;
	}
	//Returns the max value of the sensor
	float const get_max_value() const
	{
		return _max_value;
	}
	
	void purge(); //Cleans the internal object buffers and state to READY

	//Virtual methods, not using pure virtual here to optimize program memory with pic32 compiler
	//more here: http://chipkit.net/efficient-cplus-plus/
	//http://www.learncpp.com/cpp-tutorial/126-pure-virtual-functions-abstract-base-classes-and-interface-classes/
	//TODO: Esto podria tener una implementacion basica como la que hay en ORP, asi no 
	//tendriamos que implementarlo en la base.
	virtual bool const begin(void) { while (1); }
	
	float* const read(void);
	bool const read_async(void);
	float* const result_async(void);

	uint8_t read_ascii(char*);
	uint8_t result_ascii_async(char*);

	inline uint8_t const response_count(void) const { return _response_field_count; }
protected:
	//TODO: Esto no es igual en todas las clases? solo para sacar la version? no se podria hacer en la conexion?
	//y eliminamos esta fucnion de las clases derivadas? ademas eso permitira crear objetos "base" AtlasStamp compatibles
	//con cualquier modulo o no?
	virtual bool const _stamp_ready(void) { while (1); };
	
	float* const _parse_sensor_read(void);
	
	char _command_buffer[32];

	inline char* _get_response_buffer() __attribute__((always_inline))
	{
		return _response_buffer;
	}

	inline char const _read_buffer(uint8_t pos) const __attribute__((always_inline))
	{
		if (pos >= 0 && pos <= MAX_DATA_TO_READ)
		{
			return _response_buffer[pos];
		}
		return 0;
	}

	inline void _clean_buffer() __attribute__((always_inline))
	{
		_response_buffer[0] = '\0';
		_i2c_bytes_received = 0;
	}

	inline uint8_t const _bytes_in_buffer() const  __attribute__((always_inline)) { return _i2c_bytes_received; }

	uint8_t _command(char *, unsigned long);
	
	bool _stamp_connected(void);

	uint8_t _address;
	char stamp_version[5];

	//Inicialization require part of the work here and part in child class so we need the flag protected	
	bool _is_init;

	void _resize_response_count(uint8_t = 1);

	bool _command_async(char *, unsigned long);
	uint8_t _command_result();

private:
	char _response_buffer[BUFFER_SIZE];

	bool is_awake;

	float* _last_result;
	char* _unit;

	uint8_t _i2c_bytes_received;
	bool _is_busy;
	const float _max_value;
	const float _min_value;
	unsigned long _async_comand_ready_by;
	
	uint8_t _response_field_count; //TODO: no tocar directamente solo por medio del constructor o la funcion _resize_response_count()
	const uint8_t _max_response_field_count; //Maximo numero de campos de respuesta que puede tener el sensor

	inline void _clean_wire() __attribute__((always_inline)) { while (Wire.available()) { Wire.read(); } }
};

#endif


