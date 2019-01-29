/*********************************************************************
* GravityTemperature.cpp
*
* Description:
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-12
**********************************************************************/

#include "GravityTemperature.h"
#include <OneWire.h>
//#include "Debug.h"

GravityTemperature::GravityTemperature(int pin){
	this->oneWire = new OneWire(pin);
}

GravityTemperature::~GravityTemperature(){
}

//********************************************************************************************
// function name: setup ()
// Function Description: Initializes the sensor
//********************************************************************************************
void GravityTemperature::setup(){
}

//********************************************************************************************
// function name: update ()
// Function Description: Update the sensor value
//********************************************************************************************
void GravityTemperature::update()
{
	if ( millis () - tempSampleTime >= tempSampleInterval)
	{
		tempSampleTime = millis ();
		temperature = TempProcess(ReadTemperature);  // Read the current temperature from the  DS18B20
		TempProcess(StartConvert);                   // After the reading,start the convert for next reading
	}
}

//********************************************************************************************
// function name: getValue ()
// Function Description: Returns the sensor data
//********************************************************************************************
double GravityTemperature::getValue()
{
	return temperature;
}

//********************************************************************************************
// function name: TempProcess ()
// Function Description: Analyze the temperature data
//********************************************************************************************
double GravityTemperature::TempProcess(bool ch)
{
	static byte data[12];
	static byte addr[8];
	static float TemperatureSum;
	if (!ch) {
		if (!oneWire->search(addr)) {
			//Debug::println(F("No temperature sensors on chain, reset search!"));
			oneWire->reset_search();
			return 0;
		}
		if (OneWire::crc8(addr, 7) != addr[7]) {
			//Debug::println(F("CRC is not valid!"));
			return 0;
		}
		if (addr[0] != 0x10 && addr[0] != 0x28) {
			//Debug::println(F("Device is not recognized!"));
			return 0;
		}
		oneWire->reset();
		oneWire->select(addr);
		oneWire->write(0x44, 1); // start conversion, with parasite power on at the end
	}
	else {
		byte present = oneWire->reset();
		oneWire->select(addr);
		oneWire->write(0xBE); // Read Scratchpad
		for (int i = 0; i < 9; i++) { // We need 9 bytes
			data[i] = oneWire->read();
		}
		oneWire->reset_search();
		byte MSB = data[1];
		byte LSB = data[0];
		float tempRead = ((MSB << 8) | LSB); // Using two's compliment
		TemperatureSum = tempRead / 16;
	}
	return TemperatureSum;
}
