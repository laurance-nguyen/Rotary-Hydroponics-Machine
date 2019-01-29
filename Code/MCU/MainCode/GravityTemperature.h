/*********************************************************************
* GravityTemperature.h
*
* Description:
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-12
**********************************************************************/
#pragma once
#include "ISensor.h"
#include "OneWire.h"
#define StartConvert 0
#define ReadTemperature 1

class GravityTemperature : public ISensor
{
public:
	int temperaturePin;	// Temperature sensor pin
	double temperature;	// Temperature value

public:
	GravityTemperature(int pin);
	~GravityTemperature();

	// Initialization
	void  setup ();

	// Update the sensor data
	void  update ();

	// Get the sensor data
	double getValue();

private:

	OneWire * oneWire;
	unsigned  long tempSampleInterval = 850 ;
	unsigned  long tempSampleTime;

	// Analyze temperature data
	double TempProcess(bool ch);
};

