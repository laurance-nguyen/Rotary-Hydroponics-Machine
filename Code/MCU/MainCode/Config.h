#pragma once

#include <Arduino.h>

//Serial print switch
#define DEBUG_AVR
//#define DEBUG_M0

//The maximum length of the sensor filter array
#define ARRAYLENGTH 10

//SD card update data time, 60000 is 1 minute
#define SDUPDATEDATATIME 60000

//TDS sensor is selected by default, note this line uses EC sensor
#define SELECTEC

#define EEPROMADDRESS 0x57
//Sensor pin settings
#define ECPIN  A2
#define TDSPIN A2
#define PHPIN  A3
#define TEMPPIN    33

//Actuator pin settings (12V) - Control by 8 Relay 5V
#define PUMPPH          46
#define PUMPA 			47
#define PUMPB 			48

#define WATERPUMP		49
#define DISSOLVEPUMP	50
#define SOLENOID1		51  // Thoat nuoc C1
#define SOLENOID2		52  // Thoat nuoc tu C2 xuong C1
#define SOLENOID3		53  // Thoat nuoc tu C2 ra ngoai
#define SOLENOID4       45

//Float level switch
#define SWITCHF1        24  // UPPER C1
#define SWITCHF2        22  // LOWER C1
#define SWITCHF3        23  // UPPER C2



//Motor settings
#define RPWM 9  
#define LPWM 10

//Set sensor offset (calibration data)
#define PHOFFSET 0.12
#define ECKVALUE 0.6


//The maximum number of sensors
#define SENSORCOUNT 3
//The maxinum number of actuator
#define ACTUATORCOUNT 8

//The sensor corresponds to the array number, ph=0, temperature=1..., the maximum number is SENSORCOUNT-1
enum SensorNumber{
	phSensor = 0,
	temperatureSensor = 1,
	ecSensor=2,
	tdsSensor = 2,
};