
#ifndef SRC_HTTP_CONNECTION_H_
#define SRC_HTTP_CONNECTION_H_

#include <string>
#include <stdio.h>

#ifdef __WIN32
#include <windows.h>
#endif

using namespace std;

class HttpConnection {
public:
	enum HttpAction { GET, POST };
	enum HttpContentType { AUDIO_WAV, JSON };
	HttpConnection();
	~HttpConnection();
	void setup(string host, int port);
	void get(const string& param, string &response, int& httpStatus);
	void post(const string& param, const string &query, string &httpResponse, int& httpStatus);


private:
	void call(HttpAction op, const string& param, const string& query, string& response, int& httpStatus);
#ifdef __WIN32
    SOCKET conn = (SOCKET)NULL;
#endif
    string host;
    int port;
};

#endif
