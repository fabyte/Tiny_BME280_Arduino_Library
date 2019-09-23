/******************************************************************************
TinyBME280Impl.cpp

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

//Register names:
#define BME280_DIG_T1_LSB_REG			0x88
#define BME280_DIG_T1_MSB_REG			0x89
#define BME280_DIG_T2_LSB_REG			0x8A
#define BME280_DIG_T2_MSB_REG			0x8B
#define BME280_DIG_T3_LSB_REG			0x8C
#define BME280_DIG_T3_MSB_REG			0x8D
#define BME280_DIG_P1_LSB_REG			0x8E
#define BME280_DIG_P1_MSB_REG			0x8F
#define BME280_DIG_P2_LSB_REG			0x90
#define BME280_DIG_P2_MSB_REG			0x91
#define BME280_DIG_P3_LSB_REG			0x92
#define BME280_DIG_P3_MSB_REG			0x93
#define BME280_DIG_P4_LSB_REG			0x94
#define BME280_DIG_P4_MSB_REG			0x95
#define BME280_DIG_P5_LSB_REG			0x96
#define BME280_DIG_P5_MSB_REG			0x97
#define BME280_DIG_P6_LSB_REG			0x98
#define BME280_DIG_P6_MSB_REG			0x99
#define BME280_DIG_P7_LSB_REG			0x9A
#define BME280_DIG_P7_MSB_REG			0x9B
#define BME280_DIG_P8_LSB_REG			0x9C
#define BME280_DIG_P8_MSB_REG			0x9D
#define BME280_DIG_P9_LSB_REG			0x9E
#define BME280_DIG_P9_MSB_REG			0x9F
#define BME280_DIG_H1_REG				0xA1
#define BME280_CHIP_ID_REG				0xD0 //Chip ID
#define BME280_RST_REG					0xE0 //Softreset Reg
#define BME280_DIG_H2_LSB_REG			0xE1
#define BME280_DIG_H2_MSB_REG			0xE2
#define BME280_DIG_H3_REG				0xE3
#define BME280_DIG_H4_MSB_REG			0xE4
#define BME280_DIG_H4_LSB_REG			0xE5
#define BME280_DIG_H5_MSB_REG			0xE6
#define BME280_DIG_H6_REG				0xE7
#define BME280_CTRL_HUMIDITY_REG		0xF2 //Ctrl Humidity Reg
#define BME280_STAT_REG					0xF3 //Status Reg
#define BME280_CTRL_MEAS_REG			0xF4 //Ctrl Measure Reg
#define BME280_CONFIG_REG				0xF5 //Configuration Reg
#define BME280_PRESSURE_MSB_REG			0xF7 //Pressure MSB
#define BME280_PRESSURE_LSB_REG			0xF8 //Pressure LSB
#define BME280_PRESSURE_XLSB_REG		0xF9 //Pressure XLSB
#define BME280_TEMPERATURE_MSB_REG		0xFA //Temperature MSB
#define BME280_TEMPERATURE_LSB_REG		0xFB //Temperature LSB
#define BME280_TEMPERATURE_XLSB_REG		0xFC //Temperature XLSB
#define BME280_HUMIDITY_MSB_REG			0xFD //Humidity MSB
#define BME280_HUMIDITY_LSB_REG			0xFE //Humidity LSB

//****************************************************************************//
//
//  Settings and configuration
//
//****************************************************************************//

//Constructor -- Specifies default configuration
tiny::BME280::BME280( void )
{
	//Construct with these default settings
#ifdef TINY_BME280_I2C
	_hardPort = &Wire; //Default to Wire port
#endif

	settings.tempCorrection = 0; // correction of temperature - added to the result
}


//****************************************************************************//
//
//  Configuration section
//
//  This uses the stored SensorSettings to start the IMU
//  Use statements such as "mySensor.settings.commInterface = SPI_MODE;" to
//  configure before calling .begin();
//
//****************************************************************************//
uint8_t tiny::BME280::begin()
{
	delay(2);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

	#ifdef TINY_BME280_SPI
	// start the SPI library:
	SPI.begin();
	#ifdef ARDUINO_ARCH_ESP32
	SPI.setFrequency(1000000);
	// Data is read and written MSb first.
	SPI.setBitOrder(SPI_MSBFIRST);
	// Like the standard arduino/teensy comment below, mode0 seems wrong according to standards
	// but conforms to the timing diagrams when used for the ESP32
	SPI.setDataMode(SPI_MODE0);
	#else
	// Maximum SPI frequency is 10MHz, could divide by 2 here:
	SPI.setClockDivider(SPI_CLOCK_DIV32);
	// Data is read and written MSb first.
	SPI.setBitOrder(MSBFIRST);
	// Data is captured on rising edge of clock (CPHA = 0)
	// Base value of the clock is HIGH (CPOL = 1)
	// This was SPI_MODE3 for RedBoard, but I had to change to
	// MODE0 for Teensy 3.1 operation
	SPI.setDataMode(SPI_MODE3);
	#endif
	// initialize the  data ready and chip select pins:
	pinMode(chipSelectPin, OUTPUT);
	digitalWrite(chipSelectPin, HIGH);
	#endif


	#ifdef TINY_BME280_I2C
	switch(_wireType)
	{
		case(HARD_WIRE):
			_hardPort->begin(); //The caller can begin their port and set the speed. We just confirm it here otherwise it can be hard to debug.
			break;
		case(SOFT_WIRE):
		#ifdef SoftwareWire_h
			_softPort->begin(); //The caller can begin their port and set the speed. We just confirm it here otherwise it can be hard to debug.
		#endif
			break;
	}
	#endif

	//Check communication with IC before anything else
	uint8_t chipID = readRegister(BME280_CHIP_ID_REG); //Should return 0x60 or 0x58
	if(chipID != 0x58 && chipID != 0x60) // Is this BMP or BME?
		return(chipID); //This is not BMP nor BME!

	//Reading all compensation data, range 0x88:A1, 0xE1:E7
	calibration.dig_T1 = ((uint16_t)((readRegister(BME280_DIG_T1_MSB_REG) << 8) + readRegister(BME280_DIG_T1_LSB_REG)));
	calibration.dig_T2 = ((int16_t)((readRegister(BME280_DIG_T2_MSB_REG) << 8) + readRegister(BME280_DIG_T2_LSB_REG)));
	calibration.dig_T3 = ((int16_t)((readRegister(BME280_DIG_T3_MSB_REG) << 8) + readRegister(BME280_DIG_T3_LSB_REG)));

	calibration.dig_P1 = ((uint16_t)((readRegister(BME280_DIG_P1_MSB_REG) << 8) + readRegister(BME280_DIG_P1_LSB_REG)));
	calibration.dig_P2 = ((int16_t)((readRegister(BME280_DIG_P2_MSB_REG) << 8) + readRegister(BME280_DIG_P2_LSB_REG)));
	calibration.dig_P3 = ((int16_t)((readRegister(BME280_DIG_P3_MSB_REG) << 8) + readRegister(BME280_DIG_P3_LSB_REG)));
	calibration.dig_P4 = ((int16_t)((readRegister(BME280_DIG_P4_MSB_REG) << 8) + readRegister(BME280_DIG_P4_LSB_REG)));
	calibration.dig_P5 = ((int16_t)((readRegister(BME280_DIG_P5_MSB_REG) << 8) + readRegister(BME280_DIG_P5_LSB_REG)));
	calibration.dig_P6 = ((int16_t)((readRegister(BME280_DIG_P6_MSB_REG) << 8) + readRegister(BME280_DIG_P6_LSB_REG)));
	calibration.dig_P7 = ((int16_t)((readRegister(BME280_DIG_P7_MSB_REG) << 8) + readRegister(BME280_DIG_P7_LSB_REG)));
	calibration.dig_P8 = ((int16_t)((readRegister(BME280_DIG_P8_MSB_REG) << 8) + readRegister(BME280_DIG_P8_LSB_REG)));
	calibration.dig_P9 = ((int16_t)((readRegister(BME280_DIG_P9_MSB_REG) << 8) + readRegister(BME280_DIG_P9_LSB_REG)));

	calibration.dig_H1 = ((uint8_t)(readRegister(BME280_DIG_H1_REG)));
	calibration.dig_H2 = ((int16_t)((readRegister(BME280_DIG_H2_MSB_REG) << 8) + readRegister(BME280_DIG_H2_LSB_REG)));
	calibration.dig_H3 = ((uint8_t)(readRegister(BME280_DIG_H3_REG)));
	calibration.dig_H4 = ((int16_t)((readRegister(BME280_DIG_H4_MSB_REG) << 4) + (readRegister(BME280_DIG_H4_LSB_REG) & 0x0F)));
	calibration.dig_H5 = ((int16_t)((readRegister(BME280_DIG_H5_MSB_REG) << 4) + ((readRegister(BME280_DIG_H4_LSB_REG) >> 4) & 0x0F)));
	calibration.dig_H6 = ((int8_t)readRegister(BME280_DIG_H6_REG));

	// apply default settings
	setTempOverSample(1);
	setHumidityOverSample(1);
	setPressureOverSample(1);
	setStandbyTime(0);
	setFilter(0);

	setMode(MODE_NORMAL); //Go!

	return(readRegister(BME280_CHIP_ID_REG)); //Should return 0x60
}

#ifdef TINY_BME280_SPI
//Begin comm with BME280 over SPI
bool tiny::BME280::beginSPI(uint8_t csPin)
{
	chipSelectPin = csPin;

	if(begin() == 0x58) return(true); //Begin normal init with these settings. Should return chip ID of 0x58 for BMP
	if(begin() == 0x60) return(true); //Begin normal init with these settings. Should return chip ID of 0x60 for BME
	return(false);
}
#endif

#ifdef TINY_BME280_I2C
//Set the address the library should use to communicate.
//Use if address jumper is closed (0x76). Default is 0x77
void tiny::BME280::setI2CAddress(uint8_t address)
{
	I2CAddress = address; //Set the I2C address for this device
}

//Set the address the library should use to communicate and begin communication over I2C.
//Use if address jumper is closed (0x76). Default is 0x77
bool tiny::BME280::beginI2C(uint8_t address)
{
	I2CAddress = address; //Set the I2C address for this device

	if(begin() == 0x58) return(true); //Begin normal init with these settings. Should return chip ID of 0x58 for BMP
	if(begin() == 0x60) return(true); //Begin normal init with these settings. Should return chip ID of 0x60 for BME
	return(false);
}

//Begin comm with BME280 over I2C
bool tiny::BME280::beginI2C(TwoWire &wirePort)
{
	_hardPort = &wirePort;
	_wireType = HARD_WIRE;

	if(begin() == 0x58) return(true); //Begin normal init with these settings. Should return chip ID of 0x58 for BMP
	if(begin() == 0x60) return(true); //Begin normal init with these settings. Should return chip ID of 0x60 for BME
	return(false);
}

//Begin comm with BME280 over software I2C
#ifdef SoftwareWire_h
bool tiny::BME280::beginI2C(SoftwareWire& wirePort)
{
	_softPort = &wirePort;
	_wireType = SOFT_WIRE;

	if(begin() == 0x58) return(true); //Begin normal init with these settings. Should return chip ID of 0x58 for BMP
	if(begin() == 0x60) return(true); //Begin normal init with these settings. Should return chip ID of 0x60 for BME
	return(false);
}
#endif // SoftwareWire_h
#endif // TINY_BME280_I2C

//Set the mode bits in the ctrl_meas register
// Mode 00 = Sleep
// 01 and 10 = Forced
// 11 = Normal mode
void tiny::BME280::setMode(uint8_t mode)
{
	if(mode > 0b11) mode = 0; //Error check. Default to sleep mode

	uint8_t controlData = readRegister(BME280_CTRL_MEAS_REG);
	controlData &= ~( (1<<1) | (1<<0) ); //Clear the mode[1:0] bits
	controlData |= mode; //Set
	writeRegister(BME280_CTRL_MEAS_REG, controlData);
}

//Gets the current mode bits in the ctrl_meas register
//Mode 00 = Sleep
// 01 and 10 = Forced
// 11 = Normal mode
uint8_t tiny::BME280::getMode()
{
	uint8_t controlData = readRegister(BME280_CTRL_MEAS_REG);
	return(controlData & 0b00000011); //Clear bits 7 through 2
}

//Set the standby bits in the config register
//tStandby can be:
//  0, 0.5ms
//  1, 62.5ms
//  2, 125ms
//  3, 250ms
//  4, 500ms
//  5, 1000ms
//  6, 10ms
//  7, 20ms
void tiny::BME280::setStandbyTime(uint8_t timeSetting)
{
	if(timeSetting > 0b111) timeSetting = 0; //Error check. Default to 0.5ms

	uint8_t controlData = readRegister(BME280_CONFIG_REG);
	controlData &= ~( (1<<7) | (1<<6) | (1<<5) ); //Clear the 7/6/5 bits
	controlData |= (timeSetting << 5); //Align with bits 7/6/5
	writeRegister(BME280_CONFIG_REG, controlData);
}

//Set the filter bits in the config register
//filter can be off or number of FIR coefficients to use:
//  0, filter off
//  1, coefficients = 2
//  2, coefficients = 4
//  3, coefficients = 8
//  4, coefficients = 16
void tiny::BME280::setFilter(uint8_t filterSetting)
{
	if(filterSetting > 0b111) filterSetting = 0; //Error check. Default to filter off

	uint8_t controlData = readRegister(BME280_CONFIG_REG);
	controlData &= ~( (1<<4) | (1<<3) | (1<<2) ); //Clear the 4/3/2 bits
	controlData |= (filterSetting << 2); //Align with bits 4/3/2
	writeRegister(BME280_CONFIG_REG, controlData);
}

//Set the temperature oversample value
//0 turns off temp sensing
//1 to 16 are valid over sampling values
void tiny::BME280::setTempOverSample(uint8_t overSampleAmount)
{
	overSampleAmount = checkSampleValue(overSampleAmount); //Error check

	uint8_t originalMode = getMode(); //Get the current mode so we can go back to it at the end

	setMode(MODE_SLEEP); //Config will only be writeable in sleep mode, so first go to sleep mode

	//Set the osrs_t bits (7, 6, 5) to overSampleAmount
	uint8_t controlData = readRegister(BME280_CTRL_MEAS_REG);
	controlData &= ~( (1<<7) | (1<<6) | (1<<5) ); //Clear bits 765
	controlData |= overSampleAmount << 5; //Align overSampleAmount to bits 7/6/5
	writeRegister(BME280_CTRL_MEAS_REG, controlData);

	setMode(originalMode); //Return to the original user's choice
}

//Set the pressure oversample value
//0 turns off pressure sensing
//1 to 16 are valid over sampling values
void tiny::BME280::setPressureOverSample(uint8_t overSampleAmount)
{
	overSampleAmount = checkSampleValue(overSampleAmount); //Error check

	uint8_t originalMode = getMode(); //Get the current mode so we can go back to it at the end

	setMode(MODE_SLEEP); //Config will only be writeable in sleep mode, so first go to sleep mode

	//Set the osrs_p bits (4, 3, 2) to overSampleAmount
	uint8_t controlData = readRegister(BME280_CTRL_MEAS_REG);
	controlData &= ~( (1<<4) | (1<<3) | (1<<2) ); //Clear bits 432
	controlData |= overSampleAmount << 2; //Align overSampleAmount to bits 4/3/2
	writeRegister(BME280_CTRL_MEAS_REG, controlData);

	setMode(originalMode); //Return to the original user's choice
}

//Set the humidity oversample value
//0 turns off humidity sensing
//1 to 16 are valid over sampling values
void tiny::BME280::setHumidityOverSample(uint8_t overSampleAmount)
{
	overSampleAmount = checkSampleValue(overSampleAmount); //Error check

	uint8_t originalMode = getMode(); //Get the current mode so we can go back to it at the end

	setMode(MODE_SLEEP); //Config will only be writeable in sleep mode, so first go to sleep mode

	//Set the osrs_h bits (2, 1, 0) to overSampleAmount
	uint8_t controlData = readRegister(BME280_CTRL_HUMIDITY_REG);
	controlData &= ~( (1<<2) | (1<<1) | (1<<0) ); //Clear bits 2/1/0
	controlData |= overSampleAmount << 0; //Align overSampleAmount to bits 2/1/0
	writeRegister(BME280_CTRL_HUMIDITY_REG, controlData);

	setMode(originalMode); //Return to the original user's choice
}

//Validates an over sample value
//Allowed values are 0 to 16
//These are used in the humidty, pressure, and temp oversample functions
uint8_t tiny::BME280::checkSampleValue(uint8_t userValue)
{
	switch(userValue)
	{
		case(0):
			return 0;
			break; //Valid
		case(1):
			return 1;
			break; //Valid
		case(2):
			return 2;
			break; //Valid
		case(4):
			return 3;
			break; //Valid
		case(8):
			return 4;
			break; //Valid
		case(16):
			return 5;
			break; //Valid
		default:
			return 1; //Default to 1x
			break; //Good
	}
}

//Check the measuring bit and return true while device is taking measurement
bool tiny::BME280::isMeasuring(void)
{
	uint8_t stat = readRegister(BME280_STAT_REG);
	return(stat & (1<<3)); //If the measuring bit (3) is set, return true
}

//Strictly resets.  Run .begin() afterwards
void tiny::BME280::reset( void )
{
	writeRegister(BME280_RST_REG, 0xB6);

}

//****************************************************************************//
//
//  Pressure Section
//
//****************************************************************************//
uint32_t tiny::BME280::readFixedPressure( void )
{
	// Returns pressure in Pa as unsigned 32 bit integer. Output value of "96386" equals 96386Pa = 963.86 hPa
	uint8_t buffer[3];
	readRegisterRegion(buffer, BME280_PRESSURE_MSB_REG, 3);
	int32_t adc_P = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);

	int32_t var1, var2;
	uint32_t p_acc;
	var1 = ((int32_t)t_fine>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11) * (int32_t)calibration.dig_P6;
	var2 = var2 + ((var1 * ((int32_t)calibration.dig_P5)<<1));
	var2 = (var2>>2) + (((int32_t)calibration.dig_P4)<<16);
	var1 = ((((int32_t)calibration.dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + (((((int32_t)calibration.dig_P2) * var1)>>1)))>>18;
	var1 = ((((32768+var1))*((int32_t)calibration.dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p_acc = (((uint32_t)(((int32_t)1048576 - adc_P)-(var2>>12)))*3125);
	if (p_acc < 0x80000000)
	{
		p_acc = (p_acc<<1) / ((uint32_t)var1);
	}
	else
	{
		p_acc = (p_acc / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)calibration.dig_P9) * ((int32_t)(((p_acc>>3) * (p_acc>>3))>>13)))>>12;
	var2 = (((int32_t)(p_acc>>2)) * ((int32_t)calibration.dig_P8)) >> 13;
	p_acc = (uint32_t)((int32_t)p_acc + ((var1 + var2 + (int32_t)calibration.dig_P7) >> 4));

	return p_acc;
}

//****************************************************************************//
//
//  Humidity Section
//
//****************************************************************************//
uint32_t tiny::BME280::readFixedHumidity( void )
{
	// Returns humidity in %RH as unsigned 32 bit integer.
	// Output value of "47445" represents 47.445 %RH
	uint8_t buffer[2];
	readRegisterRegion(buffer, BME280_HUMIDITY_MSB_REG, 2);
	int32_t adc_H = ((uint32_t)buffer[0] << 8) | ((uint32_t)buffer[1]);

	int32_t var1;
	var1 = (t_fine - ((int32_t)76800));
	var1 = (((((adc_H << 14) - (((int32_t)calibration.dig_H4) << 20) - (((int32_t)calibration.dig_H5) * var1)) +
	((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)calibration.dig_H6)) >> 10) * (((var1 * ((int32_t)calibration.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)calibration.dig_H2) + 8192) >> 14));
	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)calibration.dig_H1)) >> 4));
	var1 = (var1 < 0 ? 0 : var1);
	var1 = (var1 > 419430400 ? 419430400 : var1);

	return (uint32_t)(var1>>12)*1000/1024;
}

//****************************************************************************//
//
//  Temperature Section
//
//****************************************************************************//

int32_t tiny::BME280::readFixedTempC( void )
{
	// Returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC.
	// t_fine carries fine temperature as global value

	//get the reading (adc_T);
	uint8_t buffer[3];
	readRegisterRegion(buffer, BME280_TEMPERATURE_MSB_REG, 3);
	int32_t adc_T = ((uint32_t)buffer[0] << 12) | ((uint32_t)buffer[1] << 4) | ((buffer[2] >> 4) & 0x0F);

	//By datasheet, calibrate
	int32_t var1, var2;

	var1 = ((((adc_T>>3) - ((int32_t)calibration.dig_T1<<1))) * ((int32_t)calibration.dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)calibration.dig_T1)) * ((adc_T>>4) - ((int32_t)calibration.dig_T1))) >> 12) *
	((int32_t)calibration.dig_T3)) >> 14;
	t_fine = var1 + var2;

	return ((t_fine * 5 + 128) >> 8) + (settings.tempCorrection * 100);
}

int32_t tiny::BME280::readFixedTempF( void )
{
	return (readFixedTempC() * 9) / 5 + 3200;
}

//****************************************************************************//
//
//  Utility
//
//****************************************************************************//
void tiny::BME280::readRegisterRegion(uint8_t *outputPointer , uint8_t offset, uint8_t length)
{
	//define pointer that will point to the external space
	uint8_t i = 0;
	char c = 0;

	#ifdef TINY_BME280_I2C
	switch(_wireType)
	{
		case(HARD_WIRE):
			_hardPort->beginTransmission(I2CAddress);
			_hardPort->write(offset);
			_hardPort->endTransmission();

			// request bytes from slave device
			_hardPort->requestFrom(I2CAddress, length);
			while ( (_hardPort->available()) && (i < length))  // slave may send less than requested
			{
				c = _hardPort->read(); // receive a byte as character
				*outputPointer = c;
				outputPointer++;
				i++;
			}
			break;
		case(SOFT_WIRE):
		#ifdef SoftwareWire_h
			_softPort->beginTransmission(settings.I2CAddress);
			_softPort->write(offset);
			_softPort->endTransmission();

			// request bytes from slave device
			_softPort->requestFrom(settings.I2CAddress, length);
			while ( (_softPort->available()) && (i < length))  // slave may send less than requested
			{
				c = _softPort->read(); // receive a byte as character
				*outputPointer = c;
				outputPointer++;
				i++;
			}
		#endif
			break;
	}
	#endif

	#ifdef TINY_BME280_SPI
	// take the chip select low to select the device:
	digitalWrite(chipSelectPin, LOW);
	// send the device the register you want to read:
	SPI.transfer(offset | 0x80);  //Ored with "read request" bit
	while ( i < length ) // slave may send less than requested
	{
		c = SPI.transfer(0x00); // receive a byte as character
		*outputPointer = c;
		outputPointer++;
		i++;
	}
	// take the chip select high to de-select:
	digitalWrite(chipSelectPin, HIGH);
	#endif

}

uint8_t tiny::BME280::readRegister(uint8_t offset)
{
	//Return value
	uint8_t result = 0;

	#ifdef TINY_BME280_I2C
	uint8_t numBytes = 1;
	switch(_wireType)
	{
		case(HARD_WIRE):
			_hardPort->beginTransmission(I2CAddress);
			_hardPort->write(offset);
			_hardPort->endTransmission();

			_hardPort->requestFrom(I2CAddress, numBytes);
			while ( _hardPort->available() ) // slave may send less than requested
			{
				result = _hardPort->read(); // receive a byte as a proper uint8_t
			}
			break;

		case(SOFT_WIRE):
		#ifdef SoftwareWire_h
			_softPort->beginTransmission(I2CAddress);
			_softPort->write(offset);
			_softPort->endTransmission();

			_softPort->requestFrom(I2CAddress, numBytes);
			while ( _softPort->available() ) // slave may send less than requested
			{
				result = _softPort->read(); // receive a byte as a proper uint8_t
			}
		#endif
			break;
	}
	#endif

	#ifdef TINY_BME280_SPI
	// take the chip select low to select the device:
	digitalWrite(chipSelectPin, LOW);
	// send the device the register you want to read:
	SPI.transfer(offset | 0x80);  //Ored with "read request" bit
	// send a value of 0 to read the first byte returned:
	result = SPI.transfer(0x00);
	// take the chip select high to de-select:
	digitalWrite(chipSelectPin, HIGH);
	#endif

	return result;
}

int16_t tiny::BME280::readRegisterInt16( uint8_t offset )
{
	uint8_t myBuffer[2];
	readRegisterRegion(myBuffer, offset, 2);  //Does memory transfer
	int16_t output = (int16_t)myBuffer[0] | int16_t(myBuffer[1] << 8);

	return output;
}

void tiny::BME280::writeRegister(uint8_t offset, uint8_t dataToWrite)
{
	#ifdef TINY_BME280_I2C
	switch(_wireType)
	{
		case(HARD_WIRE):
			_hardPort->beginTransmission(I2CAddress);
			_hardPort->write(offset);
			_hardPort->write(dataToWrite);
			_hardPort->endTransmission();
			break;
		case(SOFT_WIRE):
		#ifdef SoftwareWire_h
			_softPort->beginTransmission(I2CAddress);
			_softPort->write(offset);
			_softPort->write(dataToWrite);
			_softPort->endTransmission();
		#endif
			break;
	}
	#endif

	#ifdef TINY_BME280_SPI
	// take the chip select low to select the device:
	digitalWrite(chipSelectPin, LOW);
	// send the device the register you want to read:
	SPI.transfer(offset & 0x7F);
	// send a value of 0 to read the first byte returned:
	SPI.transfer(dataToWrite);
	// decrement the number of bytes left to read:
	// take the chip select high to de-select:
	digitalWrite(chipSelectPin, HIGH);
	#endif
}
