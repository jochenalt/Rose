

#ifdef _WIN32
#include "client/HttpConnection.h"

#include <windows.h>
#include <string>
#include <stdio.h>

#include <basics/stringhelper.h>

using namespace std;


void parseUrl(string mUrl, string &serverName, string &filepath)
{
    string::size_type n;
    string request = mUrl;

    if (request.substr(0,7) == "http://")
        request.erase(0,7);

    if (request.substr(0,8) == "https://")
        request.erase(0,8);

    n = request.find('/');
    if (n != string::npos)
    {
        serverName = request.substr(0,n);
        filepath = request.substr(n);
    }

    else
    {
        serverName = request;
        filepath = "/";
    }
}

int getContentLength(string content) {
    string srchStr1 = "Content-Length:";
    int findPos = content.find(srchStr1);
    if (findPos >= 0)
    {
        int ofset = findPos + srchStr1.length();
     	bool ok = true;
       	int tmp = stringToInt(content.substr(ofset), ok);
       	if (ok)
       		return tmp;
    }
    return -1;
}

int getHttpStatus(string content) {
    string srchStr1 = "HTTP/1.1";
    int findPos = content.find(srchStr1);
    if (findPos >= 0)
    {
        int ofset = findPos + srchStr1.length();
     	bool ok = true;
       	int tmp = stringToInt(content.substr(ofset), ok);
       	if (ok)
       		return tmp;
    }
    return -1;
}

int getHeaderLength(string content)
{
    string srchStr1 = "\r\n\r\n";
    string srchStr2 = "\n\r\n\r";
    int ofset = -1;

    int findPos = content.find(srchStr1);
    if (findPos >= 0)
    {
        ofset = findPos + srchStr1.length();
    }

    else
    {
        findPos = content.find(srchStr2);
        if (findPos >= 0)
        {
            ofset = findPos + srchStr2.length();
        }
    }
    return ofset;
}


HttpConnection::HttpConnection() {
    WSADATA wsaData;
    if ( WSAStartup(0x101, &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return;
    }
}

HttpConnection::~HttpConnection() {
    closesocket(conn);

    WSACleanup();
}

void HttpConnection::setup(string newHost, int newPort) {
	host = newHost;
	port = newPort;
	  struct hostent *hp;
	    unsigned int addr;
	    struct sockaddr_in server;

	    conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    if (conn == INVALID_SOCKET)
	        return ;

	    if(inet_addr(host.c_str())==INADDR_NONE)
	    {
	        hp=gethostbyname(host.c_str());
	    }
	    else
	    {
	        addr=inet_addr(host.c_str());
	        hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
	    }

	    if(hp==NULL)
	    {
	        closesocket(conn);
	        conn = 0;
	        return;
	    }

	    server.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
	    server.sin_family=AF_INET;
	    server.sin_port=htons(port);
	    if(connect(conn,(struct sockaddr*)&server,sizeof(server)))
	    {
	        closesocket(conn);
	        conn = 0;
	        return;
	    }
}

void HttpConnection::get(string param, string &httpResponse, int& httpStatus) {
	 const int bufSize = 512;
    char sendBuffer[bufSize], tmpBuffer[bufSize];

	    ///////////// step 2, send GET request /////////////
	    sprintf(tmpBuffer, "GET %s HTTP/1.0", param.c_str());
	    strcpy(sendBuffer, tmpBuffer);
	    strcat(sendBuffer, "\r\n");
	    sprintf(tmpBuffer, "Host: %s", host.c_str());
	    strcat(sendBuffer, tmpBuffer);
	    strcat(sendBuffer, "\r\n");
	    strcat(sendBuffer, "\r\n");
	    send(conn, sendBuffer, strlen(sendBuffer), 0);

	    ///////////// step 3 - get received bytes ////////////////
	    // Receive until the peer closes the connection
	    int contentLength = -1;
	    int headerLen = -1;
	    httpStatus = -1;
	    string response = "";
	    while ((contentLength == -1) || (headerLen == -1) || ((int)response.length() < headerLen + contentLength))
	    {
	    	const int readBufferSize = 1024;
	    	char readBuffer[readBufferSize];
	        memset(readBuffer, 0, readBufferSize);
	        int chunkSize = recv (conn, readBuffer, readBufferSize, 0);
	        if ( chunkSize <= 0 )
	            break;

	        response += string (readBuffer,chunkSize);
	        if (headerLen == -1)
	        	headerLen = getHeaderLength(response);
	        if (contentLength == -1)
	        	contentLength = getContentLength(response);
	    }

	    if (httpStatus == -1)
	    	httpStatus = getHttpStatus(response);


	    if ((int)response.length() ==  headerLen + contentLength) {
	    	httpResponse = response.substr(headerLen);
	    }
	    else {
	    	httpResponse = "";
	    	cerr << "incomplete response in " << param << endl;
	    }
}

#endif
