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
#include <vector>



class Move {
public:
	enum MoveType { SIMPLE_HEAD_NICKER, TENNIS_HEAD_NICKER, DOUBLE_HEAD_NICKER,
		            TRAVOLTA_HEAD_NICKER, ENHANCED_TRAVOLTA_HEAD_NICKER,
					DIAGONAL_HEAD_SWING, DIPPED_DIAGONAL_HEAD_SWING, ROLLED_DIPPED_DIAGONAL_HEAD_SWING, EYED_DIPPED_DIAGONAL_HEAD_SWING,
					BOLLYWOOD_HEAD_MOVE, DOUBLE_BOLLYWOOD_MOVE, SWING_DOUBLE_BOLLYWOOD_MOVE,
					BODY_WAVE, DIPPED_BODY_WAVE, SIDE_DIPPED_BODY_WAVE,
					TURN_AND_SHOW_BACK, TWERK, TURN_BACK,
					NO_MOVE};

	static int numMoves() { return (int) NO_MOVE; };

	Move() {};
	Move(const Move& p) {
		id = p.id;
		name = p.name;
	};

	Move(MoveType newId, string newName) {
		id = newId;
		name = newName;
	};

	static void setup();

	~Move() {};
	void operator= (const Move & p) {
		id = p.id;
		name = p.name;
	}
	bool operator== (const Move & p) {
		return p.id == id;
	}

	static Move& getMove(MoveType i);

	string getName() { return name; };
	MoveType getMoveType() { return id; };

	double scaleMove(double movePercentage, double speedFactor, double phase);
	double baseCurveCos(double movePercentage);
	double baseCurveFatCos(double movePercentage);
	double baseCurveTriangle(double movePercentage);
	double baseCurveTrapezoid(double movePercentage);
	double baseCurveDip(double movePercentage);
	double baseCurveFatDip(double movePercentage);

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

	Pose bollywoodHeadMove(double movePercentage);
	Pose doubleBollywoodHeadMove(double movePercentage);
	Pose swingDoubleBollywoodHeadMove(double movePercentage);

	Pose bodyWaveMove(double movePercentage);
	Pose dipBodyWaveMove(double movePercentage);
	Pose sidedDipBodyWaveMove(double movePercentage);

	Pose turnAndShowBack(double movePercentage);
	Pose twerk(double movePercentage);
	Pose turnBack(double movePercentage);

	Pose move(double movePercentage);

	MoveType id;
	string name;
	static std::vector<Move> moveLibrary;
};

class MoveMaker {
public:
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

	int switchMoveAfterNBeats;
	int passedBeatsInCurrentMove;

	SequenceModeType sequenceMode;
	vector<Move> moveLibrary;
};

#endif /* MOVEMAKER_H_ */
