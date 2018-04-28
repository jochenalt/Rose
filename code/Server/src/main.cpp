#include <iostream>
#include <thread>
#include <pthread.h>
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
#include <audio/ClockGenerator.h>

#include <beat/BTrack.h>

#include <Configuration.h>

using namespace std;

static bool globalPlayback = true;
static bool executeServoThread = true;
static std::thread* danceThread = NULL;
static std::thread* servoThread = NULL;

ClockGenerator<BeatInvocation> clockGenerator;

// there are two main threads, the audio thread that works with a frequency optimised for
// best results in detecting beats (a 512 sample probe, which is approx. 170Hz at 44100Hz sample rate),
// and the servo thread that tries to get most of the digital servos.
// The synchronisation between these two threads happens with the following flag,
// that indicates that the servo loop has fetched servoHeadPoseBuffer
// Then, the audio loop is generating new data and sets the flag to true;
static volatile bool newPoseAvailable = false;

// buffer to pass body and head from audio thread to the servo thread
static Pose servoHeadPoseBuffer;
static Pose servoBodyPoseBuffer;


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
	if (danceThread != NULL)
		delete danceThread;

	Webserver::getInstance().teardown();
	cout << "Exiting" << endl;

	// give the threads a chance to quit without given a memory error
	delay_ms(100);

	exit(1);
}

void compensateLatency(bool& beat, double& bpm, int rhythmInQuarters) {

	AudioProcessor& audioProcessor = AudioProcessor::getInstance();
	RhythmDetector& rhythmDetector = RhythmDetector::getInstance();

	static queue<uint32_t> pendingBeatTime;
	uint32_t now = millis();

	// add beats to the queue
	if (beat && (rhythmInQuarters != 0)) {
		// queue up the time when this beat is to be fired
		pendingBeatTime.push(now +  rhythmDetector.getLatencyCompensationDelay()*1000.0);

		// for now,  suppress the incoming beat, but re-fire it with incorporated latency computation
		beat = false;
	}

	// check if the queued up a beat, i.e. the first entry is at its due time
	if (!pendingBeatTime.empty() && pendingBeatTime.front() <= now) {
		pendingBeatTime.pop();

		// now it s
		beat = true;
		// if (AudioProcessor::getInstance().isMicrophoneInputUsed())
		cout << std::fixed << std::setprecision(2)
		     << "Real Beat (bpm=" <<  rhythmDetector.bpm()<< " 1/" <<  rhythmInQuarters << ") "
			 << "latency=" << audioProcessor.getCurrentLatency()
			 << "s compensation=" << rhythmDetector.getLatencyCompensationDelay() << "s"
		 	 << " move=" <<  rhythmDetector.getLatencyCompensatedRythmPercentage() << endl;
	}
}


// function to make uneven callbacks coming from audio stream clock generated
void pushToClockGenerator(double processTime, bool beat, double bpm, int rhythmInQuarters) {
	BeatInvocation call;
	call.processTime = processTime;
	call.beat = beat;
	call.bpm = bpm;
	call.rhythmInQuarters = rhythmInQuarters;


	// push that invokation to the clock generator an let them fire a small amount of time later on
	// that's the timewise buffer of the microphone  at 44100 Hz
	clockGenerator.push(processTime + Configuration::getInstance().microphoneBufferLength, call);
}



void danceThreadFunction() {
	RhythmDetector& rhythmDetector = RhythmDetector::getInstance();
	AudioProcessor& audioProcessor = AudioProcessor::getInstance();

	Dancer& dancer = Dancer::getInstance();
	newPoseAvailable = false;

	// run the clock generated thread to identify the rhytm
	while (executeServoThread) {
		BeatInvocation o;				// detected beat coming from audio (including its latency)
		bool insertDelay = true;		// indicates that the loop had no action and we need to sleep a bit

		bool beatLoopIsDue = clockGenerator.isClockDue(audioProcessor.getElapsedTime(),o);
		if (beatLoopIsDue) {
			if (o.beat){
				/*
				cout << std::fixed << std::setprecision(2)
				     << "   Beat (bpm=" <<  rhythmDetector.bpm()<< " 1/" <<  rhythmDetector.getRhythmInQuarters() << ") "
					 << "latency=" << audioProcessor.getCurrentLatency()
					 << "s comp=" << rhythmDetector.getLatencyCompensationDelay() << "s"  << "/" << rhythmDetector.getLatencyCompensationPercentage() << "% "
				 	 << "move=" << rhythmDetector.getRythmPercentage() << "/" << rhythmDetector.getLatencyCompensatedRythmPercentage() << endl;
				 	 */
			};

			// detect the beat
			rhythmDetector.loop(audioProcessor.getCurrentLatency(), o.processTime, o.beat, o.bpm, o.rhythmInQuarters);

			// compensate the microphones latency and delay the beat accordingly to hit the beat next time
			// after this call, beat-flag is modified such that it incorporated the latency
			compensateLatency(o.beat, o.bpm, o.rhythmInQuarters);

			// if no music is detected, do not dance
			dancer.setMusicDetected(AudioProcessor::getInstance().isAudioDetected());

			// we did something, continue looping
			insertDelay = false;
		}

		// hand over pose to servo thread
		if (newPoseAvailable == false) {

			// the dance move is computed.
			// underlying matrix library is not thread safe!
			// So, take care that this computation never happens simultaneously with the kinematics computation
			// TODO fix that
			dancer.danceLoop(o.beat, o.bpm, o.rhythmInQuarters);

			servoHeadPoseBuffer = dancer.getHeadPose();
			servoBodyPoseBuffer = dancer.getBodyPose();

			// a new pose is ready to be taken over by the servo thread
			newPoseAvailable = true;


			// we did something, continue looping
			insertDelay = false;
		}

		// be cpu friend. if nothing happened, sleep a bit
		if (insertDelay);
			delay_ms(1);
	}
}

void servoThreadFunction() {
	BodyKinematics& bodyKinematics = BodyKinematics::getInstance();
	ServoController& servoController = ServoController::getInstance();

	servoController.setup();
	bodyKinematics.setup();

	Point bodyBallJoint_world[6], headBallJoint_world[6];
	double bodyServoAngles_rad[6], headServoAngles_rad[6];
	Point bodyServoBallJoints_world[6], headServoBallJoints_world[6];
	Point bodyServoArmCentre_world[6], headServoArmCentre_world[6];

	// limit the frequency a new pose is sent to the servos
	TimeSampler sync;

	int servoSampleFrequency = 60;					// [Hz]
	int servoSample_ms = 1000/servoSampleFrequency; // [ms]

	while (executeServoThread) {
		if (newPoseAvailable) {
			if (sync.isDue(servoSample_ms)) {
				bodyKinematics.
					computeServoAngles(	servoBodyPoseBuffer, bodyServoArmCentre_world, bodyServoAngles_rad, bodyBallJoint_world, bodyServoBallJoints_world,
										servoHeadPoseBuffer, headServoArmCentre_world, headServoAngles_rad, headBallJoint_world, headServoBallJoints_world);

				// current pose is used up, indicate that we need a new one, such that the audio thread will set it
				newPoseAvailable = false;

				// sending all angles to the PCA9685. This
				// takes 2x4ms via I2C, so maximum loop frequency is 125Hz
				microseconds start_us = micros();
				int durationPerServo_us = (servoSample_ms*1000)/12; // [us]
				for (int i = 0;i<6;i++) {
					servoController.setAngle_rad(i,bodyServoAngles_rad[i]);
					microseconds end = micros();
					int toBe_us = durationPerServo_us*(i*2 + 1);
					int servoDelay_us = toBe_us  -  (int)(end - start_us);
					if (servoDelay_us < 200)
						servoDelay_us = 200;
					//delay_us(servoDelay_us); // necessary, otherwise the I2C line misses some calls and gets hickups approx every 20s.

					servoController.setAngle_rad(i+6,headServoAngles_rad[i]);
					end = micros();
					toBe_us = durationPerServo_us*(i*2 + 2);
					int duration_us = (int)(end - start_us);
					servoDelay_us = toBe_us  - duration_us;
					if (servoDelay_us < 200)
						servoDelay_us = 200;
					delay_us(servoDelay_us); // necessary, otherwise the I2C line misses some calls and gets hickups approx every 20s.

				}
				int duration_us = (int)(micros()-start_us);
				const int maxDuration_us = 12*durationPerServo_us ;
				if (duration_us > (maxDuration_us*15)/10) {
					cerr << "WARN: servos command via I2C took " << duration_us/1000 << "ms instead of " << maxDuration_us/1000 << "ms max." << endl;
				}

			}
		}
		else {
			delay_us(10); // this should happen very rarely, since the rthym thread is much faster than the servo thread
			               // Typically, once newPoseAvailable is set to false, the rhythm thread computes a new one
		}
	}
}

void audioThreadFunction() {
	AudioProcessor& audioProcessor = AudioProcessor::getInstance();

	// run main loop that processes the audio input and does beat detection
	while (true) {
		// do it. Returns when current content type is empty
		audioProcessor.processInput();

		// be cpu friendly when waiting for audio input
		delay_ms(1);
	}
}


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
				globalPlayback = false;
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

		audioProcessor.setVolume((float)volumeArg/100.0);
		audioProcessor.setGlobalPlayback(globalPlayback);

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

		// start own thread for rythm detection and dance moves
		// result pose is passed to servo thread
		danceThread = new std::thread([=](){
			pthread_setname_np(pthread_self(), "dance");
			danceThreadFunction();
			std::terminate();
		});


		// start a thread for servo control
		// fetch poses from dance thread
		servoThread = new std::thread([=](){
			pthread_setname_np(pthread_self(), "servo");
			servoThreadFunction();
			std::terminate();
		});

		// final loop catching audio and passing via clock generator to danceThread
		audioProcessor.setup(pushToClockGenerator);
		pthread_setname_np(pthread_self(), "audio");
		audioThreadFunction();
	}
    catch(std::exception const& e) {
    	cerr << "unexpected exception " << e.what() << endl;
    }
    catch(...) {
    	cerr << "unknown exception" << endl;
    }
    return 0;
}
