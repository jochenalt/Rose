/*
 * AudioProcessor.h
 *
 *  Created on: Mar 18, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIOPROCESSOR_H_
#define SRC_AUDIOPROCESSOR_H_

#include <ao/ao.h>

typedef void (*BeatCallbackFct)(bool beat, double Bpm);

class AudioProcessor {
public:
	AudioProcessor();
	virtual ~AudioProcessor();

	static AudioProcessor& getInstance() {
		static AudioProcessor instance;
		return instance;
	}

	// call setup before anything else
	void setup();

	// process content of a wav
	void processWav(std::vector<uint8_t>& wavData, BeatCallbackFct beatCallback);

	// get/set volume [0..1]
	void setVolume(double newVolume);
	double getVolume();

	void setPlayback(bool ok);
	bool getPlayback();
private:

	double volume = 1.0;
	bool withPlayback = true;
	ao_device* outputDevice = NULL;

};

#endif /* SRC_AUDIOPROCESSOR_H_ */
