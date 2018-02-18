/*
 * ComplementaryFilter.h
 *
 * Created: 14.12.2014 13:36:13
 *  Author: JochenAlt
 */ 


#ifndef COMPLEMENTARYFILTER_H_
#define COMPLEMENTARYFILTER_H_

#include "FixedPoint.h"

class LowPassFilter {
	public:
	
	LowPassFilter () {
		result = 0;
		compFilterRatio = 0;
	}

	LowPassFilter (float timeDuration, float sampleTime) {
		result = 0;
		init(timeDuration,sampleTime);
	}

	// pass the weight of the most recent value
	// (1-ratio) is the weight of the filtered value
	void init(float timeDuration, float sampleTime) {
		init(timeDuration/(timeDuration + sampleTime));
	};
	void init(float pFilterRatio) {
		compFilterRatio = pFilterRatio;
	};

	// get a filtered value
	float addSample(float pValue) {
		result = compFilterRatio*result + (1.0-compFilterRatio) * pValue;
		return result;
	};

	// get a filtered value
	float get(float pValue) {
		addSample(pValue);
		return get();
	};

	float get() {
		return result;
	}
	
	void set(float pValue) {
		result = pValue;
	}
	private:
		// complementary value
		float result;
		// weight of the latest value over the last complementary value
		float compFilterRatio;
};

// complementary filter that does not loose any tick. It keeps track of all
// numbers accepted and delivered and tries to keep the difference small.
// Not really the way how control theory defines that, frequency is hard to identify by this
// but sometimes it is important to deliver all values (e.g. for encoder pulses)
class LowPassFixedPoint {
	public:
	
	
	LowPassFixedPoint () {
			init(0.0);
			dontLooseTicks = false;
	}

	// pass the weight of the most recent value
	// (1-ratio) is the weight of the filtered value
	LowPassFixedPoint(float timeDuration, float pSampleTime) {
		init(timeDuration,pSampleTime);
		dontLooseTicks = false;
	};
	void init(float timeDuration, float pSampleTime) {
		init(FLOAT2FP16(timeDuration/(timeDuration + pSampleTime),12));
	};

	void init(int16_t pFilterRatio_fp12) {
		inputOutputDiff = 0;
		compFilter_fp8 = 0;
		compFilterRatio_fp12 = pFilterRatio_fp12;


	};

	// get a filtered value
	int16_fp8_t get() {
		return compFilter_fp8;
	};
	
	// get a filtered value
	int16_fp8_t get(int16_t pValue) {
		addSample(pValue);
		return get();
	};
	int16_fp8_t addSample(int16_t pValue) {
		if (dontLooseTicks) {
			int16_t value = inputOutputDiff+pValue;
			compFilter_fp8 =(int16_t)i32_rsh(	mul16s((1<<12)-compFilterRatio_fp12,value) +
												mul16s(compFilterRatio_fp12,compFilter_fp8),12);
			inputOutputDiff += (pValue-compFilter_fp8);			
		}else {
			compFilter_fp8 =(int16_t)i32_rsh(	mul16s((1<<12)-compFilterRatio_fp12,pValue) +
												mul16s(compFilterRatio_fp12,compFilter_fp8),12);
		}
		return compFilter_fp8;
	};
	void set(int16_t pValue) {
		compFilter_fp8 = pValue;
	}
	private:
	// difference of ticks accepted and delivered
	int16_t inputOutputDiff;
	// complementary value
	int16_t compFilter_fp8;
	// weight of the latest value over the last complementary value
	int16_t compFilterRatio_fp12;
	
	boolean dontLooseTicks;
};

#endif /* COMPLEMENTARYFILTER_H_ */