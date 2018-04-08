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
#include <dance/RhythmDetector.h>
#include <stewart/BodyKinematics.h>
#include <servo/PCA9685.h>
#include <servo/ServoController.h>
#include <webserver/Webserver.h>
#include <audio/AudioFile.h>
#include <audio/AudioProcessor.h>
#include <audio/SoundCardUtils.h>
#include <beat/BTrack.h>

#include <Configuration.h>

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
	cout << endl
		 << "BeatTracker -f <wav.file>        # define the track to be played" << endl
	     << "            [-h]                 # print help and configuration" << endl
	     << "            [-l]                 # measure latency" << endl
		 << "            [-port <port>]       # set port of webserver if different from 8080" << endl
		 << "            [-webroot <path>]    # set path of ./webroot" << endl
		 << "            [-v <volume 0..100>] # set volume between 0 and 100" << endl
		 << "            [-s]                 # silent, do not play audio" << endl
		 << "            [-i <n>]# start after n detected beats" << endl
	     << "            [-t]                 # servo calibration via keyboard" << endl;
}

void signalHandler(int s){
	changemode(0);
	cout << "Signal " << s << endl;
	cout.flush();
	executeServoThread = false;
	if (servoThread != NULL)
		delete servoThread;
	Webserver::getInstance().teardown();
	cout << "Exiting" << endl;

	// give the threads a chance to quit without given a memory error
	delay_ms(100);

	exit(1);
}

void compensateLatency(bool& beat, double& bpm) {

	static TimeSamplerStatic latencyTimer;
	static queue<uint32_t> pendingBeatTime;
	uint32_t now = millis();

	// add beats to the queue
	if (beat && (RhythmDetector::getInstance().getRhythmInQuarters() != 0)) {
		// compute the necessary delay to compensate microphone latency
		// assume 2/4 beat
		float secondsPerBeat = 1.0*60.0/(bpm/RhythmDetector::getInstance().getRhythmInQuarters());
		int numOfDelayedBeats = AudioProcessor::getInstance().getCurrentLatency() / secondsPerBeat + 1;
		float currentBeatDelay = fmod(numOfDelayedBeats*secondsPerBeat-AudioProcessor::getInstance().getCurrentLatency(),secondsPerBeat); // [s]

		// queue up the time when this beat is to be fired
		pendingBeatTime.push(now + currentBeatDelay*1000.0);
		// cout << "now=" << millis() << " takt=" << RhythmDetector::getInstance().getRhythmInQuarters() << " spb=" << secondsPerBeat << "delay =" << currentBeatDelay * 1000 << "ms " << endl;

		// for now,  suppress the incoming beat, but re-fire it with incorporated latency computation
		beat = false;
	}

	// check if the queued up a beat, i.e. the first entry is at its due time
	if (!pendingBeatTime.empty() && pendingBeatTime.front() <= now) {
		pendingBeatTime.pop();

		// now it s
		beat = true;
		if (AudioProcessor::getInstance().isMicrophoneInputUsed())
			cout << "   latency BEAT!" << endl;
	}
}


// there are two main threads, the audio thread that works with a frequency optimised for
// best results in detecting beats (a 512 sample probe, which is approx. 170Hz at 44100Hz sample rate),
// and the servo thread that tries to get most of the digital servos.
// The synchronisation between these two threads happens with the following flag,
// that indicates that the servo loop has fetched servoHeadPoseBuffer
// Then, the audio loop is generating new data and sets the flag to true;
volatile bool newPoseAvailable = false;

// buffer to pass body and head from audio thread to the servo thread
static Pose servoHeadPoseBuffer;
static Pose servoBodyPoseBuffer;

void sendBeatToRythmDetector(bool beat, double bpm) {
	RhythmDetector & rhythmDetector = RhythmDetector::getInstance();
	Dancer& dancer = Dancer::getInstance();

	// detect the beat
	rhythmDetector.loop(beat, bpm);

	// compensate the microphones latency and delay the beat accordingly to hit the beat next time
	// after this call, beat-flag is modified such that it incorporated the latency
	compensateLatency(beat, bpm);

	// create the move according to the beat
	// but this is done in a loop with a frequency the servos can take
	if (newPoseAvailable == false) {

		// the dance move is computed
		dancer.danceLoop(beat, bpm);

		// if no music is detected, do not dance
		dancer.setMusicDetected(AudioProcessor::getInstance().isAudioDetected());

		servoHeadPoseBuffer = dancer.getHeadPose();
		servoBodyPoseBuffer = dancer.getBodyPose();
		newPoseAvailable = true;
	}
}

// Function pointer used as call back that allows to attach dancing to
// the audio thread it is called by the audio thread after every sample probe
typedef void (*MoveCallbackFct)(bool beat, double Bpm);

int main(int argc, char *argv[]) {
	try {

		// catch SIGINT (ctrl-C)
		signal (SIGINT,signalHandler);

		// currently played filename
		string trackFilename;

		// default volume
		int volumeArg = 20;

		// default latency of the moves
		int startAfterNBeats = 4;

		// sound cards need to be initialized upfront, args might need that
		SoundCardUtils& audioUtils= SoundCardUtils::getInstance();
		audioUtils.setup();

		// write a new config file with any new option
		Configuration& configuration = Configuration::getInstance();
		bool configIsThere = configuration.load();

		// if no config file is there, call latency measurement
		if (!configIsThere) {
			double latency = AudioProcessor::getInstance().calibrateLatency();
			if (latency > 0.1)
				configuration.microphoneLatency = latency;
		}

		// if we run a webserver, this is the path where static content is stored
		string webrootPath = string(argv[0]);
		int idx = webrootPath.find_last_of("/");
		webrootPath = webrootPath.substr(0,idx) + "/webroot";

		Webserver& webserver = Webserver::getInstance();
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
					configuration.print();
					audioUtils.printSoundCards();
					webserver.print();
					printUsage();
					exit(0);
			} else if (arg == "-l") {
					double latency = AudioProcessor::getInstance().calibrateLatency();
					if (latency > 0.1)
						configuration.microphoneLatency = latency;
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
				int webserverPort = -1;
				webserverPort = stringToInt(getCmdOption(argv, argc, i), ok);
				if ((webserverPort < 1000) || (webserverPort > 9999)) {
					cerr << "port should be between 1000..9999" << endl;
					exit(1);
				}
				configuration.webserverPort = webserverPort;
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

		// write configuration data which might have changed by parameters to config file
		// i.e. every parameter passed is persistent
		configuration.save();

		// initialize all software processors
		Dancer& dancer = Dancer::getInstance();
		AudioProcessor& audioProcessor = AudioProcessor::getInstance();
		RhythmDetector & rhhymDetector = RhythmDetector::getInstance();

		webserver.setup(webrootPath);
		dancer.setup();
		dancer.setStartAfterNBeats(startAfterNBeats);
		rhhymDetector.setup();

		audioProcessor.setup(sendBeatToRythmDetector);
		audioProcessor.setVolume((float)volumeArg/100.0);
		audioProcessor.setPlayback(mplayback);

		// if a track is passed, start with that one
		if (trackFilename != "") {
			std::ifstream file (trackFilename, std::ios::binary);
			file.unsetf (std::ios::skipws);
			std::istream_iterator<uint8_t> begin (file), end;
			std::vector<uint8_t> wavContent (begin, end);
			audioProcessor.setWavContent(wavContent);
		} else {
			audioProcessor.setMicrophoneInput();
		}

		// read in the audio source
		audioProcessor.setAudioSource();

		// start own thread for kinematics and servo control
		servoThread = new std::thread([=](){
			ServoController& servoController = ServoController::getInstance();
			BodyKinematics& bodyKinematics = BodyKinematics::getInstance();

			servoController.setup();
			bodyKinematics.setup();

			TimeSamplerStatic timer;
			Point bodyBallJoint_world[6], headBallJoint_world[6];
			double bodyServoAngles_rad[6], headServoAngles_rad[6];
			Point bodyServoBallJoints_world[6], headServoBallJoints_world[6];
			Point bodyServoArmCentre_world[6], headServoArmCentre_world[6];

			// run the servo thread the indication to stop it is set outside
			while (executeServoThread) {
				if (newPoseAvailable) {
					// limit the frequency a new pose is sent to the servos
					const int servoFrequency = 70; // [Hz]
					if (timer.isDue(1000.0/servoFrequency /* [ms] */)) {

						// compute the servo angles out of the pose
						bodyKinematics.
							computeServoAngles(	servoBodyPoseBuffer, bodyServoArmCentre_world, bodyServoAngles_rad, bodyBallJoint_world, bodyServoBallJoints_world,
												servoHeadPoseBuffer, headServoArmCentre_world, headServoAngles_rad, headBallJoint_world, headServoBallJoints_world);

						// current pose is used up, indicate that we need a new one, such that the audio thread will set it
						newPoseAvailable = false;

						// sending all angles to the PCA9685. This
						// takes 2x4ms via I2C, so maximum loop frequency is 125Hz
						for (int i = 0;i<6;i++) {
							servoController.setAngle_rad(i,bodyServoAngles_rad[i]);
						}
						for (int i = 0;i<6;i++) {
							servoController.setAngle_rad(i+6,headServoAngles_rad[i]);
						}
					} else {
						delay_ms(1);
					}
				} else
					delay_us(100); // typically never happens, since the audio thread runs faster than
				                   // 125Hz, at this point in time newPoseAvailable should be set already
			}
			cout << "stopping execution of kinematics and servo control" << endl;
		});

		// run main loop that processes the audio input and does beat detection
		while (true) {
			// if content is available from whatever source, process it (i.e. perform beat detection via sendBeatToRythmDetector)
			if (audioProcessor.isWavContentUsed() ||
				audioProcessor.isMicrophoneInputUsed()) {

				// do it. Returns when current content type is empty
				audioProcessor.processInput();
			}

			// be cpu friendly when no audio input is available
			delay_ms(10);
		}
	}
    catch(std::exception const& e)
    {
    	cerr << "unexpected exception " << e.what() << endl;
    }
    catch(...)
    {
    	cerr << "unknown exception" << endl;
    }
    return 0;
}
