/*
 * BotClient.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: JochenAlt
 */

#include <client/BotClient.h>
#include "restclient/restclient.h"

BotClient::BotClient() {

}

BotClient::~BotClient() {
	// deinit RestClient. After calling this you have to call RestClient::init()
	// again before you can use it
	RestClient::disable();
}


BotClient& BotClient::getInstance() {
	static BotClient instance;
	return instance;
}


void BotClient::setup(string host) {
	// initialize RestClient
	RestClient::init();

	// get a connection object
	webserverConnection = new RestClient::Connection(host);

	// configure basic auth
	// webserverConnection->SetBasicAuth("odroid", "odroid");

	// set connection timeout to 5s
	webserverConnection->SetTimeout(5);

	// set custom user agent
	// webserverConnection->SetUserAgent("foo/cool");

	// enable following of redirects (default is off)
	// webserverConnection->FollowRedirects(true);

	// set headers
	RestClient::HeaderFields headers;
	headers["Accept"] = "application/json";
	webserverConnection->SetHeaders(headers);

	// append additional headers
	// webserverConnection->AppendHeader("X-MY-HEADER", "foo");

	// if using a non-standard Certificate Authority (CA) trust file
	// webserverConnection->SetCAInfoFilePath("/etc/custom-ca.crt");

	// set different content header for POST and PUT
	webserverConnection->AppendHeader("Content-Type", "text/json");

	// webclient is ready
	isWebClientActive = true;
}


void BotClient::getStatus() {
	RestClient::Response r = webserverConnection->get("/status");
	if (r.code != 200)
		cerr << "/status returned http code " << r.code << endl;
	std::istringstream in(r.body);
	bool ok = true;
	parseCharacter(in, '{', ok);
	parseString(in, ok); // "response"
   	parseCharacter(in, ':', ok);
	parseCharacter(in, '{', ok);
	parseString(in, ok); // "body"
   	parseCharacter(in, ':', ok);

	bodyPose.deserialize(in, ok);
	parseCharacter(in, ',', ok);
	parseString(in, ok); // "head"
   	parseCharacter(in, ':', ok);

	headPose.deserialize(in, ok);
	parseCharacter(in, ',', ok);
	parseString(in, ok); // "ambition"
   	parseCharacter(in, ':', ok);
	ambition = parseFloat(in, ok); // "ambition"
	parseCharacter(in, ',', ok);
	parseString(in, ok); // "move"
   	parseCharacter(in, ':', ok);
	int moveTmp = parseInt(in, ok); // "move"
	move = (Move::MoveType)moveTmp;
	parseCharacter(in, '}', ok);
	parseCharacter(in, ',', ok);
	parseString(in, ok); // "status"
   	parseCharacter(in, ':', ok);
   	bool status;
   	status = parseBool(in, ok);
	parseCharacter(in, '}', ok);
	if (!status)
		cerr << "rest response of /status " << r.body << endl;
}















