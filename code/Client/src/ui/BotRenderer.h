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
	enum ClothingModeType { NORMAL_MODE, TRANSPARENT_MODE, NAKED_MODE };

	BotRenderer() {};
	~BotRenderer() {};

	// display Stewart platform
	void displayBot(const TotalBodyPose& pose);

	// setup by looking for the STL files
	void setup();

	void setClothingMode (ClothingModeType mode) { clothingMode = mode; };

private:
	// read the stl files per actuator in that path
	void readSTLFiles(string path);

	STLObject servoBlock;
	STLObject mouthServoArmUpper;
	STLObject mouthServoArmLower;
	STLObject lowerLip;
	STLObject upperLip;
	STLObject mouthLever;

	STLObject baseStewart;
	STLObject baseStewartServoArm;
	STLObject baseStewartRod;
	STLObject topPlatform;


	VolumeOfRevolution body;

	ClothingModeType clothingMode = NORMAL_MODE;
};

#endif /* UI_BOTDRAWER_H_ */
