/*
 * BotClient.cpp
 *
 *  Created on: Mar 14, 2018
 *      Author: JochenAlt
 */

#include <client/BotClient.h>
#include <client/httpCall.h>

BotClient::BotClient() {

}

BotClient::~BotClient() {
}


BotClient& BotClient::getInstance() {
	static BotClient instance;
	return instance;
}


void BotClient::setup(string host, int port) {

	// webclient is ready
	isWebClientActive = true;
	conn.setup(host, port);
}


void BotClient::getStatus() {
	int httpStatus = -1;
	string httpResponse;
	conn.get("/status", httpResponse, httpStatus);

	if (httpStatus != 200)
		cerr << "/status returned http code " << httpResponse << endl;
	std::istringstream in(httpResponse);
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
   	bool status = parseBool(in, ok);
	parseCharacter(in, '}', ok);
}















