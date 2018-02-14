/*
 * MoveMaker.h
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#ifndef MOVEMAKER_H_
#define MOVEMAKER_H_

#include "basics/spatial.h"
#include <string>

class MoveMaker {
public:
	MoveMaker();
	virtual ~MoveMaker();
	static MoveMaker& getInstance();
	void setup();
	void loop(bool beat, double BPM);
	Pose& getBodyPose() { return bodyPose; };
	void switchMovePeriodically(int afterHowManyMoves);
private:
	void doNewMove();

	void createMove(double movePercentage);

	void simpleHeadNicker(double movePercentage);
	void backAndForthHeadNicker(double movePercentage);


	Pose bodyPose;
	double timeOfLastBeat;
	bool beatStarted;
	int beatCount;
	int rhythmInQuarters;
	const int NumMoveTypes = 2;
	enum MoveType { SIMPLE_HEAD_NICKER, BACK_AND_FORTH_HEAD_NICKER, NO_MOVE };
	MoveType currentMove;
	string moveName(MoveType m);

	int switchMoveAfterNBeats;
	int passedBeatsInCurrentMove;
};

#endif /* MOVEMAKER_H_ */
