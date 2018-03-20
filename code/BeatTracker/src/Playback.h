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
	static Playback& getInstance() {
		static Playback instance;
		return instance;
	}
	void setup(int sampleRate);
	void playbackSample(double volume /* 0..1 */, float outputBuffer[], int outputBufferSize);
private:
	ao_device* outputDevice = NULL;
};

#endif /* SRC_PLAYBACK_H_ */
