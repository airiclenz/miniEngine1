/*

    See www.openmoco.org for more information

    2015 Airic Lenz, C.A. Church

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

// Max sketch size size for 32k Arduinos: 28,672 bytes = 0x7000


#include "Wire.h"
#include "LiquidCrystal.h"
#include "EEPROM.h"
#include <avr/pgmspace.h>
#include <RTClib.h>           // Install via "Tools --> Manage Libraries"
// #include <MemoryFree.h>    // See http://www.arduino.cc/playground/Code/AvailableMemory


// --------------------------
// Version of the software

#define VERSION             1
#define SUBVERSION          3


// --------------------------
// Arduino pins

// pin to tell the motor drivers to go to sleep
#define MOTOR_SLEEP_PIN     2
#define MOTOR_STEP_PIN      12
#define MOTOR_DIR_PIN       13

// shutter and focus connections
#define CAMERA_PIN          3
#define FOCUS_PIN           11

// pin for LCD backlight
#define LCD_BACKLIGHT_PIN   10
// pin for reaing analog key input
#define LCD_KEY_PIN         A0


// limit switches for stopping the program
// one for each side of teh dolly
#define LIMIT_SWITCH_PIN_1  A2
#define LIMIT_SWITCH_PIN_2  A3



// --------------------------
// General action status flags:
//
// B0 = operating mode (0 = shoot-move-shoot; 1 = continuous)
// B1 = camera focussing
// B2 = start cycle immediately
// B3 = camera shooting
// B4 = program running
// B5 = camera post exposure
// B6 = motor slot open
// B7 = motor post delay is active

byte action_status = B0;

// the time we started with an action (focussing, exposure, post delay)
unsigned long action_start_time;

// --------------------------
// Camera settings

// cycle length of the whole cycle in half seconds (4 * 0.5 = 2 seconds)
unsigned int cycle_length = 4;

// camera shoot counter
unsigned int camera_shoot_count     = 0;
// the focus line will be risen before the exposure is done (in milliseconds)
unsigned int camera_focus_time      = 0;
// the time we want to expose our pictures (in milliseconds)
unsigned int camera_exp_time        = 100;
// time after exposure before the motor is moved (in milliseconds)
unsigned int camera_exp_post        = 200;

// the maximum amount of shots to do (0 = unlimited)
unsigned int camera_shot_max        = 0;

// --------------------------
// defines the camera behaviour
//
// B0 = Focus line high or low while exposing
// B1 =
// B2 =
// B3 =
// B4 =
// B5 =
// B6 =
// B7 =


byte camera_status        = B0;



// --------------------------
// Motor settings



// motor ramping types
#define RAMPING_LINEAR        0
#define RAMPING_ATANGENS      1
#define RAMPING_SINUS         2


// used motor ramping
#define MOTOR_RAMP_TYPE       RAMPING_ATANGENS // see the definitions just before this line for possible choices.

// acceleration and decelleration time (in steps)
unsigned int motor_ramp            = 500;
// total number of pulses for move (1600 steps per rev)
unsigned long motor_steps           = 0;
// number of maximum steps the motor is allowed to do  (0 = unlimited)
unsigned long motor_steps_max       = 0; // 0 = no limit
// delay after motor movement
unsigned int motor_post             = 0;
// maximum delay in microsecond (delay between steps at minimum
// speed of the motor (start and endphase)).
unsigned long motor_step_delay_max   = 15000;
// minimum delay in microseconds (delay between steps at maximum speed of the motor)
unsigned long motor_step_delay_min   = 100;

// the amount of steps done away from the defined home position
// clockwise = direction bit low (positive count)
// anti-clockwise = direction bit high (negative count)
long home_steps                    = 0;



// General Motor status flags:
//
// B0 = sleep on / off (the pin has to be set inverse --> sleep on = pin low)
// B1 = limit switches on / off
// B2 =
// B3 =
// B4 = motor direction
// B5 =
// B6 =
// B7 =

byte motor_status  = B11000000;


// the time we started the program
unsigned long program_start_time;

// --------------------------
// Display
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);



// --------------------------
// Timed-Programs (RTC) status flags:
//
// B0 = timed-program running
// B1 = active programs in the future
// B2 =
// B3 =
// B4 =
// B5 =
// B6 =
// B7 =

byte program_status = B0;

// the max number of programs available
#define program_amount    20

// the program that is curently edited, 255 = no program is editet
byte current_program =    255;

// the program that is curently running
byte running_program =    255;

// the number of programs we have defined;
byte program_count =      0;

// day flags of the programs
// B0  monday
// B1  tuesday
// B2  wednesday
// B3  thursday
// B4  friday
// B5  saturday
// B6  sunday
// B7  use weekdays (low = use single date)

byte program_weekdays[program_amount];

// start date and time of the program
DateTime program_datetime[program_amount];

// program duration in minutes
unsigned int program_duration[program_amount];

// other flags of the programs
//
// B0  enabled / disabled
// B1  move back to home
// B2
// B3
// B4
// B5
// B6
// B7

byte program_flag[program_amount];

// content of the menu (we initialize the array with some items -
// fitting the longest possible menu - have a look at mE_program
// to check the program amount!!! - all programs will be listed in
// one menu - the longest regular menu is 7 items long)
char lines[20][17];




// --------------------------
// User interface

// --------------------------
// Strings of our menu.
// This is done to store the strings in in the program flash.
// If given like this: "some string", the strings would be placed in
// the RAM.


const char string_0  [] PROGMEM = "On ";
const char string_1  [] PROGMEM = "Off";
const char string_2  [] PROGMEM = "CW ";
const char string_3  [] PROGMEM = "CCW";
const char string_4  [] PROGMEM = "Stp:";
const char string_5  [] PROGMEM = "D:";
const char string_6  [] PROGMEM = "Program started.";
const char string_7  [] PROGMEM = "Program stopped.";
const char string_8  [] PROGMEM = "enabled";
const char string_9  [] PROGMEM = "disabled";

const char string_10 [] PROGMEM = "Camera";
const char string_11 [] PROGMEM = "Motor";
const char string_12 [] PROGMEM = "Program";
const char string_13 [] PROGMEM = "General";
const char string_14 [] PROGMEM = "Settings";
const char string_15 [] PROGMEM = "Version info";

const char string_16 [] PROGMEM = "Cycle length";
const char string_17 [] PROGMEM = "Focus time";
const char string_18 [] PROGMEM = "Exp. time";
const char string_19 [] PROGMEM = "Focus behav.";
const char string_20 [] PROGMEM = "Max. shots";
const char string_21 [] PROGMEM = "Post delay";
const char string_22 [] PROGMEM = "Test shot";

const char string_23 [] PROGMEM = "Motor steps ";
const char string_24 [] PROGMEM = "Direction";
const char string_25 [] PROGMEM = "Motor home";
const char string_26 [] PROGMEM = "Motor sleep";
const char string_27 [] PROGMEM = "Max steps";
const char string_28 [] PROGMEM = "Ramp";

const char string_29 [] PROGMEM = "Operat. mode";
const char string_30 [] PROGMEM = "B-Light time";
const char string_31 [] PROGMEM = "B-Light powr";
const char string_32 [] PROGMEM = "System time ";

const char string_33 [] PROGMEM = "Save settgs.";
const char string_34 [] PROGMEM = "Autosave";
const char string_35 [] PROGMEM = "Reset";

const char string_36 [] PROGMEM = "Developers";
const char string_37 [] PROGMEM = "(alphabt. o.):";
const char string_38 [] PROGMEM = "Airic Lenz,";
const char string_39 [] PROGMEM = "Alvarocalvo,";
const char string_40 [] PROGMEM = "C.A. Church,";
const char string_41 [] PROGMEM = "Marc Lane";
const char string_42 [] PROGMEM = "For questions";
const char string_43 [] PROGMEM = "have a look at";
const char string_44 [] PROGMEM = "openmoco.org  ";
const char string_45 [] PROGMEM = "";                // reserve

const char string_46 [] PROGMEM = "Press SELECT to";

const char string_47 [] PROGMEM = "";
const char string_48 [] PROGMEM = "";
const char string_49 [] PROGMEM = "";
const char string_50 [] PROGMEM = "";
const char string_51 [] PROGMEM = "";
const char string_52 [] PROGMEM = "";

const char string_53 [] PROGMEM = "do a test shot.";

const char string_54 [] PROGMEM = "";
const char string_55 [] PROGMEM = "";
const char string_56 [] PROGMEM = "";
const char string_57 [] PROGMEM = "";
const char string_58 [] PROGMEM = "";

const char string_59 [] PROGMEM = "Set home";
const char string_60 [] PROGMEM = "";

const char string_61 [] PROGMEM = "Add program ";
const char string_62 [] PROGMEM = "add a program.";

const char string_63 [] PROGMEM = "Start time";
const char string_64 [] PROGMEM = "Weekdays";
const char string_65 [] PROGMEM = "Duration";
const char string_66 [] PROGMEM = "Move home";
const char string_67 [] PROGMEM = "Status";
const char string_68 [] PROGMEM = "Delete";

const char string_69 [] PROGMEM = "";
const char string_70 [] PROGMEM = "";
const char string_71 [] PROGMEM = "";

const char string_72 [] PROGMEM = "save settings.";

const char string_73 [] PROGMEM = "Autosave settgs:";

const char string_74 [] PROGMEM = "reset settings.";
const char string_75 [] PROGMEM = "Restoring CFG..";
const char string_76 [] PROGMEM = "Restarting..";

const char string_77 [] PROGMEM = "set motor home.";

const char string_78 [] PROGMEM = "Move to home";
const char string_79 [] PROGMEM = "Moving motor to";
const char string_80 [] PROGMEM = "home. Stand by.";

const char string_81 [] PROGMEM = "Program start:";
const char string_82 [] PROGMEM = "M T W T F S S";
const char string_83 [] PROGMEM = "Duration:";
const char string_84 [] PROGMEM = "Move home @ end:";
const char string_85 [] PROGMEM = "Program status:";

const char string_86 [] PROGMEM = "delete program.";

const char string_87 [] PROGMEM = "high w. shutter";
const char string_88 [] PROGMEM = "low w. shutter";

const char string_89 [] PROGMEM = "continuous";
const char string_90 [] PROGMEM = "shoot-move-shoot";

const char string_91 [] PROGMEM = "anti-clockwise";
const char string_92 [] PROGMEM = "clockwise";

const char string_93 [] PROGMEM = "A limit switch";
const char string_94 [] PROGMEM = "was triggered!";

const char string_95 [] PROGMEM = "Max motor step";
const char string_96 [] PROGMEM = "limit reached!";

const char string_97 [] PROGMEM = "Max camera shot";
const char string_98 [] PROGMEM = "";

const char string_99 [] PROGMEM = "Post delay  ";
const char string_100[] PROGMEM = "Motor post delay:";

const char string_101[] PROGMEM = "Max speed";
const char string_102[] PROGMEM = "Max speed delay:";

const char string_103[] PROGMEM = "Min speed";
const char string_104[] PROGMEM = "Min speed delay:";

const char string_105[] PROGMEM = "Lmt-Switches";
const char string_106[] PROGMEM = "";

/*
   const char string_107[] PROGMEM = "Jog";
   const char string_108[] PROGMEM = "Motor jog:";
   const char string_109[] PROGMEM = "Up=CW  Down=CCW";
 */


// Now set up a table to refer to the strings (a simple list
// of all the strings we defined)

const char *const string_table[] PROGMEM =
{
	string_0,   string_1,   string_2,   string_3,   string_4,
	string_5,   string_6,   string_7,   string_8,   string_9,
	string_10,  string_11,  string_12,  string_13,  string_14,
	string_15,  string_16,  string_17,  string_18,  string_19,
	string_20,  string_21,  string_22,  string_23,  string_24,
	string_25,  string_26,  string_27,  string_28,  string_29,
	string_30,  string_31,  string_32,  string_33,  string_34,
	string_35,  string_36,  string_37,  string_38,  string_39,
	string_40,  string_41,  string_42,  string_43,  string_44,
	string_45,  string_46,  string_47,  string_48,  string_49,
	string_50,  string_51,  string_52,  string_53,  string_54,
	string_55,  string_56,  string_57,  string_58,  string_59,
	string_60,  string_61,  string_62,  string_63,  string_64,
	string_65,  string_66,  string_67,  string_68,  string_69,
	string_70,  string_71,  string_72,  string_73,  string_74,
	string_75,  string_76,  string_77,  string_78,  string_79,
	string_80,  string_81,  string_82,  string_83,  string_84,
	string_85,  string_86,  string_87,  string_88,  string_89,
	string_90,  string_91,  string_92,  string_93,  string_94,
	string_95,  string_96,  string_97,  string_98,  string_99,
	string_100, string_101, string_102, string_103, string_104,
	string_105, string_106  // , string_107, string_108, string_109
};

/*
   PROGMEM prog_char *string_table[] = {
    string_0,   string_1,   string_2,   string_3,   string_4,
    string_5,   string_6,   string_7,   string_8,   string_9,
    string_10,  string_11,  string_12,  string_13,  string_14,
    string_15,  string_16,  string_17,  string_18,  string_19,
    string_20,  string_21,  string_22,  string_23,  string_24,
    string_25,  string_26,  string_27,  string_28,  string_29,
    string_30,  string_31,  string_32,  string_33,  string_34,
    string_35,  string_36,  string_37,  string_38,  string_39,
    string_40,  string_41,  string_42,  string_43,  string_44,
    string_45,  string_46,  string_47,  string_48,  string_49,
    string_50,  string_51,  string_52,  string_53,  string_54,
    string_55,  string_56,  string_57,  string_58,  string_59,
    string_60,  string_61,  string_62,  string_63,  string_64,
    string_65,  string_66,  string_67,  string_68,  string_69,
    string_70,  string_71,  string_72,  string_73,  string_74,
    string_75,  string_76,  string_77,  string_78,  string_79,
    string_80,  string_81,  string_82,  string_83,  string_84,
    string_85,  string_86,  string_87,  string_88,  string_89,
    string_90,  string_91,  string_92,  string_93,  string_94,
    string_95,  string_96,  string_97,  string_98,  string_99,
    string_100, string_101, string_102, string_103, string_104,
    string_105, string_106 // , string_107, string_108, string_109
   };
 */

// UI status flags
//
// B0 = backlight on / off
// B1 = ui repaint flag
// B2 = settings autosave flag
// B3 = settings were changed flag
// B4 = 24h / 12h time (24 = LOW, 12 = HIGH)  not used yet
// B5 = long key press
// B6 = message on screen
// B7 =

byte ui_status = B11100000;


// wait time until backlight turns off in milliseconds
unsigned int backlight_wait = 10;

// backlight level in percent
byte backlight_level=100;

// a variable to remeber when a message started
unsigned long message_start_time;

// a variable to remeber when a message started
byte message_duration;

// --------------------------
// our Real time clock object
RTC_DS1307 RTC;
// and the current time of the system
DateTime time;







// ===================================================================================
// Arduino Setup Procedure
// ===================================================================================
void setup()
{

	//Serial.begin(9600);

	pinMode(MOTOR_DIR_PIN, OUTPUT);
	pinMode(MOTOR_STEP_PIN, OUTPUT);
	pinMode(MOTOR_SLEEP_PIN, OUTPUT);

	pinMode(CAMERA_PIN, OUTPUT);
	pinMode(FOCUS_PIN, OUTPUT);

	pinMode(LCD_BACKLIGHT_PIN, OUTPUT);

	// set motor to sleep
	digitalWrite(MOTOR_SLEEP_PIN, LOW);


	// init the LCD display
	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	lcd.clear();


	// did we previously save settings to eeprom?
	// is config saved and has the config has the correct version?
	if ((is_eeprom_saved()) &&
		(is_OK_eeprom_version()))
	{
		load_config();
	}
	else
	{
		write_config();
		write_eeprom_version();
	}


	// enable backlight
	setBacklightLevel();


	byte scroll_up[8] =
	{
		B00100,
		B01110,
		B11111,
		B00100,
		B00100,
		B00000,
		B00000,
		B00000
	};

	byte scroll_down[8] =
	{
		B00000,
		B00000,
		B00000,
		B00100,
		B00100,
		B11111,
		B01110,
		B00100
	};


	// create the special chars we need for the UI
	lcd.createChar(0, scroll_up);
	lcd.createChar(1, scroll_down);

	/*
	   Serial.println();
	   Serial.print(freeMemory());
	   Serial.println(" byte free.");
	 */


	// welcome screen
	lcd.setCursor(3, 0);
	lcd.print("miniEngine");
	lcd.setCursor(6, 1);
	lcd.print("v");
	lcd.print(VERSION);
	lcd.print(".");
	lcd.print(SUBVERSION);

	delay(1500);


	// RTC Stuff
	Wire.begin();
	RTC.begin();

}





// ===================================================================================
// Arduinos eternal loop procedure
// ===================================================================================
void loop()
{

	// do the screen stuff
	do_screen();

	// programm is running
	if (action_status & B00001000)
	{

		// =================================================
		// C O N T I N U O U S   m o d e
		// =================================================
		if (action_status & B10000000)
		{

			// do some continuous steps
			doMotorContinuous();

			// if it is time or the "start a new cycle immediately" flag is set
			if ((action_status & B00100000) ||
			((program_start_time + ((unsigned long) cycle_length * 500UL * (unsigned long) camera_shoot_count)) <= millis()))
			{

				// if not exposing right now and not in camera post delay (makes this sense in continuous??)
				if (!(action_status & B01000000) &&
				    !(action_status & B00010000) &&
				    !(action_status & B00000100))
				{

					// do some continuous steps
					doMotorContinuous();

					// should we do focussing?
					if (camera_focus_time > 0)
					{
						camera_focus();
					}
					else
					{
						// shoot camera
						camera_shoot();
					}

				}

				// do some continuous steps
				doMotorContinuous();

				// delete the start immediately flag if set
				if (action_status & B00100000)
				{
					// delete the start immediately flag
					bitClear(action_status, 5);      // B11011111
				}
			}

			// do some continuous steps
			doMotorContinuous();


			// is camera focussing right now?
			if (action_status &  B01000000)
			{
				// did we focus the time we wanted to focus?
				if((action_start_time + (unsigned long) camera_focus_time) <= millis())
				{
					// stop focussing (this function automatically starts the exposure)
					camera_stop_focus();
				}
			}

			// do some continuous steps
			doMotorContinuous();

			// is camera exposing right now?
			if (action_status &  B00010000)
			{
				// did we exposed the time we wanted to expose?
				if((action_start_time + (unsigned long)camera_exp_time) <= millis())
				{
					// stop shooting
					camera_stop_shoot();
				}
			}

			// do some continuous steps
			doMotorContinuous();

			// now check the limit switches if we need to stop the program
			// this function is called as late as possible after the key reading because
			// analogRead needs some pre delay time to deliver accurate measurements...
			check_limit_switches();

		}

		// =================================================
		// S H O O T - M O V E - S H O O T   m o d e
		// =================================================
		else {

			// if it is time or the "start a new cycle immediately" flag is set
			if ((action_status & B00100000) ||
				((program_start_time + ((unsigned long) cycle_length * 500UL * (unsigned long) camera_shoot_count)) <= millis()))
			{
				// if not focussing or exposing right now and not in camera or motor post delay
				if (!(action_status & B01000000) &&
					!(action_status & B00010000) &&
					!(action_status & B00000100) &&
					!(action_status & B00000001) )
				{
					// should we do focussing?
					if (camera_focus_time > 0)
					{
						camera_focus();
					}
					else
					{
						// shoot camera
						camera_shoot();
					}
				}


				// delete the start immediately flag if set
				if (action_status & B00100000)
				{
					// toggle the start immediately flag to off
					bitClear(action_status, 5);
				}
			}


			// is camera focussing right now?
			if (action_status &  B01000000)
			{
				// did we focus the time we wanted to focus?
				if((action_start_time + (unsigned long) camera_focus_time) <= millis())
				{
					// stop focussing (this function automatically starts the exposure)
					camera_stop_focus();
				}
			}


			// is camera exposing right now?
			if (action_status &  B00010000)
			{
				// did we exposed the time we wanted to expose?
				if((action_start_time + camera_exp_time) <= millis())
				{
					// stop shooting
					camera_stop_shoot();
				}
			}


			// post exposure delay
			if (action_status & B00000100)
			{
				// did we exposed the time we wanted to expose?
				if((action_start_time + (unsigned long) camera_exp_time + (unsigned long) camera_exp_post) <= millis())
				{
					camera_stop_post();
				}
			}


			// time for the engines
			if (action_status & B00000010)
			{
				// do the blocking motor phase with error check
				doMotorPhase(true);

				motor_stop();

				// now check the limit switches if we need to stop the program
				// this function is called after the blocking motor phase because
				// analogRead needs some pre delay time to deliver accurate
				// measurements...
				check_limit_switches();

			}


			// motor post delay
			if (action_status & B00000001)
			{
				// did we waited the time we wanted to wait?
				if((action_start_time + (unsigned long) motor_post) <= millis())
				{
					// clear the waintg flag...
					bitClear(action_status, 0);      // B11111110
				}
			}

		}   // operation mode


		// if the programm is running and we are in contiuous mode
		continuous_check();

	}   // program is running

	// read the time from the RTC (valid for the next loop)
	time = RTC.now();

	// if the programm is running and we are in contiuous mode
	continuous_check();

	// the function that checks if we need to start or stop a program
	check_programs();

	// if the programm is running and we are in contiuous mode
	continuous_check();

	// check if there are programs that will be triggered in the future
	// this is done for being able to display this information ("smart P")
	check_programFuture();

}
