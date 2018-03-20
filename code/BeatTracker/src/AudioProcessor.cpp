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

}

void AudioProcessor::setup(BeatCallbackFct newBeatCallback) {
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

int getSample(bool signedSample, bool littleEndian, int bits, char sample[]) {
	int bytes = bits/8;
	int totSample = 0;
	if (littleEndian) {
		for (int i = 0;i<bytes;i++) {
			totSample = (totSample << 8)  + sample[i];
		}
	}
	else {
		for (int i = bytes-1;i>=0;i--) {
			totSample = (totSample << 8)  + sample[i];
		}
	}
	if (signedSample && (totSample > (1<<(bits-1))))
		totSample -= (1<<bits);
	return totSample;
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

	if (withPlayback)
		playback.setup(sampleRate);

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
		int playbackBufferSize = numInputSamples;
	    int playbackBuffer[playbackBufferSize];
	    int playbackBufferCount = 0;

		// process only, if the sample is big enough for a complete hop
		if (numInputSamples == hopSize) {
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
				assert(i<hopSize);
				frame[i] = inputSampleValue;
				// set frame value into output buffer to be played later on
				assert (playbackBufferCount  < playbackBufferSize);
				playbackBuffer[playbackBufferCount++] = inputSampleValue*(1<<15);
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
