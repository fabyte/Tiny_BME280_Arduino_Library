Tiny BME280 Arduino Library
========================================

Based on the [SparkFun BME280 Arduino Libary](https://github.com/sparkfun/SparkFun_BME280_Arduino_Library) this library aims to minimize the memory footprint in order to use it with tiny MCUs (like Atmel's ATtiny85/84).

This library allows the user to:

* Read temperature in C or F
* Read pressure in Pa
* Read humidity in %RH

Usage
-----

```c++
#define TINY_BME280_SPI
#include <TinyBME280.h>

tiny::BME280 sensor;

void setup()
{
  sensor.begin(); // Start using SPI and CS Pin 10
}

void loop()
{
  auto temp = sensor.readFixedTempC();
  auto hum = sensor.readFixedHumidity();
  auto pres = sensor.readFixedPressure();
}
```
See examples for more.


Supported Platforms
-------------------

**PlatformIO**

Just add `Tiny BME280` to `lib_deps` section in `platformio.ini`

**Arduino IDE**

See the [Arduino Tutorial](https://www.arduino.cc/en/Guide/Libraries) on how to install libraries.


Minimized Memory usage
----------------------

Since memory usage on class/method/function level is not easily determined (because eventually much of the usage appears in the main section), PlatformIO's [memory usage tool](http://docs.platformio.org/en/latest/faq.html#program-memory-usage) is used to estimate the used memory. For this, only the PROGRAMM section is evaluated. 

For the measurement, the sensor library runs in an minimal PlatformIO Generic ATtiny85 project with LTO enabled. The base functionality of the project is using the configured communication method (as offset value of used memory). Then, additional functionalities has been activated in order to measure the library's memory usage. 

<details><summary>Example using SPI communication</summary>

```c++
#define TINY_BME280_SPI
#include <TinyBME280.h>

tiny::BME280 sensor;

void setup()
{
  SPI.begin();
  #ifdef FUNCTIONALITY_INITIALIZATION
  sensor.begin();
  #endif
}

void loop()
{
  SPISettings settings{500000, MSBFIRST, SPI_MODE3};
  SPI.beginTransaction(settings);
  SPI.transfer(0xAF);

  #ifdef FUNCTIONALITY_TEMPERATURE
  auto temp = sensor.readFixedTempC();
  SPI.transfer(temp);
  #endif
  #ifdef FUNCTIONALITY_HUMIDITY
  auto hum = sensor.readFixedHumidity();
  SPI.transfer(hum);
  #endif
  #ifdef FUNCTIONALITY_PRESSURE
  auto pres = sensor.readFixedPressure();
  SPI.transfer(pres);
  #endif

  SPI.endTransaction();
}
```
</details>

**Results**

Functionality         | original (SPI) [bytes] | Tiny BME280 (SPI) [bytes] | 
---------------------:| :--------------------: |:-------------------------:| 
initialization        | 2662                   | 1374 (-48%)               | 
**init and read all** | **3792**               | **1672 (-56%)**           | 
read temperature      | 1430                   | 420  (-71%)               | 
read humidity         | 1342                   | 544  (-59%)               | 
read pressure         | 2370                   | 776  (-67%)               | 

Functionality         |original (I2C) [bytes] | Tiny BME280 (I2C) [bytes] |
---------------------:|:--------------------: | :------------------------:|
initialization        |2414                   | 1400 (-42%)               |
**init and read all** |**3790**               | **3262 (-14%)**           |
read temperature      |1404                   | 584  (-58%)               |
read humidity         |1320                   | 710  (-46%)               |
read pressure         |2354                   | 1008 (-57%)               |

As shown in the table, the PROGRAMM memory usage has dropped around 56% using SPI and 14% using I2C communication.

Altough further improvements could be made, the code readability, usability as well as the necessity not to rebuild the whole library and fix bugs were the reasons not to invest more time into this.


Changes towards SparkFun's library
----------------------------------

In order to minimize memory usage the following changes has been made

* Replace all floating point values with fixed point values
* The Altitude and Due Point calculation methods have been removed. Whoever is willing to replace them using fixed point value calculation is free to create a pull request
* Communication method (I2C or SPI) is configured at compile time
* Introduce namespace tiny (e.g. tiny::BME280) in order to distinguish from SparkFun's library


License Information
-------------------

This product is _**open source**_!

Please review the LICENSE.md file for license information.

Distributed as-is; no warranty is given.
