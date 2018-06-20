/*
* Author: D. Herrendoerfer
* License: GPLv2
*
* Project: Use sandwich toaster oven for reflow processing
*/

void read_keypad()
{
  i_key = analogRead(A0);
  
  if (i_key < 1000) {
    p_key = i_key;
  } else {
    if (p_key < 1000) {
      key = p_key;
      p_key = 1024;
      //Serial.println(key);
    }
  }
}

