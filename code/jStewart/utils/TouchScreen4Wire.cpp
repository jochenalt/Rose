/*
 * TouchScreen.cpp
 *
 * Created: 06.12.2014 23:21:59
 *  Author: JochenAlt
 */ 


#include "Arduino.h"
#include "TouchScreen.h"
#include "../StewartMemory.h"
#include "../setup.h"
#include "../space.h"
#include "../utils/FastMath.h"

#include <avr/wdt.h>

#define XPANE_RESISTANCE 585 /* Ohm */
#define ZPANE_RESISTANCE 245 /* Ohm */


#define SAMPLES_PER_MEASUREMENT 4				// number of ADC samples per measurement One ADC measurement takes 100us
#define MAX_ADDITIONAL_SAMPLES 4				// number of tries until a valid measurement has been reached
#define SAMPLE_NOISE 4							// expected standard error in [mm]

// compute maximum varianz out of noise in x and z axis 
// Actually, this is not the variance, but the variance / sample. Otherwise 
// the variance would get much bigger if number of samples increases
#define MAX_VARIANZ_X ((TOUCHSCREEN_ADC_BANDWIDTH*SAMPLE_NOISE*SAMPLE_NOISE*(SAMPLES_PER_MEASUREMENT-2))/TOUCH_SCREEN_X_LEN)	
#define MAX_VARIANZ_Z ((TOUCHSCREEN_ADC_BANDWIDTH*SAMPLE_NOISE*SAMPLE_NOISE*(SAMPLES_PER_MEASUREMENT-2))/TOUCH_SCREEN_Z_LEN)	

#define SLEEP_BEFORE_MEASUREMENT 300			// [us] wait until voltage has unloaded the inner capacity of the touchscreen [us] (I have tried this, seems to be as short as possible)
#define MIN_R_TOUCH 700 
#define MAX_R_TOUCH 1000


#ifndef 	TOUCHSCREEN_HAS_5WIRE

boolean TouchScreen4Wire::xPrepared = false;
boolean TouchScreen4Wire::zPrepared = false;

void TouchScreen4Wire::setup() {
	setDefaults();
}

void TouchScreen4Wire::setDefaults() {
	memory.persistentMem.touchScreenConfig.touchScreenMinX = 85;
	memory.persistentMem.touchScreenConfig.touchScreenMinZ = 150;
	memory.persistentMem.touchScreenConfig.touchScreenMaxX = 920;
	memory.persistentMem.touchScreenConfig.touchScreenMaxZ = 870;
}

int16_t readSample4Wire(uint8_t axisIdx, uint8_t pin1, uint8_t pin2, int16_t maxVariance, int16_t len, int16_t& precision, uint8_t &noOfSamples) {
	uint8_t additionalTries = 0;
	int16_t value[SAMPLES_PER_MEASUREMENT];
	
	int32_t varianceOfSamples;		// variance is computed to check if measurement is stable and can be used
	int16_t avrOfSamples;			// variance requires average
	int16_t medianOfSamples;		// median of samples is returned (removes outstanding samples)
	static int8_t sampleCount = 0;	// measure at both pins alternatively
	noOfSamples = 0;
#define ALTERNATED_PIN(p1,p2) (((sampleCount++) % 2 == 0)?pin1:pin2)
	boolean validSample;
	do {
		validSample = true;
		if (additionalTries == 0) {
			for (uint8_t i = 0;i<SAMPLES_PER_MEASUREMENT-1;i++) {
				value[i] = analogRead(ALTERNATED_PIN(pin1,pin2));
				noOfSamples++;
			}
		}
		else {
			// re-use SAMPLES-1 measurements of last trial, so re-use 0..SAMPLES-1
			// evict the first and lowest sample (it is sorted)
			for (uint8_t i = 0;i<SAMPLES_PER_MEASUREMENT-1;i++)
				value[i] = value[i+1];
		}
		// read only the missing one to SAMPLES
		value[SAMPLES_PER_MEASUREMENT-1] = analogRead(ALTERNATED_PIN(pin1,pin2));
		noOfSamples++;
		
		// now check if this sample can be used.
		// This is done by checking if the variance of the samples besides the best 
		// and worst sample does not exceed maxVariance
		// to do that, sort the samples first in ascending order 
		// (use bubblesort, efficient for small size and likely pre-sorted)
		for(uint8_t i=0; i<SAMPLES_PER_MEASUREMENT-1; i++) {
			for(uint8_t j=i+1; j<SAMPLES_PER_MEASUREMENT; j++) {
				if(value[j] < value[i]) {
					// swap elements
					int16_t temp = value[i];
					value[i] = value[j];
					value[j] = temp;
				}
			}	
		}
		// compute average and check that remaining samples are within the range
		int32_t sampleSum = 0;
		for(uint8_t i=1; i<SAMPLES_PER_MEASUREMENT-1; i++) {
			int16_t v = value[i];
			sampleSum  += v;
			
			// good oportunity to check if value is in range
			if (((axisIdx == X_INDEX) && (((v < memory.persistentMem.touchScreenConfig.touchScreenMinX) || (v > memory.persistentMem.touchScreenConfig.touchScreenMaxX)))) ||
				((axisIdx == Z_INDEX) && (((v < memory.persistentMem.touchScreenConfig.touchScreenMinZ) || (v > memory.persistentMem.touchScreenConfig.touchScreenMaxZ)))))
				validSample = false;
		}
		avrOfSamples = sampleSum /(SAMPLES_PER_MEASUREMENT-2);

		// compute varianz, but without lowest(first) and highest(last) sample
		varianceOfSamples = 0;
		for(uint8_t i=1; i<SAMPLES_PER_MEASUREMENT-1; i++) {
			int16_t diff = value[i]-avrOfSamples;
			varianceOfSamples += (diff*diff);
		}
		
		// compute median. If number of samples is even, take the average instead of the middle sample
		if (SAMPLES_PER_MEASUREMENT % 2 == 0)
			medianOfSamples = (value[(SAMPLES_PER_MEASUREMENT>>1)-1]+value[SAMPLES_PER_MEASUREMENT>>1])>>1;
		else
			medianOfSamples = value[SAMPLES_PER_MEASUREMENT>>1];
		
		
		// return median of all samples to remove bad single measurements
		if (validSample && (varianceOfSamples < maxVariance)) {
			precision = varianceOfSamples/(SAMPLES_PER_MEASUREMENT-2);
			return medianOfSamples;
		} ;
		
		// if we are here, sample was not successful, we have to leftshift the samples and add a new one
		additionalTries++;
	}
	while (additionalTries < MAX_ADDITIONAL_SAMPLES); // try another round, shift the sample and add a measurement

	static uint32_t lastTouch = millis();
	if ((lastTouch>(millis()-100UL)) && (avrOfSamples> 0)) {
		Serial.print(F(" touch err "));
		if (axisIdx == X_INDEX)
			Serial.print(F("X(#"));
		else
			Serial.print(F("Z(#"));
		Serial.print(additionalTries + SAMPLES_PER_MEASUREMENT);
		if (!validSample)
			Serial.print(F(" ,invalid "));
	
		Serial.print(F("dist="));
		Serial.print(int16_t(sqrt(uint32_t(len)*varianceOfSamples/(770UL*(SAMPLES_PER_MEASUREMENT-2)))));
		Serial.print(">");
		Serial.print(int16_t(sqrt(uint32_t(len)*maxVariance/(770UL*(SAMPLES_PER_MEASUREMENT-2)))));
		Serial.print("|");
		for (uint8_t i = 0;i<SAMPLES_PER_MEASUREMENT;i++) {
			if (i>0)
				Serial.print(",");
			Serial.print(value[i]);
		}
		Serial.println(")");
	}
	if (avrOfSamples>0) 
		lastTouch=millis();
	precision = -1;
	return -1;
}

uint16_t computeRTouch4Wire(int16_t touchX, int16_t touchZ, int16_t pressure) {
	int16_t rtouch=(float(touchX)*XPANE_RESISTANCE)*(1/1024.0) * (1024.0/pressure -1.0) - ZPANE_RESISTANCE*(1.0 - float(touchZ)/1024.0);
	return rtouch;
}

uint16_t TouchScreen4Wire::pressure() {
	// Set X+ to ground
	pinMode(TOUCH4W_ZP_PIN, OUTPUT);
	digitalWrite(TOUCH4W_ZP_PIN, LOW);
	
	// Set Z- to VCC
	pinMode(TOUCH4W_XM_PIN, OUTPUT);
	digitalWrite(TOUCH4W_XM_PIN, HIGH);
	
	// Hi-Z X- and Z+
	digitalWrite(TOUCH4W_ZM_PIN, LOW);
	pinMode(TOUCH4W_ZM_PIN, INPUT);
	digitalWrite(TOUCH4W_XP_PIN, LOW);
	pinMode(TOUCH4W_XP_PIN, INPUT);
	delayMicroseconds(SLEEP_BEFORE_MEASUREMENT);
	
	int z = analogRead(TOUCH4W_ZM_PIN);
	return z;
}


void TouchScreen4Wire::prepareXPane(boolean wait) {
	// preparation means to set the pins such that
	// the GND/5V is in z-axis and measurement
	// can happen in x-axis. Since the touchscreen
	// has an internal capacity, we have to wait 
	// until this capacity is loaded/unloaded
	// This is a waste of time, so we try to gain time
	// by setting the x-pane up after the measurement,
	// so that we dont have to wait for it when the next 
	// measurement in the next loop happens.
	if (xPrepared)
		return;
		
	xPrepared = true;
	zPrepared = false;
	// that's where the measurement happens later on
	pinMode(TOUCH4W_XP_PIN, INPUT);	
	pinMode(TOUCH4W_XM_PIN, INPUT);	
		
	pinMode(TOUCH4W_ZP_PIN, OUTPUT);
	digitalWrite(TOUCH4W_ZP_PIN, LOW);

	pinMode(TOUCH4W_ZM_PIN, OUTPUT);
	digitalWrite(TOUCH4W_ZM_PIN, HIGH);
	
    // setting touch pane to 0 and 5V requires the internal capacity of the to load 
	if (!wait)
		delayMicroseconds(SLEEP_BEFORE_MEASUREMENT); 
}


void TouchScreen4Wire::prepareZPane(boolean wait) {
	if (zPrepared)
		return;
		
	zPrepared = true;
	xPrepared = false;

	// that's where the measurement happens later on
	pinMode(TOUCH4W_ZP_PIN, INPUT);   
	pinMode(TOUCH4W_ZM_PIN, INPUT);   
	
	pinMode(TOUCH4W_XP_PIN, OUTPUT);
	digitalWrite(TOUCH4W_XP_PIN, LOW);
	pinMode(TOUCH4W_XM_PIN, OUTPUT);
	digitalWrite(TOUCH4W_XM_PIN, HIGH);
	
	// setting touch pane to 0 and 5V requires the internal capacity of the to load
	if (wait)
		delayMicroseconds(SLEEP_BEFORE_MEASUREMENT);
}	

boolean TouchScreen4Wire::sample(TouchPoint& point) {
	prepareXPane(true /* always wait if x-pane has been set up */); 
	point.rawX = readSample4Wire(X_INDEX,TOUCH4W_XP_PIN, TOUCH4W_XM_PIN, MAX_VARIANZ_X, TOUCH_SCREEN_X_LEN,point.precisionX,point.noOfSamplesX);
	
	if (point.rawX  > 0) {
		prepareZPane(true /* always wait if z-pane has been set up */);		
		point.rawZ = readSample4Wire(Z_INDEX,TOUCH4W_ZM_PIN,TOUCH4W_ZP_PIN, MAX_VARIANZ_Z,TOUCH_SCREEN_Z_LEN,point.precisionZ,point.noOfSamplesZ);
		prepareXPane(false /* setup for next loop, there's enough time, so dont wait in addition */);
		
		if (point.rawZ > 0) {
			// scale adc value to [mm] 
			// TODO make x,y fp4
			point.x_fp4=  ((int32_t)TOUCH_SCREEN_X_LEN*(1<<4))*(point.rawX-memory.persistentMem.touchScreenConfig.touchScreenMinX)/
									(memory.persistentMem.touchScreenConfig.touchScreenMaxX-memory.persistentMem.touchScreenConfig.touchScreenMinX) - (TOUCH_SCREEN_X_LEN*(1<<4)/2);
			point.z_fp4 = ((int32_t)TOUCH_SCREEN_Z_LEN*(1<<4))*(point.rawZ-memory.persistentMem.touchScreenConfig.touchScreenMinZ)/
									(memory.persistentMem.touchScreenConfig.touchScreenMaxZ-memory.persistentMem.touchScreenConfig.touchScreenMinZ) - (TOUCH_SCREEN_Z_LEN*(1<<4)/2);
			
			return true;
		}		
	}
	
	return false;
}

void TouchScreen4Wire::println() {
	Serial.println(F("touchscreen calibration"));
	Serial.print("(");
	Serial.print(memory.persistentMem.touchScreenConfig.touchScreenMinX);
	Serial.print(",");
	Serial.print(memory.persistentMem.touchScreenConfig.touchScreenMaxX);
	Serial.print(")-(");
	Serial.print(memory.persistentMem.touchScreenConfig.touchScreenMinZ);
	Serial.print(",");
	Serial.print(memory.persistentMem.touchScreenConfig.touchScreenMaxZ);
	Serial.print(")");
}

#endif