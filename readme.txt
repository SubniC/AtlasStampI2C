

## SYNC EXAMPLE

```
#include <wire.h>
#include "AtlasStampPH.h"

#define PH_SENSOR_ADDRESS 0x63

AtlasStampPh PHSensor(PH_SENSOR_ADDRESS);

void setup() {

	Serial.begin(115200);

	Wire.begin();

	if (!PHSensor.begin())
	{
		Serial.println("ERROR,PH sensor not present or failed to inicialize");
	}

	Serial.println(PHSensor.info());
	Serial.println("AtlasStamp Library test setup finish");
}

void loop() {
	float* dummyRead;
	uint32_t starttime = 0;

	dummyRead = PHSensor.read();
	Serial.printf("READ PH: %4.2f (%s) min[%4.3f] max[%4.3f] in[%lu]\n", *dummyRead, PHSensor.get_unit(), PHSensor.get_min_value(), PHSensor.get_max_value(), millis()- starttime);
	Serial.println();
}

```

## ASYNC EXAMPLE

```
#include <wire.h>
#include "AtlasStampPH.h"

#define PH_SENSOR_ADDRESS 0x63

AtlasStampPh PHSensor(PH_SENSOR_ADDRESS);

void setup() {

	Serial.begin(115200);

	Wire.begin();

	if (!PHSensor.begin())
	{
		Serial.println("ERROR,PH sensor not present or failed to inicialize");
	}

	Serial.println(PHSensor.info());
	Serial.println("AtlasStamp Library test setup finish");
}

void loop() {
	float* dummyRead;
	
	//Init the reading if sensor is in standby
	PHSensor.read_async();

	//Do things here while the module make the reading

	//Check if the reading is completed
	if (PHSensor.available())
	{
	    //Pull the result from the sensor
		dummyRead = PHSensor.result_async();
		if (*dummyRead == -2048.0f)
		{
			Serial.printf("ASYNC_READ PH: Invalid result got from Atlas module in[%lu]\n",millis()- starttime);
		}
		else
		{
			Serial.printf("ASYNC_READ PH: %4.2f in[%lu]\n", *dummyRead, millis()- starttime);
		}
		Serial.println();
	}
}
```