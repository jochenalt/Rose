/*
 * MainMemory.h
 *
 * Created: 04.04.2013 18:07:06
 *  Author: JochenAlt
 */ 


#ifndef MAINMEMORY_H_
#define MAINMEMORY_H_

#include "Arduino.h"
#include "MemoryBase.h"
#include "setup.h"
#include "IMUController.h"
#include "BallController.h"
#include "ServoLegs.h"
#include "TouchScreen.h"


class StewartMemory : public MemoryBase {
	public:
		// initialize  default values of memory for the very first start
		StewartMemory();
		void println();

	struct  {
		
		// calibration of touchscreen
		
		TouchScreenConfig touchScreenConfig;
		ServoLegsConfig servoLegsConfig;
		BallControllerConfig ballControllerConfig;		
		IMUControllerConfig imuControllerConfig;
	} persistentMem;
};


extern StewartMemory memory;


#endif /* MAINMEMORY_H_ */