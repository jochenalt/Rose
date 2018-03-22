/*
 * AudioProcessor.h
 *
 *  Created on: Mar 18, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIOPROCESSOR_H_
#define SRC_AUDIOPROCESSOR_H_

#include <audio/AudioFile.h>
#include <audio/Playback.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "audio/MicrophoneInput.h"

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
	bool isMicrophoneInputPending() { return currentInputType == MICROPHONE_INPUT; };

	// process content of passed wav content or content coming from microphone.
	// returns whenever the current content is empty (valid of wav content only)
	// needs to be called repeatedly.
	void processInput();

	// get/set volume [0..1]
	void setVolume(double newVolume);
	double getVolume();

	// switch playback on or off
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
	MicrophoneInput microphone;
	AudioFile<double> audioFile;
	int wavInputPosition = -1;
	InputType currentInputType = MICROPHONE_INPUT;
};

#endif /* SRC_AUDIOPROCESSOR_H_ */
