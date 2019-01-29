/*********************************************************************************************
* GravitySensorHub.cpp
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
* date    :  2017-04-12
*********************************************************************************************/
#include "GravitySensorHub.h"
#include "GravityPh.h"
#include "GravityTDS.h"
#include "GravityTemperature.h"
#include "Config.h"

//********************************************************************************************
// function name: sensors []
// Function Description: Store the array of sensors
// Parameters: 0 pH sensor
// Parameters: 1 Temperature sensor
// Parameters: 2 Total Dissolveed Solids sensor
//********************************************************************************************

GravitySensorHub::GravitySensorHub()
{
	for (size_t i = 0; i < this->SensorCount; i++)
	{
		this->sensors[i] = NULL;
	}

	this->sensors[phSensor] = new GravityPh();
	this->sensors[temperatureSensor] = new GravityTemperature(TEMPPIN);
	this->sensors[tdsSensor] = new GravityTDS();
	
}

//********************************************************************************************
// function name: ~ GravitySensorHub ()
// Function Description: Destructor, frees all sensors
//********************************************************************************************
GravitySensorHub::~GravitySensorHub(){
	for (size_t i = 0; i < SensorCount; i++){
		if (this->sensors[i]){
			delete this->sensors[i];
		}
	}
}

//********************************************************************************************
// function name: setup ()
// Function Description: Initializes all sensors
//********************************************************************************************
void GravitySensorHub::setup(){
	for (size_t i = 0; i < SensorCount; i++)
	{
		if (this->sensors[i])
		{
			this->sensors[i]->setup();
		}
	}
}

//********************************************************************************************
// function name: updateAll()
// Function Description: Updates all sensor values
//********************************************************************************************
void GravitySensorHub::updateAll(){
	for (size_t i = 0; i < SensorCount; i++){
		if (this->sensors[i]){
			this->sensors[i]->update();
		}
	}
}

void GravitySensorHub::update(int num){
	if (num >= SensorCount){
		return 0;
	}
	return this->sensors[num]->update();
}

//********************************************************************************************
// function name: getValueBySensorNumber ()
// Function Description: Get the sensor data
// Parameters: 0 pH sensor
// Parameters: 1 Temperature sensor
// Parameters: 2 Total Dissolveed Solids sensor
// Return Value: Returns the acquired sensor data
//********************************************************************************************
double GravitySensorHub::getValueBySensorNumber(int num){
	if (num >= SensorCount){
		return 0;
	}
	return this->sensors[num]->getValue();
}
