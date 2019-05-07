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



// ===================================================================================
void camera_focus()
{

	// enpower the focus pin
	digitalWrite(FOCUS_PIN, HIGH);

	// remember the starting time
	action_start_time = millis();

	// set the "i am focussing" flag to on
	action_status |= B01000000;

}


// ===================================================================================
// this function does not close the focus line because it will be handled in the
// function camera_shoot - called directly after this methode
void camera_stop_focus()
{

	// toggle the "i am focussing" flag to off B01000000
	bitClear(action_status, 6);

	// now shoot the camera
	camera_shoot();
}



// ===================================================================================
void camera_shoot()
{

	// close or open the focus line while shooting (relevant for shootimg behavior of different camera brands)
	if (camera_status & B10000000) {
		digitalWrite(FOCUS_PIN, HIGH);
	} else {
		digitalWrite(FOCUS_PIN, LOW);
	}

	// enpower the camera pin
	digitalWrite(CAMERA_PIN, HIGH);

	// remember the starting time
	action_start_time = millis();

	// set the "i am exposing" flag to on
	action_status |= B00010000;

}


// ===================================================================================
// blocking function for test shots
void camera_shoot(
	long exposure)
{

	camera_shoot();

	delay(exposure);

	// depower the camera pin
	digitalWrite(CAMERA_PIN, LOW);
	// depower the focus pin
	digitalWrite(FOCUS_PIN, LOW);

	// toggle the "i am exposing" flag to off B00010000
	bitClear(action_status, 4);
}


// ===================================================================================
void camera_stop_shoot()
{


	// depower the camera pin
	digitalWrite(CAMERA_PIN, LOW);
	// depower the focus pin
	digitalWrite(FOCUS_PIN, LOW);


	// toggle the "i am exposing" flag to off B00010000
	bitClear(action_status, 4);

	// increment the shoot counter
	camera_shoot_count++;

	// check if we reached the maximum shot number
	if ((camera_shot_max != 0) &&
		(camera_shoot_count >= camera_shot_max))
	{
		stop_program();

		// print a message: "Max camera shot limit reached!";
		print_message(97, 96, 3);

		return;
	}

	// if we want to have a past exposure time than start it now
	// if we are not in continuous mode
	if (!(action_status & B10000000))
	{
		// do the next step in the cycle
		camera_start_post();
	}
}


// ===================================================================================
void camera_start_post()
{

	// set the past exposure flag
	if (camera_exp_post > 0)
	{
		action_status |= B00000100;
	}
	// otherwise start the motor phase
	else
	{
		motor_start();
	}
}

// ===================================================================================
void camera_stop_post()
{

	// clear camera post delay bit
	bitClear(action_status, 2);

	// and start the motor
	motor_start();

}
