/*
 * DanceMove.h
 *
 *  Created on: Feb 16, 2018
 *      Author: jochenalt
 */

#ifndef SRC_DANCEMOVE_H_
#define SRC_DANCEMOVE_H_



#include "basics/spatial.h"
#include <string>
#include <vector>


const double bodyHeight = 120.0;

class Move {
public:
	enum MoveType { PHYSICISTS_HEAD_NICKER, TENNIS_HEAD_NICKER, DOUBLE_HEAD_NICKER,
		            TRAVOLTA_HEAD_NICKER, ENHANCED_TRAVOLTA_HEAD_NICKER,
					DIAGONAL_HEAD_SWING, DIPPED_DIAGONAL_HEAD_SWING, ROLLED_DIPPED_DIAGONAL_HEAD_SWING, EYED_DIPPED_DIAGONAL_HEAD_SWING,
					BOLLYWOOD_HEAD_MOVE, DOUBLE_BOLLYWOOD_MOVE, SWING_DOUBLE_BOLLYWOOD_MOVE,
					BODY_WAVE, DIPPED_BODY_WAVE, SIDE_DIPPED_BODY_WAVE,
					SHIMMYS, HIGH_SPEED_SHIMMYS,
					TURN_AND_SHOW_BACK, TWERK, TURN_BACK,
					NO_MOVE};

	static int numMoves() { return (int) NO_MOVE; };

	Move() {};
	Move(const Move& p) {
		id = p.id;
		name = p.name;
		lengthInBeats = p.lengthInBeats;
	};

	Move(MoveType newId, string newName, int newLengthInBeats) {
		id = newId;
		name = newName;
		lengthInBeats = newLengthInBeats;
	};

	static void setup();

	~Move() {};
	void operator= (const Move & p) {
		id = p.id;
		name = p.name;
		lengthInBeats = p.lengthInBeats;
	}
	bool operator== (const Move & p) {
		return p.id == id;
	}

	static Move& getMove(MoveType i);

	string getName() { return name; };
	MoveType getMoveType() { return id; };
	int getLength() { return lengthInBeats; };

	double scaleMove(double movePercentage, double speedFactor, double phase);
	double baseCurveCos(double movePercentage);
	double baseCurveFatCos(double movePercentage);
	double baseCurveTriangle(double movePercentage);
	double baseCurveTrapezoid(double movePercentage);
	double baseCurveDip(double movePercentage);
	double baseCurveFatDip(double movePercentage);

	// methods implementing dance moves
	Pose physicistsHeadNicker(double movePercentage);
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

	Pose shimmys(double movePercentage);
	Pose highSpeedShimmys(double movePercentage);

	Pose turnAndShowBack(double movePercentage);
	Pose twerk(double movePercentage);
	Pose turnBack(double movePercentage);

	Pose move(double movePercentage);

	MoveType id;
	string name;
	int lengthInBeats;
	static std::vector<Move> moveLibrary;
};

#endif /* SRC_DANCEMOVE_H_ */
