/*
 * MedianFilter.cpp
 *
 * Created: 22.12.2014 11:43:42
 *  Author: JochenAlt
 */ 



#include "MedianFilter.h"

/**
 * Initialize the Median Object
 */
MedianFilter::MedianFilter()
{
	count = 0;
	size = 0;

	// Initialize the values and ages
	for (uint8_t i=0; i<size; i++)
	{
		data[i] = 0;
		age[i] = 0;
	}
}

void MedianFilter::init(uint8_t pSize, float pValue) {
	count = 0;
	size = pSize;

	// Initialize the values and ages
	for (uint8_t i=0; i<size; i++)
	{
		data[i] = pValue;
		age[i] = 0;
	}
}

/**
 * Insert an Element to the Median Filter
 */
float MedianFilter::addSample(float value)
{
	uint8_t inspos = size;
	uint8_t pos;

	// Replace the Oldest entry and store the insertion position in inspos
	for (pos=0; pos<size; pos++)
	{
		if (age[pos] > 0)
			age[pos] -= 1;
		else if (inspos == size)
		{
			inspos = pos;
			data[pos] = value;
			age[pos] = size - 1;
		}
	}

	// Count the actual number of data until the size is reached
	if (count < size)
		count += 1;

	float tmpdata;
	uint8_t tmpage;
	uint8_t swapcount = 0;

	// shift the elements up to the right sorted position
	for (pos=inspos+1; pos<count; pos++)
	{
		if (data[pos-1] < data[pos])
			break;

		tmpdata         = data[pos];
		tmpage          = age[pos];
		data[pos]   = data[pos-1];
		age[pos]    = age[pos-1];
		data[pos-1] = tmpdata;
		age[pos-1]  = tmpage;
		swapcount++;
	}

	// If we already shifted up, then we need no shift down
	if (swapcount == 0)
	{
		// shift the elements down to the right sorted position
		for (pos=inspos; pos>0; pos--)
		{
			if (data[pos] < data[pos-1])
			{
				tmpdata         = data[pos];
				tmpage          = age[pos];
				data[pos]   = data[pos-1];
				age[pos]    = age[pos-1];
				data[pos-1] = tmpdata;
				age[pos-1]  = tmpage;
			}
		}
	}

	pos = count;
	if (pos & 0x01)
	{
		value = data[pos >> 1];	// Return the median if the count is an odd number
	}
	else
	{
		// Return the average value of the two median values
		pos = pos >> 1;
		value = (data[pos-1] + data[pos])/2;
	}
	
	return value;
}

float MedianFilter::get() {
	return value;
}