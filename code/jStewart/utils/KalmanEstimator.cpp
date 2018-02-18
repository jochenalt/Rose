/*
 * KalmanEstimator.cpp
 *
 * Created: 27.12.2014 13:03:34
 *  Author: JochenAlt
 */ 



#include "Arduino.h"
#include "KalmanEstimator.h"
#include "../setup.h"
#include "LowPassFilter.h"

#define SIGMA_POS 7.0 // [mm] // noise of position measurement
#define SIGMA_ACC (radians(0.5)*9810.0) // [mm/s^2]

#define R00 (SIGMA_POS*SIGMA_POS)
#define R11 (SIGMA_ACC*SIGMA_ACC)
#define dT LOOP_TIME

#define Q00 (0.25*dT*dT*dT*dT)
#define Q10 (0.5*dT*dT*dT)
#define Q20 (0.5*dT*dT)
#define Q11 (dT*dT)
#define Q21 (dT)


KalmanEstimator::KalmanEstimator () {
	k00_fp10 = FLOAT2FP16(0.2,10); 
	k01_fp10 = FLOAT2FP16(2.0,10);
	k02_fp09 = FLOAT2FP16(12.0,9);
	s00_fp14 = 0;
			
	p00_fp8 = FP16(10,8);	p10_fp4 = FP16(130,4);  p20_fp3 = FP16(600,3);
							p11_fp1 = FP16(3000,1); p21_fpm1 = 15000>>1;
													p22_fpm4 = 150000>>4;

	speedLowPass_fp.init(0.05,LOOP_TIME);
}

void KalmanEstimator::init(int16_fp4_t pos_fp4, int16_fp0_t speed_fp0, int16_fp0_t acc_fp0) {
	xpos_fp4 = pos_fp4;
	xspeed_fp0 = speed_fp0;
	xacc_fp0 = acc_fp0;

}
		

void KalmanEstimator::processPositionKalman(int16_fp4_t pPosX_fp4, int16_fp4_t pSpeedX_fp0,int16_fp0_t pAccX_fp0,  int16_fp4_t pPosPrecision_fp4) {
	
	// constants that are often needed, dt and 0,5*dt*dt
	static int16_fp14_t dT_fp14 = FLOAT2FP16(dT,14);
	static int16_fp20_t dTdT_Half_fp20 = mul16s_rsh(FLOAT2FP16(dT,14),FLOAT2FP16(dT*0.5,14),8);

	// predict the new state on base of current speed and acceleration
	// estimation = Ax (u as matrix for a move is not relevant here)
	xpos_fp4   += mul16s_rsh(xspeed_fp0,FLOAT2FP16(dT,14),10) + mul16s_rsh (xacc_fp0,dTdT_Half_fp20,16);
	xspeed_fp0 += mul16s_rsh(xacc_fp0,FLOAT2FP16(dT,14),14);	
	// acceleration remains the same in prediction
	
	// compute the error covariance
	// P = P + AP(Trans(A)) + Q
	p00_fp8 += mul16s_rsh(dT_fp14,p10_fp4,10);
	p00_fp8 += mul16s_rsh(dTdT_Half_fp20,p20_fp3,15); // irrelevant
	p10_fp4 += mul16s_rsh(dT_fp14,p11_fp1,11);
	p10_fp4 += mul16s_rsh(dTdT_Half_fp20,p21_fpm1,19);
	int16_fp4_t dtp21_fp4 = mul16s_rsh(dT_fp14,p21_fpm1,9);
	int16_fp4_t dtp22_fpm1 = mul16s_rsh(dT_fp14,p22_fpm4,11);
	FPCHECK16(dtp21_fp4,"dtp21_fp4");
	FPCHECK16(dtp22_fpm1,"dtp22_fpm1");
	FPCHECK16(p00_fp8,"p00-0");
	FPCHECK16(p10_fp4,"p10-0");


	
	p20_fp3 += dtp21_fp4>>1;
	p20_fp3 += mul16s_rsh(dTdT_Half_fp20,dtp22_fpm1,16);
	p11_fp1 += dtp21_fp4>>3;
	p21_fpm1 += dtp22_fpm1;
	FPCHECK16(p20_fp3,"p20-1");
	FPCHECK16(p11_fp1,"p11-1");
	FPCHECK16(p21_fpm1,"p21-1");
	
	p00_fp8 += mul16s_rsh(dT_fp14,p10_fp4,10);
	p00_fp8 += mul16s_rsh(dTdT_Half_fp20,p20_fp3,15);
	p10_fp4 += mul16s_rsh(dT_fp14,p20_fp3,13);
	p11_fp1 += mul16s_rsh(dT_fp14,p21_fpm1,12);
	FPCHECK16(p00_fp8,"p00-2");
	FPCHECK16(p10_fp4,"p10-2");
	FPCHECK16(p11_fp1,"p11-2");
	
	// +Q from equation above
	p00_fp8 += FLOAT2FP16(R11*Q00,8);
	p10_fp4 += FLOAT2FP16(R11*Q10,4);
	p20_fp3 += FLOAT2FP16(R11*Q20,3);
	p11_fp1 += FLOAT2FP16(R11*Q11,1);
	p21_fpm1 += FLOAT2FP32(R11*Q21,0) >>1;
	p22_fpm4 += i32_rsh((int32_t)R11,4);
	FPCHECK16(p00_fp8,"p00-3");
	FPCHECK16(p10_fp4,"p10-3");
	FPCHECK16(p20_fp3,"p20-3");
	FPCHECK16(p11_fp1,"p11-3");
	FPCHECK16(p21_fpm1,"p21-3");
	FPCHECK16(p22_fpm4,"p22-3");

	
	// Compute Kalman Gain  K = P*Trans(H)*(H*P*Trans(H)+R)^-1
	// start with S=H*P*Trans(H)+R
	// now compute ^-1
	s00_fp14 = FP32(1,22)/(p00_fp8 + FLOAT2FP16(R00,8)+(pPosPrecision_fp4<<4));
	FPCHECK16(s00_fp14,"s00");
	
	// final computation of Kalman gain K = P*Trans(H) * S
	k00_fp10 = mul16s_rsh(p00_fp8,s00_fp14,12);
	k01_fp10 = mul16s_rsh(p10_fp4,s00_fp14,8);
	k02_fp09 = mul16s_rsh(p20_fp3,s00_fp14,8);
	FPCHECK16(k00_fp10,"K00");
	FPCHECK16(k01_fp10,"K01");
	FPCHECK16(k02_fp09,"K02");
	
	// update prediction with measurement
	// x += K*(measurement-H*estimation), y = measurement-H*estimation
	int16_fp4_t y_fp4 = pPosX_fp4-xpos_fp4;
	FPCHECK16(y_fp4,"y-3");
	
	xpos_fp4	+= mul16s_rsh(k00_fp10, y_fp4,10); // x.pos = x.pos + k00*zH0 =  x.pos + k00*(pos-x.pos) = x.pos*(1-k00) + K00*pos;
	xspeed_fp0  += mul16s_rsh(k01_fp10, y_fp4,14);
	xacc_fp0	+= mul16s_rsh(k02_fp09, y_fp4,13);
#ifdef KALMAN_DEBUG
	static int16_t i = 0;
	i++;
	if (debug && (i % 500) == 0) {
		Serial.println();
		Serial.print("kalman(g=(");
		Serial.print((int16_t)(100*(FP2FLOAT(k00_fp10,10))));
		Serial.print("%/");
		Serial.print((int16_t)(100*(FP2FLOAT(k01_fp10,10))));
		Serial.print("%/");
		Serial.print((int16_t)(100*(FP2FLOAT(k02_fp09,9))));
		Serial.print("%)");

		Serial.print("s=[");
		Serial.print((int)(FP2FLOAT(xpos_fp4,4)),0,3);
		Serial.print(",");
		Serial.print((int)(xspeed_fp0),0,3);
		Serial.print(",");
		Serial.print((int)(xacc_fp0),0,4);
		Serial.print("])");
		Serial.println();
		
		
	}
#endif
	// update the error covariance matrix p
	p00_fp8 -= mul16s_rsh(k00_fp10,p00_fp8,10);
	p10_fp4 -= mul16s_rsh(k00_fp10,p10_fp4,10);
	p20_fp3 -= mul16s_rsh(k00_fp10,p20_fp3,10);
	
	p11_fp1 -= mul16s_rsh(k01_fp10,p10_fp4,13);
	p21_fpm1 -= mul16s_rsh(k01_fp10,p20_fp3,14);
	
	p22_fpm4 -= mul16s_rsh(k02_fp09,p20_fp3,16);
	
	FPCHECK16(p00_fp8,"p00-4");
	FPCHECK16(p10_fp4,"P01-4");
	FPCHECK16(p20_fp3,"P20-4");
	FPCHECK16(p11_fp1,"P11-4");
	FPCHECK16(p21_fpm1,"P21-4");
	FPCHECK16(p22_fpm4,"P22-4");
	
#ifdef KALMAN_DEBUG
	static int16_t count = 0;
	count++;
	if (debug && ((count % 500) == 0)) {
		printKalmanGain();
		printErrorCovariance();
		printInnovation();
	}
#endif
}
	
void KalmanEstimator::addSample(int16_fp4_t pos_fp4,  int16_fp0_t acc_fp0, int16_fp4_t posPrecision_fp4) {
	
	// the kalman estimator works with measured position and acceleration (derived from angles)
	// speed is computed out of a low pass filter of derived positions and acceleration is estimated (although we do not use the corrected acceleration)
	int16_fp4_t prevPos_fp4 = xpos_fp4;
		
	// apply kalman estimator based on measured position.
	// Acceleration (given by the platform's angle) and speed(computed by filter) is used for kalman-estimation
	processPositionKalman(pos_fp4, speedLowPass_fp.get(), acc_fp0, posPrecision_fp4);

	// compute the beforehand mentioned low-pass
	speedLowPass_fp.addSample(mul16s_rsh(xpos_fp4-prevPos_fp4,LOOP_FREQUENCY,4));
}