#ifdef __linux__

#include <client/httpCall.h>

#include <restclient/restclient.h>
#include <restclient/connection.h>

#include <basics/stringhelper.h>
using namespace std;



HttpConnection::HttpConnection() {
	// initialize RestClient
	RestClient::init();

}

HttpConnection::~HttpConnection() {
	// deinit RestClient. After calling this you have to call RestClient::init()
	// again before you can use it
	RestClient::disable();
}

void HttpConnection::setup(string host,int port) {

	// get a connection object
	string baseUrl = host + ":" + intToString(port);
	webserverConnection = new RestClient::Connection(baseUrl);

	// set connection timeout to 5s
	webserverConnection->SetTimeout(5);
}


void HttpConnection::get(string param, string &response, int& httpStatus) {
	// set headers
	RestClient::HeaderFields headers;
	headers["Accept"] = "application/json";
	webserverConnection->SetHeaders(headers);

	// set different content header for POST and PUT
	webserverConnection->AppendHeader("Content-Type", "text/json");

	RestClient::Response r = webserverConnection->get(param);
	httpStatus = r.code;
	response = r.body;
}


#endif
