/*
 * MoveMaker.cpp
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#include <math.h>
#include "basics/util.h"

#include "MoveMaker.h"

const double bodyHeight = 100.0;

MoveMaker::MoveMaker() {
}

MoveMaker::~MoveMaker() {
}

MoveMaker& MoveMaker::getInstance() {
	static MoveMaker mm;
	return mm;
}

void MoveMaker::setup() {
	bodyPose = Pose(Point(0,0,bodyHeight), Rotation (0,0,0));
	timeOfLastBeat = 0;
	beatStarted = false;

}

void MoveMaker::createMove(double movePercentage) {

	bodyPose = Pose(Point(0,0,bodyHeight + 60.0*sin(movePercentage*M_PI)), Rotation (0,0,0));

}
void MoveMaker::loop(bool beat, double BPM) {
	if (beat) {
		timeOfLastBeat = secondsSinceEpoch();
		beatStarted = true;
	}
	double timeSinceBeat;
	if (beatStarted) {
		timeSinceBeat = secondsSinceEpoch() - timeOfLastBeat;
		createMove(timeSinceBeat/(60.0/BPM));
	}
}
