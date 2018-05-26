
#include <iostream>
#include <assert.h>
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
#include <ui/UI.h>
#include "dance/RhythmDetector.h"
#include <Stewart/BodyKinematics.h>
#include <client/BotClient.h>

using namespace std;

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
	cout << "Client " << endl
	     << "            [-h]                 # print this" << endl
	     << "            [-port <port>]       # set port of webserver if different from 8080" << endl
	     << "            [-host <host:port>]  # define this process as client accessing this webserver" << endl
	     << "            [-odroid]            # use hard coded server address of my odroid 192.168.178.76:8080" << endl
	     << "            [-vm]                # use hard coded server address of my virtualbox 192.168.178.69:3080" << endl
	     << "            [-ui]                # start visualizer" << endl
		 << "            [-s]                 # silent, do not play audio" << endl;
}

typedef void (*BeatCallbackFct)(bool beat, double Bpm);


void signalHandler(int s){
	cout << "Signal " << s << ". Exiting";
   	UI::getInstance().tearDown();
	cout.flush();
	exit(1);
}

void sendDanceToClient(bool beat, double bpm) {
	Dancer & mm = Dancer::getInstance();
	BotClient& client = BotClient::getInstance();

	// fetch cached data from webserver  and set into Dance machine
	mm.imposeDanceParams(client.getMove(), client.getAmbition(),  client.getPose());

	// send data to ui
	UI::getInstance().setBodyPose(mm.getPose());
}


int main(int argc, char *argv[]) {
	// exit correctly when exception arises
	std::set_terminate([](){
		std::cout << "Unhandled exception\n"; std::abort();
    	UI::getInstance().tearDown();
		changemode(0);
	});

	// catch SIGINT (ctrl-C)
    signal (SIGINT,signalHandler);

    // currently played filename
    string trackFilename;

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
    	} else if (arg == "-vm") {
    	    	webserverPort = 3080;
    	    	webclientHost = "192.168.56.1";
    	} else if (arg == "-odroid") {
    	    	webserverPort = 8080;
    	    	webclientHost = "192.168.178.76";
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
    	} else {
    		cerr << "unknown option " << arg << endl;
    		exit(1);
    	}

    }

   	BotClient::getInstance().setup(webclientHost, webserverPort);
   	BodyKinematics::getInstance().setup();
    Dancer::getInstance().setup();
    RhythmDetector::getInstance().setup();
   	UI::getInstance().setup(argc,argv);

	Dancer & mm = Dancer::getInstance();
	BotClient& client = BotClient::getInstance();
	UI& ui = UI::getInstance();
	TimeSampler clientLoopTimer;
   	while (true) {
   		if (clientLoopTimer.isDue(20)) {
   			client.getStatus();

			// fetch cached data from webserver  and set into Dance machine
			mm.imposeDanceParams(client.getMove(), client.getAmbition(),  client.getPose());

			// send data to ui
			ui.setBodyPose( mm.getPose());
			ui.setMusicDetected(client.isMusicDetected());
   		}
   		else
   			delay_ms(1);
   	}

   	UI::getInstance().tearDown();
}
