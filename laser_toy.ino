#include <Servo.h>
#include <Wire.h>
#include "nunchuck_funcs.h"

// X/Y constants
const int X = 0;
const int Y = 1;

//
// Servo data
//

// Servo Constants for equipment protection
const int X_MIN = 0;
const int X_MAX = 160;
const int Y_MIN = 0;
const int Y_MAX = 125;

const int S_MIN[] = {0, 0};
const int S_MAX[] = {160, 125}; 

// Servo pin assignments
int x_pin = 6;
int y_pin = 7;
const int S_PIN[] = {6, 7};

// Default servo values
int new_x = 80;
int new_y = 80;
int s_center_pos[] = {(S_MIN[X] + S_MAX[X] / 2, S_MIN[Y] + S_MAX[Y] / 2};

// Define Servos
Servo servo[2];

//
// Laser data
// 

bool laser_on = false;
const int LASER_PIN = 3;

//
// Nunchuck data
//

// Reasonable defaults for minimum and maximum integers for range finding
int min_y = 40;
int max_y = 210;
int min_x = 40;
int max_x = 210;

int n_min[] = {40, 40};
int n_max[] = {210, 210};

int n_center_pos[] = {(n_min[X] + n_max[X] / 2, n_min[Y] + n_max[Y] / 2};

int n_delta[] = {0, 0};

// Delta smoothing factor (aka buzzword bingo!)
const int DELTA_THRESHOLD = 4;

// Button value history for long trigger pulls
int last_read_c_button, last_read_z_button;

// zoom 
bool zoom_set = false;


// Initialize Serial, servos and wiichuck communication
void setup() {
  Serial.begin(9600);
  Serial.println("Ready");
  x.attach(x_pin);
  y.attach(y_pin);
  
  nunchuck_setpowerpins();
  nunchuck_init();
  
  if (x.attached() && y.attached()) {
    Serial.println("Attached!");
  } else {
    Serial.println("NOT ATTACHED!");
  }
}

// Do it
void loop() {
  // put your main code here, to run repeatedly:
  delay(2);
  x.write(x_safe(new_x));
  y.write(y_safe(new_y));
  
  nunchuck_get_data();

  int joy_x = nunchuck_joyx();
  int joy_y = nunchuck_joyy();
  int z_button = nunchuck_zbutton();
  int c_button = nunchuck_cbutton(); 
  
  if (joy_x < 255 ) max_x = max(max_x, joy_x);
  min_x = min(min_x, joy_x);
  if (joy_y < 255) max_y = max(max_y, joy_y);
  min_y = min(min_y, joy_y);
  
  avg_x = (max_x + min_x) / 2;
  avg_y = (max_y + min_y) / 2;
  
  // button routines
  if (c_button > 0 && last_read_c_button == 0) {
    if (laser_on ) {
      laser_on = false;
      analogWrite(laser_pin, 0);
    } else {
      laser_on = true; //ACTIVATE LASERS!
      analogWrite(laser_pin, 10);
    }  
  }
  
  if (z_button > 0 && last_read_z_button == 0) {
    remaining_trim_x = trim_x + joy_x - avg_x;
    remaining_trim_y = trim_y + joy_y - avg_y;
  }
  
  last_read_c_button = c_button;
  last_read_z_button = z_button;
  

  
  // Calulate deltas
  int delta_x = (joy_x - avg_x + trim_x);
  int delta_y = (joy_y - avg_y + trim_y);
  
  // Center dead zone
  if (abs(delta_x) < DELTA_THRESHOLD ) delta_x = 0;
  if (abs(delta_y) < DELTA_THRESHOLD ) delta_y = 0;
  
  if (delta_x < remaining_trim_x ) {
    trim_x = remaining_trim_x - delta_x;
    remaining_trim_x = remaining_trim_x - trim_x;
  }

  if (delta_y < remaining_trim_y ) {
    trim_y = remaining_trim_y - delta_y;
    remaining_trim_y = remaining_trim_y - trim_y;
  }
  
  new_x = ((X_MIN + X_MAX) /2) - delta_x;
  new_y = ((Y_MIN + Y_MAX) /2) + delta_y;
  
  if ( millis() % 500 == 0) {
    
    // output
    Serial.println(String("Servo: X: " + String(x.read(), DEC) + " Y: " + String(y.read(), DEC)));
    Serial.println(String("Joy: X: " + String(joy_x, DEC) + " Y: " + String(joy_y, DEC)));
    Serial.println(String("Joy: dX: " + String(delta_x, DEC) + " dY: " + String(delta_y, DEC)));
    Serial.println(String("Joy: max_x:" + String(max_x, DEC) + " min_x: " + String(min_x, DEC) + " avg_x: " + String(avg_x, DEC)));
    Serial.println(String("Joy: max_y:" + String(max_y, DEC) + " min_y: " + String(min_y, DEC) + " avg_y: " + String(avg_y, DEC)));
    if (laser_on) {
      Serial.println("Laser on!");
    }      
  }
}

int x_safe(int x) {
  return constrain(x, X_MIN, X_MAX);
}

int y_safe(int y) {
  return constrain(y, Y_MIN, Y_MAX);
}
