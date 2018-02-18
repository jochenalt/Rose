/*
 * MedianFilter.h
 *
 * Created: 22.12.2014 11:40:55
 *  Author: JochenAlt
 */ 


#ifndef MEDIANFILTER_H_
#define MEDIANFILTER_H_

#include "Arduino.h"
class MedianFilter
{
	public:
		// Initialize the Median Object
		MedianFilter();

		void init(uint8_t size, float value);
		
		// Insert an Element to the Median Filter
		float addSample(float value);
		
		float get();

	float data[16];
	uint8_t age[16];
	uint8_t size;
	uint8_t count;
	float value;

};



#endif /* MEDIANFILTER_H_ */