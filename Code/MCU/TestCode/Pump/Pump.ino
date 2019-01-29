#include <Arduino.h>

#define relayPin1 46	//Pump pH
#define relayPin2 47	//Pump B
#define relayPin3 48	//Pump A
#define relayPin4 49
#define relayPin5 50
#define relayPin6 51
#define relayPin7 52
#define relayPin8 53
#define flowMeter1 2
#define flowMeter2 3
#define flowMeter3 18

#define RPWM 4
#define LPWM 5
int PWMSet = 60;

#define ReceivedBufferLength 15

volatile double waterFlowA = 0, waterFlowB = 0, waterFlowPh = 0;
char cmdReceivedBuffer[ReceivedBufferLength+1];
byte cmdReceivedBufferIndex;
unsigned long timeSpent = 0;

void setup(){
	Serial.begin(115200);
	Serial.println("Test Pump!...");

	pinMode(relayPin1, OUTPUT);
	pinMode(relayPin2, OUTPUT);
	pinMode(relayPin3, OUTPUT);
	pinMode(relayPin4, OUTPUT);
	pinMode(relayPin5, OUTPUT);
	pinMode(relayPin6, OUTPUT);
	pinMode(relayPin7, OUTPUT);
	pinMode(relayPin8, OUTPUT);

	pinMode(RPWM, OUTPUT);
	pinMode(LPWM, OUTPUT);

	//Interrupt settings
	attachInterrupt(0,pulseA,RISING);
	attachInterrupt(1,pulseB,RISING);
	attachInterrupt(5,pulsePh,RISING);
	setAllRelayOff();
}

void loop(){
	if (cmdSerialDataAvailable() > 0){
		Serial.println(cmdReceivedBuffer);
		commandRelay(cmdParseRelay());
	}
}

void pulseA()   //measure the quantity of square wave
{
  waterFlowA += 1000.0 / 5880.0;
	Serial.println("I'm stuck here 1");
}
void pulseB()   //measure the quantity of square wave
{
  waterFlowB += 1000.0 / 5880.0;
	Serial.println("I'm stuck here 2");
}
void pulsePh()   //measure the quantity of square wave
{
  waterFlowPh += 1000.0 / 5880.0;
	Serial.println("I'm stuck here 3");
}

byte cmdParseRelay(){
	byte modeIndex = 0;
	
	if (strstr(cmdReceivedBuffer,"P1ON") != NULL) modeIndex = 1;
	else if (strstr(cmdReceivedBuffer,"P2ON") != NULL) modeIndex = 2;
	else if (strstr(cmdReceivedBuffer,"P3ON") != NULL) modeIndex = 3;
	return modeIndex;
}

void commandRelay(byte mode){
	switch (mode){
		case 1:
		digitalWrite(relayPin1,LOW);
		static unsigned long oldTime = millis();
		Serial.println("I'm in mode 1");
		
		while(waterFlowA < 50.0){};
		
		digitalWrite(relayPin1, HIGH);
		static unsigned long newTime = millis();
		timeSpent = newTime - oldTime;
		Serial.print(waterFlowA); Serial.print(" mL in "); Serial.print(timeSpent); Serial.println(" mS");
		waterFlowA = 0;
		break;

		case 2:
		Serial.println("I'm in mode 2");
		digitalWrite(relayPin2,LOW);
		delay(3000);
		digitalWrite(relayPin2, HIGH);
		Serial.print(waterFlowB); Serial.println(" mL");
		break;

		case 3:
		Serial.println("I'm in mode 3");
		digitalWrite(relayPin3,LOW);
		delay(3000);
		digitalWrite(relayPin3, HIGH);
		Serial.print(waterFlowPh); Serial.println(" mL");
		break;
	}
}

boolean cmdSerialDataAvailable(){
	char cmdReceivedChar;
  static unsigned long cmdReceivedTimeOut = millis();
  while (Serial.available()>0){   
    if (millis() - cmdReceivedTimeOut > 500U){
      cmdReceivedBufferIndex = 0;
      memset(cmdReceivedBuffer,0,(ReceivedBufferLength+1));
    }
    cmdReceivedTimeOut = millis();
    cmdReceivedChar = Serial.read();
    if (cmdReceivedChar == '\n' || cmdReceivedBufferIndex==ReceivedBufferLength){
		cmdReceivedBufferIndex = 0;
		strupr(cmdReceivedBuffer);
		return true;
    } else{
      cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
      cmdReceivedBufferIndex++;
    }
  }
  return false;
}

void setAllRelayOff(){
	digitalWrite(relayPin1, HIGH);
	digitalWrite(relayPin2, HIGH);
	digitalWrite(relayPin3, HIGH);
	digitalWrite(relayPin4, HIGH);
	digitalWrite(relayPin5, HIGH);
	digitalWrite(relayPin6, HIGH);
	digitalWrite(relayPin7, HIGH);
	digitalWrite(relayPin8, HIGH);
}

void rotateMotor(boolean direction){ // direction: 1 is Foward 0 is Reverse
	if (direction){
		analogWrite(LPWM, 0);
		analogWrite(RPWM, PWMSet);
	} else{
		analogWrite(LPWM, PWMSet);
		analogWrite(RPWM,0);
	}
}
