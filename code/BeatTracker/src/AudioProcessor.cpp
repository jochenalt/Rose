/*
 * AudioProcessor.cpp
 *
 *  Created on: Mar 18, 2018
 *      Author: jochenalt
 */

#include <iostream>
#include <string.h>

#include <stdlib.h>
#include <signal.h>
#include <chrono>
#include <unistd.h>
#include <iomanip>
#include <thread>
#include <string>
#include <unistd.h>
#include <iomanip>
#include <algorithm>

#include <basics/stringhelper.h>
#include <basics/util.h>
#include <AudioProcessor.h>
#include <AudioFile/AudioFile.h>
#include <Playback.h>

#include <BTrack/BTrack.h>

using namespace std;

AudioProcessor::AudioProcessor() {
}

AudioProcessor::~AudioProcessor() {
    if (pulseAudioConnection)
        pa_simple_free(pulseAudioConnection);
}

const int MicrophoneSampleRate = 44100;

void AudioProcessor::setup(BeatCallbackFct newBeatCallback) {
    beatCallback = newBeatCallback;
	playback.setup(MicrophoneSampleRate);

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

void AudioProcessor::setVolume(double newVolume) {
	volume = newVolume;
}
double AudioProcessor::getVolume() {
	return volume;
}

void AudioProcessor::setPlayback(bool ok) {
	withPlayback = ok;
}

bool AudioProcessor::getPlayback() {
	return withPlayback;
}

void AudioProcessor::setWavContent(std::vector<uint8_t>& newWavData) {
	stopCurrProcessing = true;
	while (!currProcessingStopped)
		delay_ms(1);
	wavData = newWavData;
	audioFile.decodeWaveFile(wavData);
}

void AudioProcessor::setMicrophoneInput() {
	stopCurrProcessing = true;
	while (!currProcessingStopped)
		delay_ms(1);
	wavData.clear();
}

int AudioProcessor::readWavInput(float buffer[], unsigned BufferSize) {
	int numSamples = audioFile.getNumSamplesPerChannel();
	int numInputSamples = min((int)BufferSize, (int)numSamples-posInputSamples);
	int numInputChannels = audioFile.getNumChannels();

	int bufferCount = 0;
	for (int i = 0; i < numInputSamples; i++)
	{
		double inputSampleValue = 0;
		assert(posInputSamples+1 < numSamples);
		switch (numInputChannels) {
		case 1:
			inputSampleValue= audioFile.samples[0][posInputSamples + i];
			break;
		case 2:
			inputSampleValue = (audioFile.samples[0][posInputSamples + i]+audioFile.samples[1][posInputSamples + i])/2;
			break;
		default:
			inputSampleValue = 0;
			for (int j = 0;j<numInputChannels;j++)
				inputSampleValue += audioFile.samples[j][posInputSamples + i];
			inputSampleValue = inputSampleValue / numInputChannels;
		}
		buffer[bufferCount++] = inputSampleValue;
	}
	return bufferCount;
}

void AudioProcessor::processWav() {
	stopCurrProcessing = false;
	currProcessingStopped = false;

	int sampleRate = audioFile.getSampleRate();
	int numSamples = audioFile.getNumSamplesPerChannel();
	audioFile.printSummary();

	if (withPlayback)
		playback.setup(sampleRate);

	int hopSize = 128;
	int frameSize = hopSize*8; // cpu load goes up linear with the framesize
	BTrack b(hopSize, frameSize);

	// position within the input buffer
	posInputSamples = 0;
	double elapsedTime = 0;
	uint32_t startTime_ms = millis();
	while (!stopCurrProcessing && (posInputSamples < numSamples)) {

		double frame[hopSize];
		int numInputSamples = min(hopSize, numSamples-posInputSamples);
		int playbackBufferSize = numInputSamples;
	    float playbackBuffer[playbackBufferSize];
	    int playbackBufferCount = 0;

		// process only, if the sample is big enough for a complete hop
		if (numInputSamples == hopSize) {
			float inputBuffer[numInputSamples];
			int readFrames = readWavInput(inputBuffer, numInputSamples);
			for (int i = 0; i < readFrames; i++)
			{
				double inputSampleValue = inputBuffer[i];
				assert(i<hopSize);
				frame[i] = inputSampleValue;
				// set frame value into output buffer to be played later on
				assert (playbackBufferCount  < playbackBufferSize);
				playbackBuffer[playbackBufferCount++] = inputSampleValue;
			}
			posInputSamples += numInputSamples;

			// play the buffer of hopSize asynchronously
			if (withPlayback)
				playback.playbackSample(volume, playbackBuffer,playbackBufferCount);

			// detect beat and bpm of that hop size
			b.processAudioFrame(frame);

			bool beat = b.beatDueInCurrentFrame();
			double bpm = b.getCurrentTempoEstimate();

			// insert a delay to synchronize played audio and beat detection
			elapsedTime = ((double)(millis() - startTime_ms)) / 1000.0f;
			double elapsedFrameTime = (double)posInputSamples / (double)sampleRate;

			beatCallback(beat, bpm);

			if (beat)
			{
				cout << std::fixed << std::setprecision(2) << "Beat (" << b.getCurrentTempoEstimate() << ")" << std::setprecision(2) << (elapsedFrameTime) << "s" << endl;
			};
			// wait until next frame set is due
			delay_ms((elapsedFrameTime - elapsedTime)*1000.0);
		} else {
			// last frame not sufficient for a complete hop
			cout << "end of song" << endl;

			// skip the last incomplete frame
			posInputSamples  = numSamples;
		}
	}
	currProcessingStopped = true;
}


int AudioProcessor::readMicrophoneInput(float buffer[], unsigned BufferSize) {
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

void AudioProcessor::processMicrophoneInput() {
	stopCurrProcessing = false;
	currProcessingStopped = false;
	if (withPlayback)
		playback.setup(MicrophoneSampleRate);

	int hopSize = 128;
	int frameSize = hopSize*8; // cpu load goes up linear with the framesize
	BTrack b(hopSize, frameSize);

	// position within the input buffer
	int posInputSamples = 0;
	while (!stopCurrProcessing) {

		double frame[hopSize];
		int numInputSamples = hopSize;

		float microphoneBuffer[numInputSamples];
		int readSamples = readMicrophoneInput(microphoneBuffer, hopSize);
		if (readSamples != hopSize)
			cerr << "not enough samples from microphone" << endl;

		int playbackBufferSize = numInputSamples;
	    float playbackBuffer[playbackBufferSize];
	    int playbackBufferCount = 0;

		for (int i = 0; i < numInputSamples; i++)
		{
			double inputSampleValue = 0;
			inputSampleValue= microphoneBuffer[i];

			frame[i] = inputSampleValue;
			// set frame value into output buffer to be played later on
			assert (playbackBufferCount  < playbackBufferSize);
			playbackBuffer[playbackBufferCount++] = inputSampleValue;
		}
		posInputSamples += numInputSamples;

		// play the buffer of hopSize asynchronously
		if (withPlayback)
			playback.playbackSample(volume, playbackBuffer,playbackBufferCount);

		// detect beat and bpm of that hop size
		b.processAudioFrame(frame);

		bool beat = b.beatDueInCurrentFrame();
		double bpm = b.getCurrentTempoEstimate();

		// insert a delay to synchronize played audio and beat detection
		double elapsedFrameTime = (double)posInputSamples / (float)MicrophoneSampleRate;

		beatCallback(beat, bpm);

		if (beat)
		{
			cout << std::fixed << std::setprecision(2) << "Beat (" << b.getCurrentTempoEstimate() << ")" << std::setprecision(2) << (elapsedFrameTime) << "s" << endl;
		};
		// wait until next frame set is due
		// delay_ms((elapsedFrameTime - elapsedTime)*1000.0);
	}
	currProcessingStopped = true;
}


