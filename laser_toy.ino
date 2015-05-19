#include <Servo.h>
#include <Wire.h>
#include "nunchuck_funcs.h"

// X/Y constants
const int X = 0;
const int Y = 1;

// Servo data

// Servo Constants for equipment protection
const int S_MIN[] = {0, 0};
const int S_MAX[] = {160, 125}; 

// Servo pin assignments
const int S_PIN[] = {6, 7};

// Default servo values
int s_current_pos[] = {(S_MIN[X] + S_MAX[X]) / 2, (S_MIN[Y] + S_MAX[Y]) / 2};

// Define Servos
Servo servo[2];

// ***
// Laser data
// ***

bool laser_on = false;
const int LASER_PIN = 3;

//
// Nunchuck data
//

// Reasonable defaults for minimum and maximum integers for range finding
int n_min[] = {40, 40};
int n_max[] = {210, 210};

int n_center_pos[] = {(n_min[X] + n_max[X]) / 2, (n_min[Y] + n_max[Y]) / 2};

int n_current_pos[2];

//int n_delta[] = {0, 0};

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
  
  // Attach servos to pins
  for(int i = X; i <= Y; i++) {
    servo[i].attach(S_PIN[i]);
  }
  
  // Invoke nunchuck.h magic
  nunchuck_setpowerpins();
  nunchuck_init();
}

// Read joystick position
// Read button states
// Calculate...
// Write laser state
// Write servo position
void loop() {
  
  // Read joystick state
  nunchuck_get_data(); 

  n_current_pos[X] = nunchuck_joyx();
  n_current_pos[Y] = nunchuck_joyy();
  int z_button = nunchuck_zbutton();
  int c_button = nunchuck_cbutton(); 
  
  // Update maximum and minimum values if the joystick exceeds the current bounds
  // Also a hack: the 'chuck seems to send 255 to start out, so I limit to less than 255 for maximum
  if (n_current_pos[X] < 255 ) n_max[X] = max(n_max[X], n_current_pos[X]);
  n_min[X] = min(n_min[X], n_current_pos[X]);
  if (n_current_pos[Y] < 255) n_max[Y] = max(n_max[Y], n_current_pos[Y]);
  n_min[Y] = min(n_min[Y], n_current_pos[Y]);
  
  // button routines
  if (c_button > 0 && last_read_c_button == 0) {
    toggle_laser();
  }
  
  if (z_button > 0 && last_read_z_button == 0) {
    // nothing yet
  }
  
  last_read_c_button = c_button;
  last_read_z_button = z_button;
  

  update_servo_position();
  
}

// Translate the 
void update_servo_position() {
  for(int i = X; i <= Y; i++) {
    s_current_pos[i] = constrain(map(n_current_pos[i], n_min[i], n_max[i], S_MIN[i], S_MAX[i] ), S_MIN[i], S_MAX[i]);
    servo[i].write(s_current_pos[i]);
  }  
}

// Converts degrees to pwm millisecond timings
// Reverse engineered from http://www.arduino.cc/en/Reference/ServoAttach
int pwm_constraint(int degree) {
  return (degree * 10) + 544;
}
  
// turns the laser on and off by writing out to the laser_pin
// Maybe try digitalWrite?
void toggle_laser() {
    if (laser_on ) {
      analogWrite(LASER_PIN, 0);
      laser_on = false;
    } else {
      laser_on = true; //ACTIVATE LASERS!
      analogWrite(LASER_PIN, 1);
    }  
}
