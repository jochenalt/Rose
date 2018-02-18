/*
 * BandPassFilter.h
 *
 * Created: 31.01.2015 15:32:55
 *  Author: JochenAlt
 */ 


#ifndef BANDPASSFILTER_H_
#define BANDPASSFILTER_H_

#include "HighPassFilter.h"
#include "LowPassFilter.h"

// High Pass filter 1st order
class BandPassFixedPoint {
	public:
	
	
	BandPassFixedPoint () {
		highPass.init(0.0, LOOP_TIME);
		lowPass.init(0.0, LOOP_TIME);
	}

	// pass the weight of the most recent value
	// (1-ratio) is the weight of the filtered value
	BandPassFixedPoint(float pTimeDurationLowPass, float pTimeDurationHighPass, float pSampleTime) {
		init(pTimeDurationLowPass,pTimeDurationHighPass,pSampleTime);
	};

	void init(float pTimeDurationLowPass, float pTimeDurationHighPass, float pSampleTime) {
		lowPass.init(pTimeDurationLowPass,pSampleTime);
		highPass.init(pTimeDurationHighPass,pSampleTime);

	};

	// get last filtered value
	int16_fp8_t get() {
		return lowPass.get();
	};
	
	// get a filtered value
	int16_fp8_t get(int16_t pInput_fp8) {
		addSample(pInput_fp8);
		return get();
	};
	int16_fp8_t addSample(int16_t pInput_fp8) {
		highPass.addSample(pInput_fp8);
		lowPass.addSample(highPass.get());
		return lowPass.get();
	};
	void set(int16_t pValue) {
		highPass.set(pValue);
	}
	private:
		HighPassFixedPoint highPass;
		LowPassFixedPoint lowPass;
};





#endif /* BANDPASSFILTER_H_ */