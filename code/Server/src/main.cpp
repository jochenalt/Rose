#include <iostream>
#include <thread>
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
#include <fstream>
#include <iterator>
#include <queue>

#include <basics/stringhelper.h>
#include <dance/Dancer.h>

#include <basics/util.h>
#include "dance/RhythmDetector.h"
#include <stewart/BodyKinematics.h>
#include <servo/PCA9685Servo.h>
#include <servo/ServoController.h>
#include <webserver/Webserver.h>
#include <ao/ao.h>
#include <audio/AudioFile.h>
#include <audio/AudioProcessor.h>
#include <beat/BTrack.h>

using namespace std;

bool mplayback = true;
bool executeServoThread = true;
std::thread* servoThread = NULL;


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
		 << "            [-s]                 # silent, do not play audio" << endl
		 << "            [-i <n>]# start after n detected beats" << endl
	     << "            [-t]                 # servo calibration via keyboard" << endl;
}

void signalHandler(int s){
	changemode(0);
	cout << "Signal " << s << ". Exiting";
	cout.flush();
	executeServoThread = false;
	if (servoThread != NULL)
		delete servoThread;
	exit(1);
}

void compensateLatency(bool& beat, double& bpm) {

	static TimeSamplerStatic latencyTimer;
	static queue<uint32_t> pendingBeatTime;
	uint32_t now = millis();
	// static uint32_t lastBeat= millis();

	// add beats to the queue
	if (beat && (RhythmDetector::getInstance().getRhythmInQuarters() != 0)) {
		// compute the necessary delay to compensate microphone latency
		// assume 2/4 beat
		// float myBpm = 60000.0/(float)(now - lastBeat);
		// cout << "   t=" << now - lastBeat << "ms -> BPM=" << myBpm << " riq=" << RhythmDetector::getInstance().getRhythmInQuarters() << endl;
		// lastBeat = now;
		float secondsPerBeat = 1.0*60.0/(bpm/RhythmDetector::getInstance().getRhythmInQuarters());
		int numOfDelayedBeats = AudioProcessor::getInstance().getLatency() / secondsPerBeat + 1;
		float currentBeatDelay = fmod(numOfDelayedBeats*secondsPerBeat-AudioProcessor::getInstance().getLatency(),secondsPerBeat); // [s]

		// queue up the time when this beat is to be fired
		pendingBeatTime.push(now + currentBeatDelay*1000.0);
		// cout << "now=" << millis() << " takt=" << RhythmDetector::getInstance().getRhythmInQuarters() << " spb=" << secondsPerBeat << "delay =" << currentBeatDelay * 1000 << "ms " << endl;

		// for now, we suppress the incoming beat
		beat = false;
	}

	// check if the queued up a beat, i.e. the first entry is at its due time
	if (!pendingBeatTime.empty() && pendingBeatTime.front() <= now) {
		pendingBeatTime.pop();
		beat = true;
		if (AudioProcessor::getInstance().isMicrophoneInputUsed())
			cout << "   latency BEAT!" << endl;
	}
}


void sendBeatToRythmDetector(bool beat, double bpm) {
	RhythmDetector & rd = RhythmDetector::getInstance();
	Dancer& dancer = Dancer::getInstance();

	// detect the beat
	rd.loop(beat, bpm);

	// compensate the microphones latency and delay the beat accordingly to hit the beat next time
	compensateLatency(beat, bpm);

	// if no music is detected, do not dance
	dancer.setMusicDetected(AudioProcessor::getInstance().isAudioDetected());

	// create the move according to the beat
	dancer.danceLoop(beat, bpm);

}

typedef void (*MoveCallbackFct)(bool beat, double Bpm);


int main(int argc, char *argv[]) {
	// exit correctly when exception arises
	std::set_terminate([](){
		std::cout << "Unhandled exception" << endl;
		std::cout.flush();
		std::abort();
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
    	} else if (arg == "-t") {
	    	ServoController::getInstance().calibrateViaKeyBoard();
	    } else if (arg == "-s") {
	    	mplayback = false;
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
    	} else {
    		cerr << "unknown option " << arg << endl;
    		exit(1);
    	}

    }

   	Dancer& dancer = Dancer::getInstance();
   	Webserver& webserver = Webserver::getInstance();
   	AudioProcessor& audioProcessor = AudioProcessor::getInstance();
   	webserver.setup(webserverPort, webrootPath);
    dancer.setup();
    dancer.setStartAfterNBeats(startAfterNBeats);
    RhythmDetector::getInstance().setup();

    audioProcessor.setup(sendBeatToRythmDetector);
    audioProcessor.setVolume((float)volumeArg/100.0);
    audioProcessor.setPlayback(mplayback);
	if (trackFilename != "") {
		std::ifstream file (trackFilename, std::ios::binary);
		file.unsetf (std::ios::skipws);
		std::istream_iterator<uint8_t> begin (file), end;
		std::vector<uint8_t> wavContent (begin, end);
		audioProcessor.setWavContent(wavContent);
		audioProcessor.setAudioSource();
	}

	// start thread that computes the kinematics and sends angles to the servos
	servoThread = new std::thread([=](){
		cout << "starting execution of kinematics and servo control" << endl;

		TimeSamplerStatic servoTimer;
	   	Dancer& dancer = Dancer::getInstance();
	   	ServoController& servoController = ServoController::getInstance();
	   	BodyKinematics& bodyKinematics = BodyKinematics::getInstance();

	   	servoController.setup();
	   	bodyKinematics.setup();
		while (executeServoThread) {
			if (servoTimer.isDue(10)) {
				Point bodyBallJoint_world[6], headBallJoint_world[6];
				double bodyServoAngles_rad[6], headServoAngles_rad[6];
				Point bodyServoBallJoints_world[6], headServoBallJoints_world[6];
				Point bodyServoArmCentre_world[6], headServoArmCentre_world[6];

				Pose headPose, bodyPose;
				dancer.getThreadSafePose(bodyPose, headPose);
				bodyKinematics.
						computeServoAngles(bodyPose, bodyServoArmCentre_world, bodyServoAngles_rad, bodyBallJoint_world, bodyServoBallJoints_world,
										headPose, headServoArmCentre_world, headServoAngles_rad, headBallJoint_world, headServoBallJoints_world);
				for (int i = 0;i<6;i++) {
					servoController.setAngle_rad(i,bodyServoAngles_rad[i]);
					servoController.setAngle_rad(i+6,headServoAngles_rad[i]);
				}
			}
			delay_ms(1);
		}
		cout << "stopping execution of kinematics and servo control" << endl;
	});

	// run main loop that processes the audio input and does beat detection
	cout << "starting audio processing" << endl;
   	while (true) {
   		// if content is available from whatever source, process it (i.e. perform beat detection via sendBeatToRythmDetector)
   		if (audioProcessor.isWavContentUsed() ||
			audioProcessor.isMicrophoneInputUsed()) {

   			// do it. Returns when current content type is empty
   			audioProcessor.processInput();
   		}

   		// be cpu friendly when no input is available
		delay_ms(1);
   	}
}
