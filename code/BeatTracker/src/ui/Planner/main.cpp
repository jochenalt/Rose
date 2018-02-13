//============================================================================
// Name        : main.cpp
// Author      : Jochen Alt
//============================================================================

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "basics/logger.h"
#include "basics/util.h"

#include "WindowController.h"
#include "uiconfig.h"
#include "setup.h"

INITIALIZE_EASYLOGGINGPP

using namespace std;

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
    defaultConf.set(el::Level::Trace, el::ConfigurationType::Filename, "logs/manfred.log");

    el::Loggers::reconfigureLogger("default", defaultConf);

    LOG(INFO) << "Manfred Setup";
}


void printUsage(string prg) {
	cout << "usage: " << prg << " [-h] [-d \"<command>\"] [-i]" << endl
	 	 << "  [-host <ip>]        webserver ip address of engine" << endl
	 	 << "  [-port <port>]      webserver's port " << endl
	 	 << "  [-standalone]       include the engine" << endl
	 	 << "  [-virtualbox]       -host 192.168.56.1 -port 3000" << endl
	 	 << "  [-odroid]           -host 192.168.178.73 -port 8000" << endl
	 	 << "  [-h]                help" << endl
	 	 << "  [-t (test)]         componten test" << endl
		 << "  <without par>       start engine and ui" << endl;
}

void initUI(int argc, char *argv[]) {
	// initialize Logging
	setupLogging(argc, argv);

	// print help
	std::set_terminate([](){
		std::cout << "Unhandled exception\n"; std::abort();
	});

	// initialize ui
	bool UISetupOk= WindowController::getInstance().setup(argc, argv);
	if (!UISetupOk) {
		cerr << "UI initialization failed" << endl;
		exit(1);
	}
}

void loopUI() {
	delay_ms(1);
}
