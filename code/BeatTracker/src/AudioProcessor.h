/*
 * AudioProcessor.h
 *
 *  Created on: Mar 18, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIOPROCESSOR_H_
#define SRC_AUDIOPROCESSOR_H_

#include "Playback.h"
#include <pulse/simple.h>
#include <pulse/error.h>
#include <AudioFile/AudioFile.h>

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
	void setup(BeatCallbackFct beatCallback);

	// set wav content to be processed with next loop
	void setWavContent(std::vector<uint8_t>& wavData);
	void setMicrophoneInput();

	bool isWavContentPending() { return currentInputType == WAV_INPUT; };
	bool isMicrophoneInputPending() { return currentInputType == MICROPHONE_INPUT;; };

	// process content of a wav
	void processInput();

	void stopProcessing() { stopCurrProcessing = true; };

	// get/set volume [0..1]
	void setVolume(double newVolume);
	double getVolume();

	void setPlayback(bool ok);
	bool getPlayback();
private:
	enum InputType { WAV_INPUT, MICROPHONE_INPUT };
	int readMicrophoneInput(float buffer[], unsigned BufferSize);
	int readWavInput(float buffer[], unsigned BufferSize);

	volatile bool stopCurrProcessing = false;
	volatile bool currProcessingStopped = true;

	double volume = 1.0;
	bool withPlayback = true;
	BeatCallbackFct beatCallback;
	Playback playback;
	pa_simple *pulseAudioConnection = NULL;
	AudioFile<double> audioFile;
	int wavInputPosition = -1;
	InputType currentInputType = MICROPHONE_INPUT;
};

#endif /* SRC_AUDIOPROCESSOR_H_ */
