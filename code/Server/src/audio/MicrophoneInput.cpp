/*
 * MicrophoneInput.cpp
 *
 *  Created on: Mar 22, 2018
 *      Author: jochenalt
 */

#include <iostream>
#include <string.h>
#include <audio/MicrophoneInput.h>
#include <audio/SoundCardUtils.h>

#include <pulse/simple.h>

MicrophoneInput::MicrophoneInput() {
}

MicrophoneInput::~MicrophoneInput() {
    if (pulseAudioConnection) {
        pa_simple_free(pulseAudioConnection);
        pulseAudioConnection = NULL;
    }
}


void MicrophoneInput::setup(int samplerate) {

    // define microphone input connection format
    ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = MicrophoneSampleRate,
        .channels = 1
    };

    // "alsa_output.usb-C-Media_Electronics_Inc._USB_PnP_Sound_Device-00.analog-stereo.monitor";
    deviceName = SoundCardUtils::getInstance().getDefaultInputDevice().name;
    int error = 0;
    // Create the recording stream
    if (!(pulseAudioConnection = pa_simple_new(NULL, "Donna", PA_STREAM_RECORD, deviceName.c_str(), "record", &ss, NULL, NULL, &error))) {
        cout << "could not open microphone in " << deviceName << " via pa_simple_new err=" << error;
        exit(1);
    }

    microphoneLatency = pa_simple_get_latency(pulseAudioConnection, &error)/1000.0;
    cout << "using device " << deviceName << " for audio microphone input with " << MicrophoneSampleRate << "Hz and latency of " << microphoneLatency << "ms" << endl;
}


int MicrophoneInput::readMicrophoneInput(float buffer[], unsigned BufferSize) {
    	const unsigned InputBufferSize = BufferSize*2;
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
        for (unsigned i = 0;i<InputBufferSize;i+=2) {
        	float inputSample = (inputBuffer[i+1] << 8) + (inputBuffer[i]);
        	if (inputSample > (1<<(bits-1)))
        	 	inputSample -= (1<<bits);
        	inputSample /= (float)(1<<bits);
        	buffer[outBufferSize++] = inputSample;
        }
        return outBufferSize;
}
