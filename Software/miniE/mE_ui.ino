/*

    See www.openmoco.org for more information
     
    (c) 2011 Airic Lenz
     
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



const char months[12][4] = { 
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


// --------------------------
// Definition of the relation tree as parent - child relations between all screens we have.
// This is used for the navigation through the UI (main_pos).
// the order of sub items (displayed here as a rows) has to be the same as presented in the menu because
// the row count in a menu should be the same as the child count (of a specific parent) in this array.
const byte ui_relation_count = 35; 

byte ui_relations[ui_relation_count][2][2] = { 

  {  
    0,  1, 0  }
  ,                                                                    // sub items of status screen 
  {  
    1, 10, 10  }
  , {  
    1, 20, 11  }
  , {  
    1, 30, 12  }
  , {  
    1, 70, 13  }
  , {  
    1, 80, 14  }
  , {  
    1, 90, 15  }
  ,             // sub items of main menu
  { 
    10,110, 16  }
  , { 
    10,111, 17  }
  , { 
    10,112, 18  }
  , { 
    10,113, 19  }
  , { 
    10,114, 20  }
  , { 
    10,115, 21  }
  ,             // sub items of camera menu
  { 
    10,116, 22  }
  ,                                                                    // sub items of camera menu
  { 
    20,120, 23  }
  , { 
    20,121, 24  }
  , { 
    20,122, 99  }
  , { 
    20,123, 25  }
  , { 
    20,124, 26  }
  , { 
    20,125, 27  }
  ,             // sub items of motor menu
  { 
    20,126, 28  }
  , { 
    20,127, 101  }
  , { 
    20,128, 103  }
  ,                                              // sub items of motor menu
  {
    123,221, 25  }
  , {
    123,222, 59  }
  ,                                                         // sub items of motor-home menu
  { 
    30,130, 61  }
  ,                                                                    // program menu - add program item relation  
  { 
    70,170, 29  }
  , { 
    70,171, 32  }
  , { 
    70,172, 30  }
  , { 
    70,173, 31  }
  , { 
    70,174, 105  }
  ,                        // sub items of General menu
  { 
    80,180, 33  }
  , { 
    80,181, 34  }
  , { 
    80,182, 35  }                                               // sub items of Setting menu
};    


// main position in our UI.
// we start with position 0 (status screen).
byte main_pos = 0;

// latest main position in our UI.
// This is needed for optimized action screen painting
byte main_pos_old = 255;

// position on an action screen. (0 = 1st position and left-key
// will leave the screen)
byte action_pos = 0;


// menu or interactive content on the screen? 
// We start with the status screen - so we are not in a menu but on
// an "action screen"

// B0 = action screen
// B1 = menu
// B3 = text

byte screen_type = B10000000;



// position of the cursor on the screen (line 1 or 2) 
// this is interesting in menus and text screens where we "scroll" vertivally
byte menu_cursor_pos = 1;

// position in the menu - starting with 1 
byte menu_pos = 1;

// old menu position. needed for menu painting (scrolling)
byte menu_pos_old = 1; 

// count of the current menu/text lines
byte menu_length;


#define TREE_DEPTH 5

// an array to remember the positions in the tree
byte ui_tree_position_mem[TREE_DEPTH] = {1,1,1,1,1};

// the var to save the current depth of our position in the ui tree
byte ui_tree_depth = 0;


// backlight variable to remember when something happened
unsigned long backlight_time = millis();


// status screen, timed-program and clock change memory
unsigned int camera_shoot_count_old = 0;
unsigned int cycle_length_old = 0;
byte action_status_old = B0;
byte motor_status_old = B0;
unsigned int motor_steps_old; 
byte clock_seconds_old = 0;
byte program_count_old = 0;
boolean Ptoggle = true;

// the pressed key
byte lcd_key;





// ======================================================================================
// this function is called from the main loop and handles key inputs, backlight and
// initiates all further screen action that needs to be done.
// ======================================================================================
void do_screen() {


  // get the pressed button
  lcd_key = lcd_key_pressed();  


  // if the programm is running and we are in contiuous mode
  continuous_check();


  // is a message displed?
  if (ui_status & B00000010) {

    // message time exceeded?
    if ((message_start_time + ((unsigned long) message_duration * 1000UL)) <= millis() ) {

      // clear the message flag
      bitClear(ui_status, 1); // B11111101

        // set the repaint flag 
      ui_status |= B01000000;   
    }
  } 


  // if the programm is running and we are in contiuous mode
  continuous_check();


  // we have a pressed key!
  if (lcd_key != KEY_NONE) {

    // turn backlight on but do not set the backlight bit
    setBacklightLevel();

    // reset the backlight counter
    backlight_time = millis();  

    // is backlight off? 
    // only do something if the backlight was allready on.
    // first press then when backlight is off
    // is supposed to turn on the light and nothing else.
    if (ui_status & B10000000) {

      // do something - corresponding to the pressed key 
      key_action(lcd_key);

      // set the repaint flag 
      ui_status |= B01000000;   

    } 
    else { // turn backlight on again on any key press

      // set the backlight bit 
      ui_status |= B10000000;
    }

  } // end keypress


  // if the programm is running and we are in contiuous mode
  continuous_check();


  // status screen update needed? do this only if we are not planning
  // to repaint the screen (in this case it will be repainted anyway) 
  // and no message is on the screen. 
  if ((main_pos == 0) &&
    (!(ui_status & B01000000)) && 
    (!(ui_status & B00000010))) {
    // check if there are status changes to be displayed...
    paint_status_screen();   
  }

  // check if we need to repaint the clock if on the clock screen
  // only do if there is no screen message and we don't have the repaint
  // flag (in this case it will be repainted anyway) 
  if ((main_pos == 171) &&
    (!(ui_status & B01000000)) && 
    (!(ui_status & B00000010))) {

    if (time.second() != clock_seconds_old) {
      // set the keyval to NO_KEY to avoid double triggers... 
      key = KEY_NONE;
      paint_status_time();
    }    

  }


  // if the programm is running and we are in contiuous mode
  continuous_check();


  // do we need to repaint? 
  if (ui_status & B01000000) {

    // are we on an action screen?
    if (screen_type & B10000000) {
      // action repaint
      if (main_pos != 0) lcd.blink();
      action_repaint();

    } 
    else if (screen_type & B01000000){
      // menu repaint    
      lcd.noBlink();
      menu_repaint();

    } 
    else if (screen_type & B00100000) {
      // text paint
      lcd.noBlink();
      text_repaint();

    }

    // delete the repaint flag B01000000
    bitClear(ui_status, 6);

  }    


  // if the programm is running and we are in contiuous mode
  continuous_check();


  // if backlight is on, check if it needs to be turned off
  // after several time of no user action
  if (ui_status & B10000000) {

    // did we wait our prefered time since last key action?  
    if ((backlight_time + (unsigned long) backlight_wait * 1000UL) < millis()) {

      // turn off the backlight   
      digitalWrite(LCD_BACKLIGHT_PIN, LOW);

      // set the backlight bit to false B10000000
      bitClear(ui_status, 7); 
    } 
  }
} 



// ======================================================================================
// this function resets the backlight ( this function is supposed to be called from
// outside the UI to switch on or reset backlight on some events)
// ======================================================================================
void trigger_backlight() {

  // turn backlight on
  setBacklightLevel();

  // reset the backlight counter
  backlight_time = millis();  

  // set the backlight bit 
  ui_status |= B10000000;

}






// ======================================================================================
// Here we react on a key press and move through the UI tree
// ======================================================================================
void key_action(int key) {
  
  // menu pos of zero is not allowed:
  if (menu_pos < 1) menu_pos = 1;
  
  // remember our current state
  menu_pos_old = menu_pos;

  // -------------------------
  // key up was pressed  
  if (lcd_key == KEY_UP) {

    if (menu_pos > 1) {
      menu_pos--;
    }
  }

  // -------------------------
  // key down was pressed
  if (lcd_key == KEY_DOWN) {

    if (menu_pos < menu_length) {
      menu_pos++;
    }
  }


  //Serial.print("menu-pos:"); Serial.println(menu_pos, DEC);
  
  

  // variable for the new screen-ID if we change the screen horizontally
  byte ui_target;

  // -------------------------   
  // key right was pressed 
  if (lcd_key == KEY_RIGHT) {

    // if in the dynamic program list
    if (main_pos == 30) {

      // add program
      if ((menu_pos == 1) && 
        (free_programAvailable())) {

        ui_target = 130;

      } 
      else { // we are editing a specific program

        // get the current program we are editing
        if (free_programAvailable()) {
        
          // if we have more programs available, there
          // will be a menu-item "add program". 
          // therefore menu_pos is starting with 1,
          // we have to decrement 2 times
          current_program = menu_pos - 2;

        } 
        else {
          // no "add program item"
          current_program = menu_pos - 1;  
        }

        // ui target = start position of program ids + current program
        ui_target = 140 + current_program;    
      }

    } 
    else if ((main_pos >= 140) && 
      (main_pos <= 140 + program_amount - 1)) { // editing a program

      ui_target = 230 + menu_pos - 1;  
      action_pos = 255;

    } 
    else { // regular menu tree movement

      // if on the system time screen, set action pos to -1  
      if (main_pos == 70) {
        action_pos = 255;
      }
            
      // find the ui item located under the current item    
      ui_target = find_sub_item(main_pos, menu_pos);
      
      //Serial.print("target:"); Serial.println(ui_target, DEC);
    }      


    if (ui_target != 255) {

      main_pos_old = main_pos;
      main_pos = ui_target;

      // remember our current menu pos
      ui_tree_position_mem[ui_tree_depth] = menu_pos;   

      // move deeper into the tree
      ui_tree_depth++;

      // fill the screen array with the data that
      // needs to be shown on the next UI screen
      fill_screen_array(); 
    }   

  } // right key


  //Serial.print("A-mainPos:"); Serial.println(main_pos, DEC);


  // -------------------------
  // key rigth was pressed 
  if (lcd_key == KEY_LEFT) {

    // after leaving a screen and moving back up in the ui tree   
    // handle autosaving of settings: 
    checkAutosave();

    // are we still on an action screen?
    if (action_pos == 0) {

      // if in the dynamic program list:
      if ((main_pos >= 140) && 
        (main_pos < 140 + program_amount)) {

        ui_target = 30;

        // no program is edited anymore
        current_program = 255;      

      } 
      else if ((main_pos >= 230) && // if in the program-edit menu
      (main_pos <= 239)) {

        // back to the program list to the current program
        ui_target = 140 + current_program;

      } 
      else { // regular menu tree movement

        // find parent
        ui_target = find_parent_item(main_pos);
      }

      if (ui_target != 255) {

        main_pos_old = main_pos;
        main_pos = ui_target;

        // move back in the tree depth
        ui_tree_depth--;
        
        // and delete the deeper memory to avoid confusing
        // menu positions if we go back deeper in another part
        // of the tree
        clear_deeper_position_mem();

        // fill the screen array with the data that
        // needs to be shown on the next UI screen
        fill_screen_array();     
      }  

    } // action_pos == 0

    
    
    
  }


  // -------------------------
  // key select was pressed 
  if (lcd_key == KEY_SELECT) {

    // if we are on the status screen
    if (main_pos == 0) {

      // if program is not running      
      if (!(action_status & B00001000)) {

        // start the program
        start_program();

      } 
      else {

        // stop the program
        stop_program();

      }

    } 
    else if (screen_type & B01000000) {  // back to status screen if in a menu rigth now

      // set main pos to status screen
      main_pos = 0;               

      // fill the screen array with the data that needs to be shown on the status screen
      // and set / delete all needed flags
      fill_screen_array();

    } 
    else {

      action_repaint();

    }  
  }
}





// ======================================================================================
// this function paints the current action screen
// ======================================================================================
void action_repaint() {

  switch (main_pos) {

  case 0 : 
    paint_status_screen();
    // handle autosaving of settings (just in case we did not allready save) 
    checkAutosave();
    break; 


  case 110: 
    paint_status_camera_cycle();
    break;
  case 111: 
    paint_status_numeric_uint(&camera_focus_time, "ms", 0);
    break;
  case 112: 
    paint_status_numeric_uint(&camera_exp_time, "ms", 0);
    break;
  case 113: 
    paint_status_bit(&camera_status, 0, 1, 2); // focus line behaviour
    //paint_status_focus_behavior();
    break;
  case 114: 
    paint_status_numeric_uint(&camera_shot_max, " shots", 0);
    break;
  case 115: 
    paint_status_numeric_uint(&camera_exp_post, "ms", 0);
    break;
  case 116: 
    paint_action_test_shot();
    break;



  case 120: 
    paint_status_numeric_ulong(&motor_steps, " steps", 0);        // motor steps
    break;
  case 121: 
    paint_status_bit(&motor_status, 4, 1, 2);                     // motor direction 
    break;
  case 122: 
    paint_status_numeric_uint(&motor_post, "ms", 0);              // motor post delay
    break;
    //  case 124: paint_action_motor_jog();                       // motor post delay
    //           break;
  case 124: 
    paint_status_bit(&motor_status, 0, 1, 2);                     // motor sleep  
    break;
  case 125: 
    paint_status_numeric_ulong(&motor_steps_max, " steps", 0);    // max steps
    break;
  case 126: 
    paint_status_numeric_uint(&motor_ramp, " steps", 0);          // motor ramp
    break;
  case 127: 
    paint_status_numeric_ulong(&motor_step_delay_min, " microSec", 0);   // motor max speed (min delay)
    break;
  case 128: 
    paint_status_numeric_ulong(&motor_step_delay_max, " microSec", 0);   // motor min speed (max delay)
    break;



  case 130: 
    paint_action_add_program();
    break;     



  case 170: 
    paint_status_bit(&action_status, 0, 1, 2); // operation mode  
    //paint_status_operation_mode();
    break;
  case 171: 
    paint_status_time();
    break;
  case 172: 
    paint_status_numeric_uint(&backlight_wait, " sec", 3); 
    break;
  case 173: 
    paint_status_backlight_level();
    break;
  case 174: 
    paint_status_bit(&motor_status, 1, 1, 2); // limit switches sctive
    break;


  case 180: 
    paint_action_save_settings();
    break;
  case 181: 
    paint_status_bit(&ui_status, 2, 1, 2);  // autosave settings
    //paint_status_autosave_settings();
    break;
  case 182: 
    paint_action_reset_settings();
    break;         



  case 221: 
    paint_action_move_to_home();
    break;    
  case 222: 
    paint_action_set_home(); 
    break;


  case 230: 
    paint_status_program_time();
    break;     
  case 231: 
    paint_status_program_weekdays();
    break;     
  case 232: 
    paint_status_numeric_uint(&program_duration[current_program], " min", 0);
    break;     
  case 233: 
    paint_status_bit(&program_flag[current_program], 1, 1, 2); // program: move home at end 
    //paint_status_program_move_home();
    break; 
  case 234: 
    paint_status_bit(&program_flag[current_program], 0, 1, 2); // program: status   
    //paint_status_program_status();
    break;
  case 235: 
    paint_action_program_delete();
    break; 
  }  


  if (action_pos == 255) action_pos = 0;

}




// ======================================================================================
// this function paints the current menu
// ======================================================================================
void menu_repaint() {

  // calculate the positions
  // TODO: this can be optimized by shifting on menu_cursor_pos

  //Serial.print("--> menu_pos: ");
  //Serial.println(menu_pos, DEC);

  const char str_space[] = "  ";
  const char str_cursor[] = "> ";

  // step down?
  if (menu_pos_old < menu_pos) {
    if (menu_cursor_pos == 1) {
      menu_cursor_pos = 2;
    }  
  } 
  else if (menu_pos_old > menu_pos) { // step up?
    if (menu_cursor_pos == 2) {
      menu_cursor_pos = 1;
    }  
  }

  lcd.clear();

  // paint the two lines
  lcd.setCursor(2, 0);
  lcd.print(lines[menu_pos - menu_cursor_pos]);
  lcd.setCursor(2, 1);
  lcd.print(lines[menu_pos -  menu_cursor_pos + 1]);

  // paint the cursor
  if (menu_cursor_pos == 1) {
    lcd.setCursor(0, 0);
    lcd.print(str_cursor);
    lcd.setCursor(0, 1);
    lcd.print(str_space);
  }

  if (menu_cursor_pos == 2) {
    lcd.setCursor(0, 0);
    lcd.print(str_space);
    lcd.setCursor(0, 1);
    lcd.print(str_cursor);
  }

  // paint the scroll arrows
  paint_scroll_arrows();

}



// ======================================================================================
// this function paints the text parts
// ======================================================================================
void text_repaint() {

  // if we are at the last position of this screen,
  // go one step back - otherwise we will leave our line array.
  if (menu_pos == menu_length) {
    menu_pos--;
  }
  
  lcd.clear();

  // paint the two lines
  lcd.setCursor(0, 0);
  lcd.print(lines[menu_pos - 1]);

  lcd.setCursor(0, 1);
  lcd.print(lines[menu_pos]);

  paint_scroll_arrows();  
}



// ======================================================================================
// this function paints the scrolls arrows depending on the current menu/text position
// ======================================================================================
void paint_scroll_arrows() {

  const char str_space[] = " ";

  // paint the "scrollbars"
  if (menu_length > 2) {
    if (menu_pos - menu_cursor_pos >= 1 ) {
      lcd.setCursor(15, 0);
      lcd.write((byte)0); // paint the scroll_up arrow
    } 
    else {
      lcd.setCursor(15, 0);
      lcd.print(str_space);
    } 

    if (menu_pos - menu_cursor_pos + 2 < menu_length) {
      lcd.setCursor(15, 1);
      lcd.write((byte)1); // paint the scroll_down arrow
    } 
    else {
      lcd.setCursor(15, 1);
      lcd.print(str_space);
    }
  } 
  else {
    lcd.setCursor(15, 0);
    lcd.print(str_space);
    lcd.setCursor(15, 1);
    lcd.print(str_space);
  }
}



// ======================================================================================
// Here we are looking for a sub item in our UI tree. We will get the 'item_numer'th
// item below the current item in our structure. If we want for example the 3rd sub item
// of the main menu (1) we ask for find_sub_item(1, 3);
// If nothing was found we get a 255;
// ======================================================================================
byte find_sub_item(byte current, byte item_number) {

  byte item_count = 1; 

  // loop all relations
  for (int i=0; i<ui_relation_count; i++) {

    // if we have found a relation with our
    // prefered parent...
    if (ui_relations[i][0][0] == current) {

      // check if we allready reached the item number
      // we want.
      if (item_count == item_number) {

        // return the sub item
        return ui_relations[i][0][1];

      } // otherwise increment our counter and continue
      else {
        item_count++;
      }
    }
  } 

  return 255; 
}



// ======================================================================================
// Here we are looking for the parent item in our UI tree. If we want for example the 
// parent item of the main menu (1) we ask for find_parent_item(1);
// If nothing was found we get a 255;
// ======================================================================================
byte find_parent_item(byte current) {

  // loop all relations
  for (byte i=0; i<ui_relation_count; i++) {

    // if we have found a relation with our
    // prefered child...
    if (ui_relations[i][0][1] == current) {

      // return the parent item
      return ui_relations[i][0][0];

    }
  } 

  return 255; 
}



// ======================================================================================
// resets all menu variables to have a clear and correct painting after changing to 
// a new UI position in the tree
// ======================================================================================
void reset_menu_vars() {
  
  menu_cursor_pos = 1;
  menu_pos = ui_tree_position_mem[ui_tree_depth];
   
  menu_pos_old = 1;
}

// ======================================================================================
// this function resets the position memeory deeper than the current ui-tree-depth
// ======================================================================================
void clear_deeper_position_mem() {

  for (byte i = ui_tree_depth + 1; i < TREE_DEPTH; i++) {
    ui_tree_position_mem[i] = 1;
  }
}



// ======================================================================================
void setBacklightLevel() {
  if (backlight_level == 100) {
    digitalWrite(LCD_BACKLIGHT_PIN, HIGH); 
  } 
  else {
    analogWrite(LCD_BACKLIGHT_PIN, (backlight_level*255/100));
  }
}



// ======================================================================================  
void checkAutosave() {

  // were the settings changed?
  if (ui_status & B00010000) {

    // should we autosave our settings?
    if (ui_status & B00100000) {
      write_config();

    }

    // delete the settings changed flag
    bitClear(ui_status, 4); // B11101111

  }
}



// ======================================================================================  
char* addLeadingZero(unsigned int number) {

  const char noll[] = "0";
  char val[3];
  char res[3];

  if (number < 100) {

    // convert the number to a string
    itoa(number, val, 10);

    // smaller than 10? add a leading zero
    if (number < 10) {
      strcpy(res, noll);
      strcat(res, val);

    } 
    else {

      strcpy(res, val);
    }
  } 

  return res;

}


// ===================================================================================
// Function for getting a string from the flash and storing it in our line array
// ===================================================================================
void getString(byte buf_number, byte line_number) {

  // Using the string table in program memory requires the use of special functions to 
  // retrieve the data. The strcpy_P function copies a string from program space to a 
  // string in RAM. 

  // Necessary casts and dereferencing 
  strcpy_P(lines[line_number], (char*) pgm_read_word(&(string_table[buf_number]))); 

}


// ===================================================================================
// test function for multidimensional arrays to save space
// ===================================================================================
void lcdstring(byte sub_number){
  // loop all relations

  byte item_count = 0; 

  // loop all relations
  for (int i=0; i<ui_relation_count; i++) {

    // if we have found a relation with our
    // current menu level..
    if (ui_relations[i][0][0] == sub_number) {
      // load string with the current array variable ID into memory
      getString(ui_relations[i][1][0], item_count);
      item_count++;
    }
  } 

  // set the new menu length and screen type
  reset_menu_vars();
  menu_length = item_count;
  screen_type = B01000000; // menu

}


// ======================================================================================  
// this function prints a message to the screen. it uses 2 lines (string codes) and is
// displaying the message for a defined amount of seconds
// ======================================================================================  
void print_message(byte str_code1, byte str_code2, byte seconds) {

  getString(str_code1, 0);
  getString(str_code2, 1);

  // remember the time we started;
  message_start_time = millis();
  message_duration = seconds;

  // set the message flag
  ui_status |= B00000010;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(lines[0]);

  lcd.setCursor(0,1);
  lcd.print(lines[1]);

  // enable the backlight
  trigger_backlight();

}


// ======================================================================================  
void display_home() {
  
  main_pos = 0;
  trigger_backlight();
  clear_deeper_position_mem();
  lcd.noBlink();
  paint_status_screen();
  
}

