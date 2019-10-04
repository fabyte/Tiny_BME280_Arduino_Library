/*
  Put the BME280 into low power mode (aka Forced Read)

  This example shows how used the 'Forced Mode' to obtain a reading then put the
  sensor to sleep between readings.
*/

#define TINY_BME280_SPI
#include "TinyBME280.h"
tiny::BME280 mySensor;

void setup()
{
  Serial.begin(9600);

  mySensor.begin();
  mySensor.setMode(tiny::Mode::SLEEP); //Sleep for now
}

void loop()
{
  mySensor.setMode(tiny::Mode::FORCED); //Wake up sensor and take reading

  long startTime = millis();
  while(mySensor.isMeasuring() == false) ; //Wait for sensor to start measurment
  while(mySensor.isMeasuring() == true) ; //Hang out while sensor completes the reading
  long endTime = millis();

  //Sensor is now back asleep but we get get the data
  Serial.print("Measure time(ms): ");
  Serial.println(endTime - startTime);

  Serial.print(F("Temperature in Celsius:\t\t"));
  Serial.println(mySensor.readFixedTempC() / 100.0); //Output value of "5123" equals 51.23 DegC.

  Serial.print(F("Temperature in Fahrenheit:\t"));
  Serial.println(mySensor.readFixedTempF() / 100.0); //Output value of "7470" equals 74.70 DegF.

  Serial.print(F("Humidity in %:\t\t\t"));
  Serial.println(mySensor.readFixedHumidity() / 1000.0); //Output value of "47445" represents 47.445 %RH

  Serial.print(F("Pressure in hPa:\t\t"));
  Serial.println(mySensor.readFixedPressure() / 100.0); //Output value of "96386" equals 96386Pa = 963.86 hPa

  Serial.println();

  delay(1000);
}

