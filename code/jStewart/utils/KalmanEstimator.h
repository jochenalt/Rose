/*
 * KalmanEstimator.h
 *
 * Created: 27.12.2014 13:03:23
 *  Author: JochenAlt
 */ 


#ifndef KALMANESTIMATOR_H_
#define KALMANESTIMATOR_H_

#include "LowPassFilter.h"
#include "FixedPoint.h"

// #define KALMAN_DEBUG

class KalmanEstimator {


	public:
		KalmanEstimator ();
		void init(int16_fp4_t pos_fp4, int16_fp0_t speed_fp0, int16_fp0_t acc_fp0);
		
		int16_fp0_t getSpeed() {
			return speedLowPass_fp.get();
			// return xspeed_fp0;
		}
		
		int16_fp4_t getPos() {
			return xpos_fp4;
		}

		void addSample(int16_fp4_t pos_fp4, int16_fp0_t acc_fp0, int16_fp4_t posPrecision_fp4);

	private:	
		void processPositionKalman(int16_fp4_t pos_fp4, int16_fp4_t speed_fp0, int16_fp0_t acc_fp0, int16_fp4_t posPrecision_fp4);	

		// Kalman gain is 1x3 matrix of position/velocity/acc
		int16_fp10_t k00_fp10;
		int16_fp10_t k01_fp10;
		int16_fp10_t k02_fp09;
		
		// error covariance is 3x3 matrix 
		int16_fp4_t p00_fp8;
		int16_fp4_t p10_fp4;
		int16_fp4_t p20_fp3;
		int16_fp4_t p11_fp1;
		int16_fp4_t p21_fpm1;
		int16_fpm4_t p22_fpm4;

		// determinant of error covariance matrix
		int16_fp14_t s00_fp14;

		int16_fp4_t xpos_fp4;
		int16_fp0_t xspeed_fp0;
		int16_fp0_t xacc_fp0;
		LowPassFixedPoint speedLowPass_fp;

#ifdef KALMAN_DEBUG
		void printKalmanGain() {
			Serial.print("K=(");
			Serial.print(FP2FLOAT(k00_fp10,10),7,5);
			Serial.println(",");
			Serial.print("   ");
			Serial.print(FP2FLOAT(k01_fp10,10),7,5);
			Serial.println(",");
			Serial.print("   ");
			Serial.print(FP2FLOAT(k02_fp09,9),7,5);
			Serial.println(")");
		}

		void printInnovation() {
			Serial.print("S=(");
			Serial.print(FP2FLOAT(s00_fp14,14),7,7);
			Serial.println(")");
		}
		
		void printErrorCovariance() {
			Serial.print("P(");
			Serial.print(FP2FLOAT(p00_fp8,8),6,5);
			Serial.print(",");
			Serial.print(FP2FLOAT(p10_fp4,4),6,5);
			Serial.print(",");
			Serial.print(FP2FLOAT(p20_fp3,3),6,5);
			Serial.println();
			Serial.print("                ");
			Serial.print(FP2FLOAT(p11_fp1,1),6,5);
			Serial.print(",");
			Serial.print(((float)(p21_fpm1)*(1<<4),0),6,5);
			Serial.println();
			Serial.print("                              ");
			Serial.print(((float)p22_fpm4)*(1<<4),6,5);
			Serial.println(")");
		}
#endif
};


#endif /* KALMANESTIMATOR_H_ */