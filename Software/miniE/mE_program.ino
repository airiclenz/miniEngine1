/*

    See www.openmoco.org for more information
    
    2015 Airic Lenz

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
// check if there are free programs left
// ===================================================================================
boolean free_programAvailable() {
 
  if (program_count < program_amount) {
    return true;
  } else {
    return false;
  }
  
}


// ===================================================================================
// check if there are free programs left
// ===================================================================================
boolean check_programFuture() {
 
  for (byte i=0; i<program_count; i++) {
    
    // program enabled?
    if (program_flag[i] & B10000000) {
      
      // weekday program ?
      if (program_weekdays[i] & B00000001) {
        
        // set the corresponding flag and leave
        program_status |= B01000000;
        return true;
 
      } else {
        
        // date in the future?
        if (program_datetime[i].get() > time.get()) {
          
          // set the corresponding flag and leave
          program_status |= B01000000;
          return true;
        
        }
        
      }    

    }
    
  }
  
  bitClear(program_status, 6); // B10111111 
  return false;
}


// ===================================================================================
// check is we can add a new program and adds it if there is free space
// ===================================================================================
void add_newProgram() {
  
  if (free_programAvailable()) {
    
    current_program = program_count;
    program_count++;
    
    program_weekdays[current_program] = B0;
    program_datetime[current_program] = RTC.now();
    program_duration[current_program] = 1;
    program_flag[current_program] = B11000000; // program enabled; move home enabled
    
  } 
}  


// ===================================================================================
// check is we have to set the weekday flag because >= 1 weekday is used
// ===================================================================================
void check_weekdayFlag() {
    
  if (program_weekdays[current_program] & B11111110) {
    program_weekdays[current_program] |= B00000001;     
  } else {
    bitClear(program_weekdays[current_program], 0); // B00000001
  }
  
}
  
  
// ===================================================================================
// this function deletes the current program
// ===================================================================================
void delete_program() {
  
  // are there programs?
  if (program_count > 0) {
       
    // last program?
    if (current_program == (program_count - 1)) {
      program_count--;  
    } else {
      
      // do for all programs behind the current program
      for (byte p=current_program; p<(program_count-1); p++) {
      
         program_weekdays[p] = program_weekdays[p+1];
         program_datetime[p] = program_datetime[p+1];
         program_duration[p] = program_duration[p+1];
         program_flag[p]     = program_flag[p+1];
      }  
      
      // no program selected anymore
      current_program = 255;
     
      // R.I.P. program 
      program_count--;
      
    } 
    
  }

}


// ===================================================================================
// the function that checks if we need to start or stop a program
// ===================================================================================
void check_programs() {
  
  boolean dayFit = false;
        
  
  // if timed_program running
  if (program_status & B10000000) {

    // if the programm is running and we are in contiuous mode
    continuous_check();
  
    
    // duration of the program reached?
    if ( (program_start_time + ((unsigned long) program_duration[running_program] * 60000UL)) <= millis() ) {
  
      // stop the program (and move home if wished)      
      stop_program();
      
      // print a message "Program Status: Program stopped."
      print_message(85, 7, 1);
    
    }
    
  } else { // timed_program stopped:
  
    // loop all available programs
    for (byte i=0; i<program_count; i++) {
             
      // are we editing this program in this moment?
      if ((i != current_program) && 
          (program_flag[i] & B10000000)) { // program enabled?
        
        // single date or weekday program?
        if (program_weekdays[i] & B00000001) {
          
          switch(time.dayOfWeek()) {
            case 0:  if (program_weekdays[i] & B00000010) dayFit = true; break;  
            case 1:  if (program_weekdays[i] & B10000000) dayFit = true; break;  
            case 2:  if (program_weekdays[i] & B01000000) dayFit = true; break;
            case 3:  if (program_weekdays[i] & B00100000) dayFit = true; break;  
            case 4:  if (program_weekdays[i] & B00010000) dayFit = true; break;  
            case 5:  if (program_weekdays[i] & B00001000) dayFit = true; break;  
            case 6:  if (program_weekdays[i] & B00000100) dayFit = true; break;  
          }
          
        } else { // single day program
          
          if ((time.day()   == program_datetime[i].day() ) && 
              (time.month() == program_datetime[i].month() ) &&
              (time.year()  == program_datetime[i].year() )) {
                      
             dayFit = true;     
           }          
          
        }
      
        // do we need to check the time because the day is correct?  
        if (dayFit) {
          
          // check the time
          if ((time.hour()   == program_datetime[i].hour() ) && 
              (time.minute() == program_datetime[i].minute() ) &&
              (time.second() == 1))  {
           
            // time fits
            
            // set the timed-program is running flag
            program_status |= B10000000;
            
            // remeber the program that is running
            running_program = i;  
            
            // lets start the main program      
            start_program();
            
            // print a message "Program status: Program started."
            print_message(85, 6, 1);
                        
          } // time fit

        } // day fit    
                
      } // i != current_program

    }  // all programs  
    
  } // timed-program running or stopped?

}


