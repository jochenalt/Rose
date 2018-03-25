/*
 * MicrophoneInput.h
 *
 *  Created on: Mar 22, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIO_MICROPHONEINPUT_H_
#define SRC_AUDIO_MICROPHONEINPUT_H_

#include <pulse/simple.h>
#include <pulse/error.h>

using namespace std;

// with microphone we use a standard sample rate
const int MicrophoneSampleRate = 44100;
const float MicrophoneLatency = 0.7; // [s]

class MicrophoneInput {
public:
	MicrophoneInput();
	virtual ~MicrophoneInput();
	void setup(int sampleRate);
	int readMicrophoneInput(float buffer[], unsigned BufferSize);

private:
	pa_simple *pulseAudioConnection = NULL;

};

#endif /* SRC_AUDIO_MICROPHONEINPUT_H_ */
