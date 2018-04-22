/*
 * AudioSource.h
 *
 *  Created on: Apr 21, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIO_AUDIOSOURCE_H_
#define SRC_AUDIO_AUDIOSOURCE_H_

#include <audio/AudioFile.h>
#include <audio/Playback.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "audio/MicrophoneInput.h"

class AudioSource {
public:

	enum InputType { WAV_INPUT, MICROPHONE_INPUT, NO_CHANGE };

	AudioSource() {};
	virtual ~AudioSource() {};

	AudioSource& getInstance() {
		static AudioSource instance;
		return instance;
	}

	// call before use of AudioSource
	void setup();

	// get the next block of samples
	void fetchInput(int numOfSamples, double samples[]);

	// set a wav file to be delivered with the next sample
	void setWavContent(std::vector<uint8_t>& newWavData);
	void setMicrophoneInput();


	// return processing time of audio source. Is monotonous and represents the time of the played wav or the recorded microphone
	double getProcessedTime() { return processedTime; };
	double getElapsedTime();

	// get static current latency of input source
	// used in latency compensation that adapts the prediction time frame accordingly
	float getCurrentLatency();

	InputType getSourceType() { return currentInputType; };

private:
	AudioFile<double>& getCurrentWavContent() { return wavContent[wavContentSampleIndex]; };
	int readWavInput(double buffer[], unsigned BufferSize);
	InputType currentInputType = MICROPHONE_INPUT;
	InputType nextInputType = NO_CHANGE;

	MicrophoneInput microphone;				// used to get input from microphone
	int wavContentSampleIndex = 0;
	AudioFile<double> wavContent[2];		// used to get input from wav (actually no file, but an array of samples)
	int currentSampleIndex;					// current index of wav content

	int noOfInputSample = 0;
	double processedTime = 0;
	uint32_t startTime_ms = 0;
};


#endif /* SRC_AUDIO_AUDIOSOURCE_H_ */
