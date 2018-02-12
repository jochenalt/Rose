//============================================================================
// Name        : Tracker.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include <stdlib.h>
#include <chrono>
#include <unistd.h>
#include <iomanip>
#include <thread>

#include "util.h"
#include "BTrack.h"
#include "AudioFile.h"
#include <ao/ao.h>

using namespace std;


void printUsage() {
	cout << "BeatTracker -f <wav.file> [-h] [-v <volume 0..100>]" << endl;
}

void processAudioFile (string trackFilename, double volume /* [0..1] */) {
    // load input file
	AudioFile<double> audioFile;
	audioFile.load (trackFilename);
	int sampleRate = audioFile.getSampleRate();
	int numSamples = audioFile.getNumSamplesPerChannel();
	int numInputChannels = audioFile.getNumChannels();
	audioFile.printSummary();

	// initialize output device
    ao_initialize();
	ao_sample_format audioOutputFormat;
    memset(&audioOutputFormat, 0, sizeof(audioOutputFormat));
    audioOutputFormat.bits = 16;
    audioOutputFormat.channels = 1;
    audioOutputFormat.rate = sampleRate;
    audioOutputFormat.byte_format = AO_FMT_LITTLE;

    int defaultDriverHandle = ao_default_driver_id();
    ao_device* outputDevice = ao_open_live(defaultDriverHandle, &audioOutputFormat, NULL /* no options */);
    if (outputDevice == NULL) {
    	 cerr << "Could not open sound deviceError opening sound device" << endl;
         exit(1);
    }

	int hopSize = 256;
	int frameSize = 4096; // cpu load goes up linear with the framesize
	BTrack b(hopSize, frameSize);

	// position within the input buffer
	int posInputSamples = 0;
	double elapsedTime = 0;
	uint32_t startTime_ms = millis();

	while (posInputSamples < numSamples) {

		double frame[hopSize];
		int numInputSamples = min(hopSize, numSamples);
	    char outputBuffer[numInputSamples*audioOutputFormat.channels*(audioOutputFormat.bits/8)];

	    int outputBufferCount = 0;
		double outputVolumeScaler = (1<<15)*volume;

		for (int i = 0; i < numInputSamples; i++)
		{
			double inputSampleValue;
			switch (numInputChannels) {
			case 1:
				inputSampleValue= audioFile.samples[0][posInputSamples + i];
				break;
			case 2:
				inputSampleValue = (audioFile.samples[0][posInputSamples + i]+audioFile.samples[1][posInputSamples + i])/2.0;
				break;
			default:
				 inputSampleValue = 0;
				for (int j = 0;j<numInputChannels;j++)
					inputSampleValue += audioFile.samples[j][posInputSamples + i];
				inputSampleValue = inputSampleValue / numInputChannels;
			}
			frame[i] = inputSampleValue;

			// set frame value into output buffer to be played later on
			unsigned aoBufferValue = frame[i]*outputVolumeScaler;
			outputBuffer[outputBufferCount++] = (uint8_t)(aoBufferValue & 0xFF);
			outputBuffer[outputBufferCount++] = (uint8_t)(aoBufferValue >> 8);
		}

		posInputSamples += numInputSamples;

		// play the buffer of hopSize asynchronously
		ao_play(outputDevice, outputBuffer, outputBufferCount);

		// detect beat and bpm of that hop size
		b.processAudioFrame(frame);

		// insert a delay to synchronize played audio and beat detection
		elapsedTime = ((double)(millis() - startTime_ms)) / 1000.0f;
		double elapsedFrameTime = (double)posInputSamples / (double)sampleRate;
		delay_ms((elapsedFrameTime - elapsedTime)*1000.0);

		if (b.beatDueInCurrentFrame())
		{
			cout << std::fixed << std::setprecision(1) << "Beat (" << b.getCurrentTempoEstimate() << ")" << std::setprecision(1) << (elapsedFrameTime) << "s" << endl;
		};
	}

	// close audio output
    ao_close(outputDevice);
    ao_shutdown();

}

int main(int argc, char *argv[]) {
    char * arg = getCmdOption(argv, argv + argc, "-f");
    string trackFilename;
    if(arg != NULL) {
    	trackFilename = string(arg);
    }
    arg = getCmdOption(argv, argv + argc, "-h");
    if(arg != NULL) {
    	printUsage();
    }

    arg = getCmdOption(argv, argv + argc, "-v");
    int volumeArg = 20;
    if(arg != NULL) {
    	volumeArg  = atoi(arg);
    	if ((volumeArg < 0) || (volumeArg > 100))
    	{
    		cerr << "volume (" << volumeArg << ") has to be within [0..100]" << endl;
    		exit(1);
    	}
    }


    processAudioFile(trackFilename, volumeArg/100.0);
}
