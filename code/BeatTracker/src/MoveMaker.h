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

#include "DanceMove.h"


class MoveMaker {
public:
	enum SequenceModeType { AUTOMATIC_SEQUENCE, SELECTED_MOVE};

	MoveMaker();
	virtual ~MoveMaker();
	static MoveMaker& getInstance();
	Pose getDefaultPose();

	void setup();

	void setStartAfterNBeats(int n) { startAfterNBeats = n; };

	void loop(bool beat, double BPM);

	Pose& getBodyPose() { return bodyPose; };

	// set the number of moves after that the next move happens
	void switchMovePeriodically(int afterHowManyMoves);

	SequenceModeType getSequenceMode() { return sequenceMode; };

	void setSequenceMode(SequenceModeType newSequenceMode) { sequenceMode = newSequenceMode; };

	// get number of actual moves (without NO_MOVE)
	int getNumMoves() { return Move::numMoves(); };

	// get nice name of a move
	string getMoveName(Move::MoveType m) { return Move::getMove(m).getName(); };

	// set current move
	void setCurrentMove(Move::MoveType m);

	// return current move
	Move::MoveType getCurrentMove()  { return currentMove; };

private:
	void doNewMove();
	void createMove(double movePercentage);



	Pose bodyPose;
	Move::MoveType currentMove;

	int passedBeatsInCurrentMove;

	SequenceModeType sequenceMode;
	vector<Move> moveLibrary;
	int startAfterNBeats;
};

#endif /* MOVEMAKER_H_ */
