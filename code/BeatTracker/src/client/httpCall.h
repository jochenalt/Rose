
#ifndef SRC_HTTP_CONNECTION_H_
#define SRC_HTTP_CONNECTION_H_

#include <string>
#include <stdio.h>

#ifdef __WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <restclient/connection.h>
#endif

using namespace std;

class HttpConnection {
public:
	HttpConnection();
	~HttpConnection();
	void setup(string host, int port);
	void get(string param, string &response, int& httpStatus);

private:
#ifdef __WIN32
    SOCKET conn = (SOCKET)NULL;
#endif
#ifdef __linux__
    RestClient::Connection* webserverConnection = NULL;
#endif
    string host;
    int port;
};

#endif
