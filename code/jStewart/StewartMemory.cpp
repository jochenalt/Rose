/*
 * MainMemory.cpp
 *
 * Created: 04.04.2013 18:06:40
 *  Author: JochenAlt
 */ 


#include "StewartMemory.h"
#include <avr/eeprom.h>
#include "ServoLegs.h"
#include "TouchScreen.h"
#include "BallController.h"

char stewartMemory_EE[sizeof(StewartMemory::persistentMem)] EEMEM;
StewartMemory memory;

StewartMemory::StewartMemory()
: MemoryBase(stewartMemory_EE,(char*)&(persistentMem),sizeof(StewartMemory::persistentMem)) {
	// initialization for the very first start, when EEPROM is not yet initialized
	ServoLegs::setDefaults();
#ifdef TOUCHSCREEN_HAS_5WIRE
	TouchScreen5Wire::setDefaults();
#else
	TouchScreen4Wire::setDefaults();
#endif
	BallController::setDefaults();
}


void StewartMemory::println() {
	
}
	