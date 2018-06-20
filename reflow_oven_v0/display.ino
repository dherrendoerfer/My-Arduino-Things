/*
* Author: D. Herrendoerfer
* License: GPLv2
*
* Project: Use sandwich toaster oven for reflow processing
*/

void do_display()
{
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  if (prog_waiting)
  {
    lcd.setCursor(0, 0);
    lcd.print("Press select key");
    lcd.setCursor(0, 1);
    lcd.print("  to continue.");
    return;
  }

  if (count % 5 == 0) {
    lcd.setCursor(0, 0);
    lcd.print("\"");
    lcd.print(progname);
    lcd.print("\"");
  } else {
    lcd.setCursor(0, 0);
    if (waiting)
      lcd.print("temp  wait ");
    else
      lcd.print("temp  trgt ");
  }
  
  if (heating){
    lcd.setCursor(14, 0);
    lcd.print("On");
    lcd.setCursor(12, 1);
    lcd.print(power);
    lcd.print("%");
  } else {
    lcd.setCursor(13, 0);
    lcd.print("Off");
  }

  lcd.setCursor(0, 1);
  if(temperature < 0) {                   
    // If there is an error with the TC, temperature will be < 0
    lcd.print("!!error!!");
  } else {
    lcd.print(temperature,1);
    /*
    Serial.print(count);
    Serial.print(",");
    Serial.print(temperature);
    Serial.print(",");
    Serial.print(heating);
    Serial.print(",");
    Serial.println(power);
    */
  }        

  lcd.setCursor(6, 1);
  if (waiting) {
    lcd.print(countdown);
    lcd.print("s");
  } else { 
    lcd.print(prog_temp,1);
  }

}

void serialLogInit(int currentStep)
{
  Serial.println("# Starting control loop ");
  Serial.print("#  Program name: ");
  Serial.println(progname);
  Serial.println("#");
  Serial.println("# New settings are: ");
  Serial.print("#   Kp = ");
  Serial.println(Kp[currentStep], 2);
  Serial.print("#   Ki = ");
  Serial.println(Ki[currentStep], 2);
  Serial.print("#   Kd = ");
  Serial.println(Ki[currentStep], 2);
  Serial.print("#   Target  = ");
  Serial.println(target_temp[currentStep], 2);
  Serial.print("#   Current = ");
  Serial.println(temperature, 2);
  Serial.println("# ------------------------ ");
  Serial.println("# Legend: ");
  Serial.println("# seconds, status, temperature, target, error, i_error, d_error, P_part, I_part, D_part, PID_sum ");
}
