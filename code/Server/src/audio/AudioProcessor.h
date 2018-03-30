/*
 * AudioProcessor.h
 *
 *  Created on: Mar 18, 2018
 *      Author: Jochen Alt
 */

#ifndef SRC_AUDIOPROCESSOR_H_
#define SRC_AUDIOPROCESSOR_H_

#include <audio/AudioFile.h>
#include <audio/Playback.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include "audio/MicrophoneInput.h"

class AudioProcessor {
public:
	// call back type for invoking the dance processor after each sample
	typedef void (*BeatCallbackFct)(bool beat, double Bpm);

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

	// pipe the audio input to the output
	void setPlayback(bool ok);
	bool getPlayback();

	// get static current latency of input source
	// used in latency compensation that adapts the prediction time frame accordingly
	float getLatency();

	// get processed time relative to input source (wav or microphone)
	// this is set whenever the audio input is analyzed and has a precision of around 3ms
	double getProcessedTime() { return processedTime; };

	// checks if the input signal is above a certain threshold
	bool isAudioDetected() { return inputAudioDetected; };
private:
	enum InputType { WAV_INPUT, MICROPHONE_INPUT };
	int readMicrophoneInput(float buffer[], unsigned BufferSize);
	int readWavInput(float buffer[], unsigned BufferSize);

	volatile bool stopCurrProcessing = false;
	volatile bool currProcessingStopped = true;

	double volume = 1.0;
	BeatCallbackFct beatCallback;
	Playback playback;					// used to send the input source to the loudspeaker
	MicrophoneInput microphone;			// used to get input from microphone
	AudioFile<double> wavContent;		// used to get input from wav (actually no file, but an array of samples)
	int wavInputPosition = -1;			// current position within wav source
	double processedTime = 0; 			// [s] processing time of input source. Is determined by position within wav file or realtime in case of micropone input
	TimeSamplerStatic callbackTimer; 	// timer for callback as passed via setup()
	InputType currentInputType = MICROPHONE_INPUT;
	bool inputAudioDetected = false;
	LowPassFilter beatScoreFilter;
};

#endif /* SRC_AUDIOPROCESSOR_H_ */
