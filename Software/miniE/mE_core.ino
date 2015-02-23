/*

    See www.openmoco.org for more information

    (c) 2011 Airic Lenz
    
    The 1st version of this code, dealing with core functionalities, 
    was heavily inspired by the OpenMoCo Engine by C.A. Church
    and is basically based on it. Thank you for your great work! 
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


  

// ===================================================================================
void start_program() {

  // are the limit switches triggered?
  if (check_limit_switches()) {
  
    // clear the shoot counter
    camera_shoot_count = 0; 
        
    // the start immediately flag (B2) 
    action_status |= B00100000;   
    
    // turn on the programm-running flag (B4) and 
    action_status |= B00001000;   
     
    // prepare everything for the motor
    prepare_motor_values();
     
    
    // if not in continuous mode
    if (!action_status & B10000000) {   
    
      // set the motor sleep state (the value has to be set inverse)
      if (motor_status & B10000000) {
        digitalWrite(MOTOR_SLEEP_PIN, LOW);
      } else {
        digitalWrite(MOTOR_SLEEP_PIN, HIGH);
      }
    
    } else {
      // if in continuous mode, the engine has to be awake all the time
      digitalWrite(MOTOR_SLEEP_PIN, HIGH);
    }
       
       
    // remember the time we started   
    program_start_time = millis();

  }
    
}

// ===================================================================================
void stop_program() {
   
  
  
  // turn off the programm-running flag and all other flags too.
  // (this only saves the first bit - the operation mode)
  if (action_status & B10000000) {
    action_status = B10000000;  
  } else {
    action_status = B00000000;
  }
  
  /*
  bitClear(action_status, 6); // Camera focussing        B10111111
  bitClear(action_status, 5); // Start cycle immediately B11011111
  bitClear(action_status, 4); // Camera shooting         B11101111
  bitClear(action_status, 3); // programm running        B11110111
  bitClear(action_status, 2); // Camera post delay       B11111011
  bitClear(action_status, 1); // Motor slot              B11111101
  bitClear(action_status, 0); // Motor post delay        B11111110
  */  
  
  // and just in case turn off the "real" camera exposure
  digitalWrite(CAMERA_PIN, LOW);
  
  // and just in case turn off the "real" camera focus
  digitalWrite(FOCUS_PIN, LOW);
 
 
   
  // do we have to do something because a timed program stopped? 
  if ((program_status & B10000000) &&
      (running_program != 255)) {
      
    // move to home?
    if (program_flag[running_program] & B01000000) {
      
      // print a message: "Moving motor to" - "home. Stand by.";
      print_message(79, 80, 2);
      
      // move motor
      move_motor_to_home();
    }  
    
    // now forget about that
    running_program = 255;
  }
  
  
  
  // timed-program running  B01111111
  bitClear(program_status, 7);  
  
  // set the motor to sleep
  digitalWrite(MOTOR_SLEEP_PIN, LOW);

  // set the motor to sleep
  digitalWrite(MOTOR_STEP_PIN, LOW);
  
}





// ===================================================================================
boolean check_limit_switches() {
  
  // if limit-switchesk are enabled
  if (motor_status & B01000000) {
  
    // temp variable
    int val;
    
    // read 1st switch
    analogRead(LIMIT_SWITCH_PIN_1);
    
    // in continuous mode and program running?
    if ((action_status & B10000000) && 
        (action_status & B00001000)) {
      doMotorContinuous();
      doMotorContinuous();
    } else {
     delay(5); 
    }
      
    val = analogRead(LIMIT_SWITCH_PIN_1);
    
    // read 2nd switch
    analogRead(LIMIT_SWITCH_PIN_2);
    
    // in continuous and program running?
    if ((action_status & B10000000) && 
        (action_status & B00001000)) {
      doMotorContinuous();
      doMotorContinuous();
    } else {
     delay(5); 
    }
     
     
    // if we have a closed switch....
    if ((val                            >= 1000) ||
        (analogRead(LIMIT_SWITCH_PIN_2) >= 1000)) {
      
      // do what is needed when a limit switch is triggered    
      limit_switch_event();
         
      return false;  
    }
 } 
 
 return true;
 
}

// ===================================================================================
void limit_switch_event() {
  
  // ...stop the program
  stop_program();     
         
  // print a message! ("A limit switch was triggered!" 3 sec) 
  print_message(93, 94, 3);
  
  // move back just a little bit (one cycle distance)
  // first flip the direction bit:
  motor_status = motor_status ^ bit(3);
  // then prepare the motor values
  prepare_motor_values();
  // ...and finally move the motor
  doMotorPhase(false);
  
}



