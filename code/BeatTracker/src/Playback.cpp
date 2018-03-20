/*
 * Playback.cpp
 *
 *  Created on: Mar 20, 2018
 *      Author: jochenalt
 */


#include <assert.h>
#include <iostream>
#include <string.h>
#include <ao/ao.h>
#include <Playback.h>

using namespace std;

Playback::Playback() {

	outputDevice = NULL;
}

Playback::~Playback() {
	ao_shutdown();
}


void Playback::setup(int sampleRate) {
    ao_initialize();
    // initialize output device
   	ao_sample_format audioOutputFormat;
   	memset(&audioOutputFormat, 0, sizeof(audioOutputFormat));
   	audioOutputFormat.bits = 16;
   	audioOutputFormat.channels = 1;
   	audioOutputFormat.rate = sampleRate;
   	audioOutputFormat.byte_format = AO_FMT_LITTLE;
   	int defaultDriverHandle = ao_default_driver_id();
   	outputDevice = ao_open_live(defaultDriverHandle, &audioOutputFormat, NULL /* no options */);
   	if (outputDevice == NULL) {
   		cerr << "Could not open sound deviceError opening sound device" << endl;
   		exit(1);
   	}
}

void Playback::playbackSample(double volume /* 0..1 */ ,int outputBuffer[], int outputBufferSize) {
	char playBuffer[outputBufferSize*2];
	int outputBufferCount = 0;
	for (int i = 0;i<outputBufferSize;i++) {
		unsigned aoBufferValue = ((float)outputBuffer[i])*volume;
		// set frame value into output buffer to be played later on
		assert (outputBufferCount  < outputBufferSize*2);
		playBuffer[outputBufferCount] = (uint8_t)(aoBufferValue & 0xFF);
		outputBufferCount++;
		assert (outputBufferCount < outputBufferSize*2);
		playBuffer[outputBufferCount] = (uint8_t)(aoBufferValue >> 8);
		outputBufferCount++;
	}

	// play the buffer of hopSize asynchronously
	ao_play(outputDevice, playBuffer, outputBufferSize*2);
}
