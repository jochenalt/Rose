/*
 * UI.cpp
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#include <iostream>

#include <UI.h>
#include "WindowController.h"

UI::UI() {
}

UI::~UI() {
}


UI& UI::getInstance() {
	static UI instance;
	return instance;
}


void UI::setup(int argc, char *argv[]) {

	// initialize ui
	bool UISetupOk= WindowController::getInstance().setup(argc, argv);
	if (!UISetupOk) {
		cerr << "UI initialization failed" << endl;
		exit(1);
	}
}
