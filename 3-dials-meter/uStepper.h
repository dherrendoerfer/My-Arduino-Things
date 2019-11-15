/*
 * This file is part of uCNC_controller.
 *
 * Copyright (C) 2014  D.Herrendoerfer
 *
 *   uCNC_controller is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   uCNC_controller is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with uCNC_controller.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Code in this file is derived from the Arduino stepper library by:
 *    Tom Igoe, Sebastian Gassner,David Mellis, and Noah Shibley
 *  licensed under: Creative Commons Attribution-ShareAlike 3.0 License.
 *  Credit and authorship remains with the original authors.
 *
 * This source file is duplicated to faciliate the adaptation to look-ahead
 * and acceleration code regarding the use in CNC type machines.
 */

/* README:
This file contains the drivers for 3 types of stepper controllers.
By choosing the appropriate constructor you choose the driver.
Available are:

2Pin: Bipolar driver for double-h bar driver chips
3pin: Bipolad driver for A4988 or similar chips with direction and step 
      interface and enable
4pin: Unipolar stepper driver for use with ULN2003/2004 driver ICs
*/

/* Typedefs */
typedef struct {
  uint8_t pins;
  uint8_t pin1;
  uint8_t pin2;
  uint8_t pin3;
  uint8_t pin4;
  uint8_t slack;
  uint8_t halfstep;
  uint16_t holdoff_in_micros;

  uint8_t powerstate;
  int32_t cur_position;
  int8_t last_direction;
  uint32_t last_step_micros;
  uint32_t next_step_micros;
  } ustepper_stepper_t;

/* Global variables */
static uint8_t ustepper_step_p1[] = {HIGH, LOW, LOW, LOW, LOW, LOW,HIGH,HIGH};
static uint8_t ustepper_step_p2[] = { LOW, LOW,HIGH,HIGH,HIGH, LOW, LOW, LOW};
static uint8_t ustepper_step_p3[] = {HIGH,HIGH,HIGH, LOW, LOW, LOW, LOW, LOW};
static uint8_t ustepper_step_p4[] = { LOW, LOW, LOW, LOW,HIGH,HIGH,HIGH, LOW};

/* Helpers */
inline int sgn(int32_t value)
{
  return value < 0 ? -1 : 1;
}

inline int sgn_tri(int32_t value)
{
  if (value == 0)
    return 0;
  return sgn(value);
}

void ustepper_init(ustepper_stepper_t *stepper, uint8_t pins, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t halfstep, uint16_t holdoff)
{
  stepper->pins = pins;
  stepper->last_step_micros = micros();
  stepper->holdoff_in_micros = holdoff;
  stepper->next_step_micros = stepper->last_step_micros + holdoff;
  stepper->last_direction = 0;
  stepper->cur_position = 0;

  stepper->halfstep = halfstep;

  stepper->powerstate = 0;

  stepper->pin1=pin1;
  stepper->pin2=pin2;
  pinMode(stepper->pin1, OUTPUT);
  pinMode(stepper->pin2, OUTPUT);

  if (pins > 2) {
    stepper->pin3=pin3;
    pinMode(stepper->pin3, OUTPUT);
  }
  if (pins > 3) {
    stepper->pin4=pin4;
    pinMode(stepper->pin4, OUTPUT);
  }
}


void ustepper_set(ustepper_stepper_t *stepper, uint32_t step)
{
  stepper->cur_position = step;
}


void ustepper_apply(ustepper_stepper_t *stepper)
{
  int32_t thisStep;

  if (stepper->pins == 2) {
    thisStep = abs(stepper->cur_position % 4);
    
    digitalWrite(stepper->pin1, ((thisStep & 0x01) == 0));
    digitalWrite(stepper->pin2, ((thisStep & 0x02) == 0));
    return;
  }

  if (stepper->pins == 4) {
    if (stepper->halfstep)
      thisStep = ((uint32_t)stepper->cur_position % 8);
    else 
      thisStep = 2 * (((uint32_t)stepper->cur_position % 4));
    
    digitalWrite(stepper->pin1, ustepper_step_p1[thisStep]);
    digitalWrite(stepper->pin2, ustepper_step_p2[thisStep]);
    digitalWrite(stepper->pin3, ustepper_step_p3[thisStep]);
    digitalWrite(stepper->pin4, ustepper_step_p4[thisStep]);
  }
}   

/*
 * Moves the motor forward or backwards.
 */
void ustepper_step(ustepper_stepper_t *stepper, int step)
{
  stepper->cur_position += step;
  stepper->last_step_micros = micros();
  stepper->next_step_micros = stepper->last_step_micros + stepper->holdoff_in_micros;
  stepper->last_direction = step;

  if (stepper->pins == 3) {
    /*Enable stepper driver*/
    digitalWrite(stepper->pin3, LOW);
    /*Set direction pin*/
    digitalWrite(stepper->pin1,(step==1));
    /*Pulse step line*/
    digitalWrite(stepper->pin2, HIGH);
    delayMicroseconds(1);
    digitalWrite(stepper->pin2, LOW);
    return;
  }
  else {
    ustepper_apply(stepper);
  }
}

void ustepper_power(ustepper_stepper_t *stepper, uint8_t power)
{
  stepper->powerstate=power;  

  if ( power ){
    if (stepper->pins == 3) {
        /* Pull enable pin LOW*/
        digitalWrite(stepper->pin3, LOW);
    }
    else {
      ustepper_apply(stepper);
    }
  }
  else {
    digitalWrite(stepper->pin1, LOW);
    digitalWrite(stepper->pin2, LOW);

    if (stepper->pins > 2) {
      /* Pull enable pin HIGH*/
      digitalWrite(stepper->pin3, HIGH);
    }
    if (stepper->pins == 3) {
        digitalWrite(stepper->pin4, LOW);
    }
  }
}

