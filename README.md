# AtlasStampI2C

Arduino library to talk to **Atlas Scientific EZO Stamp modules in I2C mode** (PH, DO,
ORP, EC).

It wraps the EZO I2C command protocol behind a small class hierarchy, one class per
sensor type, and offers both **synchronous** and **asynchronous** reads, temperature /
pressure / salinity compensation, and sleep / wakeup control.

> **Legacy library (2019).** Kept for reference; docs refreshed in 2026, library
> behavior unchanged.

## Features

- One class per module: `AtlasStampPh`, `AtlasStampDo`, `AtlasStampOrp`, `AtlasStampEc`.
- Synchronous read (`read()`) and non-blocking asynchronous read
  (`read_async()` / `available()` / `result_async()`).
- Results as `float*` or as a C string (`read_ascii()` / `result_ascii_async()`).
- Temperature compensation (PH, DO, EC), plus pressure / salinity (DO) and K / output
  parameters (EC).
- Sleep / wakeup, on-board LED control and supply voltage readout (`get_vcc()`).

## Installation

- **Manual**: copy this folder into your Arduino `libraries/` directory and restart the
  IDE.
- **PlatformIO**: add the repository to `lib_deps`.

This library uses the standard `Wire` (I2C) library; call `Wire.begin()` before using
the sensors.

## Synchronous example

```cpp
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
  }
  delay(1000);
}
```

## Asynchronous example

`read_async()` starts a reading without blocking. Do other work, then check
`available()` and pull the value with `result_async()`. An invalid reading is reported
as `-2048.0f`.

```cpp
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
}

void loop() {
  // Start a reading if the sensor is idle.
  PHSensor.read_async();

  // ... do other work here while the module takes the reading ...

  if (PHSensor.available()) {
    float* reading = PHSensor.result_async();
    if (reading != nullptr && *reading != -2048.0f) {
      Serial.print("PH: ");
      Serial.println(*reading);
    } else {
      Serial.println("Invalid result from Atlas module");
    }
  }
}
```

See [`examples/PHReading`](examples/PHReading) for a complete sketch.

## API overview

Common to all modules (from `AtlasStamp`):

```cpp
bool   begin();              // detect and initialize the module
float* read();               // blocking read, nullptr on failure
bool   read_async();         // start a non-blocking read
bool   available();          // true when an async result is ready
float* result_async();       // fetch the async result
uint8_t read_ascii(char*);          // blocking read as C string
uint8_t result_ascii_async(char*);  // async result as C string
uint8_t address();
bool    ready();
bool    busy();
char*   get_unit();
float   get_min_value();
float   get_max_value();
float   get_vcc();
bool    led();  bool led(bool);
bool    sleep();  bool wakeup();  bool sleeping();
void    info(Stream&);
```

Temperature-compensated modules (`AtlasStampPh`, `AtlasStampDo`, `AtlasStampEc`):

```cpp
bool  set_temperature(float);
bool  set_temperature(float value, float max_divergence);
float get_temperature();
```

DO-specific (`AtlasStampDo`): `set_pressure()`, `get_pressure()`, `set_salinity()`,
`get_salinity()`.

EC-specific (`AtlasStampEc`): `set_k()`, `get_k()`, `set_output_parameter()`,
`get_output_parameter()` (output parameters `EC`, `TDS`, `S`, `SG`).

## License

[MIT](LICENSE) © 2019–2026 mdps

---

_Un proyecto de mdps · 2026 · desarrollado en Murcia._
