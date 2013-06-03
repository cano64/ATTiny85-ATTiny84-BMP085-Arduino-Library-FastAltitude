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
 * -~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-~-
 * 
 * Modified Feb 2012 by John De Cristofaro / johngineer for use on
 * ATTiny micros running the TinyWire library (TinyWireM).
 * -------------------------------------------------------
 * modified by Michal Canecky/Cano 2013-05-05
 * -calculation of altitude without using pow() and math library
 * -calculation of altitude using only integers 
 * 	(fixed for standard sea level pressure)
 * 
 ****************************************************/

#include <stdint.h>
#include <TinyWireM.h>

#define ALTITUDE_EXTRA_PRECISSION 0

#define BMP085_DEBUG 0

#define BMP085_I2CADDR 0x77

#define BMP085_ULTRALOWPOWER 0
#define BMP085_STANDARD      1
#define BMP085_HIGHRES       2
#define BMP085_ULTRAHIGHRES  3
#define BMP085_CAL_AC1           0xAA  // R   Calibration data (16 bits)
#define BMP085_CAL_AC2           0xAC  // R   Calibration data (16 bits)
#define BMP085_CAL_AC3           0xAE  // R   Calibration data (16 bits)    
#define BMP085_CAL_AC4           0xB0  // R   Calibration data (16 bits)
#define BMP085_CAL_AC5           0xB2  // R   Calibration data (16 bits)
#define BMP085_CAL_AC6           0xB4  // R   Calibration data (16 bits)
#define BMP085_CAL_B1            0xB6  // R   Calibration data (16 bits)
#define BMP085_CAL_B2            0xB8  // R   Calibration data (16 bits)
#define BMP085_CAL_MB            0xBA  // R   Calibration data (16 bits)
#define BMP085_CAL_MC            0xBC  // R   Calibration data (16 bits)
#define BMP085_CAL_MD            0xBE  // R   Calibration data (16 bits)

#define BMP085_CONTROL           0xF4 
#define BMP085_TEMPDATA          0xF6
#define BMP085_PRESSUREDATA      0xF6
#define BMP085_READTEMPCMD       0x2E
#define BMP085_READPRESSURECMD   0x34


class tinyBMP085 {
public:
    tinyBMP085();
    uint8_t begin(uint8_t mode = BMP085_ULTRAHIGHRES);  // by default go highres

    float readTemperature(void);
    int16_t readTemperature10C(void);

    int32_t readPressure(void);
    float readAltitude(int32_t sealevelPressure = 101325); // std atmosphere by default
    int32_t readAltitudemm(int32_t sealevelPressure = 101325); // std atmosphere by default
    int32_t readAltitudeSTDmm(); // std atmosphere, fast approximation using only ints
    int16_t readAltitudeSTDdm(); // std atmosphere, fast approx, 16 bits, (-3.2km to 3.2km)
    uint16_t readAltitudeSTDdm2(); // std atmosphere, fast approx, 16 bits, (0.5km to 6.5km)

    uint16_t readRawTemperature(void);
    uint32_t readRawPressure(void);

private:
    uint8_t read8(uint8_t addr);
    uint16_t read16(uint8_t addr);
    void write8(uint8_t addr, uint8_t data);

    uint8_t oversampling;

    int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
    uint16_t ac4, ac5, ac6;
};


