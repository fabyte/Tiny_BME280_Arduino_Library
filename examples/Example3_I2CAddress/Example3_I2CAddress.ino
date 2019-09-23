/*
  This example shows how to connect two sensors on the same I2C bus.

  The BME280 has two I2C addresses: 0x77 (jumper open) or 0x76 (jumper closed)
*/

#define TINY_BME280_I2C
#include "TinyBME280.h"
tiny::BME280 mySensorA; //Uses default I2C address 0x77
tiny::BME280 mySensorB; //Uses I2C address 0x76 (jumper closed)

void setup()
{
  Serial.begin(9600);
  Serial.println("Example showing alternate I2C addresses");

  mySensorA.setI2CAddress(0x77); //This is also the default value
  //The I2C address must be set before .begin() otherwise the cal values will fail to load.

  if (mySensorA.begin() == false)
    Serial.println("Sensor A connect failed");

  if (mySensorB.beginI2C(0x76) == false) //Alternativly, call mySensorB.setI2CAddress(0x76) before begin()
    Serial.println("Sensor B connect failed");
}

void loop()
{
  Serial.print(F("Temperature A in Celsius:\t"));
  Serial.println(mySensorA.readFixedTempC() / 100.0); //Output value of "5123" equals 51.23 DegC.

  Serial.print(F("Temperature A in Fahrenheit:\t"));
  Serial.println(mySensorA.readFixedTempF() / 100.0); //Output value of "7470" equals 74.70 DegF.

  Serial.print(F("Humidity A in %:\t\t"));
  Serial.println(mySensorA.readFixedHumidity() / 1000.0); //Output value of "47445" represents 47.445 %RH

  Serial.print(F("Pressure A in hPa:\t\t"));
  Serial.println(mySensorA.readFixedPressure() / 100.0); //Output value of "96386" equals 96386Pa = 963.86 hPa

  Serial.print(F("Temperature B in Celsius:\t"));
  Serial.println(mySensorB.readFixedTempC() / 100.0); //Output value of "5123" equals 51.23 DegC.

  Serial.print(F("Temperature B in Fahrenheit:\t"));
  Serial.println(mySensorB.readFixedTempF() / 100.0); //Output value of "7470" equals 74.70 DegF.

  Serial.print(F("Humidity B in %:\t\t"));
  Serial.println(mySensorB.readFixedHumidity() / 1000.0); //Output value of "47445" represents 47.445 %RH

  Serial.print(F("Pressure B in hPa:\t\t"));
  Serial.println(mySensorB.readFixedPressure() / 100.0); //Output value of "96386" equals 96386Pa = 963.86 hPa

  Serial.println();

  delay(50);
}

