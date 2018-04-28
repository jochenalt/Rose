/*
 * AudioProcessor.h
 *
 *  Created on: Mar 18, 2018
 *      Author: Jochen Alt
 */

#ifndef SRC_AUDIOPROCESSOR_H_
#define SRC_AUDIOPROCESSOR_H_

#include <basics/util.h>
#include <audio/AudioSource.h>
#include <audio/BeatGenerator.h>

#include <beat/BTrack.h>

class AudioProcessor {
public:
	enum BeatType { NO_BEAT, BEAT_GENERATION, BEAT_DETECTION };

	// call back type for invoking the dance processor after each sample
	typedef void (*BeatCallbackFct)(double processTime, bool beat, double Bpm, int rhythmInQuarters);

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

	bool isWavContentUsed() { return audioSource.getSourceType() == AudioSource::WAV_INPUT; };
	bool isMicrophoneInputUsed() { return audioSource.getSourceType() == AudioSource::MICROPHONE_INPUT; };

	// process content of passed wav content or content coming from microphone.
	// returns whenever the current content is empty (valid of wav content only)
	// needs to be called repeatedly.
	void processInput();

	// get/set volume [0..1]
	void setVolume(double newVolume);
	double getVolume();

	// pipe the audio input to the output
	void setGlobalPlayback(bool ok);
	bool getGlobalPlayback();

	// get static current latency of input source
	// used in latency compensation that adapts the prediction time frame accordingly
	double getCurrentLatency();

	// get processed time relative to input source (wav or microphone)
	// this is set whenever the audio input is analyzed and has a precision of around 3ms
	double getProcessedTime();

	double getElapsedTime();

	// checks if the input signal is above a certain threshold
	bool isAudioDetected() { return inputAudioDetected; };

	// set the audio source right after microphone or wav has been set. Used only when main loop is not yet running
	void setAudioSource();

	// measure the latency of the microphone accoustically
	double calibrateLatency();

	BeatType getCurrentBeatType() { return currentBeatType; };

private:

	const int numInputSamples = 512;
	volatile bool stopCurrProcessing = false;
	Playback playback;
	bool globalPlayback;
	BeatCallbackFct beatCallback;
	TimeSampler callbackTimer; 				// timer for callback as passed via setup()

	bool inputAudioDetected = false;		// true if music has been detected
	double volume = 1.0;					// volume used in playback

	LowPassFilter cumulativeScoreLowPass;
	LowPassFilter squaredScoreLowPass = 0;

	AudioSource audioSource;
	BTrack* beatDetector = NULL;
	BeatGenerator beatGen;
	BeatType currentBeatType = NO_BEAT;
	BeatType pendingBeatType = NO_BEAT;
	double lastBeatTime = 0;
	int rhythmInQuarters = 1;
};

#endif /* SRC_AUDIOPROCESSOR_H_ */
