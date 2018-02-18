/*
 * HighPassFilter.h
 *
 * Created: 10.01.2015 21:05:59
 *  Author: JochenAlt
 */ 


#ifndef HIGHPASSFILTER_H_
#define HIGHPASSFILTER_H_


#include "FixedPoint.h"
#include "LowPassFilter.h"

// High Pass filter 1st order
class HighPassFixedPoint {
	public:
	
	
	HighPassFixedPoint () {
		lowPass.init(0.0);
		output_fp8 = 0;
	}

	// pass the weight of the most recent value
	// (1-ratio) is the weight of the filtered value
	HighPassFixedPoint(float pTimeDuration, float pSampleTime) {
		init(pTimeDuration,pSampleTime);
	};

	void init(float pTimeDuration, float pSampleTime) {
		lowPass.init(pTimeDuration,pSampleTime);
		output_fp8 = 0;
	};

	// get last filtered value
	int16_fp8_t get() {
		return output_fp8;
	};
	
	// get a filtered value
	int16_fp8_t get(int16_t pInput_fp8) {
		addSample(pInput_fp8);
		return get();
	};
	int16_fp8_t addSample(int16_t pInput_fp8) {
		output_fp8 = pInput_fp8 - lowPass.get();
		lowPass.addSample(pInput_fp8);
		return output_fp8;
	};
	void set(int16_t pValue) {
		output_fp8 = pValue;
	}
	private:
		LowPassFixedPoint lowPass;		
		int16_fp8_t output_fp8;	
};



#endif /* HIGHPASSFILTER_H_ */