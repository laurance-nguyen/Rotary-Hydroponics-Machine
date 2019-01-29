/*********************************************************************
* MainCode.ino
*
* Description:
* This sample code is mainly used to monitor water quality
* including ph, temperature, tds.
*
* Software Environment: Arduino IDE 1.8.2
* Software download link: https://www.arduino.cc/en/Main/Software
*
* Install the library file：
* Copy the files from the github repository folder libraries to the libraries
* in the Arduino IDE 1.8.2 installation directory
*
* Hardware platform   : Arduino Mega2560
* Sensor pin:
* TDS  : A1
* PH  : A2
* RTC : I2C
* Temperature:D5
*
* SD card attached to SPI bus as follows:
* Mega:  MOSI - pin 51, MISO - pin 50, CLK - pin 52, CS - pin 53
* and pin #53 (SS) must be an output
* M0:   Onboard SPI pin,CS - pin 4 (CS pin can be changed)
*
* author  :  Shogun Ross(nguyenconglong95@gmail.com)
* version :  V1.0
* date    :  5/4/2018
**********************************************************************/
#include "Config.h"
#include "GravitySensorHub.h"
#include "GravityTDS.h"
#include "GravityTemperature.h"
#include "GravityPh.h"
#include "ModuleRelay.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <EEPROM.h>
#include <OneWire.h>


//Serial Comminication
#define ReceivedBufferLength 15


char cmdReceivedBuffer[ReceivedBufferLength + 1];
byte cmdReceivedBufferIndex;
//Initialize water flow variables
volatile double waterFlowA = 0, waterFlowB = 0, waterFlowPH = 0;

unsigned long timeSpent = 0;
unsigned long updateTime = 0;

byte PWMSet = 55;
int8_t oldMotorSpeed = 0, motorSpeed = 0, speedChange;

boolean autoState = false;

static unsigned long oldTime, newTime;

//Water Level Float Switch
const byte floatPin[3] = {SWITCHF1, SWITCHF2, SWITCHF3};
boolean switchState[3];
boolean oldSwitchState[3] = {HIGH, HIGH, HIGH}; //assume all switchs open because of pull-up resistor
boolean waterPumpOn = false;
boolean solenoid1On = false, solenoid2On = false, solenoid3On = false;
boolean doneFlag = false;
double nowEC, nowTDS, nowTemp, nowPH;
float nowMois;
unsigned long int nowLux;

double desireEC, desirePH;
double V1;
const int V2 = 61323750;
const double M1;
const double EC1;
const double EC2;

String stringEC = "EC:";
String stringPH = "PH:";
String stringTemp = "TEMP:";
String stringMois = "MOIS:";
String stringLux = "LUX:";
String stringToSend;


// Sensor monitor
GravitySensorHub sensorHub;
// Actuator module
ModuleRelay pumpA(PUMPA);
ModuleRelay pumpB(PUMPB);
ModuleRelay pumpPH(PUMPPH);
ModuleRelay waterPump(WATERPUMP);
ModuleRelay dissolvePump(DISSOLVEPUMP);
ModuleRelay solenoid1(SOLENOID1);
ModuleRelay solenoid2(SOLENOID2);
ModuleRelay solenoid3(SOLENOID3);
ModuleRelay solenoid4(SOLENOID4);	// This one isn't real.


void setup(){
	Serial.begin(9600);
	Serial3.begin(9600);

	sensorHub.setup();

	//Set the offset of the corresponding sensor calibration data
	((GravityPh *)(sensorHub.sensors[phSensor]))->setOffset(PHOFFSET);

	//Interrupt settings
	attachInterrupt(INT0, pulseA, RISING);
	attachInterrupt(INT1, pulseB, RISING);
	attachInterrupt(INT5, pulsePh, RISING);

	//Water Level Float Switch settings
	for(byte i = 0; i < 3; i++) pinMode(floatPin[i], INPUT_PULLUP);
	
	
	stringToSend = String();
	/* Serial.println(checkFloatSwitch(1));
	Serial.println(checkFloatSwitch(2));
	Serial.println(checkFloatSwitch(3)); */
	//cli(); // Disable all Interrupt before using
}

void loop(){
	if (cmdSerialDataAvailable() > 0){
		Serial.println(cmdReceivedBuffer);
		commandRelay(cmdParse());
	}

	if (cmdSerialDataESPAvailable() > 0){
		Serial.println(cmdReceivedBuffer);
		commandRelay(cmdParse());
	}
	if (waterPumpOn){
		if (checkFloatSwitch(3)){
			waterPump.off();
			waterPumpOn = false;
			Serial3.println("CLOSEPW");
		}
	}
	if (solenoid1On){
		if (!checkFloatSwitch(2)){
			solenoid1.off();
			solenoid1On = false;
			Serial3.println("CLOSES1");
			Serial.println("C1 is nearly empty. Please refill its water to continue...");
		}
	}

	if (solenoid2On){
		if (checkFloatSwitch(1)){
			solenoid2.off();
			solenoid2On = false;
			Serial3.println("CLOSES2");
			Serial.println("C1 is full. Please release its water to continue...");
		}
	}
}

boolean cmdSerialDataAvailable(){
	char cmdReceivedChar;
	static unsigned long cmdReceivedTimeOut = millis();
	while (Serial.available() > 0){
		if (millis() - cmdReceivedTimeOut > 500U){
			cmdReceivedBufferIndex = 0;
			memset(cmdReceivedBuffer, 0, (ReceivedBufferLength + 1));
		}
		cmdReceivedTimeOut = millis();
		cmdReceivedChar = Serial.read();
		if (cmdReceivedChar == '\n' || cmdReceivedBufferIndex == ReceivedBufferLength){
			cmdReceivedBufferIndex = 0;
			strupr(cmdReceivedBuffer);
			return true;
		}
		else{
			cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
			cmdReceivedBufferIndex++;
		}
	}
	return false;
}
boolean cmdSerialDataESPAvailable(){
	char cmdReceivedChar;
	static unsigned long cmdReceivedTimeOut = millis();
	while (Serial3.available() > 0){
		if (millis() - cmdReceivedTimeOut > 500U){
			cmdReceivedBufferIndex = 0;
			memset(cmdReceivedBuffer, 0, (ReceivedBufferLength + 1));
		}
		cmdReceivedTimeOut = millis();
		cmdReceivedChar = Serial3.read();
		if (cmdReceivedChar == '\n' || cmdReceivedBufferIndex == ReceivedBufferLength){
			cmdReceivedBufferIndex = 0;
			strupr(cmdReceivedBuffer);
			return true;
		}
		else{
			cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
			cmdReceivedBufferIndex++;
		}
	}
	return false;
}

byte cmdParse(){
	byte modeIndex = 0;
	//Serial.println("I'm now so sanh");
	if (strstr(cmdReceivedBuffer, "SENALL") != NULL)
		modeIndex = 1;
	else if (strstr(cmdReceivedBuffer, "SENEC") != NULL)
		modeIndex = 2;
	else if (strstr(cmdReceivedBuffer, "SENPH") != NULL)
		modeIndex = 3;
	else if (strstr(cmdReceivedBuffer, "SENTEMP") != NULL)
		modeIndex = 4;
	else if (strstr(cmdReceivedBuffer, "SENMOIS") != NULL)
		modeIndex = 5;
	else if (strstr(cmdReceivedBuffer, "SENLUX") != NULL)
		modeIndex = 6;
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCW1") != NULL)
		{modeIndex = 9; motorSpeed = 1;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCW2") != NULL)
		{modeIndex = 9; motorSpeed = 2;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCW3") != NULL)
		{modeIndex = 9; motorSpeed = 3;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCW4") != NULL)
		{modeIndex = 9; motorSpeed = 4;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCW5") != NULL)
		{modeIndex = 9; motorSpeed = 5;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCW6") != NULL)
		{modeIndex = 9; motorSpeed = 6;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCCW1") != NULL)
		{modeIndex = 9; motorSpeed = -1;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCCW2") != NULL)
		{modeIndex = 9; motorSpeed = -2;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCCW3") != NULL)
		{modeIndex = 9; motorSpeed = -3;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCCW4") != NULL)
		{modeIndex = 9; motorSpeed = -4;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCCW5") != NULL)
		{modeIndex = 9; motorSpeed = -5;}
	else if (strstr(cmdReceivedBuffer, "OPENMOTORCCW6") != NULL)
		{modeIndex = 9; motorSpeed = -6;}
	else if (strstr(cmdReceivedBuffer, "STOPMOTOR") != NULL)
		{modeIndex = 9; motorSpeed = 0;}
	else if (strstr(cmdReceivedBuffer, "AUTOON") != NULL)
		modeIndex = 10;
	else if (strstr(cmdReceivedBuffer, "OPENPA") != NULL)
		modeIndex = 11;
	else if (strstr(cmdReceivedBuffer, "OPENPB") != NULL)
		modeIndex = 12;
	else if (strstr(cmdReceivedBuffer, "OPENPPH") != NULL)
		modeIndex = 13;
	else if (strstr(cmdReceivedBuffer, "OPENPW") != NULL)
		modeIndex = 14;
	else if (strstr(cmdReceivedBuffer, "OPENPD") != NULL)
		modeIndex = 15;
	else if (strstr(cmdReceivedBuffer, "OPENS1") != NULL)
		modeIndex = 16;
	else if (strstr(cmdReceivedBuffer, "OPENS2") != NULL)
		modeIndex = 17;
	else if (strstr(cmdReceivedBuffer, "OPENS3") != NULL)
		modeIndex = 18;
	else if (strstr(cmdReceivedBuffer, "OPENLED") != NULL)
		modeIndex = 19;
	else if (strstr(cmdReceivedBuffer, "OPENDECOLED") != NULL)
		modeIndex = 20;
	else if (strstr(cmdReceivedBuffer, "AUTOOFF") != NULL)
		modeIndex = 21;
	else if (strstr(cmdReceivedBuffer, "CLOSEPA") != NULL)
		modeIndex = 22;
	else if (strstr(cmdReceivedBuffer, "CLOSEPB") != NULL)
		modeIndex = 23;
	else if (strstr(cmdReceivedBuffer, "CLOSEPPH") != NULL)
		modeIndex = 24;
	else if (strstr(cmdReceivedBuffer, "CLOSEPW") != NULL)
		modeIndex = 25;
	else if (strstr(cmdReceivedBuffer, "CLOSEPD") != NULL)
		modeIndex = 26;
	else if (strstr(cmdReceivedBuffer, "CLOSES1") != NULL)
		modeIndex = 27;
	else if (strstr(cmdReceivedBuffer, "CLOSES2") != NULL)
		modeIndex = 28;
	else if (strstr(cmdReceivedBuffer, "CLOSES3") != NULL)
		modeIndex = 29;
	else if (strstr(cmdReceivedBuffer, "CLOSELED") != NULL)
		modeIndex = 30;
	else if (strstr(cmdReceivedBuffer, "CLOSEDECOLED") != NULL)
		modeIndex = 31;
	//Serial.println(modeIndex);
	return modeIndex;
}

void commandRelay(byte mode){
	switch (mode){
	case 1:
		printAllSensor();
		stringToSend = stringEC + nowEC;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
            if (cmdSerialDataESPAvailable() > 0){
                if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		stringToSend = stringPH + nowPH;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		stringToSend = stringTemp + nowTemp;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		stringToSend = stringMois + nowMois;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		stringToSend = stringLux + nowLux;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		Serial3.println("DONESENTALL");
		break;
	case 2:
		updateSensor(ecSensor);
		stringToSend = stringEC + nowEC;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		break;
	case 3:
		updateSensor(phSensor);
		stringToSend = stringPH + nowPH;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		break;
	case 4:
		updateSensor(temperatureSensor);
		stringToSend = stringTemp + nowTemp;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		break;
	case 5:
		nowMois = 0.8f; //TODO: Read Moisture sensor;
		stringToSend = stringMois + nowMois;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		break;
	case 6:
		nowLux = 10000; //TODO: Read Lux sensor;
		stringToSend = stringLux + nowLux;
		Serial.println(stringToSend);
		Serial3.println(stringToSend);
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				if (strstr(cmdReceivedBuffer, "DONE") != NULL){
					break;
				}
			}
		}
		break;

	case 7:
		Serial3.println("GETECDESIRE");
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				commandSensor(cmdSensor());
				Serial3.println("DONE");
			}
		}
		break;
	case 8:
		Serial3.println("GETPHDESIRE");
		while (1){
			if (cmdSerialDataESPAvailable() > 0){
				commandSensor(cmdSensor());
				Serial3.println("DONE");
			}
		}
		break;

	case 9:
		Serial.println("I'm in mode 9");
		speedChange = motorSpeed - oldMotorSpeed;

		Serial.print("Motor Speed is: "); 	Serial.println(motorSpeed);
		if (speedChange > 0)
			for (int8_t i = oldMotorSpeed + 1; i < (motorSpeed + 1); i++) rotateMotor(i);
		else
			for (int8_t i = oldMotorSpeed - 1; i > (motorSpeed - 1); i--) rotateMotor(i);
		oldMotorSpeed = motorSpeed;
		Serial3.println("DONE");
		break;

	case 10:
		autoState = true;
		Serial3.println("DONE");
		break;

	case 11:
		pumpA.on();
		Serial.println("I'm in here");
		Serial3.println("DONE");
		oldTime = millis();
		//sei(); // Enable all Interrupt again
		while (waterFlowA < 50.0)
	/* 	{
			Serial.println("I'm stuck here");
		}; */
		pumpA.off();
		//cli(); // Disable all Interrupt
		newTime = millis();
		timeSpent = newTime - oldTime;
		Serial.print(waterFlowA);
		Serial.print(" mL in ");
		Serial.print(timeSpent);
		Serial.println(" mS");
		waterFlowA = 0;
		waterFlowB = 0;
		waterFlowPH = 0;

		Serial3.println("CLOSEPA");
		break;

	case 12:
		pumpB.on();
		Serial3.println("DONE");
		oldTime = millis();
		//sei(); // Enable all Interrupt again
		while (waterFlowB < 50.0);
		pumpB.off();
		//cli(); // Disable all Interrupt
		newTime = millis();
		timeSpent = newTime - oldTime;
		Serial.print(waterFlowB);
		Serial.print(" mL in ");
		Serial.print(timeSpent);
		Serial.println(" mS");
		waterFlowA = 0;
		waterFlowB = 0;
		waterFlowPH = 0;

		Serial3.println("CLOSEPB");
		break;

	case 13:
		pumpPH.on();
		Serial3.println("DONE");
		oldTime = millis();
		//sei(); // Enable all Interrupt again
		while (waterFlowPH < 20.0);
		pumpPH.off();
		//cli(); // Disable all Interrupt
		newTime = millis();
		timeSpent = newTime - oldTime;
		Serial.print(waterFlowPH);
		Serial.print(" mL in ");
		Serial.print(timeSpent);
		Serial.println(" mS");
		waterFlowA = 0;
		waterFlowB = 0;
		waterFlowPH = 0;

		Serial3.println("CLOSEPPH");
		break;

	case 14:
		waterPump.on();
		waterPumpOn = true;
		Serial3.println("DONE");
		break;

	case 15:
		dissolvePump.on();
		Serial3.println("DONE");
		break;

	case 16:
		solenoid1.on();
		solenoid1On = true;
		Serial3.println("DONE");
		break;

	case 17:
		solenoid2.on();
		solenoid2On = true;
		Serial3.println("DONE");
		break;

	case 18:
		solenoid3.on();
		solenoid3On = true;
		Serial3.println("DONE");
		break;

	case 19:
		break;

	case 20:
		break;
	/* case 21:
		autoState = false;
		break; */

	case 22:
		pumpA.off();
		Serial3.println("DONE");
		break;

	case 23:
		pumpB.off();
		Serial3.println("DONE");
		break;

	case 24:
		pumpPH.off();
		Serial3.println("DONE");
		break;

	case 25:
		waterPump.off();
		waterPumpOn = false;
		Serial3.println("DONE");
		break;

	case 26:
		dissolvePump.off();
		Serial3.println("DONE");
		break;

	case 27:
		solenoid1.off();
		solenoid1On = false;
		Serial3.println("DONE");
		break;
	
	case 28:
		solenoid2.off();
		solenoid2On = false;
		Serial3.println("DONE");
		break;

	case 29:
		solenoid3.off();
		solenoid3On = false;
		Serial3.println("DONE");
		break;

	case 30:
		break;

	case 31:
		break;
	}
}

void commandAuto(byte mode){
	switch(mode){
	/* case 10:
		autoState = true;
		break; */
	case 21:
		autoState = false;
		Serial3.println("DONE");
		break;
	}

}

void rotateMotor(int8_t motorSpeed){
	Serial.println("I'm running rotateMotor");
	if (motorSpeed > 0){
		analogWrite(LPWM, 0);
		analogWrite(RPWM, PWMSet*abs(motorSpeed));
		delay(1000);
	}
	else {
		analogWrite(LPWM, PWMSet*abs(motorSpeed));
		analogWrite(RPWM, 0);
		delay(1000);
	}
}

void pulseA(){ //measure the quantity of square wave
	waterFlowA += 1000.0 / 5880.0;
	//Serial.println("I'm stuck here 1");
}
void pulseB(){ //measure the quantity of square wave
	waterFlowB += 1000.0 / 5880.0;
	//Serial.println("I'm stuck here 2");
}
void pulsePh(){ //measure the quantity of square wave
	waterFlowPH += 1000.0 / 5880.0;
	//Serial.println("I'm stuck here 3");
}

void printAllSensor(){
	//********************************************************************************************
	// function name: sensorHub.getValueBySensorNumber (0)
	// Function Description: Get the sensor's values, and the different parameters represent the acquisition of different sensor data
	// Parameters: 0 pH sensor
	// Parameters: 1 Temperature sensor
	// Parameters: 2 Total Dissolveed Solids sensor
	// return value: returns a double type of data
	//********************************************************************************************
	// ************************* Serial debugging ************************************************
	
	updateTime = millis();
	while(1){
		sensorHub.updateAll();
		if (millis() - updateTime > 3000){
			updateTime = millis();
			Serial.print(F("pH = "));
			nowPH = sensorHub.getValueBySensorNumber(phSensor);
			Serial.print(nowPH);

			Serial.print(F("  Temp = "));
			nowTemp = sensorHub.getValueBySensorNumber(temperatureSensor);
			Serial.print(nowTemp);
#ifdef SELECTEC
			Serial.print(F("  EC = "));
			nowEC = sensorHub.getValueBySensorNumber(ecSensor);
			Serial.println(nowEC);
#endif
#ifdef SELECTTDS
			Serial.print(F("  TDS = "));
			nowTDS = sensorHub.getValueBySensorNumber(tdsSensor);
			Serial.println(nowTDS);
#endif
			nowMois = 0.8f;
			nowLux = 10000;
			break;
		}
	}
}

void updateSensor(int type){
	updateTime = millis();
	while(1){
		
		if (millis() - updateTime > 3000){
			updateTime = millis();
			switch (type){
			case phSensor:
				Serial.print(F("pH = "));
				nowPH = sensorHub.getValueBySensorNumber(phSensor);
				Serial.println(nowPH);
				break;

			case temperatureSensor:
				Serial.print(F("Temp = "));
				nowTemp = sensorHub.getValueBySensorNumber(temperatureSensor);
				Serial.println(nowTemp);
				break;

			case ecSensor:
#ifdef SELECTEC
				Serial.print(F("EC = "));
				nowEC = sensorHub.getValueBySensorNumber(ecSensor);
				Serial.println(nowEC);
#endif
#ifdef SELECTTDS
				Serial.print(F("TDS = "));
				nowTDS = sensorHub.getValueBySensorNumber(tdsSensor);
				Serial.println(nowTDS);
#endif
				break;
			}
			break;
		}
	}
}

void setECValue(float setEC){
	dissolvePump.on();
	while(1){
		sensorHub.update(ecSensor);
		nowEC = sensorHub.getValueBySensorNumber(ecSensor);
		if (!compareEC(setEC, nowEC)){
			if (nowEC < setEC){
				while (!checkFloatSwitch(1)){
					solenoid1.on();
				}
				solenoid1.off();
				V1 = (V2*(setEC-nowEC))/(EC1+EC2-setEC);//TODO: Calculate V1 to fill
				
				doseA(V1);//TODO: Open pumpA and pumpB while V
				doseB(V1);
				sensorHub.update(ecSensor);
				nowEC = sensorHub.getValueBySensorNumber(ecSensor);
				if (compareEC(setEC, nowEC)){
					waterPump.on();
					solenoid2.on();
					delay(120000);
				}
				sensorHub.update(ecSensor);
				nowEC = sensorHub.getValueBySensorNumber(ecSensor);
				if (compareEC(setEC, nowEC)){
					break;
				}
			} else{
				if(checkFloatSwitch(1)){
					solenoid1.on();//TODO: Open valve 4
					solenoid4.on();//TODO: Open valve 1
					while (!compareEC(setEC, nowEC)){
						sensorHub.update(ecSensor);
						nowEC = sensorHub.getValueBySensorNumber(ecSensor);
					}
					solenoid1.off(); //TODO: Close valve 4
					solenoid4.off();  //TODO: Close valve 1
					break;
				} else{
					solenoid4.on(); //TODO: Open valve 1
					if (checkFloatSwitch(1)){
						solenoid1.on(); //TODO: Open valve 4
						while(!compareEC(setEC, nowEC)){
							sensorHub.update(ecSensor);
							nowEC = sensorHub.getValueBySensorNumber(ecSensor);
						}
						solenoid1.off(); //TODO: Close valve 4
						solenoid4.off(); //TODO: Close valve 1
						break;
					} else{
						while (!compareEC(setEC, nowEC)){
							sensorHub.update(ecSensor);
							nowEC = sensorHub.getValueBySensorNumber(ecSensor);
						}
						solenoid4.off(); //TODO: Close valve 1
						break;
					}
				}
			}
		}
	}
	dissolvePump.off();
	Serial3.println("SENEC");
	stringToSend = stringEC + nowEC; //TODO: Send nowEC data to database
	Serial3.println(stringToSend);
	while (1){
		if (cmdSerialDataESPAvailable() > 0){
			if (strstr(cmdReceivedBuffer, "DONE") != NULL){
				break;
			}
		}
	}
}

void setPHValue(float setPH){
	dissolvePump.on();
	nowPH = sensorHub.getValueBySensorNumber(phSensor);
	while(comparePH(nowPH, setPH)){
	}

	dissolvePump.off();
	Serial3.println("SENPH");
	stringToSend = stringPH + nowPH; //TODO: Send nowEC data to database
	Serial3.println(stringToSend);
	while (1){
		if (cmdSerialDataESPAvailable() > 0){
			if (strstr(cmdReceivedBuffer, "DONE") != NULL){
				break;
			}
		}
	}
}

boolean checkFloatSwitch(char indexF){
	indexF -= 1;
	// see if switch is open or closed
	switchState[indexF] = !digitalRead(floatPin[indexF]);

	// has it changed since last time?
	if (switchState[indexF] != oldSwitchState[indexF])
		oldSwitchState[indexF] = switchState[indexF]; // remember for next time
	return switchState[indexF];
}

bool compareEC(double a, double b){
	float epsilon = 0.05; 
	return abs(a - b) < epsilon;
}
bool comparePH(double a, double b){
	float epsilon = 0.1;
	return abs(a - b) < epsilon;
}

void doseA(double V){
	pumpA.on();
	Serial3.println("OPENPA");
	oldTime = millis();
	//sei(); // Enable all Interrupt again
	while (waterFlowA < V);
	pumpA.off();
	//cli(); // Disable all Interrupt
	newTime = millis();
	timeSpent = newTime - oldTime;
	Serial.print(waterFlowA);
	Serial.print(" mL in ");
	Serial.print(timeSpent);
	Serial.println(" mS");
	waterFlowA = 0;
	Serial3.println("CLOSEPA");
}

void doseB(double V){
	pumpB.on();
	Serial3.println("OPENPB");
	oldTime = millis();
	//sei(); // Enable all Interrupt again
	while (waterFlowB < V);
	pumpB.off();
	//cli(); // Disable all Interrupt
	newTime = millis();
	timeSpent = newTime - oldTime;
	Serial.print(waterFlowB);
	Serial.print(" mL in ");
	Serial.print(timeSpent);
	Serial.println(" mS");
	waterFlowB = 0;
	Serial3.println("CLOSEPB");
}

void dosePH(double V){
	pumpPH.on();
	Serial3.println("OPENPPH");
	oldTime = millis();
	//sei(); // Enable all Interrupt again
	while (waterFlowPH < V);
	pumpPH.off();
	//cli(); // Disable all Interrupt
	newTime = millis();
	timeSpent = newTime - oldTime;
	Serial.print(waterFlowPH);
	Serial.print(" mL in ");
	Serial.print(timeSpent);
	Serial.println(" mS");
	waterFlowPH = 0;
	Serial3.println("CLOSEPPH");
}

byte cmdSensor(){
	byte modeIndex = 0;
	if (strstr(cmdReceivedBuffer, "EC:") != NULL)
		modeIndex = 1;
	else if (strstr(cmdReceivedBuffer, "PH:") != NULL)
		modeIndex = 2;
	return modeIndex;
}

void commandSensor(byte mode){
	char *cmdReceivedBufferPtr;
	switch (mode){
	case 1:
		cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "EC:");
		cmdReceivedBufferPtr += strlen("EC:");
		desireEC = strtod(cmdReceivedBufferPtr, NULL);
		break;
	case 2:
		cmdReceivedBufferPtr = strstr(cmdReceivedBuffer, "PH:");
		cmdReceivedBufferPtr += strlen("PH:");
		desirePH = strtod(cmdReceivedBufferPtr, NULL);
		break;
	}
}