/*
 * main.cpp
 *
 * Created: 21.11.2014 22:32:08
 *  Author: JochenAlt
 */ 


#include "Arduino.h"
#include "HardwareSerial.h"
#include "setup.h"

#include <avr/io.h>
#include <avr/wdt.h>

#include "PatternBlinker.h"
#include "StewartMemory.h"
#include "Trajectory.h"
#include "PlatformCircleTrajectory.h"
#include "KalmanEstimator.h"

#include "ServoLegs.h"
#include "TouchScreen.h"
#include "BallController.h"
#include "BallSquareWaveTrajectory.h"
#include "BallLissajousTrajectory.h"
#include "LineTrajectory.h"
#include "IMUController.h"

static uint8_t FancyBlinkPattern[2] = { 0b11001000,0b00000000};		// nice!
static uint8_t InitBlinkPattern[1] =  { 0b10101010};				// 
static uint8_t ErrorBlinkPattern[1] = { 0b11111111};				// 

PatternBlinker LedBlinker;
PatternBlinker ErrorLEDBlinker;

extern Platform platform;
LineTrajectory lineTrajectory;
BallSquareWaveTrajectory ballSquareWave;
BallLissajousTrajectory ballCircle;

bool debug = false;
bool ballControllerActive = true;
bool ballControllerSquareWave = false;
bool ballControllerLissajous = false;
TouchScreen5Wire touchScreen;
BallController ballController;

Trajectory platformTrajectory(platform,ballController);

enum CalibrationType { BALL_PID_CONTROLLER_PROP, BALL_PID_CONTROLLER_INT, BALL_PID_CONTROLLER_DER, NO_CALIBRATION};
CalibrationType calibType = NO_CALIBRATION;

void printMainMenuHelp() {
	Serial.println(F("Stewart Platform"));
	Serial.println(F("S       - servo menu"));
	Serial.println(F("I       - IMU menu"));
	Serial.println(F("K       - kinematics menu"));

	Serial.println(F("_       - set null positionrotate"));
	Serial.println(F("L       - start lissajous pattern"));
	Serial.println(F("b       - start ball controller"));
	Serial.println(F("d       - debug on/off"));
	Serial.println(F("t       - set rotation point"));
	Serial.println(F("+/-     - +/- of current value"));

	Serial.println(F("h       - this page"));

	Serial.println();
	servoLegs.printlnCalibrationData();
	platform.println();
	ballController.printCalibrationData();
	imu.printCalibrationData();
}

void setError() {
	ErrorLEDBlinker.setOneTime(ErrorBlinkPattern,sizeof(ErrorBlinkPattern));
}


void setup() {	
	// watch dog
	wdt_enable(WDTO_1S);

	// everyone likes a blinking LED. This one is used by the pattern blinker
	LedBlinker.setup(LED_PIN, 50);
	ErrorLEDBlinker.setup(ERROR_LED_PIN, LOOP_TIME_MS);
	LedBlinker.set(InitBlinkPattern,sizeof(InitBlinkPattern));
		
	// initialize serial 
	Serial.begin(INITIAL_BAUD_RATE);

	// setup EPPROM	and read stored values 
	memory.setup();
	
	// initialize translation vectors
	platform.setup();
	
	// initialize servos
	servoLegs.setup();

	// setup touchscreen
	touchScreen.setup();
	
	// move the servos into null position
	ballController.setup();
		
	// setup IMU
	imu.setup();
	imu.calibrate(); // calibrate current position 

	platform.moveIt();
	
	Serial.println(F("Hello. I can dance."));

	// start the nice pattern blinker
	LedBlinker.set(FancyBlinkPattern,sizeof(FancyBlinkPattern));
}


void loop() {
	while (true) {
		wdt_reset();	
		
		LedBlinker.loop();
		ErrorLEDBlinker.loop();
		memory.loop();
		if (memory.hasBeenSaved()) 
			Serial.println(F("settings have been saved saved."));

		platformTrajectory.loop();
		boolean ballIsOn = false;
		if (ballControllerActive || platformTrajectory.isBallControllerOn()) {
			ballIsOn = ballController.loop();
			static boolean lastTimeBallIsOn= false;
			if (ballIsOn) 
				lastTimeBallIsOn = true;
			else {
				if (lastTimeBallIsOn) {
					Serial.println("Noooo, ball has been removed!");
					// ball has been removed
					// start smooth trajectory to null position
					Point nullPoint;
					Rotation nullRot;
					platformTrajectory.setActualTrajectory(&lineTrajectory);
					lineTrajectory.setTarget(nullPoint,nullRot, 1000);		
				};
				lastTimeBallIsOn = false;
			}
		}
		
		if (Serial.available()) {
			char inputChar = Serial.read();
	
			// no, check interactive commands
			switch (inputChar) {
				case 'I':
					imu.printMenuHelp();
					imu.printCalibrationData();
					imu.menuController();
					LedBlinker.set(FancyBlinkPattern,sizeof(FancyBlinkPattern));
					printMainMenuHelp();
					break;
				case 'S':
					servoLegs.printMenuHelp();
					servoLegs.menuController();
					LedBlinker.set(FancyBlinkPattern,sizeof(FancyBlinkPattern));
					printMainMenuHelp();
					break;
				case 'K':
					platform.printMenuHelp();
					platform.menuController();
					LedBlinker.set(FancyBlinkPattern,sizeof(FancyBlinkPattern));
					printMainMenuHelp();
					break;
				case 'h':
					printMainMenuHelp();
					break;
				case '+':
				case '-':
					switch (calibType) {
						case BALL_PID_CONTROLLER_PROP:
							memory.persistentMem.ballControllerConfig.propWeight_fp14 += (inputChar=='+'?FLOAT2FP16(0.0005,14):FLOAT2FP16(-0.0005,14));
							ballController.calibrate();
							ballController.printCalibrationData();
							break;
						case BALL_PID_CONTROLLER_INT:
							memory.persistentMem.ballControllerConfig.integrativeWeight_fp14 += (inputChar=='+'?FLOAT2FP16(0.0001,14):FLOAT2FP16(-0.0001,14));
							ballController.calibrate();
							ballController.printCalibrationData();
							break;
						case BALL_PID_CONTROLLER_DER:
							memory.persistentMem.ballControllerConfig.derivativeWeight_fp14 += (inputChar=='+'?FLOAT2FP16(0.0002,14):FLOAT2FP16(-0.0002,14));
							ballController.calibrate();
							ballController.printCalibrationData();
							break;
						default:
							break;
					}
					break;
				case 'b':
					ballControllerActive = ballControllerActive?false:true;
					if (!ballControllerActive)
						platformTrajectory.switchBallController(false);
					Serial.println((ballControllerActive?"ball controller on":"ball controller off"));
					break;
				case '!':
					calibType = BALL_PID_CONTROLLER_PROP;
					Serial.println(F("use +/- to adjust pid controller prop factor"));
					ballController.printCalibrationData();
					break;
				case '"':
					calibType = BALL_PID_CONTROLLER_INT;
					Serial.println(F("use +/- to adjust pid controller int factor"));
					ballController.printCalibrationData();
					break;
				case '§':
					calibType = BALL_PID_CONTROLLER_DER;
					Serial.println(F("use +/- to adjust pid controller derivative factor"));
					ballController.printCalibrationData();
					break;
				case 'D':
					debug = !debug;
					Serial.print(F("debug is "));
					Serial.println((debug?F("on"):F("off")));
					break;
				case ' ': 
					platform.setTranslation(Point());
					platform.setRotationBy(Rotation());
					platform.moveIt();
					break;
				case 'L': {
					Serial.print(F("set lissajous pattern "));
					platformTrajectory.setActualTrajectory(&ballCircle);
					ballControllerLissajous = !ballControllerLissajous;
					ballControllerActive = !ballControllerLissajous;
					Serial.println((ballControllerLissajous?"on":"off"));
					platformTrajectory.switchBallController(ballControllerLissajous);
					platformTrajectory.setActualTrajectory(&ballCircle);
					ballCircle.setLissajous(TOUCH_SCREEN_X_LEN*5/(7*2),TOUCH_SCREEN_Z_LEN*5/(7*2),1.0/1.0,0,3500);
				}
				break;

				case 'B': {
					Serial.print(F("set ball controller square wave "));
					ballControllerSquareWave = !ballControllerSquareWave;
					if (ballControllerSquareWave  && ballControllerActive)
						 ballControllerActive = false;
					Serial.println((ballControllerSquareWave?"on":"off"));
					Point point1,point2,point3,point4;
					point1.setf(-TOUCH_SCREEN_X_LEN*5/(9*2),0,-TOUCH_SCREEN_Z_LEN*5/(9*2));
					point2.setf(TOUCH_SCREEN_X_LEN*5/(9*2),0,TOUCH_SCREEN_Z_LEN*5/(9*2));
					point3.setf(-TOUCH_SCREEN_X_LEN*5/(9*2),0,TOUCH_SCREEN_Z_LEN*5/(9*2));
					point4.setf(TOUCH_SCREEN_X_LEN*5/(9*2),0,-TOUCH_SCREEN_Z_LEN*5/(9*2));

					platformTrajectory.switchBallController(ballControllerSquareWave);
					platformTrajectory.setActualTrajectory(&ballSquareWave);
					ballSquareWave.setQuareWave(point1, point2, point3, point4,5000);
					}
					break;
				default:
					break;
			} // switch input char			
		} // if (Serial.available)
	}	
}

