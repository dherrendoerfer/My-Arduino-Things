/*
* Author: D. Herrendoerfer
* License: GPLv2
*
* Project: Use sandwich toaster oven for reflow processing
*/

int this_power = 0;

void do_oven()
{
  sim_heat(power);
  if (power && heating) {
    if (cycle == 0) {
      digitalWrite(HEATER,HIGH);
      this_power = power;
    }

    if (cycle > this_power && cycle > MIN_ON)
      digitalWrite(HEATER,LOW);
       
    cycle++;
    if (cycle == 100)
      cycle = 0;
  
  } else {
    digitalWrite(HEATER,LOW);
    cycle = 0;
  }
}
