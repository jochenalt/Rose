/*
 * MicrophoneInput.h
 *
 *  Created on: Mar 22, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIO_MICROPHONEINPUT_H_
#define SRC_AUDIO_MICROPHONEINPUT_H_

using namespace std;

// microphone uses a sample rate
const int MicrophoneSampleRate = 22050;

class MicrophoneInput {
public:
	MicrophoneInput();
	virtual ~MicrophoneInput();
	void setup(int sampleRate);
	int readMicrophoneInput(double buffer[], unsigned BufferSize);
	double getMicrophoneLatency();
private:
};

#endif /* SRC_AUDIO_MICROPHONEINPUT_H_ */
