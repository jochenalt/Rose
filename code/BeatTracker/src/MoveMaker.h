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
	const int NumMoveTypes = 9;

	enum MoveType { SIMPLE_HEAD_NICKER, TENNIS_HEAD_NICKER, DOUBLE_HEAD_NICKER,
		            TRAVOLTA_HEAD_NICKER, ENHANCED_TRAVOLTA_HEAD_NICKER,
					DIAGONAL_HEAD_SWING, DIPPED_DIAGONAL_HEAD_SWING, ROLLED_DIPPED_DIAGONAL_HEAD_SWING, EYED_DIPPED_DIAGONAL_HEAD_SWING,
					NO_MOVE };
	enum SequenceModeType { AUTOMATIC_SEQUENCE, SELECTED_MOVE};

	MoveMaker();
	virtual ~MoveMaker();
	static MoveMaker& getInstance();
	void setup();
	void loop(bool beat, double BPM);

	Pose& getBodyPose() { return bodyPose; };

	// set the number of moves after that the next move happens
	void switchMovePeriodically(int afterHowManyMoves);

	SequenceModeType getSequenceMode() { return sequenceMode; };

	void setSequenceMode(SequenceModeType newSequenceMode) { sequenceMode = newSequenceMode; };

	// get number of actual moves (without NO_MOVE)
	int getNumMoves() { return NumMoveTypes; };

	// get nice name of a move
	string getMoveName(MoveType m) { return moveName(m); };

	// set current move
	void setCurrentMove(MoveType m);

	// return current move
	MoveType getCurrentMove()  { return currentMove; };

private:
	void doNewMove();
	void createMove(double movePercentage);

	double scaleMove(double movePercentage, double speedFactor, double phase);
	double baseCurveCos(double movePercentage);
	double baseCurveFatCos(double movePercentage);
	double baseCurveTriangle(double movePercentage);
	double baseCurveTrapezoid(double movePercentage);
	double baseCurveDip(double movePercentage);

	// methods implementing dance moves
	Pose simpleHeadNicker(double movePercentage);
	Pose tennisHeadNicker(double movePercentage);
	Pose travoltaHeadNicker(double movePercentage);
	Pose enhancedTravoltaHeadNicker(double movePercentage);
	Pose doubleHeadNicker(double movePercentage);
	Pose diagonalSwing(double movePercentage);
	Pose dippedDiagonalSwing(double movePercentage);
	Pose rolledDippedDiagonalSwing(double movePercentage);
	Pose eyedDippedDiagonalSwing(double movePercentage);


	Pose bodyPose;
	double timeOfLastBeat;
	bool beatStarted;
	int beatCount;
	int rhythmInQuarters;
	MoveType currentMove;
	string moveName(MoveType m);

	int switchMoveAfterNBeats;
	int passedBeatsInCurrentMove;

	SequenceModeType sequenceMode;
};

#endif /* MOVEMAKER_H_ */
