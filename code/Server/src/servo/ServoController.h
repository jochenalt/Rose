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
	const static int NumOfServos = 9;
	enum SERVO_ID { STEWART_SERVO0 = 0, STEWART_SERVO1 = 1, STEWART_SERVO2 = 2, STEWART_SERVO3 = 3, STEWART_SERVO4 = 4, STEWART_SERVO5 = 5,
		            MOUTH_TURN_SERVO = 6, MOUTH_OPEN_SERVO= 7, MOUTH_TILT_SERVO = 8};
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
	Servo servo[NumOfServos];
};

#endif
#endif /* SRC_SERVO_SERVOCONTROLLER_H_ */
