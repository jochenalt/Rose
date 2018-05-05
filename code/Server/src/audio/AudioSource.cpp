/*
 * AudioSource.cpp
 *
 *  Created on: Apr 21, 2018
 *      Author: jochenalt
 */

#include <audio/AudioSource.h>
#include <audio/AudioFile.h>
#include <iostream>
#include <string.h>

#include <stdlib.h>
#include <signal.h>
#include <chrono>
#include <unistd.h>
#include <string>

#include <basics/stringhelper.h>
#include <basics/util.h>
#include <audio/MicrophoneInput.h>
#include <audio/Playback.h>
#include <Configuration.h>



void AudioSource::setup() {
	currentSampleIndex = 0;

	float microphoneSampleRate = Configuration::getInstance().microphoneSampleRate;
	microphone.setup(microphoneSampleRate);

	startTime_ms = 0;
	currentInputType = NO_CHANGE;
}


int AudioSource::readWavInput(double buffer[], unsigned BufferSize) {
	AudioFile<double>& currentSource = getCurrentWavContent();
	int numSamples = currentSource.getNumSamplesPerChannel();
	int numInputSamples = min((int)BufferSize, (int)numSamples-currentSampleIndex);
	int numInputChannels = getCurrentWavContent().getNumChannels();

	bool doubleSamples = (int)currentSource.getSampleRate() == Configuration::getInstance().microphoneSampleRate / 2;
 	if (doubleSamples)
		numInputSamples /= 2;

	int bufferCount = 0;
	for (int i = 0; i < numInputSamples; i++)
	{
		double inputSampleValue = 0;
		assert(currentSampleIndex+1 < numSamples);
		switch (numInputChannels) {
		case 1:
			inputSampleValue= currentSource.samples[0][currentSampleIndex + i];
			break;
		case 2:
			inputSampleValue = (currentSource.samples[0][currentSampleIndex + i]+currentSource.samples[1][currentSampleIndex + i])/2;
			break;
		default:
			inputSampleValue = 0;
			for (int j = 0;j<numInputChannels;j++)
				inputSampleValue += currentSource.samples[j][currentSampleIndex + i];
			inputSampleValue = inputSampleValue / numInputChannels;
		}
		if (doubleSamples) {
			buffer[bufferCount++] = (lastSample + inputSampleValue)/2.0;
			buffer[bufferCount++] = inputSampleValue;
			lastSample = inputSampleValue;
		} else
			buffer[bufferCount++] = inputSampleValue;

	}
	currentSampleIndex += numInputSamples;
	return bufferCount;
}


void AudioSource::fetchInput(int numOfSamples, double samples[]) {
	switch (nextInputType) {
		case MICROPHONE_INPUT: {
			currentInputType = MICROPHONE_INPUT;
			nextInputType = NO_CHANGE;
			sourceChanged = true;
			break;
		}
		case WAV_INPUT: {

			// switch to prepared shadow Audiofile at the other index position
			wavContentIndex = 1-wavContentIndex;
			currentSampleIndex = 0;
			currentInputType = WAV_INPUT;
			nextInputType = NO_CHANGE;
			sourceChanged = true;
			break;
		}
		default:
			// input type does not change
			break;
	}

	switch (currentInputType) {
		case MICROPHONE_INPUT: {
			if (startTime_ms == 0)
				startTime_ms = millis();

			microphone.readMicrophoneInput(samples, numOfSamples);
			processedTime += (double)numOfSamples / (double)Configuration::getInstance().microphoneSampleRate;	// [s]
			break;
		}
		case WAV_INPUT: {
			if (startTime_ms == 0)
				startTime_ms = millis();

			// clear input samples, in case the wav file does not contain enough data
			for (int i = 0;i< numOfSamples;i++)
				samples[i] = 0;

			int samplesRead = readWavInput(samples, numOfSamples);
			processedTime += (double)samplesRead/ (double)Configuration::getInstance().microphoneSampleRate;	// [s]

			if (samplesRead < numOfSamples) {
				cout << "end of song. Switching to microphone." << endl;

				// use microphone instead of wav input
				setMicrophoneInput();
			}
		}
		default:
			// no input type defined
			break;
	}
}

void AudioSource::setMicrophoneInput() {

	// next invokation from fetchInput (in a different thread)
	// will discard the wav content and grab from microphone
	nextInputType = MICROPHONE_INPUT;
}

void AudioSource::setWavContent(std::vector<uint8_t>& newWavData) {

	// read in the wav data and set index pointer to first position
	getWavContent(1-wavContentIndex).decodeWaveFile(newWavData);

	// indicate to change the source with next sampling
	// this is the flag used by the audio thread, right after this statement it grabs
	// the new wav file and discards the current one
	nextInputType = WAV_INPUT;
}


double AudioSource::getElapsedTime() {
	if (startTime_ms == 0)
		return 0;
	return (millis() - startTime_ms)/1000.0;
}

bool AudioSource::hasSourceChanged() {
	bool result = sourceChanged;
	sourceChanged = false;
	return result;
}


float AudioSource::getCurrentLatency() {
	if (currentInputType == AudioSource::MICROPHONE_INPUT)
		return Configuration::getInstance().microphoneLatency;
	else
		return 0.25;
}


