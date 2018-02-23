/*
 * MoveMaker.cpp
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */


#include "basics/util.h"

#include "MoveMaker.h"
#include "RhythmDetector.h"


MoveMaker::MoveMaker() {
}

MoveMaker::~MoveMaker() {
}

MoveMaker& MoveMaker::getInstance() {
	static MoveMaker mm;
	return mm;
}


Pose MoveMaker::getDefaultBodyPose() {
	return Pose(Point(0,0,bodyHeight), Rotation (0,0,0));
}

Pose MoveMaker::getDefaultHeadPose() {
	return Pose(Point(0,0,headHeight), Rotation (0,0,0));
}

void MoveMaker::setup() {
	bodyPose = getDefaultBodyPose();
	currentMove = Move::NO_MOVE;
	passedBeatsInCurrentMove = 0;
	startAfterNBeats = 4;
}



void MoveMaker::createMove(double movePercentage) {
	Pose nextPose = Move::getMove(currentMove).move(movePercentage);

	static TimeSamplerStatic moveTimer;

	// limit acceleration after changing the move
	if (movePercentage > 0.25)
		bodyPose.moveTo(nextPose, moveTimer.dT(), 400.0, 6.0);
	else
		bodyPose.moveTo(nextPose, moveTimer.dT(), 100.0, 2.0);
}

void MoveMaker::loop(bool beat, double BPM) {
	if (beat) {

		// first move is the classical head nicker
		if (RhythmDetector::getInstance().isFirstBeat())
			currentMove = (Move::MoveType)(0);

		// switch to next move after as soon as rhythm starts again
		passedBeatsInCurrentMove++;
		if ((sequenceMode == AUTOMATIC_SEQUENCE) &&
			(RhythmDetector::getInstance().getAbsoluteBeatCount() > startAfterNBeats) &&
			(RhythmDetector::getInstance().hasBeatStarted()) &&
			(passedBeatsInCurrentMove*RhythmDetector::getInstance().getRhythmInQuarters() >= Move::getMove(currentMove).getLength()) &&
			(RhythmDetector::getInstance().getBeatCount() == 0)) {
			doNewMove();
			passedBeatsInCurrentMove = 0;
		}
	}

	// wait 4 beats to detect the rhythm
	if ((RhythmDetector::getInstance().getAbsoluteBeatCount() > startAfterNBeats) && (RhythmDetector::getInstance().hasBeatStarted())) {
		// cout << std::fixed << std::setprecision(2) << "-->" << RhythmDetector::getInstance().getRythmPercentage() << bodyPose << endl;
		createMove(RhythmDetector::getInstance().getRythmPercentage());
	}
}

void MoveMaker::doNewMove() {
	// when all moves are shown, omit plain headnicker
	if ((int)currentMove >= Move::numMoves()-1)
		currentMove = (Move::MoveType)1; // don't restart with physicists move
	else
		currentMove = (Move::MoveType) (((int)currentMove + 1));

	cout << "new move: " << Move::getMove(currentMove).getName() << "(" << (int)currentMove << ")" << endl;
}

void MoveMaker::setCurrentMove(Move::MoveType m) {
	currentMove = m;
	passedBeatsInCurrentMove = 0;
}

