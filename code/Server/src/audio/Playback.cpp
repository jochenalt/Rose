/*
 * Playback.cpp
 *
 *  Created on: Mar 20, 2018
 *      Author: jochenalt
 */


#include <assert.h>
#include <iostream>
#include <string.h>
#include <audio/Playback.h>
#include <audio/SoundCardUtils.h>

using namespace std;

Playback::Playback() {
	pulseAudioConnection = NULL;
}

Playback::~Playback() {
	if (pulseAudioConnection != NULL) {
		 pa_simple_free(pulseAudioConnection);
		 pulseAudioConnection = NULL;
	}
}


void Playback::setup(int sampleRate) {

    /* The Sample format to use */
    ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = (uint32_t)sampleRate,
        .channels = 1
    };

    int error = 0;

    deviceName = SoundCardUtils::getInstance().getDefaultOutputDevice().name;
    pulseAudioConnection = pa_simple_new(NULL, "Donna", PA_STREAM_PLAYBACK, deviceName.c_str(), "playback", &ss, NULL, NULL, &error);
    if (pulseAudioConnection == NULL) {
        cerr << "ERR: pa_simple_new on " << deviceName << " failed: " <<  pa_strerror(error);
        exit(1);
    }

   	cout << "using device " << deviceName << " for audio playback " << sampleRate << "Hz for audio output " << endl;
}

void Playback::play(double volume /* 0..1 */ ,float outputBuffer[], int outputBufferSize) {
	if (!playback)
		return;

	char playBuffer[outputBufferSize*2];
	int outputBufferCount = 0;
	for (int i = 0;i<outputBufferSize;i++) {
		int sampleValue = outputBuffer[i]*volume*(float)(1<<16);

		// set frame value into output buffer to be played later on
		// use unsigned 16bits, little endian (U16LE)
		assert (outputBufferCount  < outputBufferSize*2);
		playBuffer[outputBufferCount++] = (uint8_t)(sampleValue & 0xFF);
		assert (outputBufferCount < outputBufferSize*2);
		playBuffer[outputBufferCount++] = (uint8_t)(sampleValue >> 8);
	}

	int error = 0;
    int bytesWritten = pa_simple_write(pulseAudioConnection, playBuffer, (size_t) outputBufferCount, &error);
    if (bytesWritten < 0) {
        cerr << "ERR: pa_simple_write failed: " << pa_strerror(error);
        exit(1);
    }
}
