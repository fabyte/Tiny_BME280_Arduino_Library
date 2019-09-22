/******************************************************************************
TinyBME280.h

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.

******************************************************************************/

#ifndef __TINY_BME280_H__
#define __TINY_BME280_H__

namespace tiny
{

//Class SensorSettings.  This object is used to hold settings data.  The application
//uses this classes' data directly.  The settings are adopted and sent to the sensor
//at special times, such as .begin.  Some are used for doing math.
//
//This is a kind of bloated way to do this.  The trade-off is that the user doesn't
//need to deal with #defines or enums with bizarre names.
//
//A power user would strip out SensorSettings entirely, and send specific read and
//write command directly to the IC. (ST #defines below)
//
struct SensorSettings
{
  public:
    uint32_t tempCorrection; // correction of temperature - added to the result
};

//Used to hold the calibration constants.  These are used
//by the driver as measurements are being taking
struct SensorCalibration
{
  public:
	uint16_t dig_T1;
	int16_t dig_T2;
	int16_t dig_T3;
	
	uint16_t dig_P1;
	int16_t dig_P2;
	int16_t dig_P3;
	int16_t dig_P4;
	int16_t dig_P5;
	int16_t dig_P6;
	int16_t dig_P7;
	int16_t dig_P8;
	int16_t dig_P9;
	
	uint8_t dig_H1;
	int16_t dig_H2;
	uint8_t dig_H3;
	int16_t dig_H4;
	int16_t dig_H5;
	int8_t dig_H6;
	
};

//This is the main operational class of the driver.

class BME280
{
  public:
    //settings
    SensorSettings settings;
	SensorCalibration calibration;
	int32_t t_fine;
	
	//Constructor generates default SensorSettings.
	//(over-ride after construction if desired)
    BME280( void );
	
	#ifdef TINY_BME280_SPI
    bool beginSPI(uint8_t csPin); //Communicate using SPI
	#endif
	#ifdef TINY_BME280_I2C
	void setI2CAddress(uint8_t i2caddress); //Set the address the library should use to communicate. Use if address jumper is closed (0x76).
    bool beginI2C(TwoWire &wirePort = Wire); //Called when user provides Wire port
	#ifdef SoftwareWire_h
	bool beginI2C(SoftwareWire &wirePort); //Called when user provides a softwareWire port
	#endif
    #endif

	uint8_t getMode(void); //Get the current mode: sleep, forced, or normal
	void setMode(uint8_t mode); //Set the current mode

	void setTempOverSample(uint8_t overSampleAmount); //Set the temperature sample mode
	void setPressureOverSample(uint8_t overSampleAmount); //Set the pressure sample mode
	void setHumidityOverSample(uint8_t overSampleAmount); //Set the humidity sample mode
	void setStandbyTime(uint8_t timeSetting); //Set the standby time between measurements
	void setFilter(uint8_t filterSetting); //Set the filter
		
	bool isMeasuring(void); //Returns true while the device is taking measurement
	
	//Software reset routine
	void reset( void );
	
	//Returns the values as 32 bit fixed point integers.
	uint32_t readFixedPressure( void );
	
	uint32_t readFixedHumidity( void );

	//Temperature related methods
	int32_t readFixedTempC( void );
	int32_t readFixedTempF( void );
	
    //The following utilities read and write

	//ReadRegisterRegion takes a uint8 array address as input and reads
	//a chunk of memory into that array.
    void readRegisterRegion(uint8_t*, uint8_t, uint8_t );
	//readRegister reads one register
    uint8_t readRegister(uint8_t);
    //Reads two regs, LSByte then MSByte order, and concatenates them
	//Used for two-byte reads
	int16_t readRegisterInt16( uint8_t offset );
	//Writes a byte;
    void writeRegister(uint8_t, uint8_t);

private:
	//Call to apply SensorSettings.
	//This also gets the SensorCalibration constants
	uint8_t begin( void );
	uint8_t checkSampleValue(uint8_t userValue); //Checks for valid over sample values

	#ifdef TINY_BME280_SPI
	uint8_t chipSelectPin;
	#endif
	#ifdef TINY_BME280_I2C
	uint8_t I2CAddress;
    uint8_t _wireType = HARD_WIRE; //Default to Wire.h
    TwoWire *_hardPort = NO_WIRE; //The generic connection to user's chosen I2C hardware
    
	#ifdef SoftwareWire_h
	SoftwareWire *_softPort = NO_WIRE; //Or, the generic connection to software wire port
	#endif // SoftwareWire_h
	#endif // TINY_BME280_I2C
};

} // namespace tiny

#endif  // End of __TINY_BME280_H__ definition check