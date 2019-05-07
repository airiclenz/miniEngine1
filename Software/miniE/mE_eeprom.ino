/*

    See www.openmoco.org for more information

    2015 Alvarocalvo, Airic Lenz

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
// Functions to manage EEPROM memory
// ======================================================================================



// ======================================================================================
boolean is_OK_eeprom_version()
{
	int v;
	int sv;
	eeprom_read(1,v);
	delay(10);
	eeprom_read(3,sv);
	delay(10);
	return (v==VERSION && sv==SUBVERSION);

}

// ======================================================================================
boolean is_eeprom_saved()
{
	byte not_saved = EEPROM.read(0);
	return(!not_saved);
}

// ======================================================================================
void write_eeprom_version()
{
	int v=VERSION;
	int sv=SUBVERSION;
	eeprom_write(1,v);
	delay(10);
	eeprom_write(3,sv);
	delay(10);

}

// ======================================================================================
void eeprom_saved(
	boolean saved ) //change the flag eeprom memory status
{
	EEPROM.write(0,!saved);
}

// ======================================================================================
void eeprom_write(
	int addr,
	byte& value,
	byte length)
{
	byte* p = (byte*)(void*)&value;

	for(byte i=0; i<length; i++)
	{
		EEPROM.write(addr++,*p++);
	}

	eeprom_saved(true);  //change our flag: the memory is saved
}

// ======================================================================================
void eeprom_write(
	int addr,
	int& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_write(addr,*p, sizeof(int));
}

// ======================================================================================
void eeprom_write(
	int addr,
	long& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_write(addr,*p, sizeof(long));
}

// ======================================================================================
void eeprom_write(
	int addr,
	unsigned int& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_write(addr,*p,sizeof(int));
}

// ======================================================================================
void eeprom_write(
	int addr,
	unsigned long& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_write(addr,*p,sizeof(long));
}

// ======================================================================================
void eeprom_write(
	int addr,
	float& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_write(addr,*p,sizeof(float));
}

// ======================================================================================
void eeprom_write(
	int addr,
	byte& value)
{
	EEPROM.write(addr,value);
	eeprom_saved(true);
}

// ======================================================================================
void eeprom_write(
	int addr,
	short int& value)
{
	EEPROM.write(addr,value);
	eeprom_saved(true);
}



// ======================================================================================
void eeprom_read(
	int addr,
	byte& value,
	byte length)
{
	byte* p = (byte*)(void*)&value;
	for(byte i=0; i<length; i++)
		*p++=EEPROM.read(addr++);
}

// ======================================================================================
void eeprom_read(
	int addr,
	byte& value)
{
	value = EEPROM.read(addr);
}

// ======================================================================================
void eeprom_read(
	int addr,
	int& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_read(addr,*p,sizeof(int));
}

// ======================================================================================
void eeprom_read(
	int addr,
	long& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_read(addr,*p,sizeof(long));

}

// ======================================================================================
void eeprom_read(
	int addr,
	unsigned int& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_read(addr,*p,sizeof(int));
}

// ======================================================================================
void eeprom_read(
	int addr,
	unsigned long& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_read(addr,*p,sizeof(value));

}

// ======================================================================================
void eeprom_read(
	int addr,
	float& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_read(addr,*p,sizeof(value));
}

// ======================================================================================
void eeprom_read(
	int addr,
	short int& value)
{
	byte* p = (byte*)(void*)&value;
	eeprom_read(addr,*p,sizeof(value));
}




// ======================================================================================
// This function writes all settings to the eeprom
// ======================================================================================
void write_config() {

	int tmp;              // tmp integer var
	int address;          // our address
	unsigned long date;   // temp date var

	// --------------------------
	// 6-299: bytes for miniE

	address =  6;

	eeprom_write(address, cycle_length);         address += 2;

	eeprom_write(address, camera_focus_time);    address += 2;
	eeprom_write(address, camera_exp_time);      address += 2;
	eeprom_write(address, camera_exp_post);      address += 2;
	eeprom_write(address, camera_shot_max);      address += 2;
	// Focus line mode
	eeprom_write(address, camera_status);        address += 1;

	eeprom_write(address, motor_ramp);           address += 2;
	eeprom_write(address, motor_steps);          address += 4;
	eeprom_write(address, motor_steps_max);      address += 4;
	// Motor sleep & direction
	eeprom_write(address, motor_status);         address += 1;
	eeprom_write(address, motor_post);           address += 2;
	eeprom_write(address, motor_step_delay_max); address += 4;
	eeprom_write(address, motor_step_delay_min); address += 4;


	//store operate mode flag
	tmp = (action_status & B10000000) ? 1 : 0;
	eeprom_write(address, tmp);                  address += 2;



	// --------------------------
	// 300-499: bytes for general settings

	address =  300;

	//write into eeprom UI configurations
	eeprom_write(address, backlight_level);      address += 1;
	eeprom_write(address, backlight_wait);       address += 2;

	// autosave settings flag
	tmp=(ui_status & B00100000) ? 1 : 0;
	eeprom_write(address, tmp);                    address += 2;

	// --------------------------
	// 500-999:  Program data

	address =  500;

	eeprom_write(address, program_count);        address += 1;

	// loop all existing programs
	for (byte p=0; p < program_count; p++)
	{

		eeprom_write(address, program_weekdays[p]);        address += 1;

		date = program_datetime[p].unixtime();
		eeprom_write(address, date);                       address += 4;

		eeprom_write(address, program_duration[p]);        address += 2;
		eeprom_write(address, program_flag[p]);            address += 1;

	}

}


// ======================================================================================
// This function reades all settings from the eeprom
// ======================================================================================
void load_config()
{

	int tmp;              // tmp integer var
	int address;          // our address
	unsigned long date;   // temp date var

	// --------------------------
	// 6-299: bytes for miniE

	address =  6;

	eeprom_read(address, cycle_length);           address += 2;

	eeprom_read(address, camera_focus_time);      address += 2;
	eeprom_read(address, camera_exp_time);        address += 2;
	eeprom_read(address, camera_exp_post);        address += 2;
	eeprom_read(address, camera_shot_max);        address += 2;
	// Focus line mode
	eeprom_read(address, camera_status);          address += 1;


	eeprom_read(address, motor_ramp);             address += 2;
	eeprom_read(address, motor_steps);            address += 4;
	eeprom_read(address, motor_steps_max );       address += 4;
	// Motor sleep & direction
	eeprom_read(address, motor_status);           address += 1;
	eeprom_read(address, motor_post);             address += 2;
	eeprom_read(address, motor_step_delay_max);   address += 4;
	eeprom_read(address, motor_step_delay_min);   address += 4;


	// Operating mode flag
	eeprom_read(address, tmp);                  address += 2;
	if(tmp==1)
	{
		action_status |= B10000000;
	}
	else
	{
		bitClear(action_status,7);   // B01111111
	}



	// --------------------------
	// 300-499: bytes for general settings

	address =  300;

	eeprom_read(address, backlight_level);     address += 1;
	eeprom_read(address, backlight_wait);      address += 2;

	// autosave settings
	eeprom_read(address, tmp);                 address += 2;
	if(tmp==1)
	{
		ui_status |= B00100000;
	}
	else
	{
		bitClear(ui_status,5);   // B11011111
	}


	// --------------------------
	// 500-750:  Program data
	// 30 programs = 241 byte

	address =  500;

	eeprom_read(address, program_count);      address += 1;

	// loop all existing programs
	for (byte p=0; p<program_count; p++)
	{

		eeprom_read(address, program_weekdays[p]);        address += 1;

		eeprom_read(address, date);                       address += 4;
		program_datetime[p] = date;

		eeprom_read(address, program_duration[p]);        address += 2;
		eeprom_read(address, program_flag[p]);            address += 1;

	}

}


// ======================================================================================
// Function to restart the Arduino
// ======================================================================================
//reset_arduino() function:
void (* reset_arduino) (void) = 0; //declare reset function @ address 0
