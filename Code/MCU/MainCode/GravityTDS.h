#pragma once

#include "ISensor.h"
#include <Arduino.h>

#define COUNT  10   //Number of sample taken
#define ReceivedBufferLength 15
#define TdsFactor 0.5  // tds = ec / 2

class GravityTDS: public ISensor{
public:
	GravityTDS();
	~GravityTDS();

public:
	void setup();                       //initialization
	void update();                      //read and calculate
	double getValue();
    float getKvalue(); 
    float getEcValue();
    void setPin(int pin);
    float setTemperature(float temp);   //set the temperature and execute temperature compensation
    float setVref(float value);         // reference voltage on ADC, default 5.0V on Arduino UNO
    void setAdcRange(float range);      // 1024 for 10bit ADC;4096 for 12bit ADC
    void setKvalueAddress(int address); // set the EEPROM address to store the k value,default address:0x08
    
    ISensor *gravityTemperature;
private:
    int pin;
    float vref;
    float adcRange;
    float temperature;
    int analogBuffer[COUNT];    // store the analog value in the array, read from ADC
    float averageVoltage = 0;
    double tdsValue = 0;
    int kValueAddress;     //the address of the K value stored in the EEPROM
    char cmdReceivedBuffer[ReceivedBufferLength+1];   // store the serial cmd from the serial monitor
    byte cmdReceivedBufferIndex;

    float kValue;       // k value of the probe,you can calibrate in buffer solution ,such as 706.5ppm(1413us/cm)@25^C
    double ecValue;      //before temperature compensation
    double ecValue25;    //after temperature compensation

    void readKValues();
    boolean cmdSerialDataAvailable();
    byte cmdParse();
    void ecCalibration(byte mode);

    //uint16_t readMedianValue(int* dataArray, uint16_t arrayLength);
};  
