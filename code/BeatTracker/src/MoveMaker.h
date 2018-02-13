/*
 * MoveMaker.h
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#ifndef MOVEMAKER_H_
#define MOVEMAKER_H_

#include "basics/spatial.h"

class MoveMaker {
public:
	MoveMaker();
	virtual ~MoveMaker();
	static MoveMaker& getInstance();
	void setup();
	void loop(bool beat, double BPM);
	Pose& getBodyPose() { return bodyPose; };
private:
	void createMove(double movePercentage);
	Pose bodyPose;
	double timeOfLastBeat;
	bool beatStarted;
};

#endif /* MOVEMAKER_H_ */
