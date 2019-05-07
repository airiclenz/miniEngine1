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






// ======================================================================================
// this function paints the status screen
// ======================================================================================
void paint_status_screen()
{

	// reset menu pos to 1
	menu_pos = 1;

	// delete the tree position memory
	ui_tree_depth = 0;
	clear_deeper_position_mem();


	// repaint needed?
	if (ui_status & B01000000) {  // repaint flag
		lcd.clear();
	}

	// if the programm is running and we are in contiuous mode
	continuous_check();


	// program running or stopped?
	if ( (ui_status & B01000000) ||  // repaint flag
	     ((action_status_old & B00001000) != (action_status & B00001000)) ) {

		action_status_old = action_status;

		getString(0, 0);   // On
		getString(1, 1);   // Off

		lcd.setCursor(0,0);
		// programm status
		if(action_status & B00001000) {
			lcd.print(lines[0]);     // on
		} else {
			lcd.print(lines[1]);     // off
		}
	}

	// if the programm is running and we are in contiuous mode
	continuous_check();


	// camera delay cycle length
	if ( (ui_status & B01000000) ||  // repaint flag
	     ((cycle_length != cycle_length_old)) ) {

		cycle_length_old = cycle_length;

		lcd.setCursor(4,0);
		float cycle = (float) cycle_length / 2;

		// if bigger than 100 then round to int
		if (cycle >= 100) {
			lcd.print(cycle, 0);
		} else {
			lcd.print(cycle, 1);
		}

		lcd.print("\"");

	}

	// if the programm is running and we are in contiuous mode
	continuous_check();


	// shot counter
	if ( (ui_status & B01000000) ||  // repaint flag
	     ((camera_shoot_count_old != camera_shoot_count)) ) {

		camera_shoot_count_old = camera_shoot_count;

		lcd.setCursor(10,0);

		if (camera_shoot_count < 10) {
			lcd.print("[   ");
		} else if (camera_shoot_count < 100) {
			lcd.print("[  ");
		} else if (camera_shoot_count < 1000) {
			lcd.print("[ ");
		} else {
			lcd.print("[");
		}

		lcd.print(camera_shoot_count);
		lcd.print("]");

	}

	// if the programm is running and we are in contiuous mode
	continuous_check();



	// motor direction
	if  ( (ui_status & B01000000) ||  // repaint flag
	      ((motor_status_old & B00001000) != (motor_status_old & B00001000)) ) {

		motor_status_old = motor_status;

		getString(2, 0);   // CW
		getString(3, 1);   // CCW
		getString(5, 2);   // D:

		lcd.setCursor(0,1);
		lcd.print(lines[2]);    // D:

		if(motor_status & B00001000) {
			lcd.print(lines[1]);     // CCW
		} else {
			lcd.print(lines[0]);     // CW
		}
	}


	// if the programm is running and we are in contiuous mode
	continuous_check();

	// motor steps
	if  ( (ui_status & B01000000) ||  // repaint flag
	      (motor_steps_old != motor_steps) ) {

		motor_steps_old = motor_steps;

		getString(4, 0);   // Stp:

		lcd.setCursor(6,1);
		lcd.print(lines[0]);   // Stp:
		lcd.print(motor_steps);

	}

	// if the programm is running and we are in contiuous mode
	continuous_check();


	// Program available
	if  ( (ui_status & B01000000) ||  // repaint flag
	      (program_count_old != program_count) ||
	      (clock_seconds_old != time.second())) {

		program_count_old = program_count;

		lcd.setCursor(15,1);

		// if program is running and a second is over:
		if ((clock_seconds_old != time.second()) &&
		    (action_status & B00001000)) {

			// toggle our toggle flag
			Ptoggle = !Ptoggle;
			clock_seconds_old = time.second();
		}

		// timed-program running?
		if (program_status & B10000000) {
			if (Ptoggle) {
				lcd.print("P");          // timed-Program running toggle state
			} else {
				lcd.print(" ");          // timed-Program running toggle state
			}

		} else {   // manually started program running:

			if (action_status & B00001000) {

				// active programs in the future?
				if (program_status & B01000000) {

					// user warning: be careful - there are programs that are
					// possibly not able to run now:
					if (Ptoggle) {
						lcd.print("!");            // warning toggle state
					} else {
						lcd.print(" ");            // warning toggle state
					}

				} else {
					lcd.print(" ");         // nothing to say
				}

			} else {    // no program is running is stopped:

				// active programs in the future?
				if (program_status & B01000000) {
					lcd.print("P");         // timed-Program(s) existing
				} else {
					lcd.print(" ");         // no timed-Program(s) existing
				}

			}

		}

	}

}




// ======================================================================================
// This function fills the text array corresponding to the position in our UI tree...
// ======================================================================================
void fill_screen_array()
{

	// our little line counter
	byte n = 0;

	// Serial.print("Main pos: "); Serial.println(main_pos);


	// -- status screen --
	if (main_pos == 0) {

		// this is an action screen
		screen_type = B10000000;   // action screen
		// this is no menu. for a right-key event we have to ensure
		// we find the fist sub item (if existing) if we want to change the
		// UI position so we need to set the menu position to 1.
		// The length is set to one too, to ensure we stay
		// at position 1.
		reset_menu_vars();
		menu_length = 1;

	}


	// -- main menu --
	if (main_pos == 1) {
		lcdstring(main_pos);
	}



	// -- camera menu --
	if (main_pos == 10) {
		lcdstring(main_pos);
	}



	// -- motor menu --
	if (main_pos == 20) {
		lcdstring(main_pos);
	}


	// -- program menu --
	if (main_pos == 30) {

		// are there programs left we can define?
		if (free_programAvailable()) {
			getString(61, n++);     // add program

			// to get a clear painting,
			// add a clear second line in case there are no programs defined
			strcpy(lines[1], "            ");
		}

		for (byte i=0; i<program_count; i++) {

			char temp[13];

			// use weekdays
			if (program_weekdays[i] & B00000001) {


				if (program_weekdays[i] & B10000000) { strcpy(temp, "M"); }
				else { strcpy(temp, "_"); }
				if (program_weekdays[i] & B01000000) { strcat(temp, "T"); }
				else { strcat(temp, "_"); }
				if (program_weekdays[i] & B00100000) { strcat(temp, "W"); }
				else { strcat(temp, "_"); }
				if (program_weekdays[i] & B00010000) { strcat(temp, "T"); }
				else { strcat(temp, "_"); }
				if (program_weekdays[i] & B00001000) { strcat(temp, "F"); }
				else { strcat(temp, "_"); }
				if (program_weekdays[i] & B00000100) { strcat(temp, "S"); }
				else { strcat(temp, "_"); }
				if (program_weekdays[i] & B00000010) { strcat(temp, "S"); }
				else { strcat(temp, "_"); }

				strcat(temp, " ");

			} else {  // use a single date

				const char minu[] =  "-";
				const char dot[] =  ".";

				byte m = program_datetime[i].month() - 1;

				strcpy(temp, addLeadingZero(program_datetime[i].day()));
				strcat(temp, dot);
				strcat(temp, " ");
				strcat(temp, months[m]);
				strcat(temp, " ");

			}

			strcat(temp, addLeadingZero(program_datetime[i].hour()));
			strcat(temp, addLeadingZero(program_datetime[i].minute()));

			strcpy(lines[n++], temp);

		}

		// set the new menu length and screen type
		reset_menu_vars();
		menu_length = n;
		screen_type = B01000000;   // menu
	}




	// -- general menu --
	if (main_pos == 70) {
		lcdstring(main_pos);
	}


	// -- settings menu --
	if (main_pos == 80) {
		lcdstring(main_pos);
	}



	// -- Information --
	if (main_pos == 90) {

		char ver[] = "0";
		char subver[] = "0";
		const char name[] = "miniE v";
		const char dot[] = ".";

		char temp[15];

		itoa(VERSION, ver, 10);
		itoa(SUBVERSION, subver, 10);

		// render the current version number
		strcpy(temp, name);
		strcat(temp, ver);
		strcat(temp, dot);
		strcat(temp, subver);
		strcat(temp, "    ");

		strcpy(lines[n++], temp);
		getString(36, n++);    // general product information
		getString(37, n++);
		getString(38, n++);
		getString(39, n++);
		getString(40, n++);
		getString(41, n++);
		getString(42, n++);
		getString(43, n++);
		getString(44, n++);

		// set the new menu length and screen type
		reset_menu_vars();
		menu_length = n;
		screen_type = B00100000;   // text
	}




	// -- Cycle length menu --
	if (main_pos == 110) {

		getString(16, n++);    // Cycle length:

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- Camera Focus time --
	if (main_pos == 111) {

		getString(17, n++);    // Focus time

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- Camera exposure --
	if (main_pos == 112) {

		getString(18, n++);    // Exp. time:

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- Focus line high while shooting --
	if (main_pos == 113) {

		getString(19, n++);    // Focus behav.
		getString(87, n++);    // high w. shutter
		getString(88, n++);    // low w. shutter

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}


	// -- Maximum number of shots to do --
	if (main_pos == 114) {

		getString(20, n++);    // Max. shots

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}


	// -- Camera post delay --
	if (main_pos == 115) {

		getString(21, n++);     // Post delay:

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- Test shot --
	if (main_pos == 116) {

		getString(46, n++);    // Press select to
		getString(53, n++);    // do a test shot.

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}





	// -- motor steps --
	if (main_pos == 120) {

		getString(23, n++);     // Motor steps:

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- motor direction --
	if (main_pos == 121) {

		getString(24, n++);     // Motor direction:
		getString(91, n++);     // clockwise
		getString(92, n++);     // anti-clockwise

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- motor post delay --
	if (main_pos == 122) {

		getString(21, n++);     // Post delay

		// set the new menu length and layer deep
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- motor home menu --
	if (main_pos == 123) {
		lcdstring(main_pos);
	}


	// -- motor sleep --
	if (main_pos == 124) {

		getString(26, n++);     // Motor sleep:
		getString(8,  n++);     // enabled
		getString(9,  n++);     // disabled

		// set the new menu length and layer deep
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- motor max steps --
	if (main_pos == 125) {

		getString(27, n++);     // Max steps

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- motor ramp --
	if (main_pos == 126) {

		getString(28, n++);     // Ramp

		// set the new menu length and layer deep
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- motor max speed delay --
	if (main_pos == 127) {

		getString(102, n++);     // Max speed delay:

		// set the new menu length and layer deep
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- motor min speed delay --
	if (main_pos == 128) {

		getString(104, n++);     // Min speed delay:

		// set the new menu length and layer deep
		menu_length = n;
		screen_type = B10000000;   // action
	}










	// -- add Program --
	if (main_pos == 130) {

		getString(46, n++);    // Press select to
		getString(61, n++);    // add a program.

		// set the new menu length and layer deep
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}



	// -- program menu - in a program --
	if ((main_pos >= 140) &&
	    (main_pos < (140 + program_amount)))  {

		getString(63, n++);   // Start time
		getString(64, n++);   // Weekdays
		getString(65, n++);   // Duration
		getString(66, n++);   // Move home
		getString(67, n++);   // Status
		getString(68, n++);   // Delete

		// set the new menu length and screen type
		reset_menu_vars();
		menu_length = n;
		screen_type = B01000000;   // menu
	}



	// -- operating mode --
	if (main_pos == 170) {

		getString(29, n++);   // Operation mode:
		getString(89, n++);   // Continuous
		getString(90, n++);   // Shoot-move-shoot

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- System time --
	if (main_pos == 171) {

		// No content needed - screen is filled
		// completely by time and date

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- Backlight delay --
	if (main_pos == 172) {

		getString(30, n++);   // B-Light time

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}



	// -- Backlight power --
	if (main_pos == 173) {

		getString(31, n++);   // B-Light powr

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}


	// -- Limit Switch Status --
	if (main_pos == 174) {

		getString(105, n++);   // Lmt-Switches:
		getString(8,  n++);   // enabled
		getString(9,  n++);   // disabled

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}




	// -- Save settings --
	if (main_pos == 180) {

		getString(46, n++);   // Press SELECT to
		getString(72, n++);   // save settings.

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- Autosave settings --
	if (main_pos == 181) {

		getString(73, n++);   // Autosave settgs:
		getString(8,  n++);   // enabled
		getString(9,  n++);   // disabled

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}

	// -- Reset all settings --
	if (main_pos == 182) {

		getString(46, n++);   // Press SELECT to
		getString(74, n++);   // reset settings.
		getString(75, n++);   // Restoring CFG...
		getString(76, n++);   // Restarting...

		// set the new menu length and screen type
		menu_length = n;
		screen_type = B10000000;   // action
	}



	// -- motor move to home position --
	if (main_pos == 221) {

		getString(46, n++);   // Press SELECT to
		getString(78, n++);   // move to home.
		getString(79, n++);   // Moving motor to
		getString(80, n++);   // home. Stand by.


		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}

	// -- motor set home position --
	if (main_pos == 222) {

		getString(46, n++);   // Press SELECT to
		getString(77, n++);   // set motor home.

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}





	// -- Program time --
	if (main_pos == 230) {

		getString(81, n++);   // Program start:

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}


	// -- Weekdays --
	if (main_pos == 231) {

		getString(82, n++);   // Weekdays:

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}

	// -- Duration --
	if (main_pos == 232) {

		getString(83, n++);   // Duration (min):

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}

	// -- Move home --
	if (main_pos == 233) {

		getString(84, n++);   // Move home @ end:
		getString(8,  n++);   // enabled
		getString(9,  n++);   // disabled

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}

	// -- Program Status --
	if (main_pos == 234) {

		getString(85, n++);   // Program status:
		getString(8,  n++);   // enabled
		getString(9,  n++);   // disabled

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}


	// -- Delete program --
	if (main_pos == 235) {

		getString(46, n++);   // Press SELECT to
		getString(86, n++);   // delete program.

		// set the new menu length and screen type
		menu_length = n;
		reset_menu_vars();
		screen_type = B10000000;   // action
	}


}




// ======================================================================================
// this function paints the cycle delay screen
// ======================================================================================
void paint_status_camera_cycle()
{

	// if key pressed
	if (key == KEY_UP) {

		cycle_length += 1 + key_count;

		// set the settings were changed flag
		ui_status |= B00010000;

	} else if ((key == KEY_DOWN) &&
	           (cycle_length > 1)) {

		if ((2 + key_count) > cycle_length) {
			cycle_length = 1;
		} else {
			cycle_length -= 1 + key_count;
		}

		// set the settings were changed flag
		ui_status |= B00010000;
	}

	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(lines[0]);

	lcd.setCursor(0,1);

	lcd.print((float) cycle_length / 2, 1);
	lcd.print("\"");

}


// ======================================================================================
// this function paints an action screen with a numeric int value
// ======================================================================================
void paint_status_numeric_uint(
	unsigned int *value,
	const char* unit,
	unsigned int minimum)
{

	// if key pressed
	if (key == KEY_UP) {
		*value += 1 + key_count;

		// set the settings were changed flag
		ui_status |= B00010000;

	} else if ((key == KEY_DOWN) &&
	           (*value > minimum)) {

		if ((*value - minimum) > key_count) {
			*value -= 1 + key_count;

		} else {
			*value = minimum;
		}

		// set the settings were changed flag
		ui_status |= B00010000;
	}


	// do we need to paint the first line?
	if (main_pos_old != main_pos) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

	} else {
		// clear only the data line
		lcd.setCursor(0,1);
		lcd.print("                ");
	}

	// paint the new data
	lcd.setCursor(0,1);
	lcd.print(*value);
	lcd.print(unit);

}

// ======================================================================================
// this function paints an action screen with a numeric long value
// ======================================================================================
void paint_status_numeric_ulong(
	unsigned long *value,
	const char* unit,
	unsigned int minimum)
{

	// if key pressed
	if (key == KEY_UP) {
		*value += 1 + key_count;

		// set the settings were changed flag
		ui_status |= B00010000;

	} else if ((key == KEY_DOWN) &&
	           (*value > minimum)) {

		if ((*value - minimum) > key_count) {
			*value -= 1 + key_count;
		} else {
			*value = minimum;
		}

		// set the settings were changed flag
		ui_status |= B00010000;
	}

	// do we need to paint the first line?
	if (main_pos_old != main_pos) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

	} else {
		// clear only the data line
		lcd.setCursor(0,1);
		lcd.print("                ");
	}

	// paint the new data
	lcd.setCursor(0,1);
	lcd.print(*value);
	lcd.print(unit);
}



// ======================================================================================
// this function paints a status screen for switch settings
// ======================================================================================
void paint_status_bit(
	byte *value,
	char bit_num,
	byte line_on,
	byte line_off)
{

	byte check_byte = B10000000 >> bit_num;

	// if key pressed
	if ((key == KEY_UP) ||
	    (key == KEY_DOWN)) {

		// toggle the bit
		*value ^= check_byte;

		// set the settings were changed flag
		ui_status |= B00010000;
	}

	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(lines[0]);

	lcd.setCursor(0,1);

	if (*value & check_byte) {
		lcd.print(lines[line_on]);
	} else {
		lcd.print(lines[line_off]);
	}


}


// ======================================================================================
// this function paints "set home" screen
// ======================================================================================
void paint_action_set_home()
{

	// if select key pressed
	if (key != KEY_SELECT) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

		lcd.setCursor(0,1);
		lcd.print(lines[1]);

	} else {

		// set home to the current position
		home_steps = 0;

		// move back into the menu after moving the motor to home
		main_pos = find_parent_item(main_pos);
		ui_tree_depth--;

		// fill the screen array with the new values
		fill_screen_array();

		// and initiate a repaint
		ui_status |= B01000000;
	}
}



// ======================================================================================
// this function paints move to home screen
// ======================================================================================
void paint_action_move_to_home()
{

	// if select key pressed
	if (key != KEY_SELECT) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

		lcd.setCursor(0,1);
		lcd.print(lines[1]);

	} else {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[2]);

		lcd.setCursor(0,1);
		lcd.print(lines[3]);

		move_motor_to_home();

		// move back into the menu after moving the motor to home
		main_pos = find_parent_item(main_pos);
		ui_tree_depth--;

		// fill the screen array with the new values
		fill_screen_array();

		// and initiate a repaint
		ui_status |= B01000000;
	}
}




// ======================================================================================
// this function paints the backlight level screen
// ======================================================================================
void paint_status_backlight_level()
{

	const byte minimum = 5;

	// if key pressed
	if ((key == KEY_UP) &&
	    (backlight_level < 100)) {

		backlight_level++;

		// set the settings were changed flag
		ui_status |= B00010000;

	} else if ((key == KEY_DOWN) &&
	           (backlight_level > minimum)) {

		backlight_level--;

		// set the settings were changed flag
		ui_status |= B00010000;
	}

	setBacklightLevel();

	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print(lines[0]);

	lcd.setCursor(0,1);

	lcd.print(backlight_level, DEC);
	lcd.print("%");

}





// ======================================================================================
// this function paints the test shoot screen
// ======================================================================================
void paint_action_test_shot()
{

	// if select key pressed
	if (key != KEY_SELECT) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

		lcd.setCursor(0,1);
		lcd.print(lines[1]);


	} else {

		// do a short blocking test shoot
		camera_shoot(100);

		// move back into the menu
		main_pos = find_parent_item(main_pos);
		ui_tree_depth--;

		// fill the screen array with the new values
		fill_screen_array();

		// and initiate a repaint
		ui_status |= B01000000;
	}

}


// ======================================================================================
// this function paints the save settings action screen
// ======================================================================================
void paint_action_save_settings()
{

	// if select key pressed
	if (key != KEY_SELECT) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

		lcd.setCursor(0,1);
		lcd.print(lines[1]);

	} else {

		// Save the settings
		write_config();

		// move back into the menu
		main_pos = find_parent_item(main_pos);
		ui_tree_depth--;

		// fill the screen array with the new values
		fill_screen_array();

		// and initiate a repaint
		ui_status |= B01000000;
	}

}



// ======================================================================================
// this function paints the "reset to defaults" screen
// ======================================================================================
void paint_action_reset_settings()
{

	// if select key pressed
	if (key != KEY_SELECT) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

		lcd.setCursor(0,1);
		lcd.print(lines[1]);


	} else {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[2]);

		// change the save flag in the eeprom
		eeprom_saved(false);

		delay(500);

		lcd.setCursor(0,1);
		lcd.print(lines[3]);

		delay(1000);

		// restart arduino to reload vars and rewrite into the eeprom
		reset_arduino();

	}
}


// ======================================================================================
// this function paints the system time screen
// ======================================================================================
void paint_status_time()
{

	//lcd.noBlink();

	int updown = 0;

	if (key == KEY_UP) {
		updown = +1;
	} else if (key == KEY_DOWN) {
		updown = -1;
	} else if (key == KEY_RIGHT) {
		action_pos++;
	} else if (key == KEY_LEFT) {
		action_pos--;
	}

	// check the action pos limit
	if (action_pos > 5) action_pos = 0;

	if (main_pos_old != main_pos) {
		lcd.clear();
	}


	if (updown != 0) {

		switch (action_pos) {

		case 0:       // hour
			time = time.unixtime() + (updown * 3600);
			break;
		case 1:       // minute
			time = time.unixtime() + (updown * 60);
			break;
		case 2:       // second
			time = time.unixtime() + (updown);
			break;
		case 3:       // year
			time = time.unixtime() + (updown * 86400L * 365);
			break;
		case 4:       // month
			time = time.unixtime() + (updown * 86400L * 30);
			break;
		case 5:       // day
			time = time.unixtime() + (updown * 86400L);
			break;

		}   // switch

		// now adjust the time
		RTC.adjust(time);

	}  // updown != 0


	lcd.setCursor(0, 0);
	if (time.hour() < 10) lcd.print("0");
	lcd.print(time.hour(), DEC);
	lcd.print(":");

	if (time.minute() < 10) lcd.print("0");
	lcd.print(time.minute(), DEC);
	lcd.print("'");

	if (time.second() < 10) lcd.print("0");
	lcd.print(time.second(), DEC);

	lcd.setCursor(0, 1);
	lcd.print(time.year(), DEC);
	lcd.print("-");

	if (time.month() < 10) lcd.print("0");
	lcd.print(time.month(), DEC);
	lcd.print("-");

	if (time.day() < 10) lcd.print("0");
	lcd.print(time.day(), DEC);


	// set the cursor for blinking at the correct position
	switch (action_pos) {
	case 0:  lcd.setCursor(1, 0); break;     // hour
	case 1:  lcd.setCursor(4, 0); break;     // minute
	case 2:  lcd.setCursor(7, 0); break;     // second
	case 3:  lcd.setCursor(3, 1); break;     // year
	case 4:  lcd.setCursor(6, 1); break;     // month
	case 5:  lcd.setCursor(9, 1); break;     // day
	}

	// remember the second
	clock_seconds_old = time.second();
}



// ======================================================================================
// this function paints the add program action screen
// ======================================================================================
void paint_action_add_program()
{

	// if select key pressed
	if (key != KEY_SELECT) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

		lcd.setCursor(0,1);
		lcd.print(lines[1]);

	} else {

		// create a new program
		add_newProgram();

		// and edit it
		main_pos = 140 + current_program;
		ui_tree_depth++;

		// fill the screen array with the new values
		fill_screen_array();

		// set the settings were changed flag
		ui_status |= B00010000;

		// and initiate a repaint
		ui_status |= B01000000;
	}

}



// ======================================================================================
// this function paints the program start time status screen
// ======================================================================================
void paint_status_program_time()
{

	int updown = 0;
	unsigned long timeTemp = program_datetime[current_program].unixtime();

	if (key == KEY_UP) {
		updown = +1;
	} else if (key == KEY_DOWN) {
		updown = -1;
	} else if (key == KEY_RIGHT) {
		action_pos++;
	} else if (key == KEY_LEFT) {
		action_pos--;
	}

	// check action_pos limits depending on the program type
	if (program_weekdays[current_program] & B00000001) {
		if (action_pos > 1) action_pos = 0;
	}  else {
		if (action_pos > 4) action_pos = 0;
	}

	if (updown != 0) {

		switch (action_pos) {

		case 0:       // hour
			program_datetime[current_program] = (updown * 3600) + timeTemp;
			break;
		case 1:       // minute
			program_datetime[current_program] = (updown * 60)+ timeTemp;
			break;
		case 2:       // year
			program_datetime[current_program] = (updown * 86400L * 365) + timeTemp;
			break;
		case 3:       // month
			program_datetime[current_program] = (updown * 86400L * 30) + timeTemp;
			break;
		case 4:       // day
			program_datetime[current_program] = (updown * 86400L) + timeTemp;
			break;
		}

		// set the settings were changed flag
		ui_status |= B00010000;
	}


	// do we need to paint the first line?
	if (main_pos_old != main_pos) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

	} else {
		// clear only the data line
		lcd.setCursor(0,1);
		lcd.print("               ");
	}

	// paint the new data
	lcd.setCursor(0,1);

	if (program_datetime[current_program].hour() < 10) lcd.print("0");
	lcd.print(program_datetime[current_program].hour(), DEC);
	lcd.print(":");

	if (program_datetime[current_program].minute() < 10) lcd.print("0");
	lcd.print(program_datetime[current_program].minute(), DEC);

	// don't show thw year if this is a weekday program
	if (!program_weekdays[current_program] & B00000001) {
		lcd.print(" ");

		lcd.print(program_datetime[current_program].year(), DEC);
		lcd.print("-");

		if (program_datetime[current_program].month() < 10) lcd.print("0");
		lcd.print(program_datetime[current_program].month(), DEC);
		lcd.print("-");

		if (program_datetime[current_program].day() < 10) lcd.print("0");
		lcd.print(program_datetime[current_program].day(), DEC);
	}


	// set the cursor for blinking at the correct position
	switch (action_pos) {

	case 0:  lcd.setCursor(1,  1); break;     // hour
	case 1:  lcd.setCursor(4,  1); break;     // minute
	case 2:  lcd.setCursor(9,  1); break;     // year
	case 3:  lcd.setCursor(12, 1); break;     // month
	case 4:  lcd.setCursor(15, 1); break;     // day
	}

}


// ======================================================================================
// this function paints the program weekdays status screen
// ======================================================================================
void paint_status_program_weekdays()
{

	boolean toggle = false;

	if ((key == KEY_UP) ||
	    (key == KEY_DOWN)) {
		toggle = true;
	} else if (key == KEY_RIGHT) {
		action_pos++;
	} else if (key == KEY_LEFT) {
		action_pos--;
	}

	// check action_pos limits
	if (action_pos > 6) action_pos = 0;

	if (toggle) {
		switch (action_pos) {

		case 0:       // monday
			program_weekdays[current_program] ^= B10000000;
			break;
		case 1:       // tuesday
			program_weekdays[current_program] ^= B01000000;
			break;
		case 2:       // wednesday
			program_weekdays[current_program] ^= B00100000;
			break;
		case 3:       // thursday
			program_weekdays[current_program] ^= B00010000;
			break;
		case 4:       // friday
			program_weekdays[current_program] ^= B00001000;
			break;
		case 5:       // saturday
			program_weekdays[current_program] ^= B00000100;
			break;
		case 6:       // sunday
			program_weekdays[current_program] ^= B00000010;
			break;
		}   // switch

		check_weekdayFlag();

		// set the settings were changed flag
		ui_status |= B00010000;
	}


	// do we need to paint the first line?
	if (main_pos_old != main_pos) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);
	} else {
		// clear only the data line
		lcd.setCursor(0,1);
		lcd.print("               ");
	}

	// paint the new data
	lcd.setCursor(0,1);
	if (program_weekdays[current_program] & B10000000) { lcd.print("x"); }
	else { lcd.print("_"); }
	lcd.setCursor(2,1);
	if (program_weekdays[current_program] & B01000000) { lcd.print("x"); }
	else { lcd.print("_"); }
	lcd.setCursor(4,1);
	if (program_weekdays[current_program] & B00100000) { lcd.print("x"); }
	else { lcd.print("_"); }
	lcd.setCursor(6,1);
	if (program_weekdays[current_program] & B00010000) { lcd.print("x"); }
	else { lcd.print("_"); }
	lcd.setCursor(8,1);
	if (program_weekdays[current_program] & B00001000) { lcd.print("x"); }
	else { lcd.print("_"); }
	lcd.setCursor(10,1);
	if (program_weekdays[current_program] & B00000100) { lcd.print("x"); }
	else { lcd.print("_"); }
	lcd.setCursor(12,1);
	if (program_weekdays[current_program] & B00000010) { lcd.print("x"); }
	else { lcd.print("_"); }


	// set the cursor for blinking at the correct position
	switch (action_pos) {

	case 0:  lcd.setCursor(0,  1); break;     // m
	case 1:  lcd.setCursor(2,  1); break;     // t
	case 2:  lcd.setCursor(4,  1); break;     // w
	case 3:  lcd.setCursor(6,  1); break;     // t
	case 4:  lcd.setCursor(8,  1); break;     // f
	case 5:  lcd.setCursor(10, 1); break;     // s
	case 6:  lcd.setCursor(12, 1); break;     // s
	}

}






// ======================================================================================
// this function paints the delete program screen
// ======================================================================================
void paint_action_program_delete()
{

	// if select key pressed
	if (key != KEY_SELECT) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print(lines[0]);

		lcd.setCursor(0,1);
		lcd.print(lines[1]);

	} else {

		// create a new program
		delete_program();

		// set the settings were changed flag
		ui_status |= B00010000;

		// back to the main-program menu
		main_pos = 30;
		ui_tree_depth--;

		// fill the screen array with the new values
		fill_screen_array();

		// and initiate a repaint
		ui_status |= B01000000;


	}

}
