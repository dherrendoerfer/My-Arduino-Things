/*
* Author: D. Herrendoerfer
* License: GPLv2
*
* Project: Dim light with SSR by synchronizing Arduino to AC
*/

unsigned long timeus = 0;
unsigned long nexttick = 0;
unsigned long time10us = 0;
int d12 = 0;

void setup()
{ 
  pinMode(13,OUTPUT);
  pinMode(12,INPUT);

  digitalWrite(13,HIGH);
  delay(3000);
  digitalWrite(13,LOW);

  timeus = micros();
  nexttick = timeus + 100;
}

void loop ()
{
  timeus = micros();

  if (digitalRead(12) != d12) {
    if (!d12) {
      time10us = 25;  //set time to 250us since phase start
      //nexttick = timeus + 100;
      digitalWrite(7,HIGH);
      d12 = 1;
    }
    else {
      d12 = 0;
      digitalWrite(7,LOW);
    } 
  }

  if ( ((long)nexttick-(long)timeus) > 0 )
    return;
  
  nexttick = timeus + 100;

  
  if (time10us == 190)
    digitalWrite(13,HIGH);

  if (time10us == 50)
    digitalWrite(13,LOW);

  if (time10us > 199)
    time10us = 0;
  else
    time10us++;

}
