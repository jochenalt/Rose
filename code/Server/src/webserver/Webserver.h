/*
 * Webserver.h
 *
 *  Created on: Mar 13, 2018
 *      Author: jochenalt
 */

#ifndef SRC_WEBSERVER_WEBSERVER_H_
#define SRC_WEBSERVER_WEBSERVER_H_
#include <stdio.h>
#include <thread>

#include "basics/spatial.h"
#include "webserver/mongoose.h"

using namespace std;

class Webserver {
public:
	Webserver();
	virtual ~Webserver();
	static Webserver& getInstance() {
		static Webserver instance;
		return instance;
	}

	// is webserver running?
	bool isActive() { return isWebserverActive; };

	// initialize and start webserver
	void setup(string webRootPath);

	// stop webserver
	void teardown() {
		terminateThread = true; // let the thread run
	}

	// flag, indicating that the webserver should stop
	bool getTerminateThread() { return terminateThread; };

	// print configuration information on webserver
	void print();
private:
	friend void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

	// function the webserver thread runs. Is terminated when activeThread is set to false
	void runningThread();

	// dispatches incoming http calls
	bool dispatch(string uri, string query, string body, string &response, bool &okOrNOk);

	// mongoose manager
	struct mg_mgr mgr;

	// mongoose server
	struct mg_server *mongoose_server = NULL;

	// store the thread
	std::thread* webserverThread  = NULL;

	// flag to indicate the flag to terminbate
	bool terminateThread = false;

	// flag to indicate if webserver is running
	bool isWebserverActive = false;

};

#endif /* SRC_WEBSERVER_WEBSERVER_H_ */
