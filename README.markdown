This is an ATTiny library for BMP085 Barometric Pressure and Temperature sensor.
Written by Michal Canecky/Cano based on library by John De Cristofaro / johngineer
based on library by Adafruit. It's designed for ATTiny microcontrollers like ATTiny85 and ATTiny84
and requires TinyWireM library for communication with the sensor

	This library is calculating altitudes
	without using pow() function and math library
	thus minimizing sketch size by about 1200 bytes
	and optionally without the use of floats altogether
	cutting another 900 bytes down, magic :)

It is done by approximation using Taylor Series, centered at 500m altitude
As you go higher (or lower) the error offset is higher, but still negligible.
The whole calculation will break at altitudes about 5-6km, but who lives there anyway :)

Uncalibrated (fixed for standard sea level pressure) function for calculating altitude.

	int32_t readAltitudeSTDmm()

is a small and fast function you will normally use to calculate altitude
using standard sea level pressure as a reference.
Please note that your reading may be offset up to 100 meters based on weather,
depending whether it's sunny or raining. It's good for calculating relative changes in altitude.
If you need a calibrated measurement and have a method of entering
the current sea level pressure from your local weather report every time you use your device
use the following function to calculate your real altitude

	int32_t readAltitudemm(int32_t sealevelPressure = 101325)

if you are not planning to use your device above 2000m you can disable higher precision,
saving another 200 bytes of your sketch size. 


	#define ALTITUDE_EXTRA_PRECISSION 0
   
in .h file

	Other functions in the library
	
	int readPressure(); //returns current pressure in Pa
	float readTemperature(); //returns current temperature in C
	int readTemperature10C(); //returns current temperature in tenths of C
	float readAltitude(); //left for compatibility with old library, returns altitude in m
	int readAltitudemm(); //returns current altitude in mm, calibrated for your local SLP
	int readAltitudeSTDmm(); //returns current altitude in mm, fixed for SSLP
	
