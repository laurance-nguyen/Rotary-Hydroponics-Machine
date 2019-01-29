/*********************************************************************
* ISensor.h
*
* Description:
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-06
**********************************************************************/
#pragma once
class ISensor {
public:
	virtual void setup()		= 0;
	virtual void update()		= 0;
	virtual double getValue()	= 0;
};