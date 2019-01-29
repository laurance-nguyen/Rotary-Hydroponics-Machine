/*********************************************************************
* GravityRtc.h
*
* Copyright (C)    2017   [DFRobot](http://www.dfrobot.com),
* GitHub Link :https://github.com/DFRobot/Gravity-I2C-SD2405-RTC-Module/
* This Library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Description:Get real-time clock data
*
* Product Links：https://www.dfrobot.com/wiki/index.php/Gravity:_I2C_SD2405_RTC_Module_SKU:_DFR0469
*
* Sensor driver pin：I2C
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-18
**********************************************************************/
#ifndef GRAVITYRTC_h
#define GRAVITYRTC_h
#pragma once
#include <Arduino.h>
#include <Wire.h>

#define RTC_Address   0x32  //RTC_Address 


class GravityRtc{
public:
	GravityRtc() {};
	~GravityRtc() {};

public:
	//Year Month Day Weekday Minute Second
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t week;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	
	//Initialize RTC time to set the corresponding year, month, day, Weekday Minute Second
	void init(const __FlashStringHelper* date, const __FlashStringHelper* time);
	
	void adjustRtc(uint16_t year,uint8_t month,uint8_t day,uint8_t week,
            	   uint8_t hour,uint8_t minute,uint8_t second);

	//initialization
	void setup();

	//Update sensor data
	void read();

	void getTime(byte& year, byte& month, byte& date, byte& DoW, byte& hour, byte& minute, byte& second); 
	byte getSecond(); 
	byte getMinute(); 
	byte getHour(bool& h12, bool& PM); 
	// In addition to returning the hour register, this function
	// returns the values of the 12/24-hour flag and the AM/PM flag.
	byte getDoW(); 
	byte getDate(); 
	byte getMonth(bool& Century); 
	// Also sets the flag indicating century roll-over.
	byte getYear(); 
	// Last 2 digits only

	// Time-setting functions
	// Note that none of these check for sensibility: You can set the
	// date to July 42nd and strange things will probably result.
		
	void setSecond(byte Second); 
	// In addition to setting the seconds, this clears the 
	// "Oscillator Stop Flag".
	void setMinute(byte Minute); 
	// Sets the minute
	void setHour(byte Hour); 
	// Sets the hour
	void setDoW(byte DoW); 
	// Sets the Day of the Week (1-7);
	void setDate(byte Date); 
	// Sets the Date of the Month
	void setMonth(byte Month); 
	// Sets the Month of the year
	void setYear(byte Year); 
	// Last two digits of the year
	void setClockMode(bool h12); 
	// Set 12/24h mode. True is 12-h, false is 24-hour.

	// Temperature function

	float getTemperature(); 

	// Alarm functions
		
	void getA1Time(byte& A1Day, byte& A1Hour, byte& A1Minute, byte& A1Second, byte& AlarmBits, bool& A1Dy, bool& A1h12, bool& A1PM); 
/* Retrieves everything you could want to know about alarm
 * one. 
 * A1Dy true makes the alarm go on A1Day = Day of Week,
 * A1Dy false makes the alarm go on A1Day = Date of month.
 *
 * byte AlarmBits sets the behavior of the alarms:
 *	Dy	A1M4	A1M3	A1M2	A1M1	Rate
 *	X	1		1		1		1		Once per second
 *	X	1		1		1		0		Alarm when seconds match
 *	X	1		1		0		0		Alarm when min, sec match
 *	X	1		0		0		0		Alarm when hour, min, sec match
 *	0	0		0		0		0		Alarm when date, h, m, s match
 *	1	0		0		0		0		Alarm when DoW, h, m, s match
 *
 *	Dy	A2M4	A2M3	A2M2	Rate
 *	X	1		1		1		Once per minute (at seconds = 00)
 *	X	1		1		0		Alarm when minutes match
 *	X	1		0		0		Alarm when hours and minutes match
 *	0	0		0		0		Alarm when date, hour, min match
 *	1	0		0		0		Alarm when DoW, hour, min match
 */
	void getA2Time(byte& A2Day, byte& A2Hour, byte& A2Minute, byte& AlarmBits, bool& A2Dy, bool& A2h12, bool& A2PM); 
	// Same as getA1Time();, but A2 only goes on seconds == 00.
	void setA1Time(byte A1Day, byte A1Hour, byte A1Minute, byte A1Second, byte AlarmBits, bool A1Dy, bool A1h12, bool A1PM); 
	// Set the details for Alarm 1
	void setA2Time(byte A2Day, byte A2Hour, byte A2Minute, byte AlarmBits, bool A2Dy, bool A2h12, bool A2PM); 
	// Set the details for Alarm 2
	void turnOnAlarm(byte Alarm); 
	// Enables alarm 1 or 2 and the external interrupt pin.
	// If Alarm != 1, it assumes Alarm == 2.
	void turnOffAlarm(byte Alarm); 
	// Disables alarm 1 or 2 (default is 2 if Alarm != 1);
	// and leaves the interrupt pin alone.
	bool checkAlarmEnabled(byte Alarm); 
	// Returns T/F to indicate whether the requested alarm is
	// enabled. Defaults to 2 if Alarm != 1.
	bool checkIfAlarm(byte Alarm); 
	// Checks whether the indicated alarm (1 or 2, 2 default);
	// has been activated.

	// Oscillator functions

	void enableOscillator(bool TF, bool battery, byte frequency); 
	// turns oscillator on or off. True is on, false is off.
	// if battery is true, turns on even for battery-only operation,
	// otherwise turns off if Vcc is off.
	// frequency must be 0, 1, 2, or 3.
	// 0 = 1 Hz
	// 1 = 1.024 kHz
	// 2 = 4.096 kHz
	// 3 = 8.192 kHz (Default if frequency byte is out of range);
	void enable32kHz(bool TF); 
	// Turns the 32kHz output pin on (true); or off (false).
	bool oscillatorCheck();;
	// Checks the status of the OSF (Oscillator Stop Flag);.
	// If this returns false, then the clock is probably not
	// giving you the correct time.
	// The OSF is cleared by function setSecond();.
	

private:
	uint8_t date[7];

	//Read RTC Time
	void readRtc();

	//Analysis RTC Time
	void processRtc();

	// Convert normal decimal numbers to binary coded decimal
	uint8_t decTobcd(uint8_t num);
	// Convert binary coded decimal to normal decimal numbers
	uint8_t bcdTodec(uint8_t num);
	// Read selected control byte: (0); reads 0x0e, (1) reads 0x0f
	uint8_t readControlByte(bool which);
	// Write the selected control byte. 
	// which == false -> 0x0e, true->0x0f.
	void writeControlByte(uint8_t control, bool which);
	void WriteTimeOn(void);
	void WriteTimeOff(void);
	unsigned long timeUpdate;

	//adjust RTC
	uint8_t dayOfTheWeek();
	// number of days since 2000/01/01, valid for 2001..2099
 	uint16_t date2days(uint16_t y, uint8_t m, uint8_t d);
	
	uint8_t conv2d(const char* p);

};

#endif
