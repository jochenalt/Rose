/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Name        : PCA9685.h
 * Author      : Georgi Todorov
 * Version     :
 * Created on  : Dec 9, 2012
 *
 * Copyright © 2012 Georgi Todorov  <terahz@geodar.com>
 */

#ifndef _PCA9685_H
#define _PCA9685_H
#include <inttypes.h>
#include "I2C.h"

// Register Definitions
/* See 7.3 Register definitions */
#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_ALLCALLADR 0x05
#define PCA9685_PRESCALE 0xFE

/* See 7.3.1 Mode register 1, MODE1 */
#define PCA9685_MODE1_RESTART 0x80
#define PCA9685_MODE1_EXTCLK 0x40
#define PCA9685_MODE1_AI 0x20
#define PCA9685_MODE1_SLEEP 0x10
#define PCA9685_MODE1_SUB1 0x08
#define PCA9685_MODE1_SUB2 0x04
#define PCA9685_MODE1_SUB3 0x02
#define PCA9685_MODE1_ALLCALL 0x01

/* 7.3.2 Mode register 2, MODE2 */
#define PCA9685_MODE2_INVRT 0x10
#define PCA9685_MODE2_OCH 0x08
#define PCA9685_MODE2_OUTDRV 0x04
#define PCA9685_MODE2_OUTNE 0x03 //Actually a mask of bits. Paragraph 7.3.2 Mode register 2, MODE2

#define PCA9685_LED0 0x6			//LED0 start register
#define PCA9685_LED0_ON_L 0x6		//LED0 output and brightness control byte 0
#define PCA9685_LED0_ON_H 0x7		//LED0 output and brightness control byte 1
#define PCA9685_LED0_OFF_L 0x8		//LED0 output and brightness control byte 2
#define PCA9685_LED0_OFF_H 0x9		//LED0 output and brightness control byte 3
#define PCA9685_LED_MULTIPLYER 4	// For the other 15 channels
#define PCA9685_ALLLED_ON_L 0xFA    //load all the LEDn_ON registers, byte 0 (turn 0-7 channels on)
#define PCA9685_ALLLED_ON_H 0xFB	//load all the LEDn_ON registers, byte 1 (turn 8-15 channels on)
#define PCA9685_ALLLED_OFF_L 0xFC	//load all the LEDn_OFF registers, byte 0 (turn 0-7 channels off)
#define PCA9685_ALLLED_OFF_H 0xFD	//load all the LEDn_OFF registers, byte 1 (turn 8-15 channels off)
#define PCA9685_CLOCK_FREQ 25000000.0 //25MHz default osc clock

#define CHANNEL(x)	((uint8_t)(x))
#define VALUE(x)	((uint16_t)(x))

#define PCA9685_VALUE_MIN	VALUE(0)
#define PCA9685_VALUE_MAX	VALUE(4096)

#define PCA9685_PWM_CHANNELS	16

enum TPCA9685FrequencyRange {
	PCA9685_FREQUENCY_MIN = 24,
	PCA9685_FREQUENCY_MAX = 1526
};

enum TPCA9685Och {
	PCA9685_OCH_STOP = 0,
	PCA9685_OCH_ACK = 1 << 3
};

//! Main class that exports features for PCA9685 chip
class PCA9685 {
public:
	PCA9685();
	virtual ~PCA9685();
	void setup(int bus,int address);

	void setPWMFreq(int);
	void setPWM(uint8_t, int, int);
	void setPWM(uint8_t, int);
	int getPWM(uint8_t);

	void Read(uint8_t nChannel, uint16_t *pOn, uint16_t *pOff);

	void setOutDriver(bool bOutDriver);
	void setInvert(bool invert);
	void dump();

	uint8_t CalcPresScale(uint16_t nFreq);
	uint16_t CalcFrequency(uint8_t nPreScale);

private:
	I2C *i2c;

	void reset(void);
};
#endif
