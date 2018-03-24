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
#include <audio/Playback.h>

using namespace std;

Playback::Playback() {
    ao_initialize();
	outputDevice = NULL;
}

Playback::~Playback() {
	ao_shutdown();
}


void Playback::setup(int sampleRate) {
	if (outputDevice != NULL) {
		ao_close(outputDevice);
		outputDevice = NULL;
	}
    // initialize output device
   	ao_sample_format audioOutputFormat;
   	memset(&audioOutputFormat, 0, sizeof(audioOutputFormat));
   	audioOutputFormat.bits = 16;
   	audioOutputFormat.channels = 1;
   	audioOutputFormat.rate = sampleRate;
   	audioOutputFormat.byte_format = AO_FMT_LITTLE; // small indian
   	audioOutputFormat.matrix = NULL;
   	int defaultDriverHandle = ao_default_driver_id();
   	defaultDriverHandle = 1; // USB sound card
   	ao_option* p_ao_option = new ao_option();
   	p_ao_option->key = (char*)"dev";
   	p_ao_option->value = (char*)"plughw:CARD=Device";

   	outputDevice = ao_open_live(defaultDriverHandle, &audioOutputFormat, p_ao_option );
   	if (outputDevice == NULL) {
   		cerr << "Could not open sound device " << defaultDriverHandle << " with "<< sampleRate << "Hz, " << audioOutputFormat.channels << " channels "  << " err=" << errno << endl;
   		exit(1);
   	}
   	cout << "using device " << defaultDriverHandle << " with " << sampleRate << "Hz for audio output " << endl;
}

void Playback::play(double volume /* 0..1 */ ,float outputBuffer[], int outputBufferSize) {
	char playBuffer[outputBufferSize*2];
	int outputBufferCount = 0;
	for (int i = 0;i<outputBufferSize;i++) {
		int aoBufferValue = outputBuffer[i]*volume*(float)(1<<16);

		// set frame value into output buffer to be played later on
		// use unsigned 16bits, little endian (U16LE)
		assert (outputBufferCount  < outputBufferSize*2);
		playBuffer[outputBufferCount++] = (uint8_t)(aoBufferValue & 0xFF);
		assert (outputBufferCount < outputBufferSize*2);
		playBuffer[outputBufferCount++] = (uint8_t)(aoBufferValue >> 8);
	}

	// play the buffer of hopSize asynchronously
	ao_play(outputDevice, playBuffer, outputBufferSize*2);
}
