/*
 * FixedPoint.cpp
 *
 * Created: 31.12.2014 10:35:39
 *  Author: JochenAlt
 */ 


#include "Arduino.h"
#include "FixedPoint.h"

// sin values in fixed point in steps of 4 degrees, slightly above 90°
// last value is a bit faked to hit 90° exactly (presumably not really required, but nicer)
// error is below 0.1%
int16_fp14_t sin_FP14_base[] { 
	0,FLOAT2FP16(0.069756473744,14),FLOAT2FP16(0.139173100960,14),
	  FLOAT2FP16(0.207911690818,14),FLOAT2FP16(0.275637355817,14),FLOAT2FP16(0.342020143326,14),
	  FLOAT2FP16(0.406736643076,14),FLOAT2FP16(0.469471562786,14),FLOAT2FP16(0.529919264233,14),
	  FLOAT2FP16(0.587785252292,14),FLOAT2FP16(0.642787609687,14),FLOAT2FP16(0.694658370459,14),
	  FLOAT2FP16(0.743144825477,14),FLOAT2FP16(0.788010753607,14),FLOAT2FP16(0.829037572555,14),
	  FLOAT2FP16(0.866025403784,14),FLOAT2FP16(0.898794046299,14),FLOAT2FP16(0.927183854567,14),
	  FLOAT2FP16(0.951056516295,14),FLOAT2FP16(0.970295726276,14),FLOAT2FP16(0.984807753012,14), 
	  FLOAT2FP16(0.994521895368,14),FLOAT2FP16(0.999390827019,14),FLOAT2FP16(1.000610351563,14) // last value is faked to hit 90° exactly
};

#define SUPPORT_POINTS 17
const int16_fp14_t asinRadSupportArray_fp14[SUPPORT_POINTS] = { 0,
	FLOAT2FP16(asin(1*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(2*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(3*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(4*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(5*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(6*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(7*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(8*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(9*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(10*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(11*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(12*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(13*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(14*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(asin(15*1.0/(SUPPORT_POINTS-1)),14),
	FLOAT2FP16(PI/2,14)
};



int16_fp14_t arcsinusRadFast_fp14(int16_fp14_t x_fp14) {
	float x = FP2FLOAT(x_fp14,14);
	if (x_fp14<0.0)
		return -arcsinusRadFast_fp14(-x_fp14);
	
	int16_fp14_t xByWidth_fp10 = x_fp14;
	int leftIndex = x_fp14>>10; // index of left support point
	int16_fp14_t ratio_fp10 = xByWidth_fp10-FP16(leftIndex,10); // ratio of x within bot/top interval. x=bot-> t = 0.0, x=top->t = 1.0
	int16_fp16_t leftValue_fp14 = asinRadSupportArray_fp14[leftIndex];
	int16_fp16_t rightValue_fp14 = asinRadSupportArray_fp14[leftIndex+1];
	int16_fp14_t result_fp14 = leftValue_fp14 + mul16s_rsh(rightValue_fp14 - leftValue_fp14,ratio_fp10,10);
	/*
		Serial.print("x=");
		Serial.print(x);
		Serial.print(" r=");
		Serial.print(FP2FLOAT(result_fp14,14),4,4);
		Serial.print(" R=");
		Serial.print(asin(x),4,4);
		Serial.print(" p=");
		Serial.print((asin(x)/FP2FLOAT(result_fp14,14)-1.0)*100,4,4);

	
		Serial.print("% leftIdx");
		Serial.print(leftIndex);
		Serial.print(" x[left]");
		Serial.print(FP2FLOAT(leftValue_fp14,14),4,4);
		Serial.print(" x[right]");
		Serial.print(FP2FLOAT(rightValue_fp14,14),4,4);
		Serial.print(" ratio_fp10");
		Serial.print(FP2FLOAT(ratio_fp10,10),4,4);

		Serial.println();
		*/
		
	return result_fp14;
}

// compute sinus with fixed point arithmetics by interpolation of a given value table 
// Works from 0°..90° only
int16_fp14_t sinFirstQuadrant_FP6(int16_fp6_t pAngleDeg_FP6) {
	uint8_t			a = pAngleDeg_FP6  >> 8;						// compute number of interpolation interval
	int16_fp14_t	l1_FP14 = sin_FP14_base[a++];					// store left interval value
	int16_fp14_t	l2_FP14 = sin_FP14_base[a];						// store right interval value
	int16_t			x_FP6= pAngleDeg_FP6 - (pAngleDeg_FP6 & 0xFF00);// position within that interval from 0..255, 
																	// so null everything besides 0..255 ( = 0xFF00)
	return l1_FP14 + mul16s_rsh(x_FP6 , l2_FP14-l1_FP14,8);			// interpolation
}

// returns sin of parameter in fixed point arithmentics	
// works from -360°..360°
int16_fp14_t sin_FP6(int16_fp6_t pAngleDeg_FP6) {
	int16_fp4_t lAngleDef_FP6	 = pAngleDeg_FP6;
	while (lAngleDef_FP6 > FP(360,6))
		lAngleDef_FP6 -= FP(360,6);
	while (lAngleDef_FP6 < 0)
		lAngleDef_FP6 += FP(360,6);

	if (lAngleDef_FP6 > FP(270,6))
		return -sinFirstQuadrant_FP6(FP(360,6)-lAngleDef_FP6);
	else if (lAngleDef_FP6 > FP(180,6))
		return -sinFirstQuadrant_FP6(lAngleDef_FP6-FP(180,6));
	else if (lAngleDef_FP6 > FP(90,6))
		return sinFirstQuadrant_FP6(FP(180,6)-lAngleDef_FP6);
	else
		return sinFirstQuadrant_FP6(lAngleDef_FP6);
}

int16_fp14_t cos_FP6(int16_fp6_t pAngleDeg_FP6) {
	return sin_FP6(pAngleDeg_FP6 + FP(90,6));
}

// version that works on base of fp8, which is maxiumum precision for 0..90°
// Works from 0°..90° only. A bit slower than the one above
int16_fp14_t sinFirstQuadrant_FP8(int16_fp8_t pAngleDeg_FP8) {
	uint8_t a = pAngleDeg_FP8 >> 10;						// compute number of interpolation interval
	int16_fp14_t l1_FP14 = sin_FP14_base[a++];				// compute left value
	int16_fp14_t l2_FP14 = sin_FP14_base[a];				// compute right value
	int16_t x_FP8 = pAngleDeg_FP8-(pAngleDeg_FP8 & 0xFC00); // position within that interval from 0..1023, 
															// so null everything besides 0..1023 ( = 0xFC00)
	return l1_FP14 + mul16s_rsh(x_FP8 , l2_FP14-l1_FP14,10); 
}

	
// returns sin of parameter in fixed point arithmentics
// works from -90°..+90°
int16_fp14_t sin_FP8(int16_fp8_t pAngleDeg_FP8) {
	ASSERT((pAngleDeg_FP8<=FP(120,8)) && (pAngleDeg_FP8>=-FP(120,8)),28434);

	int16_fp4_t lAngleDef_FP8	 = pAngleDeg_FP8;
	if (lAngleDef_FP8 >= FP(90,8))
		return sinFirstQuadrant_FP8(FP(90,8)-(lAngleDef_FP8-FP(90,8)));
	else if (lAngleDef_FP8 >= 0)
		return sinFirstQuadrant_FP8(lAngleDef_FP8);
	else if (lAngleDef_FP8 >= -FP(90,8))
		return -sinFirstQuadrant_FP8(-lAngleDef_FP8);
	else 
		return -sinFirstQuadrant_FP8(FP(90,8)+(lAngleDef_FP8 + FP(90,8)));
}


// returns cos in fp14,  works from -90°..90°
int16_fp14_t cos_FP8(int16_fp8_t pAngleDeg_FP8) {
	ASSERT((pAngleDeg_FP8<=FP(120,8)) && (pAngleDeg_FP8>=-FP(120,8)),28437);
	if (pAngleDeg_FP8 >= FP(90,8))
		return -sinFirstQuadrant_FP8(pAngleDeg_FP8-FP(90,8));
	else if (pAngleDeg_FP8 >= 0)
		return sinFirstQuadrant_FP8(FP(90,8)-pAngleDeg_FP8);
	else if (pAngleDeg_FP8 >= -FP(90,8))
		return sinFirstQuadrant_FP8(pAngleDeg_FP8+FP(90,8));
	else 
		return -sinFirstQuadrant_FP8(-pAngleDeg_FP8 -FP(90,8));
}


uint8_t LogTab[16]={ 0,9,17,25,32,39,46,52,58,64,70,75,81,86,91,95 } ;
uint16_t log2Fast_fp4(uint32_t x){
	// compute 100*ld(x) , so Log2(1024) = 1000
	uint16_t v ;
	v=3100 ;
	// shift until a bit in MSB-byte is set,
	// each byte shift reults in log += 800
	while ( (x&0xff000000L )==0 ) {
	x=x<<8 ; v -= 800 ; }
	// shift until most significant bit is set,
	// each bit-shift results in log += 100
	while ( (x&0x80000000L )==0 ) {
	x=x<<1 ; v -= 100 ; }
	// x has now form
	// 1iii ixxx xxxx xxxx xxxx xxxx xxxx xxxx
	// get the next 4 bits =iiii and
	// address table with it
	uint8_t i=(x>>27) & 0xf ;
	v += LogTab[i] ;
	int16_t vs = v;
	vs=mul16s_rsh(vs,(1<<14)/100,10);
	return vs ;
}

// returns a rough approximation of arctan in degree fp10 for small angles (error is 1% at 10°)
// assumes tan (x) = x
int16_fp10_t arctan2_fp10SmallAngle(int z, int xy) {
	return  mul16s_rsh(i32_lsh(xy,10)/z, FLOAT2FP16(180.0/PI,6),6);
}

int16_fp14_t atan_fp8(int16_fp8_t x_fp8) {

	int32_fp16_t x2_fp16 = mul16s(x_fp8,x_fp8);
	int32_fp14_t f_fp14 = i32_lsh(x_fp8,14)/(i32_rsh(x2_fp16,8)+FLOAT2FP32(0.28,8));
	if (x_fp8 > FLOAT2FP32(1.0,8)) 
		return FLOAT2FP32(HALF_PI,14) - f_fp14;

	if (x_fp8 < FLOAT2FP32(-1.0,8))
		return FLOAT2FP32(-HALF_PI,14) - f_fp14;
	
	return i32_lsh(x_fp8,14)/(FLOAT2FP32(1.0,8) + i32_rsh(x2_fp16*FLOAT2FP16(0.28,8),16));
}

int16_fp14_t atan2_fp4(int16_fp4_t y_fp4, int16_fp4_t x_fp4) {
	if (x_fp4 == 0) {
		if (y_fp4>0)
			return FLOAT2FP16(HALF_PI,14);
		if (y_fp4<0)
			return FLOAT2FP16(-HALF_PI,14);
		return 0; // actually this is undefined
	}
	
	int16_fp14_t r_fp14 = atan_fp8(i32_lsh(y_fp4,8)/x_fp4);
	if (x_fp4 > 0)
		return r_fp14;
	
	// x < 0.0
	if (y_fp4>= 0)
		return r_fp14+FLOAT2FP16(PI,14);

	return r_fp14-FLOAT2FP16(PI,14);
}