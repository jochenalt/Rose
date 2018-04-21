/*
 * Servo.cpp
 *
 *  Created on: Mar 4, 2018
 *      Author: jochenalt
 */


#include <basics/util.h>
#include <servo/Servo.h>
#include <servo/PCA9685.h>

#ifdef __linux__

Servo::Servo() {

}

Servo::~Servo() {
}


void Servo::setup(PCA9685* myController, int newChannel, double newFrequency, bool newReverse, double newMinAngle, double newMaxAngle, double newNullAngle) {
	channel = newChannel;
	pca9685 = myController;
	frequency = newFrequency;
	reverse = newReverse;
	calibrate (newMinAngle, newMaxAngle, newNullAngle);
}

void Servo::calibrate(double newMinAngle, double newMaxAngle, double newNullAngle) {
	minAngle = newMinAngle;
	maxAngle = newMaxAngle;
	nullAngle = newNullAngle;
}


void Servo::setAngle(double newAngle) {
	// values of KST DS215MG
    const double middlePositionImpulseLen_us = 1520.0;
    const double impulseLenDiffPerDegree_us = 1000/90.0;
    const double minImpulseLen_us = 887;
    const double maxImpulseLen_us = 2470;

    angle = constrain(newAngle, minAngle, maxAngle);

    double normalizedAngle = (angle + nullAngle);
    if (reverse)
    	normalizedAngle = -normalizedAngle;

    double target_impulse_len_us = (middlePositionImpulseLen_us + impulseLenDiffPerDegree_us * normalizedAngle);
    target_impulse_len_us = constrain(target_impulse_len_us,minImpulseLen_us, maxImpulseLen_us );

    // PCA9685 resolves with 12 bit, which gives 4096 pwm values
    int pwmValue = target_impulse_len_us/(1000000.0/frequency)*4096.0;

    // send pwm value to PCA9685 (channels start with 1! )
    pca9685->setPWM(channel+1, pwmValue);
/*
    int pwmCheck = pca9685->getPWM(channel+1);
    if (pwmCheck != pwmValue)
    	cerr << "servo " << channel << ": pwm =" << pwmCheck << " instead of " << pwmValue << endl;
    	*/
}

#endif
