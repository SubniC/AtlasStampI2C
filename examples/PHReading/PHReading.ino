/*
 * PHReading - basic synchronous read from an Atlas Scientific PH EZO module (I2C).
 *
 * Wiring: connect the EZO module to the I2C bus (SDA/SCL) and power.
 * The default PH EZO I2C address is 0x63.
 */

#include <Wire.h>
#include "AtlasStampPH.h"

#define PH_SENSOR_ADDRESS 0x63

AtlasStampPh PHSensor(PH_SENSOR_ADDRESS);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!PHSensor.begin()) {
    Serial.println("ERROR, PH sensor not present or failed to initialize");
  }

  // Print module info (address, version, range, VCC, ...).
  PHSensor.info(Serial);
  Serial.println();
}

void loop() {
  float* reading = PHSensor.read();

  if (reading != nullptr) {
    Serial.print("PH: ");
    Serial.print(*reading);
    Serial.print(" ");
    Serial.println(PHSensor.get_unit());
  } else {
    Serial.println("PH read failed");
  }

  delay(1000);
}
