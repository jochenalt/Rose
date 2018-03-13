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
#include "mongoose.h"

using namespace std;

class Webserver {
public:
	Webserver();
	virtual ~Webserver();
	static Webserver& getInstance() {
		static Webserver instance;
		return instance;
	}

	void setup(int port, string webRootPath);
	void teardown() {
		terminateThread = true; // let the thread run
	}

	void setStatus(const Pose& newBodyPose, const Pose& newHeadPose, const string newMoveName, float newAmbition);
private:
	friend void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

	// function the webserver thread runs. Is terminated when activeThread is set to false
	void runningThread();

	// dispatches incoming http calls
	bool dispatch(string uri, string query, string body, string &response, bool &okOrNOk);

	// mongoose manager
	struct mg_mgr mgr;

	// store the thread
	std::thread* webserverThread  = NULL;

	// flag to indicate the flag to terminbate
	bool terminateThread = false;

};

#endif /* SRC_WEBSERVER_WEBSERVER_H_ */
