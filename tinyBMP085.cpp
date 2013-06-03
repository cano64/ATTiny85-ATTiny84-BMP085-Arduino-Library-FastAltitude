/*************************************************** 
 * This is a library for the BMP085 Barometric Pressure & Temp Sensor
 * 
 * Designed specifically to work with the Adafruit BMP085 Breakout 
 * ----> https://www.adafruit.com/products/391
 * 
 * These displays use I2C to communicate, 2 pins are required to  
 * interface
 * Adafruit invests time and resources providing this open source code, 
 * please support Adafruit and open-source hardware by purchasing 
 * products from Adafruit!
 * 
 * Written by Limor Fried/Ladyada for Adafruit Industries.  
 * BSD license, all text above must be included in any redistribution
 * 
 * -~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~
 * modified to work with the TinyWire Master library (TinyWireM)
 * by johngineer 10-31-2011 -- boo!
 * 
 * NOTE: to make use of the debug code herein you MUST include the
 * ATTiny serial library in your main program.
 * 
 * -------------------------------------------------------
 * modified by Michal Canecky/Cano 2013-05-05
 * -calculation of altitude without using pow() and math library
 * -calculation of altitude using only integers 
 * 	(fixed for standard sea level pressure)
 * 
 ****************************************************/

#include "tinyBMP085.h"
#include <util/delay.h>



tinyBMP085::tinyBMP085() {
}


uint8_t tinyBMP085::begin(uint8_t mode) {
    if (mode > BMP085_ULTRAHIGHRES) 
        mode = BMP085_ULTRAHIGHRES;
    oversampling = mode;

    TinyWireM.begin();

    if (read8(0xD0) != 0x55) return false;

    /* read calibration data */
    ac1 = read16(BMP085_CAL_AC1);
    ac2 = read16(BMP085_CAL_AC2);
    ac3 = read16(BMP085_CAL_AC3);
    ac4 = read16(BMP085_CAL_AC4);
    ac5 = read16(BMP085_CAL_AC5);
    ac6 = read16(BMP085_CAL_AC6);

    b1 = read16(BMP085_CAL_B1);
    b2 = read16(BMP085_CAL_B2);

    mb = read16(BMP085_CAL_MB);
    mc = read16(BMP085_CAL_MC);
    md = read16(BMP085_CAL_MD);

    return true;
}

uint16_t tinyBMP085::readRawTemperature(void) {
    write8(BMP085_CONTROL, BMP085_READTEMPCMD);
    _delay_ms(5);
    return read16(BMP085_TEMPDATA);
}

uint32_t tinyBMP085::readRawPressure(void) {
    uint32_t raw;

    write8(BMP085_CONTROL, BMP085_READPRESSURECMD + (oversampling << 6));

    if (oversampling == BMP085_ULTRALOWPOWER) 
        _delay_ms(5);
    else if (oversampling == BMP085_STANDARD) 
        _delay_ms(8);
    else if (oversampling == BMP085_HIGHRES) 
        _delay_ms(14);
    else 
        _delay_ms(26);

    raw = read16(BMP085_PRESSUREDATA);
    raw <<= 8;
    raw |= read8(BMP085_PRESSUREDATA+2);
    raw >>= (8 - oversampling);
    return raw;
}


// returns pressure in Pa
int32_t tinyBMP085::readPressure(void) {
    int32_t UT, UP, B3, B5, B6, X1, X2, X3, p;
    uint32_t B4, B7;

    UT = readRawTemperature();
    UP = readRawPressure();
    //see manual http://media.digikey.com/pdf/Data%20Sheets/Bosch/BMP085.pdf page 13

    // do temperature calculations
    X1=((UT-(int32_t)(ac6))*((int32_t)(ac5))) >> 15;
    X2=((int32_t)mc << 11)/(X1+(int32_t)md);
    B5=X1 + X2;

    // do pressure calcs
    B6 = B5 - 4000;
    X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
    X2 = ((int32_t)ac2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) >> 2;

    X1 = ((int32_t)ac3 * B6) >> 13;
    X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
    B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

    if (B7 < 0x80000000) {
        p = (B7 << 1) / B4;
    } 
    else {
        p = (B7 / B4) << 1;
    }
    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;

    p = p + ((X1 + X2 + (int32_t)3791)>>4);
    return p;
}

//return temperature in C
float tinyBMP085::readTemperature(void) {
    return readTemperature10C() / 10.0;
}

//return temperature in 0.1C
int16_t tinyBMP085::readTemperature10C(void) {
    int32_t UT, X1, X2, B5;     // following ds convention

    UT = readRawTemperature();

    X1 = ((UT - (int32_t)ac6) * ((int32_t)ac5)) >> 15;
    X2 = ((int32_t)mc << 11) / (X1 + (int32_t)md);
    B5 = X1 + X2;
    return (B5 + 8) >> 4;
}

//return altitude in meters 
//for compatibility with the original library
float tinyBMP085::readAltitude(int32_t sealevelPressure) {
    return readAltitudemm(sealevelPressure) / 1000.00;
}

// return altitude in mili meters, approximation using Taylor series
// by not using pow() we will save about 1200 bytes of sketch size 
int32_t tinyBMP085::readAltitudemm(int32_t sealevelPressure) {

    //TODO get rid of floats as well

    int32_t pressure = readPressure();

    int32_t ax1 = sealevelPressure - 101325;
    int32_t ax2 = ax1 * ax1;

    float b = 0.111542;
    b += -2.09483E-7 * (float)ax1;
    b += 1.23043E-12 * (float)ax2;
#if ALTITUDE_EXTRA_PRECISSION == 1
    int32_t ax3 = ax2 * ax1;
    b += -8.86586E-18 * (float)ax3;
#endif

    int32_t px1 = pressure - 101325;
    int32_t px2 = px1 * px1;

	  int32_t altitude = 4.433E7 - 3.92585E8 * b - 786.388 * b * px1 + 0.00335128 * b * px2;
#if ALTITUDE_EXTRA_PRECISSION == 1
    int32_t px3 = px2 * px1;
	  altitude += -2.12801E-8 * b * px3;
#endif
	
    return altitude;

}

// return altitude in milimetes based on standard pressure at sea level
// this is the function you will normally use for altitude readings unless you 
// want to check weather report sea level presssure for your area every time you want to use the device
// uncalibrated altitude reading may be offset 100 meters, depending if it's sunny or raining

int32_t tinyBMP085::readAltitudeSTDmm(void) {
    
    int32_t pressure = readPressure();
    int32_t moo = (int32_t)95000 - pressure;
    int32_t moo2 = moo * moo;
    int32_t altitude = 540418; //0th term
    altitude += (((int32_t)22455 * moo) >> 8); //1st term
    altitude += (moo2 >> 12) + (moo2 >> 13) + (moo2 >> 17); //2nd term

#if ALTITUDE_EXTRA_PRECISSION == 1
    int32_t moo64 = moo >> 6;
    int32_t moo364 = moo64 * moo64 * moo64;
    altitude += (moo >> 12) + (moo >> 17) + (moo >> 18); //1st term, extra precission
    altitude += (moo364 >> 11) + (moo364 >> 13) + (moo364 >> 17) + (moo364 >> 18) + (moo364 >> 21); //3rd term for extra precission
#endif

    return altitude;
}

/*
return altitude in decimeters based on standard sea level pressure as 16 bit integer
for super duper low memory footprint, if you need to store a lot of data in RAM
but your altitude will be limited to about 3.2km
Comparison of  accuracy of this method to using precise calculation using pow()

pressure    pow()     this      difference

105005 Pa		-301 m		-299 m		2.9 m
100010 Pa		110 m 		110 m 		0.4 m
95015 Pa		539 m 		539 m 		0.0 m
90020 Pa		986 m 		986 m 		-0.2 m
85025 Pa		1455 m		1453 m		-1.9 m
80030 Pa		1946 m		1938 m		-7.4 m
75035 Pa		2462 m		2443 m		-19.2 m
70040 Pa		3008 m		2967 m		-40.8 m
above that, int16 overflows
*/

int16_t tinyBMP085::readAltitudeSTDdm(void) {
    int32_t pressure = readPressure();
    int32_t moo = (int32_t)95000 - pressure;
    int16_t altitude = 5404; //0th term
    altitude += ((28742 * moo) >> 15); //1st term
    altitude += ((moo * moo) >> 18); //2nd term

    return altitude;
}

uint16_t tinyBMP085::readAltitudeSTDdm2(void) {
    int32_t pressure = readPressure();
    int32_t moo = (int32_t)95000 - pressure;
    uint16_t altitude = 5404;
    altitude += ((28742 * moo) >> 15);
    altitude += ((moo * moo) >> 18);

    return altitude;
}

uint8_t tinyBMP085::read8(uint8_t a) {
    uint8_t ret;

    TinyWireM.beginTransmission(BMP085_I2CADDR); // start transmission to device 
    TinyWireM.send(a); // sends register address to read from
    TinyWireM.endTransmission(); // end transmission

    TinyWireM.beginTransmission(BMP085_I2CADDR); // start transmission to device 
    TinyWireM.requestFrom(BMP085_I2CADDR, 1);// send data n-bytes read
    ret = TinyWireM.receive(); // receive DATA
    //TinyWireM.endTransmission(); // end transmission 

    return ret;
}

uint16_t tinyBMP085::read16(uint8_t a) {
    uint16_t ret;

    TinyWireM.beginTransmission(BMP085_I2CADDR); // start transmission to device 
    TinyWireM.send(a); // sends register address to read from
    TinyWireM.endTransmission(); // end transmission

    TinyWireM.beginTransmission(BMP085_I2CADDR); // start transmission to device 
    TinyWireM.requestFrom(BMP085_I2CADDR, 2);// send data n-bytes read
    ret = TinyWireM.receive(); // receive DATA
    ret <<= 8;
    ret |= TinyWireM.receive(); // receive DATA
    //TinyWireM.endTransmission(); // end transmission

    return ret;
}

void tinyBMP085::write8(uint8_t a, uint8_t d) {
    TinyWireM.beginTransmission(BMP085_I2CADDR); // start transmission to device 
    TinyWireM.send(a); // sends register address to read from
    TinyWireM.send(d);  // write data
    TinyWireM.endTransmission(); // end transmission
}
