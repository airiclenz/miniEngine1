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



#define  key_initial_delay   500    // delay between keys on longnently pressed key
#define  key_rate            150    // the base key rate (delay) we start with when long pressing a key

#define  KEY_NONE            255
#define  KEY_RIGHT           0
#define  KEY_UP              1
#define  KEY_DOWN            2
#define  KEY_LEFT            3
#define  KEY_SELECT          4
#define  KEY_MENU            5

#define  key_0_val           50     // right  (old: 30)
#define  key_1_val           225    // up     (old: 240)
#define  key_2_val           400    // down   (old: 360)
#define  key_3_val           600    // left   (old: 580)
#define  key_4_val           800    // select (old: 760) 


byte key = KEY_NONE;
byte oldkey = KEY_NONE;
int analogValue;


  // the dynamically calculated key rate (speed up during the long press)
unsigned long key_rate_dynamic = key_rate;
  // the key amount we send out allready during the long press
unsigned long key_count;
  // the time we started with a key press
unsigned long key_press_start;


 


// ======================================================================================
int lcd_key_pressed() {
    
  analogValue = analogRead(LCD_KEY_PIN);
  
  //switch debounce delay
  delay(4); 
  
  // this calculation gives the button a slight range to allow for a little contact resistance noise.
  // it double checks the keypress. If the two readings are not equal +/-k value after debounce delay, 
  // it tries again. if (adc_key_in > 760)  return btnNONE;  
  if (5 < abs(analogRead(LCD_KEY_PIN) - analogValue)) {
    
    return KEY_NONE;
  } 
  
  // convert into key press
  key = lcd_get_key(analogValue);    
  
  // no key pressed
  if (key == KEY_NONE) {
    oldkey = KEY_NONE;
    return KEY_NONE;
  } 
    
  // single press detected
  if (key != oldkey) {                
      
        
    // delete the long key flag and the counter because we have a single press
    if (ui_status & B00000100) {
      bitClear(ui_status, 2); // B00000100
      key_count = 0;
      
      // reset the delays for long key presses
      key_rate_dynamic = key_rate;
    }
        
    oldkey = key;  
    if (key >=0) {
      return key;
    }
    
  }
  
  
  // long-key-press
  if (key == oldkey) {
    
    // if we have a long key press
    if (ui_status & B00000100) {
      
      // initial delay and rate delay over?
      if ( (key_press_start + key_initial_delay + (key_count * key_rate_dynamic)) < millis()) {
          
          key_count++;
                    
          // calculate the new key-rate-delay for speed up
          if (key_count > 2) {
            key_rate_dynamic = round(key_rate - (key_count / 3));
            
            // allow no zero values
            if (key_rate_dynamic < 0) {
             key_rate_dynamic = 0; 
            }
          }
          
          return key;
      }
      
    } else {
      // set the long key press flag and remember the time
      ui_status |= B00000100;
      key_press_start = millis();
    }
    
    
  }

  
  return KEY_NONE;
  
}



// Convert ADC value to key number
// ======================================================================================
int lcd_get_key(unsigned int input) { 
  
  if (input < key_0_val) {
    return 0;
  } else if (input < key_1_val) {
    return 1;          
  } else if (input < key_2_val) {
    return 2;   
  } else if (input < key_3_val) {
    return 3;   
  } else if (input < key_4_val) {
    return 4;   
  } else {
    return KEY_NONE;    
  }
}
