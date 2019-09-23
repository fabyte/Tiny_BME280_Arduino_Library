/*
  This example shows how to connect two sensors on the same SPI bus.
*/

#define TINY_BME280_SPI
#include "TinyBME280.h"
tiny::BME280 mySensorA; //Uses SPI CS PIN 9
tiny::BME280 mySensorB; //Uses SPI CS PIN 10

void setup()
{
  Serial.begin(9600);
  Serial.println("Example showing alternate I2C addresses");

  if (mySensorA.beginSPI(9) == false)
    Serial.println("Sensor A connect failed");

  if (mySensorB.beginSPI(10) == false) //Alternativly, call mySensorB.begin() since CS Pin is 10 as default
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

