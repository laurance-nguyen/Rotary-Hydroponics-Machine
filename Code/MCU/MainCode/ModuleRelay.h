#ifndef MODULERELAY_h
#define MODULERELAY_h
#pragma once
#include "Config.h"
#include <Arduino.h>

class ModuleRelay {
	private:

		//Port number that is attached to the relay.
		int pin;
	
		/**
			ON and OFF signal.
			If invert signal:
				onSignal = LOW
				offSignal = HIGH
			If not invert signal:
				onSignal = HIGH
				offSignal = LOW
		*/
		int onSignal;
		int offSignal;
	public:
		ModuleRelay();
		/**
			Constructor.
			@param IN_pin - a digital port number that is attached to the relay.
		*/
		ModuleRelay(const int pin);
		
		/**
			Constructor.
			@param IN_pin - a digital port number that is attached to the relay.
			@param invert - invert relay signal:
				true - LOW is a ON signal;
				false - HIGH is a OFF signal.
		*/
		ModuleRelay(const int pin, const boolean invert);

		~ModuleRelay();

		//Turns on the relay if it is off.
		void on();

		//Turns off the relay if it is on.
		void off();

		/**
			Checks if the relay is on.
			@return true if the relay is on, 
			false if the relay is off.
		*/
		boolean isOn();

		/**
			Checks if the relay is off.
			@return true if the relay is off, 
			false if the relay is on.
		*/
		boolean isOff();

	private:

		//Initialization of module.
		void init();


		//Turns on the relay.
		void turnOn();

		//Turns off the relay.
		void turnOff();

		/**
			Wrintes a input signal to the relay.
			@param value - a new relay signal.
		*/
		void write(const int signal);

		/**
			Reads and returns the relay signal.
			@return relay signal
		*/
		int read();
};

#endif