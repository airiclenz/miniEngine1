/*

    See www.openmoco.org for more information

    2015 Airic Lenz

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

// milliseconds needed to let the motor settle and break after
// action (before sleep is activated)
#define    motor_break_delay 6


// count the number of pulses
unsigned long step_count = 0;

// delta between min speed delay and max speed delay
unsigned long delay_delta;

// the ramp that is used (after corrections)
unsigned long used_motor_ramp;

// constinuous mode variables
float steps_per_milli;

// the number of steps we need to to for the next continuous phase
unsigned long steps_to_do;

// the time
unsigned long now;

// counter fot all continuous steps done
unsigned long continuous_steps_done;

// when did we check the limit switches the last time?
unsigned long limit_switch_last_time;

// tollgle flagg for measuring the limit switches and keys - but not at the samt time
byte analog_measure_toggle;


// ===================================================================================
void motor_start()
{

	// start the motor phase if we have motor_steps defined
	if (motor_steps > 0)
	{

		prepare_motor_values();

		// open the slot for the motor
		action_status |= B00000010;
	}
}

// ===================================================================================
void motor_stop()
{

	// close the time slot for the motor
	bitClear(action_status, 1);

	// if we want a motor post delay, activate is now
	if (motor_post > 0)
	{

		// set the corresponding flag
		action_status |= B00000001;

		// remember the time we started
		action_start_time = millis();
	}

}



// ===================================================================================
void doMotorContinuous()
{

	now = millis();

	//               |  time total since start * s_p_m = theoret. steps total |
	steps_to_do =    ( (float) (now - program_start_time) * steps_per_milli   ) - continuous_steps_done;


	// check if our next move will reach the max step limit
	if ( (motor_steps_max != 0) &&
	     ((abs((unsigned long) home_steps) + (unsigned long) steps_to_do) >= motor_steps_max))
	{

		// stop the program
		stop_program();

		// print a message: "Max motor step limit reached!";
		print_message(95, 96, 3);

		return;
	}


	// do all the steps we need to do
	for (long i=0; i < steps_to_do; i++)
	{

		// Move stepper one pulse using delay just calculated
		digitalWrite(MOTOR_STEP_PIN, HIGH);
		delayMicroseconds(5);
		digitalWrite(MOTOR_STEP_PIN, LOW);

		//delayMicroseconds(200);
		delayMicroseconds(motor_step_delay_min);

		continuous_steps_done++;
	}

	// add the steps to the home counter
	// depends on the current motor direction
	if (motor_status & B00001000)
	{
		home_steps -= (long) steps_to_do;
	}
	else
	{
		home_steps += (long) steps_to_do;
	}

}




// ===================================================================================
boolean doMotorPhase(boolean check_for_errors)
{

	step_count = 0;

	if (check_for_errors)
	{

		// check if our next move will reach the max step limit
		if ( (motor_steps_max != 0) &&
		     ((abs(home_steps) + motor_steps) > motor_steps_max))
		{

			// stop the program
			stop_program();

			// print a message: "Max motor step limit reached!";
			print_message(95, 96, 3);

			return false;
		}
	}

	// wake up the engine
	digitalWrite(MOTOR_SLEEP_PIN, HIGH);


	// while there are still steps to do...
	while (step_count < motor_steps)
	{

		now = millis();


		// do some analog readings
		if (limit_switch_last_time + 50UL <= now)
		{

			int val;

			if      (analog_measure_toggle == 0) val = analogRead(LIMIT_SWITCH_PIN_1);
			else if (analog_measure_toggle == 1) val = analogRead(LIMIT_SWITCH_PIN_2);
			else val = analogRead(LCD_KEY_PIN);

			if (analog_measure_toggle == 2)
			{

				// if we have a pressed SELECT key
				if ((val > key_3_val) && (val < key_4_val))
				{
					// stop the program
					stop_program();
					display_home();
					delay (250);
					return true;
				}

				// if we should check for error and
				// if the limit switches are enabled
			}
			else if ((check_for_errors) &&
			           (motor_status & B01000000))
			{

				// if we have a triggered limit switch
				if (val >= 1000) {
					limit_switch_event();
					return false;
				}
			}

			// toggle the toggle flag to the next position (3 positions)
			if (analog_measure_toggle == 2) analog_measure_toggle = 0;
			else analog_measure_toggle++;

			// remember the last time we measured
			limit_switch_last_time = now;
		}


		// determine the ramp delay for the current step
		unsigned long step_delay = calculate_ramp_delay();


		// Move stepper one pulse using the delay just calculated
		digitalWrite(MOTOR_STEP_PIN, HIGH);
		delayMicroseconds(5);
		digitalWrite(MOTOR_STEP_PIN, LOW);
		delayMicroseconds(step_delay);
		step_count++;


		// add the step to the home variables
		// depending on the motor direction

		if (motor_status & B00001000) home_steps--;
		else home_steps++;

	}



	// wait a little time to let the motor settle and break
	delay (motor_break_delay);

	// bring the engine back to sleep if desired
	if (motor_status & B10000000)
	{
		digitalWrite(MOTOR_SLEEP_PIN, LOW);
	}

	return true;

}


// ===================================================================================
unsigned long calculate_ramp_delay()
{

	unsigned long res;
	float x_scale;

	// acceleration phase
	if (step_count < used_motor_ramp)
	{

		// x scale represents the position in the current speed-phase between 0 (beginning) and 1 (end)
		x_scale = (float) step_count / (float) used_motor_ramp;
	}

	// reached deceleration phase
	else if (step_count > (motor_steps - used_motor_ramp))
	{

		// x scale represents the position in the current speed-phase between 0 (beginning) and 1 (end)
		x_scale = (float) (motor_steps - step_count) / (float) used_motor_ramp;

	}

	// reached max speed phase
	else
	{

		// wait a little bit because here should be a calculation which requires some time
		// (each calc required a different amount);
		switch(MOTOR_RAMP_TYPE)
		{
			case RAMPING_LINEAR:    delayMicroseconds(50); break;
			case RAMPING_ATANGENS:  delayMicroseconds(200); break;
			case RAMPING_SINUS:     delayMicroseconds(200); break;
		}

		// now return the result
		return motor_step_delay_min;
	}

	switch(MOTOR_RAMP_TYPE)
	{
		case RAMPING_LINEAR:
			res = x_scale * delay_delta;
			break;
		case RAMPING_ATANGENS:
			res = (atan(15.0 * x_scale) / 1.51) * delay_delta;      // more steep curve
			break;
		case RAMPING_SINUS:
			res = (((cos(PI * (x_scale - 1))) / 2) + 0.5) * delay_delta;
			break;
	}


	if (motor_step_delay_min > (motor_step_delay_max - res))
	{
		return motor_step_delay_min;
	}
	else
	{
		return motor_step_delay_max - res;
	}

}


// ===================================================================================
void prepare_motor_values()
{

	// if ramp is longer than half of the total phase - shorten it (/2):
	if (motor_ramp > (motor_steps >> 1))
	{
		used_motor_ramp = motor_steps >> 1;
	}
	else
	{
		used_motor_ramp = motor_ramp;
	}

	// avoid divisons by zero in the next step
	if (used_motor_ramp == 0) used_motor_ramp = 1;

	// calculate some initial values needed for the ramping curve:
	// if ramping was cropped, get the factor it was cropped (0 >= n <= 1
	float ramp_fac = (float) used_motor_ramp / (float) motor_ramp;
	// and slightly amplify it for better performance with the arc-tan curve
	ramp_fac = sqrt(sqrt(ramp_fac));
	// multiply the new speed delta with our factor
	delay_delta = ramp_fac * (float) (motor_step_delay_max - motor_step_delay_min);


	// Set the stepper direction and
	// determine the algebraic sign for the home counter
	if (motor_status & B00001000)
	{
		digitalWrite(MOTOR_DIR_PIN, HIGH);
	}
	else
	{
		digitalWrite(MOTOR_DIR_PIN, LOW);
	}


	// calculate the amount of steps per millisecond we have to
	// do in case we are in continuous mode
	steps_per_milli = (float) motor_steps / ((float) cycle_length * 500.0);

	continuous_steps_done = 0;

}





// ===================================================================================
void move_motor_to_home()
{

	// set the step values for the motor and remember the old ones
	long old_steps = motor_steps;
	motor_steps = abs(home_steps);

	boolean direction_old;

	// remeber the old direction
	if (motor_status & B00001000)
	{
		direction_old = true;
	}
	else
	{
		direction_old = false;
	}


	// set the correct direction to move back
	if (home_steps > 0)
	{
		motor_status |= B00001000;
	}
	else
	{
		bitClear(motor_status, 3);
	}

	// wait until the user has release the SELECT key
	delay(350);

	// prepare
	prepare_motor_values();

	// move
	doMotorPhase(false);

	// set the old values
	motor_steps = old_steps;
	home_steps = 0;

	// set the old direction
	if (direction_old)
	{
		motor_status |= B00001000;
	}
	else
	{
		bitClear(motor_status, 3);
	}

}



// ======================================================================================
void continuous_check()
{
	// if the programm is running and we are in contiuous mode
	if ((action_status & B10000000) &&
	    (action_status & B00001000))
	{
		// do some continuous steps
		doMotorContinuous();
	}
}
