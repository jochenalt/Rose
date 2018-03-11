/*
 * BotDrawer.h
 *
 * Draw the bot given by stl files in openGL
 *
 *  Created on: 18.08.2016
 *      Author: JochenAlt
 */

#ifndef UI_BOTDRAWER_H_
#define UI_BOTDRAWER_H_

#include "STLObject.h"
#include "basics/spatial.h"
#include "VolumeOfRevolution.h"

class BotRenderer {
public:
	BotRenderer() {};

	// display Stewart platform
	void displayBot(const Pose& bodyPose, const Pose& headPose);

	// setup by looking for the STL files
	void setup();

	void setStripper(bool ok) { isStripper = ok; };

private:
	// read the stl files per actuator in that path
	void readSTLFiles(string path);

	STLObject head;
	STLObject eyeBall;
	STLObject iris;

	STLObject baseStewartPlatform;
	STLObject baseStewartServoArm;
	STLObject baseStewartRod;

	STLObject intermediateStewartPlatform;
	STLObject stewartRod;

	STLObject headStewartPlatform;
	STLObject topStewartServoArm;

	VolumeOfRevolution body;
	bool isStripper = false; // Donna is not a slut
};

#endif /* UI_BOTDRAWER_H_ */
