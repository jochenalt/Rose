/*
 * Servo.h
 *
 * Created: 28.11.2014 22:00:15
 *  Author: JochenAlt
 */ 


#ifndef SERVO_LEGS_
#define SERVO_LEGS_

#include "Arduino.h"
#include <Servo/Servo.h>
#include "setup.h"
#include "FixedPoint.h"

class ServoLegsConfig {
	public:
		// calibrated null positions of servos
		int16_t servoNullPos[6];
};

class ServoLegs {
	private:
		Servo motor[6];
		int16_t currentPosition_fp4[6];
	public:
		ServoLegs() {};

		void printAngles();
		static void printDefaults();
		static void setDefaults();
		void setup();		
		void incNullPosition(uint8_t motorIdx, int16_t inc);
		int16_t getNullPosition(uint8_t pIdx);
		void setServoAngleByLegCorner(uint8_t cornerNo, int16_fp4_t xWorld_fp4, int16_fp4_t yWorld_fp4, int16_fp4_t zWorld_fp4);
		int16_fp4_t getMotorAngle(uint8_t pIdx);
		void defineCurrentPositionAsNull();
		void setMotorAngle(uint8_t pIdx, int16_fp4_t angleDegree_fp4);
		
		void printMenuHelp();
		void menuController();
		void printCalibrationData();
		void printlnCalibrationData();

private:
		int16_fp4_t computeServoAngleByCorner(uint8_t cornerNo, int16_fp4_t xWorld_fp4, int16_fp4_t yWorld_fp4, int16_fp4_t zWorld_fp4);

};

extern ServoLegs servoLegs;




#endif /* SERVO_LEGS_ */