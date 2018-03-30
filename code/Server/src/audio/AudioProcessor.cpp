/*
 * AudioProcessor.cpp
 *
 *  Created on: Mar 18, 2018
 *      Author: Jochen Alt
 */

#include <audio/AudioFile.h>
#include <iostream>
#include <string.h>

#include <stdlib.h>
#include <signal.h>
#include <chrono>
#include <unistd.h>
#include <iomanip>
#include <thread>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <algorithm>

#include <basics/stringhelper.h>
#include <basics/util.h>
#include <audio/AudioProcessor.h>
#include <audio/Playback.h>

#include <beat/BTrack.h>

using namespace std;

AudioProcessor::AudioProcessor() {
	beatCallback = NULL;
}

AudioProcessor::~AudioProcessor() {
}

void AudioProcessor::setup(BeatCallbackFct newBeatCallback) {
    beatCallback = newBeatCallback;
	microphone.setup(MicrophoneSampleRate);
	inputAudioDetected = false;
	// playback is setup when started

	// music detection requires 1s of music before flagging it as music
	beatScoreFilter.init(5);
}

void AudioProcessor::setVolume(double newVolume) {
	volume = newVolume;
}
double AudioProcessor::getVolume() {
	return volume;
}

void AudioProcessor::setPlayback(bool ok) {
	playback.setPlayback(ok);
}

bool AudioProcessor::getPlayback() {
	return playback.getPlayback();
}

void AudioProcessor::setWavContent(std::vector<uint8_t>& newWavData) {
	// indicate that current processing is to be stopped
	stopCurrProcessing = true;

	// wait until processing is really stopped
	while (!currProcessingStopped)
		delay_ms(1);

	// read in the wav data and set index pointer to first position
	wavContent.decodeWaveFile(newWavData);
	wavInputPosition = 0;

	// playback is done with same sample rate like the input wav data
	playback.setup(wavContent.getSampleRate());

	// current input is wav data
	currentInputType = WAV_INPUT;
}

void AudioProcessor::setMicrophoneInput() {
	// flag that processing loop should stop
	stopCurrProcessing = true;

	// wait until processing loop stopped
	while (!currProcessingStopped)
		delay_ms(1);

	// indicate that wav input is no longer used
	wavInputPosition = -1;

	// playback is set to standard sample rate
	playback.setup(MicrophoneSampleRate);

	// initialize the
	microphone.setup(MicrophoneSampleRate);

	// current input is microphone
	currentInputType = MICROPHONE_INPUT;
}

int AudioProcessor::readWavInput(float buffer[], unsigned BufferSize) {
	int numSamples = wavContent.getNumSamplesPerChannel();
	int numInputSamples = min((int)BufferSize, (int)numSamples-wavInputPosition);
	int numInputChannels = wavContent.getNumChannels();

	int bufferCount = 0;
	for (int i = 0; i < numInputSamples; i++)
	{
		double inputSampleValue = 0;
		inputSampleValue= wavContent.samples[0][wavInputPosition + i];
		assert(wavInputPosition+1 < numSamples);
		switch (numInputChannels) {
		case 1:
			inputSampleValue= wavContent.samples[0][wavInputPosition + i];
			break;
		case 2:
			inputSampleValue = (wavContent.samples[0][wavInputPosition + i]+wavContent.samples[1][wavInputPosition + i])/2;
			break;
		default:
			inputSampleValue = 0;
			for (int j = 0;j<numInputChannels;j++)
				inputSampleValue += wavContent.samples[j][wavInputPosition + i];
			inputSampleValue = inputSampleValue / numInputChannels;
		}
		buffer[bufferCount++] = inputSampleValue;
	}
	wavInputPosition += bufferCount;
	return bufferCount;
}

void AudioProcessor::processInput() {
	stopCurrProcessing = false;
	currProcessingStopped = false;

	// hop size is the number of samples that will be fed into beat detection
	int hopSize = 128; // approx. 3ms at 44100Hz

	// framesize is the number of samples that will be considered in this loop
	// cpu load goes up linear with the framesize
	int frameSize = hopSize*16;
	BTrack beatDetector(hopSize, frameSize);

	uint32_t startTime_ms = millis();

	int sampleRate = 0;
	while (!stopCurrProcessing) {

		int numInputSamples = hopSize;
		float inputBuffer[numInputSamples];
		int readSamples  = 0;

		if (currentInputType == MICROPHONE_INPUT) {
			readSamples = microphone.readMicrophoneInput(inputBuffer, numInputSamples);
			sampleRate = MicrophoneSampleRate;

		}
		if (currentInputType == WAV_INPUT) {
			readSamples = readWavInput(inputBuffer, numInputSamples);

			sampleRate = wavContent.getSampleRate();
			if (readSamples < numInputSamples) {
				cout << "end of song. Switching to microphone." << endl;
				beatDetector.initialise(hopSize, frameSize);

				// indicate that current processing stopped (setMicrophoneInput waits for this)
				currProcessingStopped = true;

				// use microphone instead of wav input
				setMicrophoneInput();
				return;
			}
		}
		if (readSamples != numInputSamples)
			cerr << "not enough samples " << readSamples << "vs " << numInputSamples << " type=" << currentInputType << endl;

		int beatDetectionBufferSize = numInputSamples;
		double beatDetectionBuffer[beatDetectionBufferSize];

		int playbackBufferSize = numInputSamples;
	    float playbackBuffer[playbackBufferSize];

	    int playbackBufferCount = 0;
	    int beatDetectionCount = 0;
		for (int i = 0; i < numInputSamples; i++)
		{
			double inputSampleValue= inputBuffer[i];

			// set beat detection buffer
			assert (beatDetectionCount  < beatDetectionBufferSize);
			beatDetectionBuffer[beatDetectionCount++] = inputSampleValue;

			// set frame value into output buffer to be played later on
			assert (playbackBufferCount  < playbackBufferSize);
			playbackBuffer[playbackBufferCount++] = inputSampleValue;
		}

		// play the buffer of hopSize asynchronously
		playback.play(volume, playbackBuffer,playbackBufferCount);

		// detect beat and bpm of that hop size
		beatDetector.processAudioFrame(beatDetectionBuffer);
		bool beat = beatDetector.beatDueInCurrentFrame();
		double bpm = beatDetector.getCurrentTempoEstimate();

		if (beat){
			cout << std::fixed << std::setprecision(2) << "Beat (" << beatDetector.getCurrentTempoEstimate() << ")"  << endl;
	    };

		// check if the signal is really music. low pass scoring to ensure that small pauses are not
		// misinterpreted as end of music
		double score = beatDetector.getLatestCumulativeScoreValue();
		beatScoreFilter.set(score);
		const double scoreThreshold = 10.;
		inputAudioDetected = (beatScoreFilter >= scoreThreshold);

		// call callback to rythm identifier and dance move generator
		// but do this with a lower frequency (50fps)
		if (callbackTimer.isDue(1000/50)  || (beat)) {
			callbackTimer.setDueNow();
			processedTime = millis()/1000.0;
			beatCallback(beat, bpm);
		}

		if (currentInputType == WAV_INPUT) {
			// insert a delay to synchronize played audio and beat detection before entering the next cycle
			double elapsedTime = ((double)(millis() - startTime_ms)) / 1000.0f;  	// [s]
			double processedTime = (double)wavInputPosition / (double)sampleRate;	// [s]
			// wait such that elapsed time and processed time is synchronized
			double timeAhead_ms = (processedTime - elapsedTime)*1000.0;
			if (timeAhead_ms > 1.0)
				delay_ms(timeAhead_ms);
		}
	}
	currProcessingStopped = true;
}

float AudioProcessor::getLatency() {
	if (currentInputType == MICROPHONE_INPUT)
		return MicrophoneLatency;
	else
		return 0.5;
}

