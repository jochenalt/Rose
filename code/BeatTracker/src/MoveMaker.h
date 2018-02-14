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

	double scaleMove(double movePercentage, double speedFactor, double phase);
	double baseCurveCos(double movePercentage);
	double baseCurveFatCos(double movePercentage);
	double baseCurveTriangle(double movePercentage);
	double baseCurveTrapezoid(double movePercentage);

	// methods implementing dance moves
	Pose simpleHeadNicker(double movePercentage);
	Pose travoltaHeadNicker(double movePercentage);
	Pose enhancedTravoltaHeadNicker(double movePercentage);
	Pose tennisHeadNicker(double movePercentage);


	Pose bodyPose;
	double timeOfLastBeat;
	bool beatStarted;
	int beatCount;
	int rhythmInQuarters;
	const int NumMoveTypes = 4;
	enum MoveType { SIMPLE_HEAD_NICKER, TENNIS_HEAD_NICKER, TRAVOLTA_HEAD_NICKER, ENHANCED_TRAVOLTA_HEAD_NICKER, NO_MOVE };
	MoveType currentMove;
	string moveName(MoveType m);

	int switchMoveAfterNBeats;
	int passedBeatsInCurrentMove;
};

#endif /* MOVEMAKER_H_ */
