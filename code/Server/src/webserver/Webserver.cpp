/*
 * Webserver.cpp
 *
 *  Created on: Mar 13, 2018
 *      Author: Jochen Alt
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <iterator>

#include "basics/stringhelper.h"
#include "basics/util.h"
#include <basics/base64.h>

#include <dance/Dancer.h>

#include <webserver/mongoose.h>
#include <webserver/Webserver.h>
#include <audio/AudioProcessor.h>
#include <Configuration.h>


using namespace std;

Webserver::Webserver() {

}

Webserver::~Webserver() {

}

using namespace std;

static struct mg_serve_http_opts s_http_server_opts;
static sock_t sock[2];

// we need only two threads, one for the requesting the status every 20ms and one for all interactions like file upload
static const int s_num_worker_threads = 2;

// session id used per connection
static unsigned long s_next_id = 0;

// This info is passed to the worker thread
struct work_request {
  unsigned long conn_id;  // needed to identify the connection where to send the reply
  // optionally, more data that could be required by worker
};

// This info is passed by the worker thread to mg_broadcast
struct work_result {
  unsigned long conn_id;
  int sleep_time;
};

// define an mongoose event handler function that is called whenever a request comes in
void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    switch (ev)
    {
    	case MG_EV_ACCEPT:
    		nc->user_data = (void *)++s_next_id;
   			break;
    	case MG_EV_HTTP_REQUEST: {
    			static int counter  = 0;
    			counter ++;
    			work_request req;
    			req.conn_id = (unsigned long)nc->user_data;
    			if (write(sock[0], &req, sizeof(req)) < 0)
    				cerr << "Writing worker sock" << endl;

    			struct http_message *hm = (struct http_message *) ev_data;
    			string uri(hm->uri.p, hm->uri.len);
    			string query(hm->query_string.p, hm->query_string.len);
    	        string body(hm->body.p, hm->body.len);

    	        // check body encoding
    	        for (unsigned i=0;i<MG_MAX_HTTP_HEADERS;i++) {
    	        	if (hm->header_names[i].p != NULL) {
    	        		const string encodingName = "Content-Transfer-Encoding";
    	        		if (strncmp(hm->header_names[i].p, encodingName.c_str(), encodingName.length()) == 0) {
							if (strncmp(hm->header_values[i].p, "base64", 6) == 0)
								body = base64_decode(body);
						}
    	        	} else
    	        		break; // stop with first null-header
    	        }
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
    			counter--;

   			break;
    	}
        case MG_EV_CLOSE: {
        	if (nc->user_data)
        		nc->user_data = NULL;
        	break;
        }
    default:
        break;
    }
}


static void on_work_complete(struct mg_connection *nc, int ev, void *ev_data) {
  char s[32];
  struct mg_connection *c = NULL;
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
	  if (c->user_data != NULL) {
		struct work_result *res = (struct work_result *)ev_data;
		if ((unsigned long)c->user_data == res->conn_id) {
			sprintf(s, "conn_id:%lu sleep:%d", res->conn_id, res->sleep_time);
			mg_send_head(c, 200, strlen(s), "Content-Type: text/plain");
			mg_printf(c, "%s", s);
		}
	  }
  }
}

void* worker_thread_proc(void *param) {
  struct mg_mgr *mgr = (struct mg_mgr *) param;
  struct work_request req = {0};

  while (!Webserver::getInstance().getTerminateThread()) {
    if (read(sock[1], &req, sizeof(req)) < 0)
    	cerr << "Reading worker sock" << endl;
    int r = rand() % 10;
    sleep(r);
    struct work_result res = {req.conn_id, r};
    mg_broadcast(mgr, on_work_complete, (void *)&res, sizeof(res));
  }
  return NULL;
}


void Webserver::setup(string webRootPath) {
	mg_mgr_init(&mgr, NULL);
	int port = Configuration::getInstance().webserverPort;
	string portStr (intToString(port));
	struct mg_connection *nc = mg_bind(&mgr, portStr.c_str(), ev_handler);
	if (nc == NULL) {
		cerr << "Cannot bind webserver to %i" << port << ". Maybe the server is already running?" << endl;
		exit(1);
	}

	// ok, I have no idea what these socket pairs are good for, its coming from the multithreaded example from mongoose
	if (mg_socketpair(sock, SOCK_STREAM) == 0) {
		cerr << "Cannot open socket pair" << endl;
		exit(1);
	}

	// Set up HTTP server parameters
	mg_set_protocol_http_websocket(nc);
	mg_enable_multithreading(nc);
	s_http_server_opts.document_root = (new string(webRootPath))->c_str();
	cs_stat_t st;
	if (mg_stat(s_http_server_opts.document_root, &st) != 0) {
		cerr << "Cannot find webroot directory " << webRootPath;
	}

	for (int i = 0; i < s_num_worker_threads; i++) {
		 mg_start_thread(worker_thread_proc, &mgr);
	}

	// start a thread to run all webserver threads, such that this call returns immediately
	webserverThread = new std::thread(&Webserver::runningThread, this);

	// webserver is running
	isWebserverActive = true;
}

void Webserver::runningThread() {
	pthread_setname_np(pthread_self(), "webserver");

	terminateThread = false;

	while (!terminateThread ) {
		  mg_mgr_poll(&mgr, 200);
	}

	mg_mgr_free(&mgr);

	std::terminate(); // terminate that thread
}

void Webserver::print() {
	cout << endl
		<< "Webserver runs with " << s_num_worker_threads << " worker threads on port " << Configuration::getInstance().webserverPort << "." << endl;
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

string getURLParamValue(vector<string> names, vector<string> values, string key, bool& ok) {
	string value;
	ok = getURLParameter(names,values,key, value);
	return value;
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
		s << "\"status\":true";
	} else {
		s << "\"status\":false, \"error\":" << getLastError() << ", \"errormessage\":" << stringToJSonString(getErrorMessage(getLastError()));
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

	vector<string> urlParamNames;
	vector<string> urlParamValues;

	compileURLParameter(query,urlParamNames,urlParamValues);

	if (hasPrefix(uri, "/status")) {
		string engineCommand = uri.substr(string("/status").length());

		Dancer& dancer = Dancer::getInstance();
		AudioProcessor& audio = AudioProcessor::getInstance();

		std::ostringstream out;
		Pose head;
		MouthPose mouth;
		dancer.getThreadSafePose(head, mouth);
		out << "{ \"response\": "
		    <<     "{ \"head\":" << head.toString()
		    <<     ", \"mouth\":" << mouth.toString()
			<<     ", \"ambition\":" << dancer.getAmbition()
		    <<     ", \"move\":" << (int)dancer.getCurrentMove()
		    <<     ", \"music\":" << boolToJSonString(audio.isMusicDetected())
		    <<     ", \"auto\":" << boolToJSonString(dancer.getSequenceMode())
			<<     "} , " << getResponse(true)
		    << "}";
		response = out.str();
		okOrNOk = true;

		return okOrNOk;
	}

	cout << "command " << uri << " " << query << endl;
	if (hasPrefix(uri, "/console")) {
		string engineCommand = uri.substr(string("/console").length());
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

			bool ok = true;
			string name = getURLParamValue(urlParamNames,urlParamValues, "name",ok);
			cout << "receiving song " << name << endl;

			std::vector<uint8_t> wavContent;
			unsigned bodyLen = body.length();
			wavContent.resize(bodyLen);
			for (unsigned i = 0;i<bodyLen;i++) {
				wavContent[i] = body[i];
			}

			AudioProcessor::getInstance().setWavContent(wavContent);
			cout << "song " << name << " started." << endl;

			okOrNOk = true;

			response = getResponse(okOrNOk);

			return true;
		}
		else if (hasPrefix(engineCommand, "/ambition")) {
			bool ok = true;
			string value = getURLParamValue(urlParamNames,urlParamValues, "value",ok);
			if (ok) {
				double ambition = stringToFloat(value, ok);
				if (ok && (ambition >=0) && (ambition <=1.0)) {
					Dancer::getInstance().setAmbition(ambition);
				}
			}

			std::ostringstream out;
			out << "{ " << getResponse(ok) << "}";
			response = out.str();

			okOrNOk = true;
			return okOrNOk;
		}
		else if (hasPrefix(engineCommand, "/movemode")) {
			bool ok = true;
			string value = getURLParamValue(urlParamNames,urlParamValues, "value",ok);
			if (ok) {
				int moveMode = stringToInt(value, ok);
				Dancer::getInstance().setSequenceMode((Dancer::SequenceModeType)moveMode);
			}

			std::ostringstream out;
			out << "{ " << getResponse(ok)
			    << "}";
			response = out.str();

			okOrNOk = true;
			return okOrNOk;
		}

		else if (hasPrefix(engineCommand, "/move")) {
			bool ok = true;
			string value = getURLParamValue(urlParamNames,urlParamValues, "value",ok);
			if (ok) {
				int moveNo = stringToInt(value, ok);
				Dancer::getInstance().setCurrentMove((Move::MoveType)moveNo);
			}

			std::ostringstream out;
			out << "{ " << getResponse(ok)
			    << "}";
			response = out.str();
			okOrNOk = true;
			return okOrNOk;
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



	okOrNOk = false;
	return false;
}





