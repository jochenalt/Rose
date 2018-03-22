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
}

AudioProcessor::~AudioProcessor() {
}

void AudioProcessor::setup(BeatCallbackFct newBeatCallback) {
    beatCallback = newBeatCallback;
	playback.setup(MicrophoneSampleRate);
	microphone.setup(MicrophoneSampleRate);
}

void AudioProcessor::setVolume(double newVolume) {
	volume = newVolume;
}
double AudioProcessor::getVolume() {
	return volume;
}

void AudioProcessor::setPlayback(bool ok) {
	withPlayback = ok;
}

bool AudioProcessor::getPlayback() {
	return withPlayback;
}

void AudioProcessor::setWavContent(std::vector<uint8_t>& newWavData) {
	// indicate that current processing is to be stopped
	stopCurrProcessing = true;

	// wait until processing is really stopped
	while (!currProcessingStopped)
		delay_ms(1);

	// read in the wav data and set index pointer to first position
	audioFile.decodeWaveFile(newWavData);
	wavInputPosition = 0;

	// playback is done with same sample rate like the input wav data
	playback.setup(audioFile.getSampleRate());

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

	// current input is microphone
	currentInputType = MICROPHONE_INPUT;
}

int AudioProcessor::readWavInput(float buffer[], unsigned BufferSize) {
	int numSamples = audioFile.getNumSamplesPerChannel();
	int numInputSamples = min((int)BufferSize, (int)numSamples-wavInputPosition);
	int numInputChannels = audioFile.getNumChannels();

	int bufferCount = 0;
	for (int i = 0; i < numInputSamples; i++)
	{
		double inputSampleValue = 0;
		assert(wavInputPosition+1 < numSamples);
		switch (numInputChannels) {
		case 1:
			inputSampleValue= audioFile.samples[0][wavInputPosition + i];
			break;
		case 2:
			inputSampleValue = (audioFile.samples[0][wavInputPosition + i]+audioFile.samples[1][wavInputPosition + i])/2;
			break;
		default:
			inputSampleValue = 0;
			for (int j = 0;j<numInputChannels;j++)
				inputSampleValue += audioFile.samples[j][wavInputPosition + i];
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

	int hopSize = 128;
	int frameSize = hopSize*8; // cpu load goes up linear with the framesize
	BTrack b(hopSize, frameSize);

	double elapsedTime = 0;
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
			sampleRate = audioFile.getSampleRate();
			if (readSamples < numInputSamples) {
				cout << "end of song. Switching to microphone." << endl;

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
		if (withPlayback)
			playback.play(volume, playbackBuffer,playbackBufferCount);

		// detect beat and bpm of that hop size
		b.processAudioFrame(beatDetectionBuffer);

		bool beat = b.beatDueInCurrentFrame();
		double bpm = b.getCurrentTempoEstimate();

		// insert a delay to synchronize played audio and beat detection
		elapsedTime = ((double)(millis() - startTime_ms)) / 1000.0f;   			// [s]
		double processedTime = (double)wavInputPosition / (double)sampleRate;	// [s]

		beatCallback(beat, bpm);

		if (beat){
			cout << std::fixed << std::setprecision(2) << "Beat (" << b.getCurrentTempoEstimate() << ")" << std::setprecision(2) << endl;
		};
		// wait such that elapsed time and processed time is synchronized
		delay_ms((processedTime - elapsedTime)*1000.0);
	}
	currProcessingStopped = true;
}
