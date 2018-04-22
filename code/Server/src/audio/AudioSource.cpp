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
	wavContentSampleIndex = 0;

	float microphoneSampleRate = Configuration::getInstance().microphoneSampleRate;
	microphone.setup(microphoneSampleRate);

	noOfInputSample = 0;
	startTime_ms = 0;
	currentInputType = NO_CHANGE;
}


int AudioSource::readWavInput(double buffer[], unsigned BufferSize) {
	int numSamples = getCurrentWavContent().getNumSamplesPerChannel();
	int numInputSamples = min((int)BufferSize, (int)numSamples-currentSampleIndex);
	int numInputChannels = getCurrentWavContent().getNumChannels();

	int bufferCount = 0;
	for (int i = 0; i < numInputSamples; i++)
	{
		double inputSampleValue= getCurrentWavContent().samples[0][currentSampleIndex + i];
		assert(currentSampleIndex+1 < numSamples);
		switch (numInputChannels) {
		case 1:
			inputSampleValue= getCurrentWavContent().samples[0][currentSampleIndex + i];
			break;
		case 2:
			inputSampleValue = (getCurrentWavContent().samples[0][currentSampleIndex + i]+getCurrentWavContent().samples[1][currentSampleIndex + i])/2;
			break;
		default:
			inputSampleValue = 0;
			for (int j = 0;j<numInputChannels;j++)
				inputSampleValue += getCurrentWavContent().samples[j][currentSampleIndex + i];
			inputSampleValue = inputSampleValue / numInputChannels;
		}
		buffer[bufferCount++] = inputSampleValue;
	}
	currentSampleIndex += bufferCount;
	return bufferCount;
}


void AudioSource::fetchInput(int numOfSamples, double samples[]) {
	switch (nextInputType) {
		case MICROPHONE_INPUT: {
			currentInputType = MICROPHONE_INPUT;
			nextInputType = NO_CHANGE;
			break;
		}
		case WAV_INPUT: {
			// switch to prepared shadow Audiofile at the other index position
			wavContentSampleIndex = 1-wavContentSampleIndex;
			currentSampleIndex = 0;
			currentInputType = WAV_INPUT;
			nextInputType = NO_CHANGE;

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
			noOfInputSample += numOfSamples;
			processedTime = (double)noOfInputSample / (double)Configuration::getInstance().microphoneSampleRate;	// [s]
			break;
		}
		case WAV_INPUT: {
			if (startTime_ms == 0)
				startTime_ms = millis();

			// clear input samples, in case the wav file does not contain enough data
			for (int i = 0;i< numOfSamples;i++)
				samples[i] = 0;

			int samplesRead = readWavInput(samples, numOfSamples);
			noOfInputSample += samplesRead;
			processedTime = (double)noOfInputSample / (double)getCurrentWavContent().getSampleRate();;	// [s]

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
	nextInputType = MICROPHONE_INPUT;
}

void AudioSource::setWavContent(std::vector<uint8_t>& newWavData) {
	// indicate to change the source with next sampling
	nextInputType = WAV_INPUT;

	// read in the wav data and set index pointer to first position
	wavContent[1-wavContentSampleIndex].decodeWaveFile(newWavData);
}


double AudioSource::getElapsedTime() {
	if (startTime_ms == 0)
		return 0;
	return (millis() - startTime_ms)/1000.0;
}

float AudioSource::getCurrentLatency() {
	if (currentInputType == AudioSource::MICROPHONE_INPUT)
		return Configuration::getInstance().microphoneLatency;
	else
		return 0.35-Configuration::getInstance().microphoneBufferLength;
}

