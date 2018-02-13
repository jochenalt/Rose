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

class BotDrawer {
public:
	BotDrawer() {};
	static BotDrawer& getInstance() {
		static BotDrawer instance;
		return instance;
	}

	// display the bot with the given joint angles in the current openGL window
	void displayBot();

	// setup by looking for the STL files
	void setup();
private:
	// read the stl files per actuator in that path
	void readSTLFiles(string path);


	STLObject body;
};

#endif /* UI_BOTDRAWER_H_ */
