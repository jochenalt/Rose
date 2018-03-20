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

#include <ao/ao.h>

#include <basics/stringhelper.h>
#include <basics/util.h>
#include <AudioProcessor.h>
#include <AudioFile/AudioFile.h>
#include <BTrack/BTrack.h>

using namespace std;

AudioProcessor::AudioProcessor() {

}

AudioProcessor::~AudioProcessor() {

	ao_shutdown();
}

void AudioProcessor::setup(BeatCallbackFct newBeatCallback) {
    ao_initialize();

    beatCallback = newBeatCallback;
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
}

void AudioProcessor::processWav() {
	stopCurrProcessing = false;
	currProcessingStopped = false;
    // load input filec, char *argv
	AudioFile<double> audioFile;
	audioFile.decodeWaveFile(wavData);
	int sampleRate = audioFile.getSampleRate();
	int numSamples = audioFile.getNumSamplesPerChannel();
	int numInputChannels = audioFile.getNumChannels();
	audioFile.printSummary();

	// initialize output device
	ao_sample_format audioOutputFormat;
	memset(&audioOutputFormat, 0, sizeof(audioOutputFormat));
	audioOutputFormat.bits = 16;
	audioOutputFormat.channels = 1;
	audioOutputFormat.rate = sampleRate;
	audioOutputFormat.byte_format = AO_FMT_LITTLE;
	int defaultDriverHandle = ao_default_driver_id();
	outputDevice = NULL;
	outputDevice = ao_open_live(defaultDriverHandle, &audioOutputFormat, NULL /* no options */);
	if (outputDevice == NULL) {
		cerr << "Could not open sound deviceError opening sound device" << endl;
		exit(1);
	}
	int hopSize = 128;
	int frameSize = hopSize*8; // cpu load goes up linear with the framesize
	BTrack b(hopSize, frameSize);

	// position within the input buffer
	int posInputSamples = 0;
	double elapsedTime = 0;
	uint32_t startTime_ms = millis();
	while (!stopCurrProcessing && (posInputSamples < numSamples)) {

		double frame[hopSize];
		int numInputSamples = min(hopSize, numSamples-posInputSamples);
		int outputBufferSize = numInputSamples*audioOutputFormat.channels*(audioOutputFormat.bits/8);
	    char outputBuffer[outputBufferSize];

	    int outputBufferCount = 0;
		double outputVolumeScaler = (1<<15)*volume;

		// process only, if the sample is big enough for a complete hop
		if (numInputSamples == hopSize) {
			for (int i = 0; i < numInputSamples; i++)
			{
				double inputSampleValue;
				assert(posInputSamples+1 < numSamples);
				switch (numInputChannels) {
				case 1:
					inputSampleValue= audioFile.samples[0][posInputSamples + i];
					break;
				case 2:
					inputSampleValue = (audioFile.samples[0][posInputSamples + i]+audioFile.samples[1][posInputSamples + i]);
					break;
				default:
					inputSampleValue = 0;
					for (int j = 0;j<numInputChannels;j++)
						inputSampleValue += audioFile.samples[j][posInputSamples + i];
					inputSampleValue = inputSampleValue / numInputChannels;
				}
				assert(i<hopSize);
				frame[i] = inputSampleValue;
				// set frame value into output buffer to be played later on
				unsigned aoBufferValue = frame[i]*outputVolumeScaler;
				assert (outputBufferCount  < outputBufferSize);
				outputBuffer[outputBufferCount] = (uint8_t)(aoBufferValue & 0xFF);
				outputBufferCount++;
				assert (outputBufferCount < outputBufferSize);
				outputBuffer[outputBufferCount] = (uint8_t)(aoBufferValue >> 8);
				outputBufferCount++;
			}
			posInputSamples += numInputSamples;

			// play the buffer of hopSize asynchronously
			if (withPlayback)
				ao_play(outputDevice, outputBuffer, outputBufferCount);
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
	ao_close(outputDevice);
	currProcessingStopped = true;
}
