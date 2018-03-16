

#include "client/httpCall.h"

#include <windows.h>
#include <string>
#include <stdio.h>

#include <basics/stringhelper.h>

using namespace std;


HINSTANCE hInst;
WSADATA wsaData;


SOCKET connectToServer(string szServerName, WORD portNum)
{
    struct hostent *hp;
    unsigned int addr;
    struct sockaddr_in server;
    SOCKET conn;

    conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (conn == INVALID_SOCKET)
        return (SOCKET)NULL;

    if(inet_addr(szServerName.c_str())==INADDR_NONE)
    {
        hp=gethostbyname(szServerName.c_str());
    }
    else
    {
        addr=inet_addr(szServerName.c_str());
        hp=gethostbyaddr((char*)&addr,sizeof(addr),AF_INET);
    }

    if(hp==NULL)
    {
        closesocket(conn);
        return (SOCKET)NULL;
    }

    server.sin_addr.s_addr=*((unsigned long*)hp->h_addr);
    server.sin_family=AF_INET;
    server.sin_port=htons(portNum);
    if(connect(conn,(struct sockaddr*)&server,sizeof(server)))
    {
        closesocket(conn);
        return (SOCKET)NULL;
    }
    return conn;
}


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

void  readUrl(int port, string url, string &httpResponse, int& httpStatus)
{
    const int bufSize = 512;
    char sendBuffer[bufSize], tmpBuffer[bufSize];
    SOCKET conn;
    string serverName, filepath;

    parseUrl(url, serverName, filepath);

    ///////////// step 1, connect //////////////////////
    conn = connectToServer(serverName, port);

    ///////////// step 2, send GET request /////////////
    sprintf(tmpBuffer, "GET %s HTTP/1.0", filepath.c_str());
    strcpy(sendBuffer, tmpBuffer);
    strcat(sendBuffer, "\r\n");
    sprintf(tmpBuffer, "Host: %s", serverName.c_str());
    strcat(sendBuffer, tmpBuffer);
    strcat(sendBuffer, "\r\n");
    strcat(sendBuffer, "\r\n");
    send(conn, sendBuffer, strlen(sendBuffer), 0);

    ///////////// step 3 - get received bytes ////////////////
    // Receive until the peer closes the connection
    int contentLength = -1;
    int headerLen = -1;
    httpStatus = -1;
    string response("");
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

    closesocket(conn);

    if ((int)response.length() ==  headerLen + contentLength) {
    	httpResponse = response.substr(headerLen);
    }
    else {
    	httpResponse = "";
    	cerr << "incomplete response in " << url << endl;
    }
}


void callHttp(string host, int port, string param, string& httpResponse, int &httpStatus)
{
    string request = "http://" + host ;
    if (param[0] != '/')
    	request += "/";
    request += param;

    char *szUrl = (char*)request.c_str();

    if ( WSAStartup(0x101, &wsaData) != 0) {
    	cerr << request << " failed" << endl;
    	httpStatus = 404;
        return;
    }

    readUrl(port, szUrl, httpResponse, httpStatus);

    WSACleanup();
}


