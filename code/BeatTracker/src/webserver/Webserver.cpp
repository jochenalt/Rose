/*
 * Webserver.cpp
 *
 *  Created on: Mar 13, 2018
 *      Author: jochenalt
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <vector>

#include "basics/stringhelper.h"
#include "basics/util.h"

#include "mongoose.h"

#include <webserver/Webserver.h>
#include "MoveMaker.h"
using namespace std;

Webserver::Webserver() {

}

Webserver::~Webserver() {
	mg_mgr_free(&mgr);
}


using namespace std;

static struct mg_serve_http_opts s_http_server_opts;

// define an mongoose event handler function that is called whenever a request comes in
void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    switch (ev)
    {
    	case MG_EV_HTTP_REQUEST: {
    			struct http_message *hm = (struct http_message *) ev_data;
    			string uri(hm->uri.p, hm->uri.len);
    			string query(hm->query_string.p, hm->query_string.len);
    	        string body(hm->body.p, hm->body.len);

    	        bool ok;
    			string response;
    			// if our dispatcher knows the command, it creates a response and returns true.
    			// Otherwise assume that we deliver static content.
    			bool processed = Webserver::getInstance().dispatch(uri, query, body, response, ok);
    			if (processed) {
    				if (ok) {
    					mg_printf(nc, "HTTP/1.1 200 OK\r\n"
    						"Content-Type: text/plain\r\n"
    						"Content-Length: %d\r\n\r\n%s",
    							(int) response.length(), response.c_str());
    				} else {
    					mg_printf(nc, "HTTP/1.1 500 Server Error\r\n"
    							"Content-Length: %d\r\n\r\n%s",
    							(int) response.length(), response.c_str());
    				}
    			} else {
    				// no API call, serve static content
    				mg_serve_http(
    						nc, (http_message*) ev_data, s_http_server_opts);
    			}
    		break;
    	}
    default:
        break;
    }
}


void Webserver::setup(int port, string webRootPath) {
	mg_mgr_init(&mgr, NULL);
	string portStr (intToString(port));
	struct mg_connection *nc = mg_bind(&mgr, portStr.c_str(), ev_handler);
	if (nc == NULL) {
		cerr << "Cannot bind webserver to %i" << port << ". Maybe the server is already running?";
		exit(1);
	}

	// Set up HTTP server parameters
	mg_set_protocol_http_websocket(nc);
	s_http_server_opts.document_root = (new string(webRootPath))->c_str();
	cs_stat_t st;
	if (mg_stat(s_http_server_opts.document_root, &st) != 0) {
		cerr << "Cannot find web_root directory " << webRootPath;
		exit(1);
	}

	cout << "webserver is running at port " << port << endl;
	terminateThread = false;
	webserverThread = new std::thread(&Webserver::runningThread, this);
}

void Webserver::runningThread() {
	while (!terminateThread) {
		// check and dispatch incoming http requests (dispatched by CommandDispatcher) and wait for timeout ms max.
		mg_mgr_poll(&mgr, 10);
	}
	std::terminate(); // terminate that thread
}



bool getURLParameter(vector<string> names, vector<string> values, string key, string &value) {
	for (int i = 0;i<(int)names.size();i++) {
		if (names[i].compare(key) == 0) {
			value = values[i];
			return true;
		}
	}
	return false;
}


void compileURLParameter(string uri, vector<string> &names, vector<string> &values) {
	names.clear();
	values.clear();

	std::istringstream iss(uri);
	std::string token;
	while (std::getline(iss, token, '&'))
	{
		// extract name and value of parameter
		int equalsIdx = token.find("=");
		if (equalsIdx > 0) {
			string name = token.substr(0,equalsIdx);
			string value = token.substr(equalsIdx+1);

			names.insert(names.end(), name);
			values.insert(values.end(), urlDecode(value));
		};
	}
}

// retrurns a standardized response
string getResponse(bool ok) {
	std::ostringstream s;
	if (ok) {
		s << "\"ok\":true";
	} else {
		s << "\"ok\":false, \"error\":" << getLastError() << ", \"errormessage\":" << stringToJSonString(getErrorMessage(getLastError()));
	}
	string response = s.str();
	return response;
}

// central dispatcher of all url requests arriving at the webserver
// returns true, if request has been dispatched successfully. Otherwise the caller
// should assume that static content is to be displayed.
bool Webserver::dispatch(string uri, string query, string body, string &response, bool &okOrNOk) {

	response = "";
	string urlPath = getPath(uri);

	vector<string> urlParamName;
	vector<string> urlParamValue;

	compileURLParameter(query,urlParamName,urlParamValue);

	if (hasPrefix(uri, "/console")) {
		string engineCommand = uri.substr(string("/status").length());

		if (hasPrefix(engineCommand, "/on")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		}
		else if (hasPrefix(engineCommand, "/off")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		}
		else if (hasPrefix(engineCommand, "/song")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		}
		else if (hasPrefix(engineCommand, "/ambition")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		}
		else if (hasPrefix(engineCommand, "/move")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		} else {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		}
	}

	if (hasPrefix(uri, "/audio")) {
		string engineCommand = uri.substr(string("/audio").length());

		if (hasPrefix(engineCommand, "/file")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		} else if (hasPrefix(engineCommand, "/aux")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		} else if (hasPrefix(engineCommand, "/playback")) {
			okOrNOk = true;
			response = getResponse(okOrNOk);
			return true;
		}
	}

	if (hasPrefix(uri, "/status")) {
		string engineCommand = uri.substr(string("/status").length());

		MoveMaker mm = MoveMaker::getInstance();
		std::ostringstream out;
		out << "{ \"response\"= { \"body\"=";
		mm.getBodyPose().serialize(out);
		out << ", \"head\"=";
		mm.getHeadPose().serialize(out);
		out << ", \"ambition\"=" << mm.getAmbition();
		out << ", \"move\"=" << (int)mm.getCurrentMove();
		out << "} , " << getResponse(true);
		out << "}";
		response = out.str();
		return true;
	}

	okOrNOk = false;
	return false;
}




