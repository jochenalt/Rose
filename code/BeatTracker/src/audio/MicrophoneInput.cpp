/*
 * MicrophoneInput.cpp
 *
 *  Created on: Mar 22, 2018
 *      Author: jochenalt
 */

#include <iostream>
#include <string.h>
#include <audio/MicrophoneInput.h>

MicrophoneInput::MicrophoneInput() {
}

MicrophoneInput::~MicrophoneInput() {
    if (pulseAudioConnection)
        pa_simple_free(pulseAudioConnection);
}


void MicrophoneInput::setup(int samplerate) {

    // define microphone input connection format
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = MicrophoneSampleRate,
        .channels = 2
    };

    int error = 0;
    // Create the recording stream
    if (!(pulseAudioConnection = pa_simple_new(NULL, "Donna", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
        cout << "could not open microphone via pa_simple_new err=" << error;
        exit(1);
    }
}



int MicrophoneInput::readMicrophoneInput(float buffer[], unsigned BufferSize) {
    	const unsigned InputBufferSize = BufferSize*4;
        uint8_t inputBuffer[InputBufferSize];

        // record data from microphone connection
        int error;
        if (pa_simple_read(pulseAudioConnection, inputBuffer, InputBufferSize, &error) < 0) {
            cerr << "reading microphone input: pa_simple_read failed: %i\n" << error << endl;
            exit(1);
        }

        int bits = 16;
        int outBufferSize = 0;
        // decode buffer in PA_SAMPLE_S16LE format
        for (unsigned i = 0;i<InputBufferSize;i+=4) {
        	float inputSample1 = (inputBuffer[i+1] << 8) + (inputBuffer[i]);
        	if (inputSample1 > (1<<(bits-1)))
        		inputSample1 -= (1<<bits);
        	inputSample1 /= (float)(1<<bits);
        	float inputSample2 = (inputBuffer[i+3] << 8) + (inputBuffer[i+2]);
        	if (inputSample2 > (1<<(bits-1)))
        		inputSample2 -= (1<<bits);
        	inputSample2 /= (float)(1<<bits);

        	buffer[outBufferSize++] = (inputSample2 + inputSample1)/2.0;
        }
        return outBufferSize;
}
