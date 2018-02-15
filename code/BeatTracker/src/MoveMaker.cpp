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
const double globalPhaseShift = -0.5;

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
	switchMoveAfterNBeats = 8;
	passedBeatsInCurrentMove = 0;
	beatCount = 0;
	rhythmInQuarters = 0;
}

double MoveMaker::scaleMove(double movePercentage, double speedFactor, double phase) {
	return fmod(movePercentage*speedFactor + phase, 4.0);
}


double MoveMaker::baseCurveCos(double movePercentage) {
	return cos(movePercentage/4.0*2.0*M_PI);
}


double MoveMaker::baseCurveFatCos(double movePercentage) {
	double x = movePercentage/4.0*2.0*M_PI;
	return (cos(2.0*x+M_PI)*0.25+1.25)*cos(x);
}


//
//      |\    /
//      |-\--/---
//      |  \/
//
double MoveMaker::baseCurveTriangle(double movePercentage) {
	if (movePercentage <= 2)
		return 1.0-movePercentage;
	else
		return movePercentage-3.0;
}

//
//      |\             /
//      |-------------
//      |      \/
//
double MoveMaker::baseCurveDip(double movePercentage) {
	return pow(baseCurveCos(movePercentage),21.0);
}

//       __
//      |  \      /
//      |---\----/---
//      |    \__/
//
double MoveMaker::baseCurveTrapezoid(double movePercentage) {
	if (movePercentage <= 1.0)
		return 1.0;
	else
		if (movePercentage <= 2.0)
			return 1.0-(movePercentage-1.0)*2.0;
		else
			if (movePercentage <= 3.0)
				return -1.0;
			else
				return -1.0 + (movePercentage-3.0)*2.0;
}


Pose MoveMaker::simpleHeadNicker(double movePercentage) {
	// used move curves
	double m1 = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));

	return Pose(Point(0,0,bodyHeight + 50.0*m1), Rotation (0,0,0));
}

Pose MoveMaker::travoltaHeadNicker(double movePercentage) {

	// used move curves
	double m1 = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double m2 = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 0.75+globalPhaseShift));

	return Pose(Point(0,30*m1,bodyHeight + 50.0*m1),Rotation (0,-radians(30)*m2,radians(20)*m2));
}


Pose MoveMaker::enhancedTravoltaHeadNicker(double movePercentage) {

	// used move curves
	double m1 = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double m2 = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 0.75+globalPhaseShift));
	double m3 = baseCurveCos(scaleMove(movePercentage, 4.0, globalPhaseShift));

	return Pose(Point(0,30*m1,bodyHeight + 50.0*m1),Rotation (-radians(20)*m3,-radians(30)*m3,radians(45)*m2));
}


Pose MoveMaker::tennisHeadNicker(double movePercentage) {

	double mNick = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mBase = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));

	return Pose(Point(0,0,bodyHeight + 50.0*mNick),Rotation (0,-radians(45)*mDip,-radians(45)*mBase));
}

Pose MoveMaker::doubleHeadNicker(double movePercentage) {
	double mLeftRight = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mNick = baseCurveCos(scaleMove(movePercentage, 4.0, globalPhaseShift));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift));

	return Pose(Point(0,40.0*mLeftRight,bodyHeight + 30.0*mNick),Rotation (0,-radians(0)*mDip,-radians(20)*mLeftRight-radians(60)*mDip));
}

Pose MoveMaker::diagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,0,0));
}

Pose MoveMaker::dippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,0,radians(30)*mEndDip));
}

Pose MoveMaker::rolledDippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));
	double mEndDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));
	double mRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, 1.0 + globalPhaseShift));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,radians(20)*mRoll,radians(20)*mRoll + radians(30)*mEndDip));
}


Pose MoveMaker::eyedDippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift));
	double mEyeRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, 1.0 + globalPhaseShift));


	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mHipDip),Rotation (radians(20)*mEyeRoll,0,radians(45)*mEndDip));
}

void MoveMaker::createMove(double movePercentage) {
	Pose nextPose;
	switch (currentMove) {
		case NO_MOVE:break;
		case SIMPLE_HEAD_NICKER:nextPose = simpleHeadNicker(movePercentage);break;
		case TENNIS_HEAD_NICKER:nextPose = tennisHeadNicker(movePercentage);break;
		case TRAVOLTA_HEAD_NICKER:nextPose = travoltaHeadNicker(movePercentage);break;
		case ENHANCED_TRAVOLTA_HEAD_NICKER:nextPose = enhancedTravoltaHeadNicker(movePercentage);break;
		case DOUBLE_HEAD_NICKER:nextPose = doubleHeadNicker(movePercentage);break;
		case DIAGONAL_HEAD_SWING: nextPose = diagonalSwing(movePercentage); break;
		case DIPPED_DIAGONAL_HEAD_SWING: nextPose = dippedDiagonalSwing(movePercentage); break;
		case ROLLED_DIPPED_DIAGONAL_HEAD_SWING: nextPose = rolledDippedDiagonalSwing(movePercentage); break;
		case EYED_DIPPED_DIAGONAL_HEAD_SWING: nextPose = eyedDippedDiagonalSwing(movePercentage); break;

		default:
			simpleHeadNicker(movePercentage);
	}
	static TimeSamplerStatic moveTimer;

	// limit acceleration after changing the move
	if (movePercentage > 0.25)
		bodyPose.moveTo(nextPose, moveTimer.dT(), 400.0, 5.0);
	else
		bodyPose.moveTo(nextPose, moveTimer.dT(), 100.0, 2.0);
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
			// cout << std::fixed << std::setprecision(2) << "Rhythm is 1/" << rhythmInQuarters << " tbp=" << timeSinceBeat << " s/beat=" <<  timePerBeat << "s" << "1:" << abs(timePerBeat - timeSinceBeat) << " 2:" << abs(2.0*timePerBeat - timeSinceBeat) << endl;
			cout << std::fixed << std::setprecision(2) << "Rhythm is 1/" << rhythmInQuarters << " s/beat=" <<  timePerBeat << "s" << endl;

		}

		timeOfLastBeat = secondsSinceEpoch();

		// first move is the classical head nicker
		if (!beatStarted)
			currentMove = SIMPLE_HEAD_NICKER;

		beatStarted = true;
		beatCount++;

		// switch to next move after some time
		passedBeatsInCurrentMove++;
		if ((sequenceMode == AUTOMATIC_SEQUENCE) && (beatCount > 3) && (passedBeatsInCurrentMove == switchMoveAfterNBeats)) {
			doNewMove();
			passedBeatsInCurrentMove = 0;
		}
	}

	// wait 4 beats to detect the rhythm
	if (beatCount > 3) {
		// compute elapsed time since last beat
		timeSinceBeat = secondsSinceEpoch() - timeOfLastBeat;

		double movePercentage = (beatCount % (4/rhythmInQuarters) )*rhythmInQuarters + timeSinceBeat/(60.0/BPM);
		// cout << std::fixed << std::setprecision(2) << "(" << (beatCount % (4/rhythmInQuarters) ) << ") t=" << timeSinceBeat << "s " << " 60/BPM=" << 60.0/BPM <<"s rhyt=" << rhythmInQuarters << "% =" << movePercentage << " "  << endl;

		createMove(movePercentage );

	}
}

void MoveMaker::doNewMove() {
	// when all moves are shown, omit plain headnicker
	if ((int)currentMove == NumMoveTypes-1)
		currentMove = (MoveType)1;
	else
		currentMove = (MoveType) (((int)currentMove + 1));

	cout << "new move: " << moveName(currentMove) << "(" << (int)currentMove << ")" << endl;
}

void MoveMaker::switchMovePeriodically(int afterHowManyMoves) {
	switchMoveAfterNBeats = afterHowManyMoves;
}

string MoveMaker::moveName(MoveType m) {
	switch (m) {
		case NO_MOVE:             				return "no move";
		case SIMPLE_HEAD_NICKER:				return "simple head nicker";
		case TRAVOLTA_HEAD_NICKER:      		return "travolta head nicker";
		case ENHANCED_TRAVOLTA_HEAD_NICKER: 	return "enhanced travolta head nicker";
		case DOUBLE_HEAD_NICKER:      			return "double head nicker";
		case TENNIS_HEAD_NICKER:        		return "tennis head nicker";
		case DIAGONAL_HEAD_SWING:				return "diagonal head swing";
		case DIPPED_DIAGONAL_HEAD_SWING:		return "dipped diagonal head swing";
		case ROLLED_DIPPED_DIAGONAL_HEAD_SWING:	return "rolled dipped diagonal head swing";
		case EYED_DIPPED_DIAGONAL_HEAD_SWING:	return "eyed dipped diagonal head swing";

		default:
			return "";
	}
}

void MoveMaker::setCurrentMove(MoveType m) {
	currentMove = m;
	passedBeatsInCurrentMove = 0;
}

