#ifdef __linux__

#include <client/httpCall.h>

#include <restclient/restclient.h>
#include <restclient/connection.h>

using namespace std;

RestClient::Connection* webserverConnection = NULL;


void  callHttp(string host, int port, string param, string &response, int& httpStatus) {
	// initialize RestClient
	RestClient::init();

	// get a connection object
	webserverConnection = new RestClient::Connection(host);

	// set connection timeout to 5s
	webserverConnection->SetTimeout(5);

	// set headers
	RestClient::HeaderFields headers;
	headers["Accept"] = "application/json";
	webserverConnection->SetHeaders(headers);

	// set different content header for POST and PUT
	webserverConnection->AppendHeader("Content-Type", "text/json");

	RestClient::Response r = webserverConnection->get("/status");
	httpStatus = r.code;
	response = r.body;

	// deinit RestClient. After calling this you have to call RestClient::init()
	// again before you can use it
	RestClient::disable();
}

#endif
