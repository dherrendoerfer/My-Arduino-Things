/*
* Author: D. Herrendoerfer
* License: GPLv2
*
* Project: Use sandwich toaster oven for reflow processing
*/


int loopCount = 0;

float calcPID(float temp, float target, int step_index)
{
  float output;

  if (pid_init) {
    ier_prev = 0;
    err_prev = target - temp;
//    err_prev = 0;
    pid_init = 0;
    loopCount = 1;
  }

  err = target - temp;
  ier = ier_prev + err;
  der = err_prev -err;
  
  float P_part = Kp[step_index] * err;
  float I_part = Ki[step_index] * ier * dt;
  float D_part = Kd[step_index] * der / dt;
  
  Serial.print(loopCount);
  Serial.print(", ");
  Serial.print( power?"1, ":"0, ");
  Serial.print(temp,2);
  Serial.print(", ");
  Serial.print(target);
  Serial.print(", ");
  Serial.print(err,2);
  Serial.print(", ");
  Serial.print(ier,2);
  Serial.print(", ");
  Serial.print(der,2);
  Serial.print(", ");
  Serial.print(P_part,2);
  Serial.print(", ");
  Serial.print(I_part,2);
  Serial.print(", ");
  Serial.print(D_part,2);
  
  output = P_part + I_part + D_part;

  Serial.print(", ");
  Serial.println(output,2);
  
  err_prev = err;
  ier_prev = ier;
  
  output = output * p_level[step_index];
  
  loopCount++;
  return output;
}

float do_adjust(float oventemp)
{
  //Return a power value equal to that neede to keep the oven at same temp
  
  return oventemp / 3.0;
}

void resetPID() 
{
  pid_init = 1;
}

int do_temp()
{
  if (prog_temp  == 0 || temperature > 290) {
    // safety off
    heating = 0;
    power = 0; 
    
    return 0;
  }
    
  if ((int)temperature > 0) {
    power = (int) calcPID(temperature, prog_temp, prog_step);
//    power += (int) do_adjust(temperature);

    if (power >= 0){
      if (power > 100)
        power=100;
      if (power < 0)
        power=0;

      heating = 1;
    } else {
      power = 0;
      heating = 0;
    }
  } else {
    heating = 0;  // No heating if the sensor is not working.
  }

  if ((int)temperature >= (prog_temp-prog_window)) {
    return 1;  // report target reached. 
  }
  
  return 0;
}

void next_step() 
{
  if (steps > prog_step + 1) {
    prog_step++;
    //resetPID();
    prog_ready = 0;
  }
}

void prog_heating()
{
  // Load a program step
  if (!prog_ready){
    prog_temp   = target_temp[prog_step];
    prog_action = action[prog_step];
    prog_wait   = wait[prog_step];
    prog_window = window_size[prog_step];
    prog_ready = 1;
  }
  
  // Action:
  // 0 : just heat
  // 1 : wait for select key
  // 2 : heat,hold,wait time
  // 3 : heat,hold, wait for select key
  if (prog_action == 0) {
    if (do_temp()) {
      next_step();
    }
  } 

  if (prog_action == 1) {
      prog_action = 10;
      prog_waiting = 1;
  } 

  if (prog_action == 10) {
    if (prog_waiting == 0) {
      next_step();
    }
  }

  if (prog_action == 2) {
    if (do_temp()) {
      countdown = prog_wait;
      prog_action = 40;
      waiting = 1;
    }
  } 

  if (prog_action == 3) {
    if (do_temp()) {
      countdown = prog_wait;
      prog_action = 30;
      prog_waiting = 1;
    }
  } 

  // Hold & wait for select
  if (prog_action == 30) {
    do_temp();
    if (prog_waiting == 0) {
      next_step();
    }
  }

  // Hold & wait
  if (prog_action == 40) {
    do_temp();
    if (countdown == 0) {
      waiting = 0;
      next_step();
    }
  }

  if (prog_action == 5) {
    countdown = prog_wait;
    prog_action = 50;
    waiting = 1;
  }

  // just wait
  if (prog_action == 50) {
    if (countdown == 0) {
      waiting = 0;
      next_step();
    }
  }  
}
