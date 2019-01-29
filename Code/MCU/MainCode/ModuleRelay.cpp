#include "ModuleRelay.h"
#include "Config.h"


//*****************************************
ModuleRelay::ModuleRelay(const int pin):

ModuleRelay::ModuleRelay(pin, false) {
}

ModuleRelay::ModuleRelay(const int pin, const boolean invert) {
	this->pin = pin;
	if (invert) {
		this->onSignal = HIGH;
		this->offSignal = LOW;
	} else {
		this->onSignal = LOW;
		this->offSignal = HIGH;
	}
	init();
}

/**
	Destructor.
	Turns off the relay before deleting the object.
*/
ModuleRelay::~ModuleRelay() {
	turnOff();
}

/**
	Initialization of module.
	Turns off the relay.
*/
void ModuleRelay::init() {
	pinMode(this->pin, OUTPUT);
	off();
}

void ModuleRelay::on() {
	if (isOff()) {
		turnOn();
	}
}

void ModuleRelay::off() {
	if (isOn()) {
		turnOff();
	}
}

boolean ModuleRelay::isOn() {
	return read() == this->onSignal;
}

boolean ModuleRelay::isOff() {
	return read() == this->offSignal;
}

void ModuleRelay::turnOn() {
	write(this->onSignal);
}

void ModuleRelay::turnOff() {
	write(this->offSignal);
}

void ModuleRelay::write(const int signal) {
	digitalWrite(this->pin, signal);
}
	
int ModuleRelay::read() {
	return digitalRead(this->pin);
}