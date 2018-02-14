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
	currentMove = NO_MOVE;
	switchMoveAfterNBeats = 4;
	passedBeatsInCurrentMove = 0;
	beatCount = 0;
	rhythmInQuarters = 0;
}

void MoveMaker::simpleHeadNicker(double movePercentage) {
	// move with a simple sin curve that has its max at the beat (phase shift by PI/4)
	bodyPose = Pose(Point(0,0,bodyHeight + 70.0*sin(movePercentage*M_PI + M_PI/4)), Rotation (0,0,0));
}

void MoveMaker::backAndForthHeadNicker(double movePercentage) {

	double t = sin(sin(movePercentage/4.0*2.0*M_PI)*M_PI/2.0);
	double backAndForthMove = t;

	bodyPose = Pose(Point(50.0*backAndForthMove,0,bodyHeight + 70.0*sin(movePercentage*M_PI + M_PI/4)), Rotation (0,0,0));
}

void MoveMaker::createMove(double movePercentage) {
	cout << "%=" << std::fixed << std::setprecision(2) << movePercentage << " ";
	switch (currentMove) {
		case NO_MOVE:break;
		case SIMPLE_HEAD_NICKER:simpleHeadNicker(movePercentage);break;
		case BACK_AND_FORTH_HEAD_NICKER:backAndForthHeadNicker(movePercentage);break;
		default:
			simpleHeadNicker(movePercentage);
	}
	// bodyPose = Pose(Point(0,0,bodyHeight + 60.0*sin(movePercentage*M_PI)), Rotation (0,0,0));

}
void MoveMaker::loop(bool beat, double BPM) {
	double timeSinceBeat;
	double timePerBeat = (60.0/BPM); // in seconds



	if (beat) {
		// detect rhythm
		if (((rhythmInQuarters == 0) && (beatCount  == 3))) {
			timeSinceBeat = secondsSinceEpoch() - timeOfLastBeat;

			// detect 1/1 or 1/2 rhythm
			rhythmInQuarters = 1;
			if (abs(timePerBeat - timeSinceBeat) > abs(2.0*timePerBeat - timeSinceBeat))
				rhythmInQuarters = 2;
			if (abs(timePerBeat - timeSinceBeat) > abs(4.0*timePerBeat - timeSinceBeat))
				rhythmInQuarters = 4;
			cout << "Rhythm is 1/" << rhythmInQuarters << endl;
		}

		timeOfLastBeat = secondsSinceEpoch();
		if (!beatStarted)
			currentMove = SIMPLE_HEAD_NICKER;

		beatStarted = true;
		beatCount++;

		if ((switchMoveAfterNBeats > 0) && (passedBeatsInCurrentMove++ == switchMoveAfterNBeats)) {
			doNewMove();
			passedBeatsInCurrentMove = 0;
		}

	}


	// wait 4 beats to detect the rhythm
	if (beatCount > 3) {
		// compute elapsed time since last beat
		timeSinceBeat = secondsSinceEpoch() - timeOfLastBeat;

		createMove( (beatCount % 4 ) + timeSinceBeat/rhythmInQuarters/(60.0/BPM));
	}
}

void MoveMaker::doNewMove() {
	currentMove = (MoveType) (((int)currentMove + 1) % NumMoveTypes);

	cout << "new move: " << moveName(currentMove) << " " << endl;
}

void MoveMaker::switchMovePeriodically(int afterHowManyMoves) {
	switchMoveAfterNBeats = afterHowManyMoves;
}

string MoveMaker::moveName(MoveType m) {
	switch (m) {
		case NO_MOVE:return "no move";
		case SIMPLE_HEAD_NICKER:return "simple head nicker";
		case BACK_AND_FORTH_HEAD_NICKER:return "back and forth head nicker";
		default:
			return "";
	}
}

