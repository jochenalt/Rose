/*
 * MicrophoneInput.h
 *
 *  Created on: Mar 22, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIO_MICROPHONEINPUT_H_
#define SRC_AUDIO_MICROPHONEINPUT_H_

using namespace std;

class MicrophoneInput {
public:
	MicrophoneInput();
	virtual ~MicrophoneInput();
	void setup(int sampleRate);
	bool readMicrophoneInput(double buffer[], unsigned BufferSize);
private:
};

#endif /* SRC_AUDIO_MICROPHONEINPUT_H_ */
