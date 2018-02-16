/*
 * MoveMaker.cpp
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#include <math.h>
#include "basics/util.h"

#include "MoveMaker.h"
#include "RhythmDetector.h"

const double bodyHeight = 100.0;
const double globalPhaseShift = -0.25;

std::vector<Move> Move::moveLibrary;

Move& Move::getMove(MoveType m) {
	setup();
	return moveLibrary[(int)m];
}

void Move::setup() {
	if (moveLibrary.size() == 0) {
		moveLibrary.resize((int)NO_MOVE+1);

		moveLibrary[(int)SIMPLE_HEAD_NICKER] = Move(SIMPLE_HEAD_NICKER, "phyisist's move");
		moveLibrary[(int)TENNIS_HEAD_NICKER] = Move(TENNIS_HEAD_NICKER, "tennis spectators's move");
		moveLibrary[(int)DOUBLE_HEAD_NICKER] = Move(DOUBLE_HEAD_NICKER, "weasel's move");

		moveLibrary[(int)TRAVOLTA_HEAD_NICKER] = Move(TRAVOLTA_HEAD_NICKER, "travolta move");
		moveLibrary[(int)ENHANCED_TRAVOLTA_HEAD_NICKER] = Move(ENHANCED_TRAVOLTA_HEAD_NICKER, "enhanced travolta move");

		moveLibrary[(int)DIAGONAL_HEAD_SWING] = Move(DIAGONAL_HEAD_SWING, "diagonal head move");
		moveLibrary[(int)DIPPED_DIAGONAL_HEAD_SWING] = Move(DIPPED_DIAGONAL_HEAD_SWING, "dipped diagonal head move");
		moveLibrary[(int)ROLLED_DIPPED_DIAGONAL_HEAD_SWING] = Move(ROLLED_DIPPED_DIAGONAL_HEAD_SWING, "rolle dipped diagonal move");
		moveLibrary[(int)EYED_DIPPED_DIAGONAL_HEAD_SWING] = Move(EYED_DIPPED_DIAGONAL_HEAD_SWING, "eye-rolling diagonal move");

		moveLibrary[(int)BOLLYWOOD_HEAD_MOVE] = Move(BOLLYWOOD_HEAD_MOVE, "bollywood move");
		moveLibrary[(int)DOUBLE_BOLLYWOOD_MOVE] = Move(DOUBLE_BOLLYWOOD_MOVE, "double bollywood move");
		moveLibrary[(int)SWING_DOUBLE_BOLLYWOOD_MOVE] = Move(SWING_DOUBLE_BOLLYWOOD_MOVE, "swinging bollywood move");

		moveLibrary[(int)BODY_WAVE] = Move(BODY_WAVE, "body wave");
		moveLibrary[(int)DIPPED_BODY_WAVE] = Move(DIPPED_BODY_WAVE, "dipped body wave");
		moveLibrary[(int)SIDE_DIPPED_BODY_WAVE] = Move(SIDE_DIPPED_BODY_WAVE, "rolling body wave");

		moveLibrary[(int)TURN_AND_SHOW_BACK] = Move(TURN_AND_SHOW_BACK, "show your back");
		moveLibrary[(int)TWERK] = Move(TWERK, "twerk");
		moveLibrary[(int)TURN_BACK] = Move(TURN_BACK, "show your front");
		moveLibrary[(int)NO_MOVE] = Move(NO_MOVE, "no move");
	}
}


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
	currentMove = Move::NO_MOVE;
	switchMoveAfterNBeats = 8;
	passedBeatsInCurrentMove = 0;
}



double Move::scaleMove(double movePercentage, double speedFactor, double phase) {
	return fmod(movePercentage*speedFactor + phase, 4.0);
}


double Move::baseCurveCos(double movePercentage) {
	return cos(movePercentage/4.0*2.0*M_PI);
}


double Move::baseCurveFatCos(double movePercentage) {
	double x = movePercentage/4.0*2.0*M_PI;
	return (cos(2.0*x+M_PI)*0.25+1.25)*cos(x);
}


//
//      |\    /
//      |-\--/---
//      |  \/
//
double Move::baseCurveTriangle(double movePercentage) {
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
double Move::baseCurveDip(double movePercentage) {
	return pow(baseCurveCos(movePercentage),21.0);
}

double Move::baseCurveFatDip(double movePercentage) {
	return pow(baseCurveCos(movePercentage),3.0);
}

//       __
//      |  \      /
//      |---\----/---
//      |    \__/
//
double Move::baseCurveTrapezoid(double movePercentage) {
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



Pose Move::simpleHeadNicker(double movePercentage) {
	// used move curves
	double mUpDown = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));

	return Pose(Point(0,0,bodyHeight + 50.0*mUpDown), Rotation (0,0,0));
}

Pose Move::travoltaHeadNicker(double movePercentage) {

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 0.75+globalPhaseShift));

	return Pose(Point(0,30*mBase,bodyHeight + 50.0*mBase),Rotation (0,-radians(30)*mUpDown,radians(20)*mUpDown));
}


Pose Move::enhancedTravoltaHeadNicker(double movePercentage) {

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 0.75+globalPhaseShift));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 4.0, globalPhaseShift));

	return Pose(Point(0,30*mBase,bodyHeight + 50.0*mBase),Rotation (-radians(20)*mSwing,-radians(30)*mSwing,radians(45)*mUpDown));
}


Pose Move::tennisHeadNicker(double movePercentage) {

	double mNick = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mBase = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, 1.25 + globalPhaseShift)));

	return Pose(Point(0,0,bodyHeight + 50.0*mNick),Rotation (0,-radians(45)*mDip,-radians(45)*mBase));
}

Pose Move::doubleHeadNicker(double movePercentage) {
	double mLeftRight = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mNick = baseCurveCos(scaleMove(movePercentage, 4.0, globalPhaseShift));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.25 + globalPhaseShift));

	return Pose(Point(0,40.0*mLeftRight,bodyHeight + 30.0*mNick),Rotation (0,-radians(0)*mDip,-radians(20)*mLeftRight-radians(60)*mDip));
}

Pose Move::diagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,0,0));
}

Pose Move::dippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.25 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.25 + globalPhaseShift)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.25 + globalPhaseShift));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,0,radians(30)*mEndDip));
}

Pose Move::rolledDippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.25 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.25 + globalPhaseShift)));
	double mEndDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, 1.25 + globalPhaseShift)));
	double mRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, 1.25 + globalPhaseShift));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,-radians(20)*mRoll,-radians(20)*mRoll + radians(30)*mEndDip));
}


Pose Move::eyedDippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 0.0 + globalPhaseShift));
	double mEyeRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, 1.25 + globalPhaseShift));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mHipDip),Rotation (radians(20)*mEyeRoll,0,radians(0)*mEndDip));
}

Pose Move::bollywoodHeadMove(double movePercentage) {
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.25 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));
	double mHeadMove = baseCurveFatCos(scaleMove(movePercentage, 1.0, 1.5 + globalPhaseShift));

	return Pose(Point(0,50.0*mBase,bodyHeight + 20.0*mDip),Rotation (radians(45)*mHeadMove, 0,0));
}


Pose Move::doubleBollywoodHeadMove(double movePercentage) {
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 0.5, 0.0 + globalPhaseShift));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.5 + globalPhaseShift));
	double mHeadMove = baseCurveCos(scaleMove(movePercentage, 4.0, 1.25 + globalPhaseShift));

	return Pose(Point(0,00.0*mBase + 20*mHeadMove,bodyHeight + 0.0*mDip),Rotation (0, 0,radians(30)*mDip));
}

Pose Move::swingDoubleBollywoodHeadMove(double movePercentage) {
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 0.5, 2.0 + globalPhaseShift));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.5 + globalPhaseShift));
	double mHeadMove = baseCurveCos(scaleMove(movePercentage, 4.0, 1.25 + globalPhaseShift));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 2.0, 1.0 + globalPhaseShift));

	return Pose(Point(0,00.0*mBase + 20*mHeadMove,bodyHeight + 50.0*(1.0-fabs(mDip))),Rotation (0,radians(30)*mSwing,radians(30)*mDip));
}

Pose Move::bodyWaveMove(double movePercentage) {

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, 0.75 + globalPhaseShift));

	return Pose(Point(0,0,bodyHeight + 50.0*mBase),Rotation (0,-radians(20)*mWave,0));
}


Pose Move::dipBodyWaveMove(double movePercentage) {

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, 0.25 + globalPhaseShift));
	double mDip= baseCurveDip(scaleMove(movePercentage, 1.0, 1.25 + globalPhaseShift));

	return Pose(Point(0,0,bodyHeight + 50.0*mBase),Rotation (0,-radians(15)*mWave,radians(45)*mDip));
}


Pose Move::sidedDipBodyWaveMove(double movePercentage) {

	// used move curves
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, 0.0  + globalPhaseShift));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, globalPhaseShift));
	return Pose(Point(0,50.0*mSideHip,bodyHeight + 50.0*mBase),Rotation (0,-radians(15)*mWave,0));
}

Pose Move::turnAndShowBack(double movePercentage) {
	// used move curves
	double mHeadNicker= baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mTurn= movePercentage/4.0;
	return Pose(Point(0,0,bodyHeight + 50.0*mHeadNicker), Rotation (0,0,radians(mTurn*180.0)));
}

Pose Move::twerk(double movePercentage) {

	// used move curves
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, 0.0  + globalPhaseShift));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.5 + globalPhaseShift));
	return Pose(Point(0,50.0*mSideHip,bodyHeight),Rotation (0,-radians(30)*mWave+radians(15),radians(180.0)));
}

Pose Move::turnBack(double movePercentage) {
	// used move curves
	double mHeadNicker = baseCurveCos(scaleMove(movePercentage, 2.0,globalPhaseShift));
	double mTurn= movePercentage/4.0;

	return Pose(Point(0,0,bodyHeight + 50.0*mHeadNicker), Rotation (0,0,radians(180.0-180.0*mTurn)));
}


Pose Move::move(double movePercentage) {
	switch (id) {
		case SIMPLE_HEAD_NICKER:return simpleHeadNicker(movePercentage);break;
		case TENNIS_HEAD_NICKER:return tennisHeadNicker(movePercentage);break;
		case TRAVOLTA_HEAD_NICKER:return  travoltaHeadNicker(movePercentage);break;
		case ENHANCED_TRAVOLTA_HEAD_NICKER:return enhancedTravoltaHeadNicker(movePercentage);break;
		case DOUBLE_HEAD_NICKER:return doubleHeadNicker(movePercentage);break;
		case DIAGONAL_HEAD_SWING: return diagonalSwing(movePercentage); break;
		case DIPPED_DIAGONAL_HEAD_SWING: return dippedDiagonalSwing(movePercentage); break;
		case ROLLED_DIPPED_DIAGONAL_HEAD_SWING: return rolledDippedDiagonalSwing(movePercentage); break;
		case EYED_DIPPED_DIAGONAL_HEAD_SWING: return eyedDippedDiagonalSwing(movePercentage); break;
		case BOLLYWOOD_HEAD_MOVE: return bollywoodHeadMove(movePercentage); break;
		case DOUBLE_BOLLYWOOD_MOVE: return  doubleBollywoodHeadMove(movePercentage); break;
		case SWING_DOUBLE_BOLLYWOOD_MOVE: return swingDoubleBollywoodHeadMove(movePercentage); break;
		case BODY_WAVE: return bodyWaveMove(movePercentage); break;
		case DIPPED_BODY_WAVE: return dipBodyWaveMove(movePercentage); break;
		case SIDE_DIPPED_BODY_WAVE: return sidedDipBodyWaveMove(movePercentage); break;
		case TURN_AND_SHOW_BACK:return turnAndShowBack(movePercentage); break;
		case TWERK:			return twerk(movePercentage); break;
		case TURN_BACK:		return turnBack(movePercentage); break;
		default:
			return Pose();
	}
}


void MoveMaker::createMove(double movePercentage) {
	Pose nextPose = Move::getMove(currentMove).move(movePercentage);

	static TimeSamplerStatic moveTimer;

	// limit acceleration after changing the move
	/*
	if (movePercentage > 0.25)
		bodyPose.moveTo(nextPose, moveTimer.dT(), 400.0, 6.0);
	else
		bodyPose.moveTo(nextPose, moveTimer.dT(), 100.0, 3.0);
		*/
	bodyPose = nextPose;
}

void MoveMaker::loop(bool beat, double BPM) {
	if (beat) {
		// first move is the classical head nicker
		if (RhythmDetector::getInstance().isFirstBeat())
			currentMove = (Move::MoveType)(0);

		// switch to next move after as soon as rhythm starts again
		passedBeatsInCurrentMove++;
		if ((sequenceMode == AUTOMATIC_SEQUENCE) &&
			(RhythmDetector::getInstance().hasBeatStarted()) &&
			(passedBeatsInCurrentMove*RhythmDetector::getInstance().getRhythmInQuarters() >= switchMoveAfterNBeats) &&
			(RhythmDetector::getInstance().getBeatCount() == 0)) {
			doNewMove();
			passedBeatsInCurrentMove = 0;
		}
	}

	// wait 4 beats to detect the rhythm
	if (RhythmDetector::getInstance().hasBeatStarted()) {
		// cout << std::fixed << std::setprecision(2) << "-->" << RhythmDetector::getInstance().getRythmPercentage() << bodyPose << endl;
		createMove(RhythmDetector::getInstance().getRythmPercentage());
	}
}

void MoveMaker::doNewMove() {
	// when all moves are shown, omit plain headnicker
	if ((int)currentMove >= Move::numMoves()-1)
		currentMove = (Move::MoveType)0;
	else
		currentMove = (Move::MoveType) (((int)currentMove + 1));

	cout << "new move: " << Move::getMove(currentMove).getName() << "(" << (int)currentMove << ")" << endl;
}

void MoveMaker::switchMovePeriodically(int afterHowManyMoves) {
	switchMoveAfterNBeats = afterHowManyMoves;
}

void MoveMaker::setCurrentMove(Move::MoveType m) {
	currentMove = m;
	passedBeatsInCurrentMove = 0;
}

