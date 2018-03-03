/**
 * @file servo.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "PCA9685Servo.h"

#define MAX_12BIT	(0xFFF)
#define MAX_8BIT	(0xFF)
#define MAX_ANGLE	(180)

#define MID_COUNT	(uint16_t) (.5 + ((204.8 * SERVO_CENTER_DEFAULT_US) / 1000))

PCA9685Servo::PCA9685Servo(int bus, int address): PCA9685(bus, address), m_nLeftUs(SERVO_LEFT_DEFAULT_US), m_nRightUs(SERVO_RIGHT_DEFAULT_US) {
	setInvert(false);
	setOutDriver(true);
	setPWMFreq(50);
	CalcLeftCount();
	CalcRightCount();
}

PCA9685Servo::~PCA9685Servo(void) {
}

void PCA9685Servo::SetLeftUs(uint16_t nLeftUs) {
	assert(nLeftUs < m_nRightUs);

	m_nLeftUs = nLeftUs;
	CalcLeftCount();
}

uint16_t PCA9685Servo::GetLeftUs(void) const {
	return m_nLeftUs;
}

void PCA9685Servo::SetRightUs(uint16_t nRightUs) {
	assert(nRightUs > m_nLeftUs);

	m_nRightUs = nRightUs;
	CalcRightCount();
}

uint16_t PCA9685Servo::GetRightUs(void) const {
	return m_nRightUs;
}

void PCA9685Servo::CalcLeftCount(void) {
	m_nLeftCount = (uint16_t) (.5 + ((204.8 * m_nLeftUs) / 1000));
}

void PCA9685Servo::CalcRightCount(void) {
	m_nRightCount = (uint16_t) (.5 + ((204.8 * m_nRightUs) / 1000));
}

void PCA9685Servo::Set(uint8_t nChannel, uint16_t nData) {

	if (nData > m_nRightCount) {
		nData = m_nRightCount;
	} else if (nData < m_nLeftCount) {
		nData = m_nLeftCount;
	}

	setPWM(nChannel, nData);
}

void PCA9685Servo::Set(uint8_t nChannel, uint8_t nData) {

	if (nData == 0) {
		setPWM(nChannel, m_nLeftCount);
	} else if (nData == (MAX_8BIT + 1) / 2) {
		setPWM(nChannel, MID_COUNT);
	}  else if (nData == MAX_8BIT) {
		setPWM(nChannel, m_nRightCount);
	} else {
		const uint16_t nCount = m_nLeftCount + (.5 + ((float) (m_nRightCount - m_nLeftCount) / MAX_8BIT) * nData);
		setPWM(nChannel, nCount);
	}
}

void PCA9685Servo::SetAngle(uint8_t nChannel, uint8_t nAngle) {

	if (nAngle == 0) {
		setPWM(nChannel, m_nLeftCount);
	} else if (nAngle == 90) {
		setPWM(nChannel, MID_COUNT);
	}  else if (nAngle >= 180) {
		setPWM(nChannel, m_nRightCount);
	} else if (nAngle < 90) {
		const uint16_t nCount = m_nLeftCount + (uint16_t) (.5 + ((float) (MID_COUNT - m_nLeftCount) / 90 ) * nAngle);
		setPWM(nChannel, nCount);
	} else {
		const uint16_t nCount = (2 * MID_COUNT) - m_nRightCount +  (uint16_t) (.5 + ((float) (m_nRightCount - MID_COUNT) / 90 ) * nAngle);
		setPWM(nChannel, nCount);
	}
}

#define ANGLE(x) ((uint8_t)(x))

void ledTest() {
	int bus = 1;
	int address = 40;

	std::cout << "set PCA9685 on bus " << bus << " on address " << address << std::endl;
	PCA9685 led(bus,address);
	led.setPWMFreq(1000);
	for (int i = 0;i<1000;i++) {
		std::cout << i << ".th run" << std::endl;
		for (int l = 0;l<15;l++) {
			for (int v = 0;i<4096;i++) {
				std::cout << "set channel " << l << "=" << v << std::endl;
				led.setPWM(0,v);
			}
		}
	}
}

void servoTest() {

		PCA9685Servo servo(1,8);

		// MG90S Micro Servo
		servo.SetLeftUs(700);
		servo.SetRightUs(2400);

		servo.dump();

		servo.SetAngle(CHANNEL(0), ANGLE(90));
		servo.SetAngle(CHANNEL(1), ANGLE(90));
		servo.SetAngle(CHANNEL(2), ANGLE(90));
		servo.SetAngle(CHANNEL(3), ANGLE(90));

		puts("Servo 0:90  Servo 1:90");
		puts("Servo 2:90  Servo 3:90");

		sleep(4);

		for (;;) {
			servo.SetAngle(CHANNEL(0), ANGLE(90));
			servo.SetAngle(CHANNEL(1), ANGLE(0));
			servo.SetAngle(CHANNEL(2), ANGLE(0));
			servo.SetAngle(CHANNEL(3), ANGLE(0));
			puts("");
			puts("Servo 0:90  Servo 1:0");
			puts("Servo 2:0  Servo 3:0");
			sleep(2);
			servo.SetAngle(CHANNEL(0), ANGLE(180));
			servo.SetAngle(CHANNEL(1), ANGLE(180));
			servo.SetAngle(CHANNEL(2), ANGLE(90));
			servo.SetAngle(CHANNEL(3), ANGLE(90));
			puts("");
			puts("Servo 0:180 Servo 1:180");
			puts("Servo 2:90  Servo 3:90");
			sleep(2);
			servo.SetAngle(CHANNEL(0), ANGLE(0));
			servo.SetAngle(CHANNEL(1), ANGLE(90));
			servo.SetAngle(CHANNEL(2), ANGLE(180));
			servo.SetAngle(CHANNEL(3), ANGLE(180));
			puts("");
			puts("Servo 0:0   Servo 1:90");
			puts("Servo 2:180  Servo 3:180");
			sleep(2);
		}
}
