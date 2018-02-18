/*
 * ServoLeg.cpp
 *
 * Created: 28.11.2014 22:00:03
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "ServoLegs.h"
#include "setup.h"
#include "space.h"
#include "FastMath.h"
#include "FixedPoint.h"
#include "StewartMemory.h"
#include "PatternBlinker.h"
#include <avr/wdt.h>

ServoLegs servoLegs;
extern PatternBlinker LedBlinker;

uint8_t currentMotorIdx = 0;
void setCurrentMotor(uint8_t motorIdx) {
	currentMotorIdx = motorIdx;
	Serial.print(F("considering motor #"));
	Serial.println(motorIdx);
	servoLegs.setMotorAngle(currentMotorIdx, servoLegs.getMotorAngle(currentMotorIdx)+FP(10,4));
	delay(100);
	servoLegs.setMotorAngle(currentMotorIdx, servoLegs.getMotorAngle(currentMotorIdx)-FP(10,4));
}

void amendNullPosition(int16_t increment) {
	servoLegs.incNullPosition(currentMotorIdx,increment);
	Serial.print(F("inc nullposition of motor #"));
	Serial.print(currentMotorIdx);
	Serial.print(F(" to "));
	Serial.println(servoLegs.getNullPosition(currentMotorIdx));
	servoLegs.setMotorAngle(currentMotorIdx, 0);
	memory.delayedSave(5000);
}


void ServoLegs::setDefaults() {
	memory.persistentMem.servoLegsConfig.servoNullPos[0] = -6;
	memory.persistentMem.servoLegsConfig.servoNullPos[1] = -6;
	memory.persistentMem.servoLegsConfig.servoNullPos[2] = +9;
	memory.persistentMem.servoLegsConfig.servoNullPos[3] = -2;
	memory.persistentMem.servoLegsConfig.servoNullPos[4] = +6;
	memory.persistentMem.servoLegsConfig.servoNullPos[5] = +5;
}

void ServoLegs::setup() {
	motor[0].attach(SERVO1_PIN);
	motor[1].attach(SERVO2_PIN);
	motor[2].attach(SERVO3_PIN);
	motor[3].attach(SERVO4_PIN);
	motor[4].attach(SERVO5_PIN);
	motor[5].attach(SERVO6_PIN);
	
	for (uint8_t i = 0;i<6;i++)
		setMotorAngle(i,0);
}

void ServoLegs::setMotorAngle(uint8_t pIdx, int16_fp4_t angleDegree_fp4) {
	if ((angleDegree_fp4 < (FP(-30,4))) || (angleDegree_fp4 > FP(+85,4))) {
		Serial.print(F("servo #"));
		Serial.print(pIdx);
		Serial.print(F(" cannot go to "));
		Serial.print(FP2FLOAT(angleDegree_fp4,4));
		Serial.println(F("°"));
	} else {
		// 0.th, 2nd and 4th servo have a mirrored x axis 
		int16_fp4_t motorDegree_fp4 = 0;
		if ((pIdx == 0) || (pIdx == 2) || (pIdx == 4)) {
			motorDegree_fp4 = FP(90,4)-angleDegree_fp4   + FP(memory.persistentMem.servoLegsConfig.servoNullPos[pIdx],4);
		}
		else {
			motorDegree_fp4 = FP(90,4)+angleDegree_fp4   + FP(memory.persistentMem.servoLegsConfig.servoNullPos[pIdx],4);
		}		

		// limit to 0..180 (not really necessary here, hopefully)
		motorDegree_fp4  = constrain(motorDegree_fp4 , 0,FP(180,4));

		int motor_us = map(motorDegree_fp4, 0, FP(180,4), MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
		motor[pIdx].writeMicroseconds(motor_us);
		
		currentPosition_fp4[pIdx] = angleDegree_fp4;
	}
}

int16_fp4_t ServoLegs::getMotorAngle(uint8_t pIdx) {
	return currentPosition_fp4[pIdx];
}


int16_fp4_t ServoLegs::computeServoAngleByCorner(uint8_t cornerNo, int16_fp4_t xWorld_fp4, int16_fp4_t yWorld_fp4, int16_fp4_t zWorld_fp4)
{
	// Annahme tA[#Eckpunkt,achse] ist im Koordinatensystem O0, d.h. dem Weltkoordinatensystem der oberen Platform in Ruhelage
	// Koordinaten der Servos zum WeltKoordinatensystem.
	// 0,0,-120°, -120°, 120°, 120°
	// float s[6] = { 0, 0, -0.86602, -0.86602,  0.86602,  0.86602 };
	// float c[6] = { 1, 1, -0.5  , -0.5  , -0.5  , -0.5   };
	
	// buffer sin and cos values for rotation matrix around y axis per servo
	static int16_fp14_t sinServoAngle_fp14[6];
	static int16_fp14_t cosServoAngle_fp14[6];
	static int16_fp14_t isInitialized = false;
	if (isInitialized == false) {
		for (int servoIdx = 0;servoIdx<6;servoIdx++) {
			sinServoAngle_fp14[servoIdx] = sin_FP6(FLOAT2FP16(ServoAngleDegree[servoIdx],6)); 
			cosServoAngle_fp14[servoIdx] = cos_FP6(FLOAT2FP16(ServoAngleDegree[servoIdx],6));
		}
		isInitialized=true;
	}
	

	// Eckpunkte der Plattform im Weltkoordinatensystem in die jeweiligen Servokoordinatensystem verschieben
	// Danach enthält tA[#Eckpunkt,ache] die Eckpunkte im Koordinatensystem des jeweiligen Servos (aber noch ohne Rotation)
	int16_fp4_t xServoTrans_fp4 = xWorld_fp4 - ServoPivotCoord_fp4[cornerNo][X_INDEX];
	int16_fp4_t yServoTrans_fp4 = yWorld_fp4 - ServoPivotCoord_fp4[cornerNo][Y_INDEX];
	int16_fp4_t zServoTrans_fp4 = zWorld_fp4 - ServoPivotCoord_fp4[cornerNo][Z_INDEX];
	
	// rotate by y axis:
	// rotation matrix = ( cos(a) -sin(a)  )
	//					 ( sin(a)  cos(a)  )
	int16_t xServo_fp4 = mul16s_rsh(cosServoAngle_fp14[cornerNo],xServoTrans_fp4,14)- mul16s_rsh(sinServoAngle_fp14[cornerNo],zServoTrans_fp4,14);
	int16_t yServo_fp4 = yServoTrans_fp4;
	int16_t zServo_fp4 = mul16s_rsh(sinServoAngle_fp14[cornerNo],xServoTrans_fp4,14) + mul16s_rsh(cosServoAngle_fp14[cornerNo],zServoTrans_fp4,14);
	
	// the 0.th, 2th and 4th servo has a mirrored x-axis
	if (( cornerNo % 2) == 0)
		xServo_fp4 = -xServo_fp4;
	/*
	if (debug) {
		Serial.print(" C[");
			Serial.print(cornerNo);
			Serial.print("] = (");
			Serial.print(xServo,1,5);
			Serial.print(",");
			Serial.print(yServo,1,5);
			Serial.print(",");
			Serial.print(zServo,1,5);
			Serial.println(")");
		}
	*/
	
	// (len of vector)^2
	// TODO replace i32 by i16 and replace sqrt(float)
	int32_t xServo2_fp4		= mul16s_rsh(xServo_fp4,xServo_fp4,4);
	int32_t yServo2_fp4		= mul16s_rsh(yServo_fp4,yServo_fp4,4);
	int32_t zServo2_fp4		= mul16s_rsh(zServo_fp4,zServo_fp4,4);

	int32_t xyServo2_fp4	= xServo2_fp4 + yServo2_fp4;
	int32_t len2_fp4		= xyServo2_fp4 + zServo2_fp4;

	int32_t cc_fp14 =  	(i32_lsh(len2_fp4,11) - ((int32_t)(LEG_LENGTH*LEG_LENGTH - SERVO_LEVER_LEN*SERVO_LEVER_LEN)<<15))
						/ 	(mul16s(sqrtFasti32(xyServo2_fp4),SERVO_LEVER_LEN));
	
	int16_fp14_t angleRad_fp14;
	// in case the position is too this equation cannot be computed since param of arcsin is > 1
	// in that case we assume the maximu of asin(1) = PI/2this cant be computed since the angle is way over the limits of the serv
	if (abs(cc_fp14)>= FLOAT2FP16(1.0,14))
		angleRad_fp14  = FLOAT2FP16(HALF_PI*sign(cc_fp14),14) - atan2_fp4(xServo_fp4,yServo_fp4);
	else
		angleRad_fp14  = arcsinusRadFast_fp14(cc_fp14)        - atan2_fp4(xServo_fp4,yServo_fp4);
		
	int16_fp4_t angleDeg_fp4 = mul16s_rsh(angleRad_fp14,FLOAT2FP16(RAD_TO_DEG,9),19);
	return angleDeg_fp4;
}


void ServoLegs::setServoAngleByLegCorner(uint8_t cornerNo, int16_fp4_t xWorld_fp4, int16_fp4_t yWorld_fp4, int16_fp4_t zWorld_fp4) {
	int16_fp14_t angle_fp4 = computeServoAngleByCorner(cornerNo, xWorld_fp4,yWorld_fp4,zWorld_fp4);
	servoLegs.setMotorAngle(cornerNo,angle_fp4);
}

void ServoLegs::printCalibrationData() {
	Serial.print( F("servos=("));
	Serial.print( currentPosition_fp4[0],0,5);
	for (uint8_t i = 1;i<6;i++) {
		Serial.print(",");
		Serial.print( currentPosition_fp4[i]);
	}
	Serial.print(") null=(");
	Serial.print( memory.persistentMem.servoLegsConfig.servoNullPos[0],0,4);
	for (uint8_t i = 1;i<6;i++) {
		Serial.print(",");
		Serial.print( memory.persistentMem.servoLegsConfig.servoNullPos[i]);
	}
	Serial.print(")");
};

void ServoLegs::printlnCalibrationData() {
	printCalibrationData();
	Serial.println();
};

void ServoLegs::printAngles() {
		Serial.println( F("Servos=("));
		Serial.print( currentPosition_fp4[0],0,5);
		for (uint8_t i = 1;i<6;i++) {
			Serial.print(",");
			Serial.print( currentPosition_fp4[i],0,5);
		}
		Serial.print(")");
};
void ServoLegs::printDefaults() {
	Serial.println( F("#  null angle"));
	for (uint8_t i = 0;i<6;i++) {
		Serial.print( i );
		Serial.print( " ");
		Serial.print( memory.persistentMem.servoLegsConfig.servoNullPos[i],0,4);
		Serial.println();
	}
};


void ServoLegs::incNullPosition(uint8_t motorIdx, int16_t inc) {
	memory.persistentMem.servoLegsConfig.servoNullPos[motorIdx] += inc;
}

int16_t ServoLegs::getNullPosition(uint8_t pIdx) {
	return memory.persistentMem.servoLegsConfig.servoNullPos[pIdx];
}

void ServoLegs::defineCurrentPositionAsNull() {
	for (uint8_t idx = 0;idx<6;idx++) {
		// 0.th, 2nd and 4th servo has a mirrored x axis
		if ((idx == 0) || (idx == 2) || (idx == 4)) {
			memory.persistentMem.servoLegsConfig.servoNullPos[idx] -= currentPosition_fp4[idx] >>4;

			// motor[pIdx].write(-angleDegree + 90 + memory.ee.servoNullPos[pIdx] );
		}
		else {
			memory.persistentMem.servoLegsConfig.servoNullPos[idx] += currentPosition_fp4[idx] >>4;
			// motor[pIdx].write(angleDegree + 90 + memory.ee.servoNullPos[pIdx] );
		}
	}
	memory.delayedSave(5000);
}




void ServoLegs::printMenuHelp() {
	Serial.println(F("Servo Legs"));
	Serial.println(F("1..6    - consider servo"));
	Serial.println(F("+/-     - amend nullposition"));
	Serial.println(F("h       - help"));
	Serial.println(F("esc     - exit"));
	
	printlnCalibrationData();
}


void ServoLegs::menuController() {
	while (true)  {
		LedBlinker.loop();

		wdt_reset();
		
		if (Serial.available()) {
			static char inputChar;
			inputChar = Serial.read();
			switch (inputChar) {
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
					setCurrentMotor(inputChar-'1');
					break;
				case '+':
				case '-':
					amendNullPosition((inputChar=='+')?1:-1);
					break;
				case 'h':
					printMenuHelp();
					break;
				case '0':
				case '\e':
					return;	
				default:
					break;
			} // switch
		} // if (Serial.available())
	} // while true
}
