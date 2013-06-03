#include <TinyWireM.h>
#include <tinyBMP085.h>
tinyBMP085 bmp;
  
void setup() {
  bmp.begin();  
}
  
void loop() {

    int pres = bmp.readPressure();
    int temp = bmp.readTemperature10C();
    int alt = bmp.readAltitude();
    int altMM = bmp.readAltitudemm();
    int altSTDdm = bmp.readAltitudeSTDdm();
    
    delay(500);
}