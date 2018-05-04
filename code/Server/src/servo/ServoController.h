/*
 * ServoController.h
 *
 *  Created on: Mar 4, 2018
 *      Author: jochenalt
 */

#ifndef SRC_SERVO_SERVOCONTROLLER_H_
#define SRC_SERVO_SERVOCONTROLLER_H_

#ifdef __linux__
#include "PCA9685.h"
#include "Servo.h"


class ServoController {
public:
	ServoController();
	virtual ~ServoController();

	// initialize PCA9685 and all servos
	void setup();
	static ServoController& getInstance() {
		static ServoController instance;
		return instance;
	}

	// pwm frequency the servos are controlled with
	float getServoFrequency();

	// show an old-fashioned console used to calibrate min/max/null of servos online
	void calibrateViaKeyBoard();

	// get angle last set
	double  getAngle(int servoNo);

	void setAngle_deg(int servoNo, double angle_deg);
	void setAngle_rad(int servoNo, double newAngle_rad);

	double getMinAngle(int servoNo);
	double getMaxAngle(int servoNo);
	double getNullAngle(int servoNo);

	void setMinAngle(int servoNo, double newValue);
	void setMaxAngle(int servoNo, double newValue);
	void setNullAngle(int servoNo, double newValue);

private:
	PCA9685 pca9685;

	const static int numServos = 12;
	Servo servo[numServos];
};

#endif
#endif /* SRC_SERVO_SERVOCONTROLLER_H_ */
