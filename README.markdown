This is an ATTiny library for BMP085 Barometric Pressure and Temperaure sensor.
Written by Michal Canecky/Cano based on library by John De Cristofaro / johngineer 
based on library by Adafruit. It's designed for ATTiny microcontrollers like ATTiny85 and ATTiny84
and requires TinyWireM library

	This library is calculating altitudes 
	without using pow() function and math library 
	thus minimizing sketch size by about 1200 bytes
	and optionally without the use of floats either
	cutting another 1100 bytes down, magic :)

It is done by approximation using Taylor Series, centered at the sea level. 
As you go higher (or lower) the error offset is higher.
	At 1000m the offset is about 3m
	At 2000m the offset is about 22m
	At 3000m the offset is about 80m

Uncalibrated (set for standard sea level pressure) function for calculating altitude
without using floats is (will be) available as well saving another 1100 bytes of sketch size

	int32_t readAltitudeSTDmm() 
	
is a small and super fast function you will normally use for calculating altitude 
using standart sea level pressure as a refference. 
Please note that your reading may be offset up to 100 meters based on weather, 
depending whether it's sunny or raining. But that's all right unless you want to enter current
sea level pressure from your local wether report every time you use your device.
If you know your current sea level pressure you can use the following function to
calculate your real altitude

	int32_t readAltitudemm(int32_t sealevelPressure = 101325)

