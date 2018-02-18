/*
 * BallController.h
 *
 * Created: 07.12.2014 18:48:10
 *  Author: JochenAlt
 */ 


#ifndef BALLCONTROLLER_H_
#define BALLCONTROLLER_H_


#include "Arduino.h"
#include "setup.h"
#include "TouchScreen.h"
#include "InverseKinematics.h"
#include "FastMath.h"
#include "LowPassFilter.h"
#include "KalmanEstimator.h"
#include "TimePassedBy.h"

#undef BALL_CONTROLLER_DEBUG

class BallControllerConfig {
	public:
		// weights of ball PID controller
		int16_fp4_t propWeight_fp14;
		int16_fp4_t integrativeWeight_fp14;
		int16_fp4_t derivativeWeight_fp14;
};


class ControllerPane {
	public:
		ControllerPane () {
		}
		
		void calibrate();
		void setup();
		void init();
		int16_fp4_t computeControlAngle(uint8_t axisIdx,  int16_fp4_t pos_fp4, int16_t speed_, int16_t acc_);
		void set(int16_fp4_t pToBePos_fp4, int16_fp0_t pToBeSpeed_fp0);


		int16_fp4_t toBePos_fp4;
		int16_fp0_t toBeSpeed_fp0;
	private:
		int16_fp4_t lastErrorPos_fp4;
		int32_fp14_t errorIntegral_fp14;
		
		LowPassFixedPoint alphaLowPass_fp8;
};


class BallController {
	public:
		BallController() {
		}
		
		static void setDefaults();		
		void setup();
		void setBallPosition(Point tp, int16_t speedX /* [mm/s] */, int16_t speedZ /* [mm/s] */);
		void calibrate();
		void printCalibrationData();
		boolean loop();

		void computePlattformMovement(int16_fp4_t posX_fp4, int16_fp4_t posZ_fp4, int16_fp4_t angleX_fp4, int16_fp4_t angleZ_fp4  /* [°] */);
		void printStatistics( uint16_t &validSamples, uint16_t &totalSamples, uint32_t &measurements, uint32_t &kalmanTime,uint16_t& kalmanCount, uint32_t &touchTime, uint32_t& controlTime, uint16_t& numberOfControls);
		boolean touchPointPlausible(TouchPoint rawTouchPoint, int16_t posX_fp4, int16_t speedX_fp0, int16_t accX_fp0, int16_t posZ_fp4, int16_t speedZ_fp0, int16_t accZ_fp0, uint32_t valueMicros);

		ControllerPane x;
		ControllerPane z;
		uint32_t lastTimeTouchDetected;

		TimePassedBy loopTime;
		
		KalmanEstimator stateEstimatorX;
		KalmanEstimator stateEstimatorZ;
		uint16_t yPos_fp4;
};


#endif /* BALLCONTROLLER_H_ */