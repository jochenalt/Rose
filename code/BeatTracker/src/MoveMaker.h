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
	friend class Move;
public:
	enum SequenceModeType { AUTOMATIC_SEQUENCE, SELECTED_MOVE};

	MoveMaker();
	virtual ~MoveMaker();
	static MoveMaker& getInstance();
	Pose getDefaultBodyPose();
	Pose getDefaultHeadPose();

	// setup move maker before calling getBodyPose or getHeadPose
	void setup();

	// set the number of beats awaited until the first move starts
	void setStartAfterNBeats(int n) { startAfterNBeats = n; };

	// call this, and the move is updated. Should be invoked with 25Hz
	void loop(bool beat, double BPM);

	// current body pose within the move
	Pose& getBodyPose() { return pose.body; };

	// current head pose within the move
	Pose& getHeadPose() { return pose.head; };

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

	// scale the amplitude of all moves by ambition
	void setAmbition(int percentage) { ambition = percentage/100.0; };
	double getAmbition() { return ambition; };
private:
	void doNewMove();
	void createMove(double movePercentage);


	TotalBodyPose pose;
	Move::MoveType currentMove;
	int passedBeatsInCurrentMove;
	SequenceModeType sequenceMode;
	vector<Move> moveLibrary;
	int startAfterNBeats;
	double ambition = 1.0;
};

#endif /* MOVEMAKER_H_ */
