/*********************************************************************
* GravitySensorHub.h
*
* Copyright (C)    2017   [DFRobot](http://www.dfrobot.com),
* GitHub Link :https://github.com/DFRobot/watermonitor
* This Library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Description:
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-07
**********************************************************************/

#pragma once
#include "ISensor.h"
#include "Config.h"
/*
sensors :
0,ph
1,temperature
2,tds
*/
class GravitySensorHub
{
private:
	static const int SensorCount = SENSORCOUNT;

public:
	//********************************************************************************************
	// function name: sensors []
	// Function Description: Store the array of sensors
	// Parameters: 0 pH sensor
	// Parameters: 1 Temperature sensor
	// Parameters: 2 Total Dissolveed Solids sensor
	//********************************************************************************************
	ISensor *sensors[SensorCount] = {0};
public:
	GravitySensorHub();
	~GravitySensorHub();

	// Initialize all sensors
	void  setup ();

	// Update all sensor values
	void  updateAll ();

	// Update sensor values
	void update(int num);

	// Get the sensor data
	double getValueBySensorNumber(int num);
};
