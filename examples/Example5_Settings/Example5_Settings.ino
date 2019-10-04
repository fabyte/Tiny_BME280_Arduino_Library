/*
  Control the various settings of the BME280

  This example shows how to set the various filter, and oversample settings of the BME280.
*/

#define TINY_BME280_I2C
#include "TinyBME280.h"
tiny::BME280 mySensor;

void setup()
{
  Serial.begin(9600);
  Serial.println("Example showing alternate I2C addresses");

  Wire.begin();
  Wire.setClock(400000); //Increase to fast I2C speed!

  mySensor.beginI2C();

  mySensor.setFilter(1); //0 to 4 is valid. Filter coefficient. See 3.4.4
  mySensor.setStandbyTime(0); //0 to 7 valid. Time between readings. See table 27.

  mySensor.setTempOverSample(1); //0 to 16 are valid. 0 disables temp sensing. See table 24.
  mySensor.setPressureOverSample(1); //0 to 16 are valid. 0 disables pressure sensing. See table 23.
  mySensor.setHumidityOverSample(1); //0 to 16 are valid. 0 disables humidity sensing. See table 19.

  mySensor.setMode(tiny::Mode::NORMAL); //SLEEP, FORCED, NORMAL is valid. See 3.3
}

void loop()
{
  Serial.print(F("Temperature in Celsius:\t\t"));
  Serial.println(mySensor.readFixedTempC() / 100.0); //Output value of "5123" equals 51.23 DegC.

  Serial.print(F("Temperature in Fahrenheit:\t"));
  Serial.println(mySensor.readFixedTempF() / 100.0); //Output value of "7470" equals 74.70 DegF.

  Serial.print(F("Humidity in %:\t\t\t"));
  Serial.println(mySensor.readFixedHumidity() / 1000.0); //Output value of "47445" represents 47.445 %RH

  Serial.print(F("Pressure in hPa:\t\t"));
  Serial.println(mySensor.readFixedPressure() / 100.0); //Output value of "96386" equals 96386Pa = 963.86 hPa

  Serial.println();

  delay(50);
}
