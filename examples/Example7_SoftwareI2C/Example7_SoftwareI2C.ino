#include <SoftwareWire.h> //SoftwareWire by Testato. Installed from library manager.

//We use pins 6 and 7 in this example but others can be used
SoftwareWire myWire(6, 7); //SDA, SCL

#define TINY_BME280_I2C
#include "TinyBME280.h"
tiny::BME280 mySensor;

void setup()
{
  Serial.begin(9600);
  Serial.println("Example showing alternate I2C addresses");

  myWire.begin();
  if(mySensor.beginI2C(myWire) == false) Serial.println("Sensor A connect failed");
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
