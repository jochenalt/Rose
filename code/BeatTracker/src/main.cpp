#include <iostream>

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
#include <dance/Dancer.h>

#include <basics/util.h>
#include <BTrack/BTrack.h>
#include <AudioFile/AudioFile.h>
#include <ui/UI.h>
#include "dance/RhythmDetector.h"
#include <Stewart/BodyKinematics.h>
#include <servo/PCA9685Servo.h>
#include <servo/ServoController.h>
#include <webserver/Webserver.h>
#include <client/BotClient.h>

#ifdef __linux__
#include <ao/ao.h>
#endif

using namespace std;

bool runUI = false;
bool playback = true;


string getCmdOption(char ** begin, int argc, int i ) {
	assert ((i>=0) && (i<argc));
	char** arg = begin + i;
	return string(*arg);
}

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
	     << "            [-port <port>]       # set port of webserver if different from 8080" << endl
	     << "            [-host <host>]       # define this process as client accessing this webserver" << endl
		 << "            [-webroot <path>]    # set path of ./webroot" << endl
		 << "            [-v <volume 0..100>] # set volume between 0 and 100" << endl
		 << "            [-ui]                # start visualizer" << endl
		 << "            [-s]                 # silent, do not play audio" << endl
		 << "            [-i <n>]# start after n detected beats" << endl
	     << "            [-t]                 # servo calibration via keyboard" << endl;
}

typedef void (*BeatCallbackFct)(bool beat, double Bpm);

#ifdef __linux__
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
    ao_device* outputDevice = NULL;
    if (playback) {
		outputDevice = ao_open_live(defaultDriverHandle, &audioOutputFormat, NULL /* no options */);
		if (outputDevice == NULL) {
			 cerr << "Could not open sound deviceError opening sound device" << endl;
			 exit(1);
		}
    }
	int hopSize = 128;
	int frameSize = hopSize*8; // cpu load goes up linear with the framesize
	BTrack b(hopSize, frameSize);

	// position within the input buffer
	int posInputSamples = 0;
	double elapsedTime = 0;
	uint32_t startTime_ms = millis();

	while (posInputSamples < numSamples) {

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
					inputSampleValue = (audioFile.samples[0][posInputSamples + i]+audioFile.samples[1][posInputSamples + i])/2.0;
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
			if (playback)
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

			delay_ms((elapsedFrameTime - elapsedTime)*1000.0);
		} else {
			// last frame not sufficient for a complete hop
			cout << "end of song" << endl;
		}
	}

	// close audio output
	if (playback) {
		ao_close(outputDevice);
		ao_shutdown();
	}
}


#endif


void signalHandler(int s){
#ifdef __linux__
	changemode(0);
#endif
	cout << "Signal " << s << ". Exiting";
    if (runUI) {
    	UI::getInstance().tearDown();
    }
	cout.flush();
	exit(1);
}



void sendBeatToRythmDetector(bool beat, double bpm) {
	RhythmDetector & rd = RhythmDetector::getInstance();
	Dancer & mm = Dancer::getInstance();

	rd.loop(beat, bpm);
	mm.danceLoop(beat, bpm);
	if (runUI) {
		UI::getInstance().setBodyPose(mm.getBodyPose(), mm.getHeadPose());
	}
}

void sendDanceToClient(bool beat, double bpm) {
	Dancer & mm = Dancer::getInstance();
	BotClient& client = BotClient::getInstance();

	// fetch cached data from webserver  and set into Dance machine
	mm.imposeDanceParams(client.getMove(), client.getAmbition(), client.getBodyPose(), client.getHeadPose());

	// send data to ui
	if (runUI) {
		UI::getInstance().setBodyPose(mm.getBodyPose(), mm.getHeadPose());
	}
}

typedef void (*MoveCallbackFct)(bool beat, double Bpm);


int main(int argc, char *argv[]) {
	// exit correctly when exception arises
	std::set_terminate([](){
		std::cout << "Unhandled exception\n"; std::abort();
	    if (runUI) {
	    	UI::getInstance().tearDown();
	    }
		changemode(0);
	});

	// catch SIGINT (ctrl-C)
    signal (SIGINT,signalHandler);

    // currently played filename
    string trackFilename;

    // default volume
    int volumeArg = 20;

    // default latency of the moves
    int startAfterNBeats = 4;

    // if we run a webserver, this is the path where static content is stored
    string webrootPath = string(argv[0]);
	int idx = webrootPath.find_last_of("/");
	webrootPath = webrootPath.substr(0,idx) + "/webroot";

	// dont run the engine on yourself but call the webserver
	bool isWebClient = false;

	// run a webserver to serve a client
	bool isWebServer = true;

	// if client, this is the host of the webserver
	string webclientHost = "127.0.0.1";

    int webserverPort = 8080;
    for (int i = 1;i<argc;i++) {
    	string arg = getCmdOption(argv, argc,i);
    	if (arg == "-f") {
    		if (i+1 >= argc) {
    			cerr << "-f requires a filename" << endl;
    			exit(1);
    		}
    		trackFilename = getCmdOption(argv, argc, i+1);
    		i++;
    	} else if (arg == "-h") {
    	    	printUsage();
#ifdef __linux__
    	} else if (arg == "-t") {
	    	ServoController::getInstance().calibrateViaKeyBoard();
#endif
	    } else if (arg == "-s") {
	    	playback = false;
	    } else if (arg == "-port") {
    		if (i+1 >= argc) {
    			cerr << "-port requires a number 0..100" << endl;
    			exit(1);
    		}
	    	i++;
	    	bool ok = true;
	    	webserverPort = -1;
	    	webserverPort = stringToInt(getCmdOption(argv, argc, i), ok);
	    	if ((webserverPort < 1000) || (webserverPort > 9999)) {
	    		cerr << "port should be between 1000..9999" << endl;
	    		exit(1);
	    	}
	    } else if (arg == "-client") {
	    		isWebServer= false;
	    		isWebClient = true;
	    } else if (arg == "-host") {
    		if (i+1 >= argc) {
    			cerr << "-host requires a string like 127.0.0.1" << endl;
    			exit(1);
    		}
	    	i++;
	    	webclientHost = getCmdOption(argv, argc, i);
	    } else if (arg == "-webroot") {
    		if (i+1 >= argc) {
    			cerr << "-webroot required a path, e.g. " << argv[0] << "/webroot" << endl;
    			exit(1);
    		}
    		i++;
	    	webrootPath = getCmdOption(argv, argc, i);
	    } else if (arg == "-v") {
    		if (i+1 >= argc) {
    			cerr << "-v requires a number 0..100" << endl;
    			exit(1);
    		}
    		arg = getCmdOption(argv, argc, i+1);
    		i++;
        	volumeArg  = atoi(arg.c_str());
        	if ((volumeArg < 0) || (volumeArg > 100))
        	{
        		cerr << "volume (" << volumeArg << ") has to be within [0..100]" << endl;
        		exit(1);
        	}
    	} else if (arg == "-i") {
    		if (i+1 >= argc) {
    			cerr << "-i requires a number" << endl;
    			exit(1);
    		}
    		arg = getCmdOption(argv, argc, i+1);
    		i++;
    		startAfterNBeats  = atoi(arg.c_str());
    		if (startAfterNBeats <2) {
    			cerr << "-i requires a number >=2" << endl;
    			exit(1);
    		}
    	} else if (arg == "-ui") {
    	    runUI = true;
    	} else {
    		cerr << "unknown option " << arg << endl;
    		exit(1);
    	}

    }

    if (isWebServer) {
    	Webserver::getInstance().setup(webserverPort, webrootPath);
    }
    if (isWebClient) {
    	BotClient::getInstance().setup(webclientHost, webserverPort);
    }
	BodyKinematics::getInstance().setup();
    Dancer::getInstance().setup();
    RhythmDetector::getInstance().setup();

    Dancer::getInstance().setStartAfterNBeats(startAfterNBeats);
#ifdef __linux__
    if (isWebServer) {
    	ServoController::getInstance().setup();
    }
#endif
    if (runUI) {
    	UI::getInstance().setup(argc,argv);
    }

#ifdef __linux__
    if (isWebServer)
    	processAudioFile(trackFilename, volumeArg/100.0, sendBeatToRythmDetector);
#endif
    if (isWebClient) {
		Dancer & mm = Dancer::getInstance();
		BotClient& client = BotClient::getInstance();
		TimeSamplerStatic clientLoopTimer;
    	while (true) {
    		if (clientLoopTimer.isDue(20)) {
    			client.getStatus();

				// fetch cached data from webserver  and set into Dance machine
				mm.imposeDanceParams(client.getMove(), client.getAmbition(), client.getBodyPose(), client.getHeadPose());

				// send data to ui
				if (runUI) {
					UI::getInstance().setBodyPose(mm.getBodyPose(), mm.getHeadPose());
				}

    		}
    		else
    			delay_ms(1);
    	}
    }

    if (runUI)
    	UI::getInstance().tearDown();

}
