/*
 * ServoController.cpp
 *
 *  Created on: Mar 4, 2018
 *      Author: jochenalt
 */

#ifdef __linux__

#include <assert.h>
#include <iostream>
#include "basics/util.h"
#include <servo/ServoController.h>
#include <stewart/BodyKinematics.h>

using namespace std;

ServoController::ServoController() {
}

ServoController::~ServoController() {
}


float ServoController::getServoFrequency() {
	// PCA9685 has descrete frequency values (s.a. datasheet)
	// take the one which is closest to servo frequency of 333Hz
	// (above 333Hz would be 338 Hz)
	return 333;
};

void ServoController::setup() {
	const int i2cBus = 1; 			// this is /dev/ic2-1
	const int i2cAddress = 0x40; 	// i2c address of PCA 9685 from datasheet
	const int servoFrequency = getServoFrequency(); // [Hz] working frequency of KST 215 MG V3 (from datasheet)
	// initialize PCA 9685 board via Adafruit library
	cout << "servo setup via /dev/i2c-" << i2cBus << " at " << servoFrequency << " Hz" << endl;
	pca9685.setup(i2cBus,i2cAddress,servoFrequency);

	BodyKinematics& bodyKin = BodyKinematics::getInstance();
	double bottomServoLimit = degrees(bodyKin.getBodyConfig().bottomServoLimit_rad);
	double topServoLimit = degrees(bodyKin.getBodyConfig().topServoLimit_rad);

	// set of each servo with defaiut null values
	servo[ 0].setup(&pca9685,  0, servoFrequency, false, bottomServoLimit, topServoLimit, 0-4);
	servo[ 1].setup(&pca9685,  1, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-3);
	servo[ 2].setup(&pca9685,  2, servoFrequency, false, bottomServoLimit, topServoLimit, 0-7);
	servo[ 3].setup(&pca9685,  3, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-7);
	servo[ 4].setup(&pca9685,  4, servoFrequency, false, bottomServoLimit, topServoLimit, -1);
	servo[ 5].setup(&pca9685,  5, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-3);

	bottomServoLimit = degrees(bodyKin.getHeadConfig().bottomServoLimit_rad);
	topServoLimit = degrees(bodyKin.getHeadConfig().topServoLimit_rad);

	servo[ 6].setup(&pca9685,  6, servoFrequency, false, bottomServoLimit, topServoLimit, 0 + -3);
	servo[ 7].setup(&pca9685,  7, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-0);
	servo[ 8].setup(&pca9685,  8, servoFrequency, false, bottomServoLimit, topServoLimit, 0-13);
	servo[ 9].setup(&pca9685,  9, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-1);
	servo[10].setup(&pca9685, 10, servoFrequency, false, bottomServoLimit, topServoLimit, 0-0);
	servo[11].setup(&pca9685, 11, servoFrequency, true,  bottomServoLimit, topServoLimit, -30-6);
}

double ServoController::getMinAngle(int servoNo) {
	assert ((servoNo>= 0) && (servoNo < numServos));
	return servo[servoNo].minAngle;
}
double ServoController::getMaxAngle(int servoNo) {
	assert ((servoNo>= 0) && (servoNo < numServos));

	return servo[servoNo].maxAngle;
}

double ServoController::getNullAngle(int servoNo) {
	assert ((servoNo>= 0) && (servoNo < numServos));

	return servo[servoNo].nullAngle;
}

void ServoController::setMinAngle(int servoNo, double newValue) {
	assert ((servoNo>= 0) && (servoNo < numServos));

	servo[servoNo].minAngle = newValue;
}
void ServoController::setMaxAngle(int servoNo, double newValue) {
	assert ((servoNo>= 0) && (servoNo < numServos));

	servo[servoNo].maxAngle = newValue;
}

void ServoController::setNullAngle(int servoNo, double newValue) {
	assert ((servoNo>= 0) && (servoNo < numServos));

	servo[servoNo].nullAngle = newValue;
}

double ServoController::getAngle(int servoNo) {
	return servo[servoNo].getAngle();
}


void ServoController::setAngle_rad(int servoNo, double newAngle_rad) {
	servo[servoNo].setAngle(degrees(newAngle_rad));
}

void ServoController::setAngle_deg(int servoNo, double angle) {
	servo[servoNo].setAngle(angle);
}


void ServoController::calibrateViaKeyBoard() {
	setup();
	int currentStewart = 0;
	int currentServo = 0;
	int currentAngle = 0;
	cout << "select stewart patform: (a,b)" << endl
		 << "select servo          : (0,1,2,3,4,5)" << endl
	     << "set null value        : n" << endl
	     << "change angle          : +/-" << endl;

	changemode(1);
    while (true) {

    	if (kbhit()) {
    		char inp= getchar();

			switch (inp) {
			case 'a':
				currentStewart = 0;
				break;
			case 'b':
				currentStewart = 1;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
  			    currentServo = inp - '0';
				break;
			case '+':
				currentAngle += 1;
				servo[currentServo + currentStewart*6].setAngle(currentAngle);
				break;
			case 'n': {
				setNullAngle(currentServo + currentStewart*6, currentAngle + getNullAngle(currentServo + currentStewart*6));
				setAngle_deg(currentServo + currentStewart*6, 0);
				break;
			}
			case '-':
				currentAngle -= 1;
				servo[currentServo + currentStewart*6].setAngle(currentAngle);
				break;
			}
			currentAngle = getAngle(currentServo + currentStewart*6);
			cout << ((currentStewart == 0)?"A":"B") << "/" << currentServo << " -> " << currentAngle << " deg null="  << getNullAngle(currentServo + currentStewart*6) << endl;
    	}
    }
	changemode(0);

}

#endif
