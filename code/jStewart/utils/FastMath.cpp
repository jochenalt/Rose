#include "Arduino.h"
#include "FastMath.h"
#include "FixedPoint.h"
	
// With 10 points we have a maximum error of 0.37%. Seems to be ok.
#define SUPPORT_POINTS 11
/*
const float sinSupportArray[SUPPORT_POINTS] = { 0,            // 0°
												sin(radians(1*90/(SUPPORT_POINTS-1))),   //  9°
												sin(radians(2*90/(SUPPORT_POINTS-1))),   // 18°
												sin(radians(3*90/(SUPPORT_POINTS-1))),   // 27°
												sin(radians(4*90/(SUPPORT_POINTS-1))),   // 36°
												sin(radians(5*90/(SUPPORT_POINTS-1))),   // 45°
												sin(radians(6*90/(SUPPORT_POINTS-1))),   // 54°
												sin(radians(7*90/(SUPPORT_POINTS-1))),   // 63°
												sin(radians(8*90/(SUPPORT_POINTS-1))),   // 72°
												sin(radians(9*90/(SUPPORT_POINTS-1))),   // 81°
												sin(radians(10*90/(SUPPORT_POINTS-1))),  // 90°
												};  

*/
const float asinSupportArray[SUPPORT_POINTS] = { 0,		
												 asin(1*1.0/(SUPPORT_POINTS-1)), 
												 asin(2*1.0/(SUPPORT_POINTS-1)), 
												 asin(3*1.0/(SUPPORT_POINTS-1)), 
												 asin(4*1.0/(SUPPORT_POINTS-1)), 
												 asin(5*1.0/(SUPPORT_POINTS-1)), 
												 asin(6*1.0/(SUPPORT_POINTS-1)), 
												 asin(7*1.0/(SUPPORT_POINTS-1)), 
												 asin(8*1.0/(SUPPORT_POINTS-1)), 
												 asin(9*1.0/(SUPPORT_POINTS-1)),
												 PI/2
												};



float interpolate (const float supportPoints[], float rezRangePerInterval, float x)
{
	float xByWidth = x*rezRangePerInterval;
	int leftBoundary = int(xByWidth); // index of left support point
	float ratioWithinInterval = xByWidth-leftBoundary; // ratio of x within bot/top interval. x=bot-> t = 0.0, x=top->t = 1.0
	float leftValue = supportPoints[leftBoundary];
	float rightValue = supportPoints[leftBoundary+1];
	return leftValue + ratioWithinInterval * (rightValue - leftValue);	
}

float arcsinusRadFast(float x) {
	if (x<0.0)
		return -arcsinusRadFast(-x);
		
	float result = interpolate(asinSupportArray, (SUPPORT_POINTS-1) / 1.0, x);
	return result ; 
}

extern "C" uint16_t sqrt32_round(uint32_t);
extern "C" uint16_t sqrt32_floor(uint32_t);

uint16_t sqrtFasti32 (uint32_t q){
	return sqrt32_floor(q);
}

uint8_t sqrt_i16 (uint16_t q)
{
	uint8_t res = 0;
	uint8_t mask = 1 << 7;
	
	asm("0:	add  %[res], %[mask]"   "\n"
	"	mul  %[res], %[res]"    "\n"
	"	cp   %A[q], R0"         "\n"
	"	cpc  %B[q], R1"         "\n"
	"	brsh 1f"                "\n"
	"	sub  %[res], %[mask]"   "\n"
	"1:	lsr  %[mask]"           "\n"
	"	brne 0b"                "\n"
	"	clr  __zero_reg__"
	: [res] "+r" (res), [mask] "+r" (mask)
	: [q] "r" (q));
	
	return res;
}


// compute atanfast by approximately as desribed in wikipedia
// max. deviation <0.005 rad
float atanFast(float x) {
	// return FP2FLOAT(atan_fp4(FLOAT2FP16(x,8)),14);
	if (x> 1.0)
		return (PI/2.0) - (x/(x*x+0.28));
	if (x<-1.0)
		return (-PI/2.0) - (x/(x*x+0.28));
		
	return (x/(1.0+ 0.28*x*x));
}


float atan2Fast(float y, float x) {
	return FP2FLOAT(atan2_fp4(FLOAT2FP16(y,4), FLOAT2FP16(x,4)),14);
	if (x > 0.0)
		return atanFast(y/x);
		
	if (x == 0.0) {
		if (y>0.0)
			return (PI/2.0);
		if (y<0.0)
			return (-PI/2.0);
		return 0.0; // actually this is undefined
	} 
	
	// x < 0.0
	if (y>= 0.0) 
		return atanFast(y/x)+PI;

	return atanFast(y/x)-PI;
}




int16_t average(int m, int16_t a[]) {
	int32_t sum=0;
	for(uint8_t i=0; i<m; i++)
	sum +=a[i];
	return(sum/m);
}

int16_t median(int n, int16_t x[]) {
	float temp;
	int16_t i, j;
	// the following two loops sort the array x in ascending order
	for(i=0; i<n-1; i++) {
		for(j=i+1; j<n; j++) {
			if(x[j] < x[i]) {
				// swap elements
				temp = x[i];
				x[i] = x[j];
				x[j] = temp;
			}
		}
	}
	
	if(n%2==0) {
		// if there is an even number of elements, return mean of the two elements in the middle
		return((x[n>>1] + x[(n>>1) - 1]) >> 1);
		} else {
		// else return the element in the middle
		return x[n>>1];
	}
}

int32_t varianz (int m, int16_t x[]) {
	int16_t avr = average(m,x);
	int32_t varianz = 0;
	for (int i = 0;i<m;i++) {
		int16_t diff = x[i]-avr;

		varianz += diff*diff;
	}
	return varianz;
}



