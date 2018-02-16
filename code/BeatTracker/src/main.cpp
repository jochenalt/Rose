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
#include <string>
#include <unistd.h>
#include <iomanip>
#include <algorithm>
#include <ao/ao.h>

#include "basics/util.h"
#include "basics/logger.h"
#include "BTrack.h"
#include "AudioFile.h"
#include "UI.h"
#include "MoveMaker.h"
#include "RhythmDetector.h"


INITIALIZE_EASYLOGGINGPP

using namespace std;


char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}


void printUsage() {
	cout << "BeatTracker -f <wav.file>        # define the track to be played" << endl
	     << "            [-h]                 # print this" << endl
		 << "            [-v <volume 0..100>] # set volume between 0 and 100" << endl
		 << "            [-ui]                # start visualizer" << endl;
}



typedef void (*BeatCallbackFct)(bool beat, double Bpm);

void processAudioFile (string trackFilename, double volume /* [0..1] */, BeatCallbackFct beatCallback) {
    // load input filec, char *argv
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

	int hopSize = 512;
	int frameSize = hopSize*4; // cpu load goes up linear with the framesize
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


		bool beat = b.beatDueInCurrentFrame();
		double bpm = b.getCurrentTempoEstimate();

		// insert a delay to synchronize played audio and beat detection
		elapsedTime = ((double)(millis() - startTime_ms)) / 1000.0f;
		double elapsedFrameTime = (double)posInputSamples / (double)sampleRate;

		beatCallback(beat, bpm);

		if (beat)
		{
			cout << std::fixed << std::setprecision(1) << "Beat (" << b.getCurrentTempoEstimate() << ")" << std::setprecision(1) << (elapsedFrameTime) << "s" << endl;
		};

		delay_ms((elapsedFrameTime - elapsedTime)*1000.0);

	}

	// close audio output
    ao_close(outputDevice);
    ao_shutdown();

}


bool exitMode = false;

void signalHandler(int s){
	exitMode = true;
	cout << "Signal " << s << ". Exiting";
	cout.flush();
	exit(1);
}


void setupLogging(int argc, char *argv[]) {
	// catch SIGINT (ctrl-C)
    signal (SIGINT,signalHandler);

	// setup logger
	el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.set(el::Level::Error,el::ConfigurationType::Format, "%datetime %level [%func] [%loc] %msg");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Filename, "logs/manfred.log");

    defaultConf.set(el::Level::Info,el::ConfigurationType::Format, "%datetime %level %msg");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Filename, "logs/manfred.log");

    defaultConf.set(el::Level::Debug, el::ConfigurationType::ToStandardOutput,std::string("false"));
    // defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled,std::string("false"));

    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, std::string("%datetime %level [%func] [%loc] %msg"));
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Filename, "logs/manfred.log");

    // logging from uC is on level Trace
    defaultConf.set(el::Level::Trace, el::ConfigurationType::ToStandardOutput,std::string("false"));
    defaultConf.set(el::Level::Trace, el::ConfigurationType::Format, std::string("%datetime %level [uC] %msg"));
    defaultConf.set(el::Level::Trace, el::ConfigurationType::Filename, "logs/dancer.log");

    el::Loggers::reconfigureLogger("default", defaultConf);

    LOG(INFO) << "Private Dancer Setup";
}


bool runUI = false;

void sendBeatToRythmDetector(bool beat, double bpm) {
	RhythmDetector & rd = RhythmDetector::getInstance();
	MoveMaker& mm = MoveMaker::getInstance();

	rd.loop(beat, bpm);
	mm.loop(beat, bpm);
	if (runUI)
		UI::getInstance().setBodyPose(mm.getBodyPose());
}

typedef void (*MoveCallbackFct)(bool beat, double Bpm);


int main(int argc, char *argv[]) {
	// print help
	std::set_terminate([](){
		std::cout << "Unhandled exception\n"; std::abort();
	});

	// initialize Logging
	setupLogging(argc, argv);


	char * arg = getCmdOption(argv, argv + argc, "-f");
    string trackFilename;
    if(arg != NULL) {
    	trackFilename = string(arg);
    }

    if (cmdOptionExists(argv, argv + argc, "-h")) {
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

    runUI = cmdOptionExists(argv, argv + argc, "-ui");

    if (runUI)
    	UI::getInstance().setup(argc,argv);

    MoveMaker::getInstance().setup();
    RhythmDetector::getInstance().setup();

    processAudioFile(trackFilename, volumeArg/100.0, sendBeatToRythmDetector);
}
