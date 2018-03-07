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

class BotDrawer {
public:
	BotDrawer() {};

	// display Stewart platform
	void displayBot(const Pose& bodyPose, const Pose& headPose);

	// setup by looking for the STL files
	void setup();

private:
	// read the stl files per actuator in that path
	void readSTLFiles(string path);

	STLObject chickenHead;
	STLObject chickenEyeBall;
	STLObject chickenEyeIris;
	STLObject chickenBeak;

	STLObject stewartBase;
	STLObject stewartPlate;
	STLObject stewartServoArm;
	STLObject stewartRod;

	STLObject stewartSmallRod;
	STLObject stewartHead;
	STLObject stewartSmallServoArm;

	VolumeOfRevolution body;
};

#endif /* UI_BOTDRAWER_H_ */
