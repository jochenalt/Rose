/*
 * MoveMaker.h
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#ifndef MOVEMAKER_H_
#define MOVEMAKER_H_

#include <string>

#include <dance/Move.h>
#include "basics/spatial.h"



class Dancer  {
	friend class Move;
public:
	enum SequenceModeType { AUTOMATIC_SEQUENCE, SELECTED_MOVE};

	Dancer ();
	virtual ~Dancer ();
	static Dancer & getInstance();
	Pose getDefaultBodyPose();
	Pose getDefaultHeadPose();

	// setup move maker before calling getBodyPose or getHeadPose
	void setup();

	// set the number of beats awaited until the first move starts
	void setStartAfterNBeats(int n) { startAfterNBeats = n; };

	// call this with audio data to update the move. Should be invoked with 25Hz
	void danceLoop(bool beat, double BPM);

	// call this with data coming from the webserver to define the current dancing parameters withou generating them
	void imposeDanceParams(Move::MoveType newCurrentMove, double newAmbition,
			               const Pose& newBodyPose, const Pose& newHeadPose);

	// current body pose within the move
	Pose& getBodyPose() { return pose.body; };

	// current head pose within the move
	Pose& getHeadPose() { return pose.head; };

	// get current pose from a different thread than the one running danceLoop()
	void getThreadSafePose(Pose& bodyPose, Pose& headPose);

	// set the number of moves after that the next move happens
	void switchMovePeriodically(int afterHowManyMoves);

	// sequence mode is if we change the move automatically or stick to the same move forever
	SequenceModeType getSequenceMode() { return sequenceMode; };
	void setSequenceMode(SequenceModeType newSequenceMode) { sequenceMode = newSequenceMode; };

	// get number of actual moves (without NO_MOVE)
	int getNumMoves() { return Move::numMoves(); };

	// get nice name of a move
	string getMoveName(Move::MoveType m) { return Move::getMove(m).getName(); };

	// switch immediatelly to the passed dance move
	void setCurrentMove(Move::MoveType m);

	// return currently danced move
	Move::MoveType getCurrentMove()  { return currentMove; };

	// scale the amplitude of all moves by ambition
	void setAmbition(double newAmbition) { ambition = newAmbition; };
	double getAmbition() { return ambition; };

	void setMusicDetected(bool isMusicMetected) {
		musicDetected = isMusicMetected;
	}

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

	ExclusiveMutex poseMutex;
	bool musicDetected = false;
};

#endif /* MOVEMAKER_H_ */
