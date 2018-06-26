/*
* Author: D. Herrendoerfer
* License: GPLv2
*
* Project: Use sandwich toaster oven for reflow processing
*
* Connections are:
* For the LCD:  PIN4
*               PIN5
*               PIN6
*               PIN7
*               PIN8
*               PIN9
*
* For the temp-probe:
*               PIN2 (SCK)
*               PIN3 (CS)
*               PIN11 (SO)
*
* For the SSR to the Heater:
*               PIN13 
*/


#include <LiquidCrystal.h>
#include <MAX6675.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define SCKu 2             // SCK pin of MAX6675
#define CS 3               // CS pin on MAX6675
#define SO 11              // SO pin of MAX6675
#define units 1            // Units to readout temp (0 = raw, 1 = ˚C, 2 = ˚F)
float temperature = 0.0;   // Temperature output variable

#define HEATER 13
#define MIN_ON 5         

#define MAX_STEPS 12

// Initialize the MAX6675 Library for our chip
MAX6675 temp(CS,SO,SCKu,units);

void setup() {
  Serial.begin(115000); //Fast, logging is expensive
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("OvenControl 1.0");

  pinMode(HEATER, OUTPUT);
}


int stop = 0;
int d_mode = 1;   //Display mode

int count = 0;
int heating = 0;
int waiting = 0;
int power = 0;    // heating power in %
int cycle = 0;

char progname[32] = "";

/* PID Processing

Kp = Proptional Constant.
Ki = Integral Constant.
Kd = Derivative Constant.
err = Expected Output - Actual Output ie. error;
ier  = ier from previous loop + err; ( i.e. integral error )
der  = err - err from previous loop; ( i.e. differential error)
dt = execution time of loop.

output = Kp * err + (Ki * ier * dt) + (Kd * der /dt);
*/

float Kp[MAX_STEPS] = {2.0};
float Ki[MAX_STEPS] = {0.03};
float Kd[MAX_STEPS] = {0.7};

int steps = 1;
int action[MAX_STEPS];
int wait[MAX_STEPS] = {0};

float p_level[MAX_STEPS] = {1.0};
float target_temp[MAX_STEPS] = {0};
float window_size[MAX_STEPS] = {0};

int prog_step = 0;

float err = 0;
float err_prev = 0;
float ier = 0;
float ier_prev = 0;
float der = 0;

int pid_init = 1;

float dt = 1;

int prog_ready = 0;
int prog_done  = 0;
float prog_temp = 0.0;
float prog_window = 0.0;
int   prog_wait = 0;
int   prog_action = 0;
int prog_waiting = 0;

unsigned long time = millis();
unsigned long next_time_display = time + 3500;  //Spread the events somewhat
unsigned long next_time_key     = time + 3200;
unsigned long next_time_power   = time + 4975;
unsigned long next_time_sec     = time + 1000;
unsigned long seconds = 0;
unsigned long countdown = 0;


//
// The programs, as they are called can set all elements
// of the FSM that the controller is running
//

void off() 
{
  steps = 1;
  action[0] = 0;
  wait[0] = 0;

  target_temp[0] = 0;
  Kp[0] = 0.0;
  Ki[0] = 0.0;
  Kd[0] = 0.0;
  
  p_level[0] = 0.0;
  window_size[0] = 1.0;

  sprintf(progname,"off");
  pid_init = 1;

  serialLogInit(0);
}

void preheat() 
{
  steps = 1;
  action[0] = 0;
  wait[0] = 0;
  
  target_temp[0] = 40;
  Kp[0] = 2.0;
  Ki[0] = 0.007;
  Kd[0] = 3.0;
  
  p_level[0] = 0.7;
  window_size[0] = 1.0;

  sprintf(progname,"preheat");
  pid_init = 1;

  serialLogInit(0);
}

void heat() 
{
  steps = 1;
  action[0] = 0;
  wait[0] = 0;

  target_temp[0] = 150;
  Kp[0] = 1.2;
  Ki[0] = 0.05;
  Kd[0] = 3.0;
  
  p_level[0] = 1.0;
  window_size[0] = 0.0;

  sprintf(progname,"heat");
  pid_init = 1;
  
  serialLogInit(0);
}

void curve_215() 
{
  int s=0;

  // Step 0:
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 40;
  Kp[s] = 3.0;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;

  // Step 1:
  action[s] = 1;
  wait[s] = 0;
  target_temp[s] = 40;
  Kp[s] = 3.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;

  // Step 2: 100 deg start
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 100;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 2.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 3: 120 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 120;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 4: 130 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 130;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 5: 135 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 135;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 6: 140 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 140;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 7: 145 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 145;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 8: 150 
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 150;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;

  // Step 9: 225 10s 
  action[s] = 2;
  wait[s] = 10;
  target_temp[s] = 225;
  Kp[s] = 5.0;
  Ki[s] = 0.007;
  Kd[s] = 3.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;

  // Step 10: 0  
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 0;
  Kp[s] = 5.0;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;
  
  steps = s;
  sprintf(progname,"curve-215");
  pid_init = 1;

  serialLogInit(0);
}

void curve_240() 
{
  int s=0;

  // Step 0:
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 40;
  Kp[s] = 4.0;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;

  // Step 1:
  action[s] = 1;
  wait[s] = 0;
  target_temp[s] = 40;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;

  // Step 2: 150 deg start
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 150;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 3: 155 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 155;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 4: 160 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 160;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 5: 165 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 165;
  Kp[s] = 4.5;
  Ki[s] = 0.07;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 6: 170 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 170;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 7: 175 30s
  action[s] = 2;
  wait[s] = 30;
  target_temp[s] = 175;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 8: 180 
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 180;
  Kp[s] = 4.5;
  Ki[s] = 0.007;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 9: 255 10s 
  action[s] = 2;
  wait[s] = 10;
  target_temp[s] = 255;
  Kp[s] = 8.0;
  Ki[s] = 0.007;
  Kd[s] = 3.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;

  // Step 10: 255 10s 
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 0;
  Kp[s] = 10.0;
  Ki[s] = 0.0;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 0.0;
  s++;
  
  steps = s;

  sprintf(progname,"curve-240");
  pid_init = 1;

  serialLogInit(0);
}

void test() 
{
  int s=0;
  
  // Step 1:
  action[s] = 5;
  wait[s] = 20;
  target_temp[s] = 250;
  Kp[s] = 4.0;
  Ki[s] = 0.01;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 2:
  action[s] = 1;
  wait[s] = 0;
  target_temp[s] = 230;
  Kp[s] = 4.0;
  Ki[s] = 0.01;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;

  // Step 3:
  action[s] = 0;
  wait[s] = 0;
  target_temp[s] = 230;
  Kp[s] = 4.0;
  Ki[s] = 0.01;
  Kd[s] = 0.0;
  p_level[s] = 1.0;
  window_size[s] = 1.0;
  s++;
  
  steps = s;
  sprintf(progname,"test");
  pid_init = 1;

  serialLogInit(0);
}

//
// The mode selector for the running programs from above
// you can add any here, and implement above
//

int mode = 2;

void do_mode(int tmode)
{
  switch(tmode){
    case 0:
      // Lower LIMIT - ignore
      mode=1;
      break;
    case 1:
      // off
      off();
      break;
    case 2:
      // preheat
      preheat();
      break;
    case 3:
      // heat
      heat();
      break;
    case 4:
      // curve
      curve_215();
      break;
    case 5:
      // curve
      curve_240();
      break;
    case 6:
      // test
      test();
      break;
    case 7:
      // Upper LIMIT - ignore
      mode=6;
      break;
  }
  
  // reset the fsm
  prog_ready = 0;
  prog_step = 0;
  countdown = 0;
  waiting = 0;
  heating = 0;
}

int i_key = 1024;
int p_key = 1024;
int key = 1024; // Input buttons

void loop()
{
  // preselect settings 1st time.
  do_mode(mode);
  
  // Scheduler 
  while (!stop) {
    time = millis();

    // Scheduling:
    if ((long)(time - next_time_power) > 0) {
      next_time_power += 50; // repeat every 50ms
      do_oven();
      continue;
    }

    if ((long)(time - next_time_key) > 0) {
      next_time_key += 100; // repeat every 100ms
      read_keypad();
      continue;
    }

    if ((long)(time - next_time_sec) > 0) {
      next_time_sec += 1000; // repeat every 1000ms
      seconds++;
      if (countdown > 0)
        countdown--;
      continue;
    }
    
    // User interface:
    if ((long)(time - next_time_display) > 0 || key != 1024) {
      
      if (key == 1024)
        next_time_display += 1000;
      else
        next_time_display = millis() + 2000; 

      temperature = temp.read_temp();
//      temperature = sim_read();

      // Pass through the heating FSM handler 
      prog_heating();      
      
      if (!prog_waiting)
      {
        if (key == 0) { // Left and right keys switch mode
          // right key
          mode++;
          do_mode(mode);
          count=0;
          prog_ready = 0;
        }
        if (key > 400 && key < 415) {
          // left key
          mode--;
          do_mode(mode);
          count=0;
          prog_ready = 0;
        }

        if (d_mode == 1) {  // Up and down keys for display mode 1
          if (key > 97 && key < 105) {
            // up key
            prog_temp += 5.0;
          }
          if (key > 245 && key < 260) {
            // down key
            prog_temp -= 5.0;
          }
        }
      }

      if (key > 635 && key < 645) {
        // select key
        prog_waiting = 0;
      }
      
      if (d_mode == 1) { // Display mode 1 
        do_display();
      }
      
      count++;
      key = 1024; //reset keys
    }
  }  
}
