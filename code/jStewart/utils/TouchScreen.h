/*
 * TouchScreen.h
 *
 * Created: 06.12.2014 23:22:10
 *  Author: JochenAlt
 */ 


#ifndef TOUCHSCREEN_H_
#define TOUCHSCREEN_H_

#include "setup.h"

class TouchScreenConfig {
	public:
			int16_t touchScreenMinX;
			int16_t touchScreenMinZ;
			int16_t touchScreenMaxX;
			int16_t touchScreenMaxZ;
};

class TouchPoint {
	public:
	
	TouchPoint(void) {
		x_fp4 = z_fp4 = 0;
	}
	void print(String s) {
		Serial.print(s);
		print();
	}

	void print() {
		Serial.print("(");
		Serial.print(rawX,0,3);
		Serial.print(",");

		Serial.print(x_fp4>>4,0,3);
		// Serial.print("[+/-");
		// Serial.print(precisionX,0,2);
		// Serial.print("],");
		Serial.print("/");
		Serial.print(rawZ,0,3);
		Serial.print(",");
		Serial.print(z_fp4>>4,0,3);
		// Serial.print("[+/-");
		// Serial.print(precisionZ,0,2);

		Serial.print(")");
	}


	int16_t x_fp4, z_fp4;

	int16_t rawX;
	int16_t rawZ;
	int16_t precisionX;
	int16_t precisionZ;
	int16_t rtouch;
	uint8_t noOfSamplesX;
	uint8_t noOfSamplesZ;

};

#ifdef TOUCHSCREEN_HAS_5WIRE

class TouchScreen5Wire {
	public:
	TouchScreen5Wire() {};
	static void setup();
	boolean sample(TouchPoint& point);
	void println();
	static void setDefaults();

	private:
	void prepareXPane(boolean wait);
	void prepareZPane(boolean wait);

	static boolean xPrepared ;
	static boolean zPrepared ;

};
extern TouchScreen5Wire touchScreen;
#else
class TouchScreen4Wire {
	public:
	
		TouchScreen4Wire() {};
		static void setup();
		boolean sample(TouchPoint& point);
		void println();
		static void setDefaults();

	private:
		void prepareXPane(boolean wait);
		void prepareZPane(boolean wait);
		uint16_t pressure();

		static boolean xPrepared ;
		static boolean zPrepared ;

};
extern TouchScreen4Wire touchScreen;
#endif
#endif /* TOUCHSCREEN_H_ */