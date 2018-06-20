/*
* Author: D. Herrendoerfer
* License: GPLv2
*
* Project: Use sandwich toaster oven for reflow processing
*/

float sim_temp = 20.0;

float sim_read()
{
  return sim_temp;
}

void sim_heat(int power)
{
  sim_temp += power / 500.0;
  
  if (!power)
    sim_temp -= sim_temp * 0.00096;
}
