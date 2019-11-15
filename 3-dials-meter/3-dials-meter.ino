/*
 * Licensed under the Creative Commons Zero (CC0) 
 */

#include "uStepper.h"

#include <BME280I2C.h>
#include <Wire.h>

#define SERIAL_BAUD 9600
BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,

ustepper_stepper_t mySteppers[3]; 

void setup() {
  // put your setup code here, to run once:

  ustepper_init(&mySteppers[0], 4,  7,9, 8,10, 0, 3000);
  ustepper_init(&mySteppers[1], 4,  3, 5, 4, 6, 0, 3000);
  ustepper_init(&mySteppers[2], 4, 17,15,16,14, 0, 3000);

  Wire.begin();
  Serial.begin(9600);

  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }

  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }
}


/*
 Find the zero position by doing a full wipe counter-clockwise.
 Dial needs to run into the block.
*/
void initMeter(ustepper_stepper_t stepper[], uint8_t dials)
{
  int i;  
  int dial;

  for (dial=0;dial<dials;dial++)
    ustepper_power(&mySteppers[dial],1);
  for (i=0;i<1600;i++) {
    for (dial=0;dial<dials;dial++)
      ustepper_step(&mySteppers[dial],-1);
    delayMicroseconds(2000);
  }
  for (dial=0;dial<dials;dial++)
    ustepper_power(&mySteppers[dial],0);

  for (dial=0;dial<dials;dial++)
    ustepper_set(&mySteppers[dial],0);
  
  delay(300);

  for (dial=0;dial<dials;dial++)
    ustepper_power(&mySteppers[dial],1);
  for (i=0;i<785;i++) {
    for (dial=0;dial<dials;dial++)
      ustepper_step(&mySteppers[dial],1);
    delayMicroseconds(2000);
  }
  for (dial=0;dial<dials;dial++)
    ustepper_power(&mySteppers[dial],0);
  
  delay(800);
}




void loop() {
  // put your main code here, to run repeatedly:

  float temp(NAN), hum(NAN), pres(NAN);
  uint32_t t1,t2;

  uint16_t pos1 = 0;
  uint16_t pos2 = 0; 
  uint16_t pos3 = 0;
  uint16_t val1, val2, val3;
  uint16_t counter;

  initMeter(&mySteppers[0], 3);

  pos1 = mySteppers[0].cur_position;
  pos2 = mySteppers[1].cur_position; 
  pos3 = mySteppers[2].cur_position;

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  bme.read(pres, temp, hum, tempUnit, presUnit);
  delay(100);

  while (true)
  {
    if (counter % 50 == 0) {
      bme.read(pres, temp, hum, tempUnit, presUnit);

      val1 = (temp+7.5) * (1570 / 45);
      val2 = (hum-5) * (1570 / 90);
      val3 = ((pres/1000.0)-95) * (1570 / 10);
      delay(17);

      // Hard Limits
      if (val1 > 785)
        val1 = 785;
      else if (val1 < -785)
        val1 = -785;

      if (val2 > 785)
        val2 = 785;
      else if (val2 < -785)
        val2 = -785;

      if (val3 > 785)
        val3 = 785;
      else if (val3 < -785)
        val3 = -785;

    }
    else
      delay(20);

    counter++;

    if (pos1 < val1){
      ustepper_step(&mySteppers[0],1);
      pos1++;
    }
    else if (pos1 > val1){
      ustepper_step(&mySteppers[0],-1);
      pos1--;
    }
    else if (pos1 == val1){
      ustepper_power(&mySteppers[0],0);
    }

    if (pos2 < val2){
      ustepper_step(&mySteppers[1],1);
      pos2++;
    }
    else if (pos2 > val2){
      ustepper_step(&mySteppers[1],-1);
      pos2--;
    }
    else if (pos2 == val2){
      ustepper_power(&mySteppers[1],0);
    }

    if (pos3 < val3){
      ustepper_step(&mySteppers[2],1);
      pos3++;
    }
    else if (pos3 > val3){
      ustepper_step(&mySteppers[2],-1);
      pos3--;
    }
    else if (pos3 == val3){
      ustepper_power(&mySteppers[2],0);
    }

    if (counter % 300 == 0){
      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print("°"+ String(tempUnit == BME280::TempUnit_Celsius ? 'C' :'F'));
      Serial.print("\t\tHumidity: ");
      Serial.print(hum);
      Serial.print("% RH");
      Serial.print("\t\tPressure: ");
      Serial.print(pres);
      Serial.println(" Pa");
      Serial.println(t2-t1);
      Serial.print(val1);
      Serial.print("->");
      Serial.println(pos1);
      Serial.print(val2);
      Serial.print("->");
      Serial.println(pos2);
      Serial.print(val3);
      Serial.print("->");
      Serial.println(pos3);
    }
  }
}
