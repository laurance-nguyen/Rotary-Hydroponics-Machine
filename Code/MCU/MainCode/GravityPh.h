/*********************************************************************
* GravityPh.h
*
* Sensor driver pin：A2 
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-07
**********************************************************************/
#pragma once
#include <Arduino.h>
#include "ISensor.h"
#include "Config.h"

class GravityPh: public ISensor{
public:
	// pH sensor pin
	int pin;
	// Offset compensation
	float offset;

public:
	GravityPh();
	~GravityPh();
	// Initialization
	void  setup ();
	// Update the sensor data
	void  update ();
	// Get the sensor data
	double getValue();
	// Set offset
	void setOffset(float offset);

private:
	int pHArray[ARRAYLENGTH];    // Stores the average value of the sensor return data
	float pHValue;
};
