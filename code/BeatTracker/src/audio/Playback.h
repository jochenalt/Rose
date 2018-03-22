/*
 * Playback.h
 *
 *  Created on: Mar 20, 2018
 *      Author: jochenalt
 */

#ifndef SRC_PLAYBACK_H_
#define SRC_PLAYBACK_H_

#include <ao/ao.h>

class Playback {
public:
	Playback();
	virtual ~Playback();

	// initialize the playback with the given sample rate.
	void setup(int sampleRate);

	// play a sample with the sample rate as defined in setup. Samples should between 0..1
	void play(double volume /* 0..1 */, float outputBuffer[], int outputBufferSize);
private:
	ao_device* outputDevice = NULL;
};

#endif /* SRC_PLAYBACK_H_ */
