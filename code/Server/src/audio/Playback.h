/*
 * Playback.h
 *
 *  Created on: Mar 20, 2018
 *      Author: jochenalt
 */

#ifndef SRC_PLAYBACK_H_
#define SRC_PLAYBACK_H_

#include <string.h>
#include <pulse/simple.h>
#include <pulse/error.h>

using namespace std;

class Playback {
public:
	Playback();
	virtual ~Playback();

	// initialize the playback with the given sample rate.
	void setup(int sampleRate);

	// play a sample with the sample rate as defined in setup. Samples should between 0..1
	void play(double volume /* 0..1 */, double outputBuffer[], int outputBufferSize);

	void setPlayback (bool ok) { playback = ok; };
	bool getPlayback() { return playback; };
private:
	bool playback = true;
	pa_simple* pulseAudioConnection = NULL;
	pa_sample_spec ss;
    string deviceName;
};

#endif /* SRC_PLAYBACK_H_ */
