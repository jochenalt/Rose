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
 * Name        : PCA9685.cpp
 * Author      : Georgi Todorov
 * Version     :
 * Created on  : Dec 9, 2012
 *
 * Copyright © 2012 Georgi Todorov  <terahz@geodar.com>
 */

#include "assert.h"
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>      /* Standard I/O functions */
#include <fcntl.h>
#include <syslog.h>		/* Syslog functionallity */
#include <inttypes.h>
#include <errno.h>
#include <math.h>

#include "PCA9685.h"

//! Constructor takes bus and address arguments
/*!
 \param bus the bus to use in /dev/i2c-%d.
 \param address the device address on bus
 */
PCA9685::PCA9685(int bus, int address) {
#ifdef NEW
	i2c = new I2CInterface();
	i2c->setup("/dev/i2c-1", 0x40);
	i2c->open( I2CInterface::ReadWrite );

#else
	i2c = new I2C(bus,address);
	reset();
	setPWMFreq(1000);
#endif
}

PCA9685::~PCA9685() {
	delete i2c;
}
//! Sets PCA9685 mode to 00
void PCA9685::reset() {

		i2c->write_byte(PCA9685_MODE1, 0x00); //Normal mode
		i2c->write_byte(PCA9685_MODE2, 0x04); //totem pole

}
//! Set the frequency of PWM
/*!
 \param freq desired frequency. 40Hz to 1000Hz using internal 25MHz oscillator.
 */
void PCA9685::setPWMFreq(int freq) {

		uint8_t prescale_val = (PCA9685_CLOCK_FREQ / 4096 / freq)  - 1;
		i2c->write_byte(PCA9685_MODE1, 0x10); //sleep
		i2c->write_byte(PCA9685_PRESCALE, prescale_val); // multiplyer for PWM frequency
		i2c->write_byte(PCA9685_MODE1, 0x80); //restart
		i2c->write_byte(PCA9685_MODE2, 0x04); //totem pole (default)
}

//! PWM a single channel
/*!
 \param led channel (1-16) to set PWM value for
 \param value 0-4095 value for PWM
 */
void PCA9685::setPWM(uint8_t led, int value) {
	setPWM(led, 0, value);
}
//! PWM a single channel with custom on time
/*!
 \param led channel (1-16) to set PWM value for
 \param on_value 0-4095 value to turn on the pulse
 \param off_value 0-4095 value to turn off the pulse
 */
void PCA9685::setPWM(uint8_t led, int on_value, int off_value) {
		i2c->write_byte(PCA9685_LED0_ON_L + PCA9685_LED_MULTIPLYER * (led - 1), on_value & 0xFF);
		i2c->write_byte(PCA9685_LED0_ON_H + PCA9685_LED_MULTIPLYER * (led - 1), on_value >> 8);
		i2c->write_byte(PCA9685_LED0_OFF_L + PCA9685_LED_MULTIPLYER * (led - 1), off_value & 0xFF);
		i2c->write_byte(PCA9685_LED0_OFF_H + PCA9685_LED_MULTIPLYER * (led - 1), off_value >> 8);
}



//! Get current PWM value
/*!
 \param led channel (1-16) to get PWM value from
 */
int PCA9685::getPWM(uint8_t led){
	int ledval = 0;
	ledval = i2c->read_byte(PCA9685_LED0_OFF_H + PCA9685_LED_MULTIPLYER * (led-1));
	ledval = ledval & 0xf;
	ledval <<= 8;
	ledval += i2c->read_byte(PCA9685_LED0_OFF_L + PCA9685_LED_MULTIPLYER * (led-1));
	return ledval;
}

void PCA9685::setOutDriver(bool outDriver) {
	uint8_t Data = i2c->read_byte(PCA9685_MODE2);

	Data &= ~PCA9685_MODE2_OUTDRV;

	if (outDriver) {
		Data |= PCA9685_MODE2_OUTDRV;
	}

	i2c->write_byte(PCA9685_MODE2, Data);
}

void PCA9685::setInvert(bool invert) {
	uint8_t Data = i2c->read_byte(PCA9685_MODE2);

	Data &= ~PCA9685_MODE2_INVRT;

	if (invert) {
		Data |= PCA9685_MODE2_INVRT;
	}

	i2c->write_byte(PCA9685_MODE2, Data);
}

void PCA9685::dump() {
		uint8_t reg = i2c->read_byte(PCA9685_MODE1);

		printf("MODE1 - Mode register 1 (address 00h) : %02Xh\n", reg);
		printf("\tbit 7 - RESTART : Restart %s\n", reg & PCA9685_MODE1_RESTART ? "enabled" : "disabled");
		printf("\tbit 6 - EXTCLK  : %s\n", reg & PCA9685_MODE1_EXTCLK ? "Use EXTCLK pin clock" : "Use internal clock");
		printf("\tbit 5 - AI      : Register Auto-Increment %s\n", reg & PCA9685_MODE1_AI ? "enabled" : "disabled");
		printf("\tbit 4 - SLEEP   : %s\n", reg & PCA9685_MODE1_SLEEP ? "Low power mode. Oscillator off" : "Normal mode");
		printf("\tbit 3 - SUB1    : PCA9685 %s to I2C-bus subaddress 1\n", reg & PCA9685_MODE1_SUB1 ? "responds" : "does not respond");
		printf("\tbit 2 - SUB1    : PCA9685 %s to I2C-bus subaddress 2\n", reg & PCA9685_MODE1_SUB2 ? "responds" : "does not respond");
		printf("\tbit 1 - SUB1    : PCA9685 %s to I2C-bus subaddress 3\n", reg & PCA9685_MODE1_SUB3 ? "responds" : "does not respond");
		printf("\tbit 0 - ALLCALL : PCA9685 %s to LED All Call I2C-bus address\n", reg & PCA9685_MODE1_ALLCALL ? "responds" : "does not respond");

		reg = i2c->read_byte(PCA9685_MODE2);

		printf("\nMODE2 - Mode register 2 (address 01h) : %02Xh\n", reg);
		printf("\tbit 7 to 5      : Reserved\n");
		printf("\tbit 4 - INVRT   : Output logic state %sinverted\n", reg & PCA9685_MODE2_INVRT ? "" : "not ");
		printf("\tbit 3 - OCH     : Outputs change on %s\n", reg & PCA9685_MODE2_OCH ? "ACK" : "STOP command");
		printf("\tbit 2 - OUTDRV  : The 16 LEDn outputs are configured with %s structure\n", reg & PCA9685_MODE2_OUTDRV ? "a totem pole" : "an open-drain");
		printf("\tbit 10- OUTNE   : %01x\n", reg & 0x3);

		reg = i2c->read_byte(PCA9685_PRESCALE);

		printf("\nPRE_SCALE register (address FEh) : %02Xh\n", reg);
		printf("\t Frequency : %d Hz\n", CalcFrequency(reg));

		printf("\n");

		uint16_t on, off;

		for (uint8_t nLed = 0; nLed <= 15; nLed ++) {
			Read(nLed, &on, &off);
			printf("LED%d_ON  : %04x\n", nLed, on);
			printf("LED%d_OFF : %04x\n", nLed, off);
		}

		printf("\n");

		Read(16, &on, &off);
		printf("ALL_LED_ON  : %04x\n", on);
		printf("ALL_LED_OFF : %04x\n", off);
}

void PCA9685::Read(uint8_t nChannel, uint16_t *pOn, uint16_t *pOff) {
	assert(pOn != 0);
	assert(pOff != 0);

	uint8_t reg;

	if (nChannel <= 15) {
		reg = PCA9685_LED0_ON_L + (nChannel << 2);
	} else {
		reg = PCA9685_ALLLED_ON_L;
	}

	if (pOn != 0) {
		*pOn = i2c->read_byte(reg);
	}

	if (pOff) {
		*pOff = i2c->read_byte(reg + 2);
	}
}


#define DIV_ROUND_UP(n,d)	(((n) + (d) - 1) / (d))

uint8_t PCA9685::CalcPresScale(uint16_t nFreq) {
	nFreq = (nFreq > PCA9685_FREQUENCY_MAX ? PCA9685_FREQUENCY_MAX : (nFreq < PCA9685_FREQUENCY_MIN ? PCA9685_FREQUENCY_MIN : nFreq));

	const float f = (float) PCA9685_CLOCK_FREQ / 4096;

	const uint8_t Data = (uint8_t) DIV_ROUND_UP(f, nFreq) - 1;

	return Data;
}

uint16_t PCA9685::CalcFrequency(uint8_t nPreScale) {
	uint16_t f_min;
	uint16_t f_max;
	const float f = (float) PCA9685_CLOCK_FREQ / 4096;
	const uint16_t Data = (uint16_t) DIV_ROUND_UP(f, ((uint16_t) nPreScale + 1));

	for (f_min = Data; f_min > PCA9685_FREQUENCY_MIN; f_min--) {
		if (CalcPresScale(f_min) != nPreScale) {
			break;
		}
	}

	for (f_max = Data; f_max < PCA9685_FREQUENCY_MAX; f_max++) {
		if (CalcPresScale(f_max) != nPreScale) {
			break;
		}
	}

	return (f_max + f_min) / 2;
}
