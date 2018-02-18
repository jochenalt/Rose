/*
 * BallController.cpp
 *
 * Created: 07.12.2014 18:47:56
 *  Author: JochenAlt
 */ 


#include "Arduino.h"
#include "BallController.h"
#include "ServoLegs.h"
#include "FixedPoint.h"
#include "StewartMemory.h"

void ControllerPane::calibrate() {
	// filter variables have been changed
	alphaLowPass_fp8.init(0.03,LOOP_TIME);
}

void ControllerPane::setup() {
	init();
	errorIntegral_fp14 = 0;
}

void ControllerPane::init() {
	calibrate();
	alphaLowPass_fp8.set(0);
	lastErrorPos_fp4 = 0;
	
	toBePos_fp4 = 0;
	toBeSpeed_fp0 = 0;
}

void ControllerPane::set(int16_fp4_t pToBePos_fp4, int16_fp0_t pToBeSpeed_fp0) {
	toBeSpeed_fp0 = pToBeSpeed_fp0;
	toBePos_fp4  = pToBePos_fp4;
}


int16_fp4_t ControllerPane::computeControlAngle(uint8_t axisIdx,  int16_fp4_t pos_fp4, int16_fp0_t speed_fp0, int16_fp0_t acc_fp0) {
	// create a smooth path from current position to the next to-be position, 
	// so patch the to be position with a first order system
	// f = 20/(20 + delta-position) 
	int16_t PathFilter_fp10  = FP32(150,14) / (FP32(150,4) + abs(pos_fp4-toBePos_fp4)); // always positive
	int16_fp4_t errorPos_fp4 = mul16s_rsh(pos_fp4-toBePos_fp4,PathFilter_fp10,10) ;
	int16_fp0_t errorSpeed_fp0 = mul16s_rsh(speed_fp0-toBeSpeed_fp0,PathFilter_fp10,10);

	// compute sum of all errors, used for integrative part of controller to minimize all deviations
	// corrects a systematic error, i.e. by a non horizontal plane"
	errorIntegral_fp14 += mul16s_rsh(LOOP_TIME*(1<<14), errorPos_fp4,4);
	
	// PID Controller works separate for x and y pane
	// outcome is correction factor of position which is interpreted as corrective force
	int16_fp4_t corrForce_fp4 =	mul16s_rsh(memory.persistentMem.ballControllerConfig.propWeight_fp14,errorPos_fp4,14) +
								mul16s_rsh(memory.persistentMem.ballControllerConfig.integrativeWeight_fp14,i32_rsh(errorIntegral_fp14,8),14)+
								mul16s_rsh(memory.persistentMem.ballControllerConfig.derivativeWeight_fp14,errorSpeed_fp0,10);


	// filter final result and limit to -/+ 15°
	int16_fp4_t alpha_fp4 = alphaLowPass_fp8.get(constrain(corrForce_fp4,FP(-15,4),FP(15,4)));
#ifdef BALL_CONTROLLER_DEBUG
	if (debug) {
		if (axisIdx == X_INDEX)
			Serial.print(" X=");
		else
			Serial.print(" Z=");

		Serial.print("(");
		Serial.print(FP2FLOAT(pos_fp4,4),1,3);
		Serial.print(",");
		Serial.print(speed_fp0,0,3);
		Serial.print(",");
		Serial.print(acc_fp0,0,4);
		Serial.print(")");

		Serial.print("err=(");
		Serial.print(FP2FLOAT(errorPos_fp4,4),0,3);
		Serial.print(",");
		Serial.print(FP2FLOAT(errorIntegral_fp12,12),0,4);
		Serial.print(",");
		Serial.print(errorSpeed_fp0,0,4);

		Serial.print(")=");
		Serial.print(FP2FLOAT(corrForce_fp4,4),1,2);
		Serial.print("°");
	}
#endif
	// store some values
	lastErrorPos_fp4 = errorPos_fp4;
	return alpha_fp4;
}

void BallController::setBallPosition(Point tp, int16_fp0_t speedX_fp0 /* [mm/s] */, int16_fp0_t speedZ_fp0 /* [mm/s] */) {
	x.set(tp.x_fp4,speedX_fp0);
	z.set(tp.z_fp4,speedZ_fp0);
}

void BallController::setDefaults() {
	// small ball
	memory.persistentMem.ballControllerConfig.propWeight_fp14        = FLOAT2FP16(0.1000,14);
	memory.persistentMem.ballControllerConfig.integrativeWeight_fp14 = FLOAT2FP16(0.0060,14);
	memory.persistentMem.ballControllerConfig.derivativeWeight_fp14  = FLOAT2FP16(0.0355,14);
}
		
				
void BallController::setup() {
	x.setup();
	z.setup();
	loopTime.setDueTime(LOOP_TIME_MS);
	lastTimeTouchDetected = 0;
	yPos_fp4 = 0;

}

void BallController::calibrate() {
	// assume that variables have been stored outside
	memory.delayedSave(5000);

	x.calibrate();
	z.calibrate();
}

void BallController::printCalibrationData() {
	Serial.print(F("ball position PID Controller =("));
	Serial.print(FP2FLOAT(memory.persistentMem.ballControllerConfig.propWeight_fp14,14),4,3);
	Serial.print(",");
	Serial.print(FP2FLOAT(memory.persistentMem.ballControllerConfig.integrativeWeight_fp14,14),4,3);
	Serial.print(",");
	Serial.print(FP2FLOAT(memory.persistentMem.ballControllerConfig.derivativeWeight_fp14,14),4,3);
	Serial.println(")");
}

boolean BallController::touchPointPlausible(TouchPoint rawTouchPoint, int16_t posX_fp4, int16_t speedX_fp0, int16_t accX_fp0, int16_t posZ_fp4, int16_t speedZ_fp0, int16_t accZ_fp0, uint32_t valueTimeStamp_us) {
	uint16_t dT_fp15 =i32_rsh(micros() - valueTimeStamp_us,5); // approx. us are 2^20, that counts as 1000000
	
	int16_t estimationX_fp4 = posX_fp4 + mul16s_rsh(speedX_fp0 , dT_fp15,15);
	int16_t estimationZ_fp4 = posZ_fp4 + mul16s_rsh(speedZ_fp0 , dT_fp15,15);

	int32_t distanceSquare_fp0 = mul16s_rsh((rawTouchPoint.x_fp4-estimationX_fp4), (rawTouchPoint.x_fp4-estimationX_fp4),8) + mul16s_rsh((rawTouchPoint.z_fp4-estimationZ_fp4),(rawTouchPoint.z_fp4-estimationZ_fp4),8);
	const int16_t maxSpeedEstimation_fp0 = 2000;
	int32_t max_Squarefp0 = mul16s_rsh(dT_fp15,maxSpeedEstimation_fp0,15)*mul16s_rsh(dT_fp15,maxSpeedEstimation_fp0,15);

#ifdef BALL_CONTROLLER_DEBUG
	if (debug && (distanceSquare_fp0 >= max_Squarefp0))	{
		TouchPoint estimation;
		estimation.x_fp4 = posX_fp4;
		estimation.z_fp4 = posZ_fp4;
		rawTouchPoint.print("implausible tp");
		Serial.print("pos=(");
		Serial.print(posX_fp4>>4);
		Serial.print(",");
		Serial.print(posZ_fp4>>4);
		Serial.print(") speed=(");
		Serial.print(speedX_fp0);
		Serial.print(",");
		Serial.print(speedZ_fp0);
		Serial.print(")");
		
		Serial.print(" dT=");
		Serial.print(uint32_t(1000000UL*FP2FLOAT(dT_fp15,15)));

		Serial.print("us dist=");
		Serial.print(sqrt_i16(distanceSquare_fp0));
		Serial.print(" max=");
		Serial.print(sqrt_i16(max_Squarefp0));
		Serial.print(" tp=(");
		Serial.print(FP2FLOAT(posX_fp4,4));
		Serial.print(",");
		Serial.print(FP2FLOAT(speedX_fp0,0));
		Serial.println(")");
	}
#endif
	
	return (distanceSquare_fp0 < max_Squarefp0);
}


boolean BallController::loop() {
	static uint16_t validSamples = 0;
	static uint16_t totalSamples = 0;
	static boolean newSample = false;
	static uint32_t measurements = 0;
	static int16_fp4_t posX_fp4 = 0;
	static int16_fp4_t posZ_fp4 = 0;
	static int16_fp0_t speedX_fp0 = 0;
	static int16_fp0_t speedZ_fp0 = 0;
	static int16_fp0_t accX_fp0 = 0;
	static int16_fp0_t accZ_fp0 = 0;
	int16_t precisionX = 0;
	int16_t precisionZ = 0;
	static uint32_t valueTimeMicros = 0;

	TouchPoint rawTouchPoint;
	bool touchDetected = false; // true if ball is touching the plate or time is not right
	static boolean ballIsOn = false;
	
	static uint32_t kalmanSum = 0;
	static uint16_t kalmanCount = 0;
	static uint32_t touchSum = 0;

	uint16_t deltaMS;
	uint32_t now = millis();
	static LowPassFixedPoint yPositionLowPass (0.2,LOOP_TIME);
	if (loopTime.isDue_ms(LOOP_TIME_MS,deltaMS) ) {
		uint32_t loopStartFull = micros();
		do {
			uint32_t loopStart = micros();
			totalSamples++;
			boolean valid = touchScreen.sample(rawTouchPoint);

			if (valid) {
				touchSum += (micros()-loopStart);
				measurements += rawTouchPoint.noOfSamplesX;
				measurements += rawTouchPoint.noOfSamplesZ;

				ballIsOn = true;
				yPos_fp4 = yPositionLowPass.addSample(FP(10,4));

				validSamples++;
				
				if (lastTimeTouchDetected == 0) {
					Serial.println("touchpoint!");
					x.init();
					z.init();
					lastTimeTouchDetected = now;
					
					stateEstimatorX.init(rawTouchPoint.x_fp4 >> 4,0,0);
					stateEstimatorZ.init(rawTouchPoint.z_fp4 >> 4,0,0);	
					
					// right after touch point we need to tell the kalman filter to accept this first position as stable one
					// otherwise it would assume origin as set point and slowly accept this position
					for (int i = 0;i<200/LOOP_TIME_MS;i++) {
						stateEstimatorX.addSample(rawTouchPoint.x_fp4,0,FP(rawTouchPoint.precisionX,4));
						stateEstimatorZ.addSample(rawTouchPoint.z_fp4,0,FP(rawTouchPoint.precisionZ,4));						
					}

					posX_fp4 = rawTouchPoint.x_fp4;
					posZ_fp4 = rawTouchPoint.z_fp4;
					speedX_fp0 = 0;
					speedZ_fp0 = 0;
					accX_fp0 = 0;
					accZ_fp0 = 0;

					yPositionLowPass.set(0);
					valueTimeMicros = 0;
				} else {
					// plausibility check, sometimes the touchscreen yields rubbish, so check if point is within expected range
					bool plausible = true;

					if ((valueTimeMicros > 0)) {
						uint8_t tries = 6; // in one loop we have time for 4 tries
						plausible = touchPointPlausible(rawTouchPoint, posX_fp4, speedX_fp0, accX_fp0, posZ_fp4, speedZ_fp0, accZ_fp0,valueTimeMicros);
						while (!plausible && (tries >0)) {
							setError();
							valid = touchScreen.sample(rawTouchPoint);	
							tries--;
							if (valid) 
								plausible = touchPointPlausible(rawTouchPoint, posX_fp4, speedX_fp0, accX_fp0, posZ_fp4, speedZ_fp0, accZ_fp0,valueTimeMicros);
						}
					}
					if (debug) {
						Serial.print("t=");
						Serial.print(deltaMS);
						if (!plausible)
							Serial.print(F(" INV "));
						else
							Serial.print(F("     "));

						rawTouchPoint.print("tp");
					}
					if (plausible) {
						accX_fp0 = platform.getAccX();
						accZ_fp0 = platform.getAccX();
						posX_fp4 = stateEstimatorX.getPos();
						posZ_fp4 = stateEstimatorZ.getPos();
						speedX_fp0 = stateEstimatorX.getSpeed();
						speedZ_fp0 = stateEstimatorZ.getSpeed();

						stateEstimatorX.addSample(rawTouchPoint.x_fp4,accX_fp0,FP(rawTouchPoint.precisionX,4));
						stateEstimatorZ.addSample(rawTouchPoint.z_fp4,accZ_fp0,FP(rawTouchPoint.precisionZ,4));
						kalmanSum += (micros()-loopStart);
						kalmanCount++;

						posX_fp4 = stateEstimatorX.getPos();
						posZ_fp4 = stateEstimatorZ.getPos();
						speedX_fp0 = stateEstimatorX.getSpeed();
						speedZ_fp0 = stateEstimatorZ.getSpeed();

						valueTimeMicros = micros();
												
						newSample = true;
						touchDetected = true;
						lastTimeTouchDetected = now;
					}
				}
			} 
			// after 500ms of no touchpoint assume ball is gone
			if ((lastTimeTouchDetected > 0) && (lastTimeTouchDetected < (now-500))) {
				Serial.println(F("No touch detected since 200ms."));
				lastTimeTouchDetected = 0; // apparently no touch 
				touchDetected  = false;
				newSample = false;
				ballIsOn = false;
				yPositionLowPass.set(0);
			}
		}
		while (!newSample && touchDetected);

		static uint32_t controlTimeSum = 0;
		static uint16_t controlTimes  = 0;
		
		if (touchDetected && newSample) {
			newSample = false;
	
			int16_fp4_t angleX_fp4 = x.computeControlAngle(X_INDEX,posX_fp4, speedX_fp0, accX_fp0);
			int16_fp4_t angleZ_fp4 = z.computeControlAngle(Z_INDEX,posZ_fp4, speedZ_fp0, accZ_fp0);
			
			// compute movement out of the compensating angle
			computePlattformMovement(posX_fp4,posZ_fp4, angleX_fp4, angleZ_fp4);	
			controlTimeSum += (micros() -loopStartFull);
			controlTimes ++;
		}

		// show some stats like freqency and ratio of valid touchscreen samples
		printStatistics(validSamples,totalSamples, measurements, kalmanSum, kalmanCount, touchSum, controlTimeSum, controlTimes);
	}
	
	return ballIsOn;
}

void BallController::printStatistics( uint16_t &validSamples, uint16_t &totalSamples, uint32_t &measurements, uint32_t &kalmanTime,uint16_t& kalmanCount, uint32_t &touchTime, uint32_t& controlTime, uint16_t& controlTimeCount) {
	static uint16_t loopCount = 0;
	static uint32_t t = millis();
	
	// display a stats frequency message every 5 seconds
	if (loopCount >= (int16_t)(LOOP_FREQUENCY*5)) {
		uint32_t now = millis();
		Serial.print(F("Stats f="));
		Serial.print((1000.0*loopCount)/(millis()-t));
		Serial.print(F("Hz valid="));
		Serial.print((100.0*validSamples)/(totalSamples));
		Serial.print(F("% #measurements/sample="));
		Serial.print((((float)measurements)/totalSamples)/2.0,1,2);
		Serial.print(" perf touch=");
		Serial.print(((float)touchTime)/totalSamples, 1,4);
		Serial.print(F("us perf kalman="));
		Serial.print(((float)kalmanTime)/kalmanCount,1,4);
		Serial.print(" us perf control=");
		Serial.print(((float)controlTime)/controlTimeCount, 1,4);
		Serial.println("us");

		totalSamples = 0;
		validSamples = 0;
		kalmanTime = 0;
		kalmanCount = 0;
		touchTime = 0;
		measurements = 0;
		loopCount = 0;
		t = now;
		controlTimeCount = 0;
		controlTime = 0;
	}
	loopCount++;
}



void BallController::computePlattformMovement(int16_fp4_t posX_fp4, int16_fp4_t posZ_fp4, int16_fp4_t angleX_fp4, int16_fp4_t angleZ_fp4  /* [°] */) {
	// the compensating acceleration is produced by a translation and a rotation (just to make it
	// look good, otherwise it would look like those thousands of examples on youtube)
	// The platform does a translation under the ball against the direction of the difference between
	// actual and to-be position. The rotation is done by rotating around the top of the ball.
	// In the end, this looks like the
	// a correction of the x axis is done by rotation around the z-axis. Correction of z-axis is done by rotation around X
	Rotation rotation;
	rotation.set(angleZ_fp4,0,angleX_fp4);
	Point touchpoint;
	touchpoint.set(x.toBePos_fp4,FP(BALL_RADIUS,4), z.toBePos_fp4);

	// compute the translation with a filter in an arbitrary manner.
	// We choose a translation depending on the deviation of the position, but dampened 
#define MAX_TRANSLATION	40.0
	int16_t diffPosX_fp4 = mul16s(posX_fp4-x.toBePos_fp4,FP32(30,4))/(FP32(30,4) + abs(posX_fp4-x.toBePos_fp4));
	int16_t diffPosZ_fp4 = mul16s(posZ_fp4-z.toBePos_fp4,FP32(30,4))/(FP32(30,4) + abs(posZ_fp4-z.toBePos_fp4));

	int16_t transX_fp4 = mul16s_rsh(diffPosX_fp4,(MAX_TRANSLATION*(1<<10)/(TOUCH_SCREEN_X_LEN/2)),10);
	int16_t transZ_fp4 = mul16s_rsh(diffPosZ_fp4,(MAX_TRANSLATION*(1<<10)/(TOUCH_SCREEN_Z_LEN/2)),10);

	static int16_t lastTransX_fp4 = 0;
	static int16_t lastTransZ_fp4 = 0;
	
	// out of that translation, compute the inducing acceleration produced by the translation
	// and substract that acceleration from the acceleration induced by the rotation, i.e. the angle
	// derived from s=0,5*a*dT^2 => a = 2s/(dT^2)
	angleX_fp4 -= mul16s_rsh(transX_fp4-lastTransX_fp4,((int16_fp4_t)degrees((1<<4)*(2.0/(9810*LOOP_TIME*LOOP_TIME)))),4);
	angleZ_fp4 -= mul16s_rsh(transZ_fp4-lastTransZ_fp4,((int16_fp4_t)degrees((1<<4)*(2.0/(9810*LOOP_TIME*LOOP_TIME)))),4);

	// store translation
	lastTransX_fp4 = transX_fp4;
	lastTransZ_fp4 = transZ_fp4;

	// the rotation is not around the origin, but the touchpoint of the platform
	Point nullPoint(0,0,0);
	nullPoint.set(transX_fp4,
				  FP(BALL_RADIUS,4) + yPos_fp4,
				  transZ_fp4);
	platform.setRotationByPoint(touchpoint,rotation,nullPoint);
	
	// actually move the servos
	platform.moveIt();
	
	if (debug)
		Serial.println("end");
}
