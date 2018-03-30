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
#include <dance/RhythmDetector.h>

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
	beatScoreFilter.init(20);
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
	// set new data and indicate to change the source
	nextWavContent = newWavData;

	// indicate that current processing is to be stopped
	stopCurrProcessing = true;

	nextInputType = WAV_INPUT;
}

void AudioProcessor::setAudioSource() {

	if (nextInputType == WAV_INPUT) {
		// read in the wav data and set index pointer to first position
		currentWavContent.decodeWaveFile(nextWavContent);

		// playback is done with same sample rate like the input wav data
		playback.setup(currentWavContent.getSampleRate());

		// likely a new rythm
		RhythmDetector::getInstance().setup();

		// current input is wav data
		currentInputType = WAV_INPUT;

		// clear input, has been saved
		nextWavContent.clear();

		// reset position in wav content to start
		wavInputPosition = 0;

		cout << "switching audio source to wav input" << endl;
	}
	if (nextInputType == MICROPHONE_INPUT) {
		currentInputType = MICROPHONE_INPUT;

		// playback is set to standard sample rate
		playback.setup(MicrophoneSampleRate);

		// initialize the
		microphone.setup(MicrophoneSampleRate);

		cout << "switching to microphone input" << endl;
	}

	// do not switch source again until explicitely set
	nextInputType = NO_CHANGE;

}
void AudioProcessor::setMicrophoneInput() {
	// flag that processing loop should stop
	stopCurrProcessing = true;

	// current input is microphone
	nextInputType = MICROPHONE_INPUT;
}

int AudioProcessor::readWavInput(float buffer[], unsigned BufferSize) {
	int numSamples = currentWavContent.getNumSamplesPerChannel();
	int numInputSamples = min((int)BufferSize, (int)numSamples-wavInputPosition);
	int numInputChannels = currentWavContent.getNumChannels();

	int bufferCount = 0;
	for (int i = 0; i < numInputSamples; i++)
	{
		double inputSampleValue = 0;
		inputSampleValue= currentWavContent.samples[0][wavInputPosition + i];
		assert(wavInputPosition+1 < numSamples);
		switch (numInputChannels) {
		case 1:
			inputSampleValue= currentWavContent.samples[0][wavInputPosition + i];
			break;
		case 2:
			inputSampleValue = (currentWavContent.samples[0][wavInputPosition + i]+currentWavContent.samples[1][wavInputPosition + i])/2;
			break;
		default:
			inputSampleValue = 0;
			for (int j = 0;j<numInputChannels;j++)
				inputSampleValue += currentWavContent.samples[j][wavInputPosition + i];
			inputSampleValue = inputSampleValue / numInputChannels;
		}
		buffer[bufferCount++] = inputSampleValue;
	}
	wavInputPosition += bufferCount;
	return bufferCount;
}

void AudioProcessor::processInput() {
	stopCurrProcessing = false;

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

			sampleRate = currentWavContent.getSampleRate();
			if (readSamples < numInputSamples) {
				cout << "end of song. Switching to microphone." << endl;
				beatDetector.initialise(hopSize, frameSize);

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
	// check if the source needs to be changed
	setAudioSource();
}

float AudioProcessor::getLatency() {
	if (currentInputType == MICROPHONE_INPUT)
		return MicrophoneLatency;
	else
		return 0.5;
}

