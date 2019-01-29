#include "GravityTDS.h"
#include "Config.h"
#include "Algorithm.h"
#include "GravityTemperature.h"
#include <EEPROM.h>

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

GravityTDS::GravityTDS(){
	this->pin = TDSPIN;
  this->temperature = 25.0;
  this->vref = 5.0;
  this->adcRange = 1024.0;
  this->kValueAddress = 8;
  this->kValue = 1.0;
  this->gravityTemperature = new GravityTemperature(TEMPPIN);
}

GravityTDS::~GravityTDS(){}

void GravityTDS::setPin(int pin){
	this->pin = pin;
}

void GravityTDS::setup(){
	pinMode(this->pin,INPUT);
	readKValues();
}

float GravityTDS::getKvalue(){
	return this->kValue;
}

float GravityTDS::setTemperature(float temp){
	this->temperature = temp;
}

float GravityTDS::setVref(float value){
	this->vref = value;
}

void GravityTDS::setAdcRange(float range){
  this->adcRange = range;
}

void GravityTDS::setKvalueAddress(int address){
  this->kValueAddress = address;
}

//extern uint16_t readMedianValue(int* dataArray, uint16_t arrayLength);

void GravityTDS::update(){
	for (uint8_t i = 0; i < COUNT; i++){
		this->analogBuffer[i] = analogRead(this->pin);	// Read an analog value every 20ms
		delay(10);
	}
  this->gravityTemperature->update();
  delay(2000);
  setTemperature(gravityTemperature->getValue());   //update temperature value for this calculation
  
	this->averageVoltage = readMedianValue(this->analogBuffer, COUNT);
	this->averageVoltage = this->averageVoltage * this->vref / this->adcRange; 					// read the analog value more stable by the median filtering algorithm, and convert to voltage value
	this->ecValue=(133.42*this->averageVoltage*this->averageVoltage*this->averageVoltage - 255.86*this->averageVoltage*this->averageVoltage + 857.39*this->averageVoltage)*this->kValue;
	this->ecValue25  =  this->ecValue / (1.0+0.02*(this->temperature-25.0));  // Temperature compensation
	this->tdsValue = ecValue25 * TdsFactor; // Convert EC value to TDS value
  //Serial.print("The value of Temp to calculate= "); Serial.println(this->temperature);

/* 	if(cmdSerialDataAvailable() > 0){
    ecCalibration(cmdParse());  // if received serial cmd from the serial monitor, enter into the calibration mode
  } */
}

double GravityTDS::getValue(){
	#ifdef SELECTTDS
    return tdsValue;
  #endif
  #ifdef SELECTEC
    return ecValue25/1000;
  #endif
}

float GravityTDS::getEcValue(){
  return ecValue25/1000;
}

void GravityTDS::readKValues(){
  EEPROM_read(this->kValueAddress, this->kValue);  
  if(EEPROM.read(this->kValueAddress)==0xFF && EEPROM.read(this->kValueAddress+1)==0xFF && EEPROM.read(this->kValueAddress+2)==0xFF && EEPROM.read(this->kValueAddress+3)==0xFF){
    this->kValue = 1.0;   // default value: K = 1.0
    EEPROM_write(this->kValueAddress, this->kValue);
  }
}

boolean GravityTDS::cmdSerialDataAvailable(){
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
    }else{
      cmdReceivedBuffer[cmdReceivedBufferIndex] = cmdReceivedChar;
      cmdReceivedBufferIndex++;
    }
  }
  return false;
}

byte GravityTDS::cmdParse(){
  byte modeIndex = 0;
  if(strstr(cmdReceivedBuffer, "ENTER") != NULL) 
      modeIndex = 1;
  else if(strstr(cmdReceivedBuffer, "EXIT") != NULL) 
      modeIndex = 3;
  else if(strstr(cmdReceivedBuffer, "CAL:") != NULL)   
      modeIndex = 2;
  return modeIndex;
}

void GravityTDS::ecCalibration(byte mode){
  char *cmdReceivedBufferPtr;
  static boolean ecCalibrationFinish = 0;
  static boolean enterCalibrationFlag = 0;
  float KValueTemp,rawECsolution;
  switch(mode){
    case 0:
    if(enterCalibrationFlag)
      Serial.println(F("Command Error"));
    break;
      
    case 1:
    enterCalibrationFlag = 1;
    ecCalibrationFinish = 0;
    Serial.println();
    Serial.println(F(">>>Enter Calibration Mode<<<"));
    Serial.println(F(">>>Please put the probe into the standard buffer solution<<<"));
    Serial.println();
    break;
     
    case 2:
    cmdReceivedBufferPtr=strstr(cmdReceivedBuffer, "CAL:");
    cmdReceivedBufferPtr+=strlen("CAL:");
    rawECsolution = strtod(cmdReceivedBufferPtr,NULL)/(float)(TdsFactor);
    rawECsolution = rawECsolution*(1.0+0.02*(temperature-25.0));
    if(enterCalibrationFlag){
      // Serial.print("rawECsolution:");
      // Serial.print(rawECsolution);
      // Serial.print("  ecvalue:");
      // Serial.println(ecValue);
      KValueTemp = rawECsolution/(133.42*averageVoltage*averageVoltage*averageVoltage - 255.86*averageVoltage*averageVoltage + 857.39*averageVoltage);  //calibrate in the  buffer solution, such as 707ppm(1413us/cm)@25^c
      if((rawECsolution>0) && (rawECsolution<2000) && (KValueTemp>0.25) && (KValueTemp<4.0)){
        Serial.println();
        Serial.print(F(">>>Confrim Successful,K:"));
        Serial.print(KValueTemp);
        Serial.println(F(", Send EXIT to Save and Exit<<<"));
        kValue =  KValueTemp;
        ecCalibrationFinish = 1;
      } else{
        Serial.println();
        Serial.println(F(">>>Confirm Failed, Try Again<<<"));
        Serial.println();
        ecCalibrationFinish = 0;
      }        
    }
    break;

    case 3:
    if(enterCalibrationFlag){
      Serial.println();
      if(ecCalibrationFinish){
        EEPROM_write(kValueAddress, kValue);
        Serial.print(F(">>>Calibration Successful, K Value Saved"));
      } else Serial.print(F(">>>Calibration Failed"));       
      Serial.println(F(", Exit Calibration Mode<<<"));
      Serial.println();
      ecCalibrationFinish = 0;
      enterCalibrationFlag = 0;
    }
    break;
  }
}

/*uint16_t readMedianValue(int* dataArray, uint16_t arrayLength){
	uint16_t i, j, tempValue;
	// Sorting an array with bubbling
	for (j = 0; j < arrayLength - 1; j++)
	{
		for (i = 0; i < arrayLength - 1 - j; i++)
		{
			if (dataArray[i] > dataArray[i + 1])
			{
				tempValue = dataArray[i];
				dataArray[i] = dataArray[i + 1];
				dataArray[i + 1] = tempValue;
			}
		}
	}
	// Calculated median
	if ((arrayLength & 2) > 0) //Distinguish Odd or Even number
	{
		// The array has an odd number of elements, returning one element in the middle
		tempValue = dataArray[(arrayLength - 1) / 2];
	}
	else
	{
		// The array has an even number of elements, return the average of the two elements in the middle
		tempValue = (dataArray[arrayLength / 2] + dataArray[arrayLength / 2 - 1]) / 2;
	}
	return tempValue;
}
*/