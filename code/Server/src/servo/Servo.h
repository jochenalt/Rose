/*
 * Servo.h
 *
 *  Created on: Mar 4, 2018
 *      Author: Jochen Alt
 */

#ifndef SRC_SERVO_SERVO_H_
#define SRC_SERVO_SERVO_H_

#ifdef __linux__
#include "PCA9685.h"

class Servo {
	friend class ServoController;
public:
	Servo();
	virtual ~Servo();
	void setup(PCA9685* pca9685, int channel /* 0..15 */, double newFrequency = 100, bool reverse = false, double minAngle = -85, double maxAngle = 60, double nullAngle = 0);
	void calibrate(double minAngle = -85, double maxAngle = 60, double nullAngle = 0);

	void setAngle(double angle_deg);
	double getAngle() { return angle; };

private:
	int channel = 0;				// one servo per channel 0..15
	PCA9685* pca9685 = 0;			// interface to PCA9685 PWM driver
	double frequency = 0;			// pwm frequency sent to the servos (300Hz)
	bool reverse = false;			// true, if servo is supposed to move anti-clockwise
	double minAngle = 0;
	double maxAngle = 0;
	double nullAngle = 0;

	double angle = 0;
	int counter = 0;
};
#endif
#endif /* SRC_SERVO_SERVO_H_ */
