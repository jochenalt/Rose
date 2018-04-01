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
const int MicrophoneSampleRate = 22050;

class MicrophoneInput {
public:
	MicrophoneInput();
	virtual ~MicrophoneInput();
	void setup(int sampleRate);
	int readMicrophoneInput(double buffer[], unsigned BufferSize);
	double getMicrophoneLatency() { return 0.6; };
private:
};

#endif /* SRC_AUDIO_MICROPHONEINPUT_H_ */
