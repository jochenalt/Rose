/*
 * DanceMove.cpp
 *
 *  Created on: Feb 16, 2018
 *      Author: jochenalt
 */

#include <math.h>

#include <DanceMove.h>


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

		moveLibrary[(int)PHYSICISTS_HEAD_NICKER] = Move(PHYSICISTS_HEAD_NICKER, "phyisist's move", 8);
		moveLibrary[(int)TENNIS_HEAD_NICKER] = Move(TENNIS_HEAD_NICKER, "tennis spectators's move",8);
		moveLibrary[(int)DOUBLE_HEAD_NICKER] = Move(DOUBLE_HEAD_NICKER, "weasel's move",8);

		moveLibrary[(int)TRAVOLTA_HEAD_NICKER] = Move(TRAVOLTA_HEAD_NICKER, "travolta move",8);
		moveLibrary[(int)ENHANCED_TRAVOLTA_HEAD_NICKER] = Move(ENHANCED_TRAVOLTA_HEAD_NICKER, "enhanced travolta move",8);

		moveLibrary[(int)DIAGONAL_HEAD_SWING] = Move(DIAGONAL_HEAD_SWING, "diagonal head move",8);
		moveLibrary[(int)DIPPED_DIAGONAL_HEAD_SWING] = Move(DIPPED_DIAGONAL_HEAD_SWING, "dipped diagonal head move",8);
		moveLibrary[(int)ROLLED_DIPPED_DIAGONAL_HEAD_SWING] = Move(ROLLED_DIPPED_DIAGONAL_HEAD_SWING, "rolle dipped diagonal move",8);
		moveLibrary[(int)EYED_DIPPED_DIAGONAL_HEAD_SWING] = Move(EYED_DIPPED_DIAGONAL_HEAD_SWING, "eye-rolling diagonal move",8);

		moveLibrary[(int)BOLLYWOOD_HEAD_MOVE] = Move(BOLLYWOOD_HEAD_MOVE, "bollywood move",8);
		moveLibrary[(int)DOUBLE_BOLLYWOOD_MOVE] = Move(DOUBLE_BOLLYWOOD_MOVE, "double bollywood move",8);
		moveLibrary[(int)SWING_DOUBLE_BOLLYWOOD_MOVE] = Move(SWING_DOUBLE_BOLLYWOOD_MOVE, "swinging bollywood move",8);

		moveLibrary[(int)BODY_WAVE] = Move(BODY_WAVE, "body wave",8);
		moveLibrary[(int)DIPPED_BODY_WAVE] = Move(DIPPED_BODY_WAVE, "dipped body wave",8);
		moveLibrary[(int)SIDE_DIPPED_BODY_WAVE] = Move(SIDE_DIPPED_BODY_WAVE, "rolling body wave",8);

		moveLibrary[(int)TURN_AND_SHOW_BACK] = Move(TURN_AND_SHOW_BACK, "show your back",2);
		moveLibrary[(int)TWERK] = Move(TWERK, "twerk",8);
		moveLibrary[(int)TURN_BACK] = Move(TURN_BACK, "show your front",2);
		moveLibrary[(int)NO_MOVE] = Move(NO_MOVE, "no move",0);
	}
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



Pose Move::physicistsHeadNicker(double movePercentage) {
	// used move curves
	double startPhase = 0.25 + globalPhaseShift;
	double mUpDown = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase));

	return Pose(Point(0,0,bodyHeight + 50.0*mUpDown), Rotation (0,0,0));
}

Pose Move::tennisHeadNicker(double movePercentage) {
	double startPhase = 0.25 + globalPhaseShift;

	double mBase = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, startPhase + 0.175));
	double mUpDown = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase));
	double mDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, startPhase + 0.875)));

	return Pose(Point(0,0,bodyHeight + 50.0*mUpDown),Rotation (0,-radians(60)*mDip,-radians(45)*mBase));
}

Pose Move::doubleHeadNicker(double movePercentage) {
	double mLeftRight = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, 2.0 + globalPhaseShift));
	double mNick = baseCurveCos(scaleMove(movePercentage, 4.0, globalPhaseShift));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.125 + globalPhaseShift));

	return Pose(Point(0,40.0*mLeftRight,bodyHeight + 30.0*mNick),Rotation (0,-radians(0)*mDip,-radians(20)*mLeftRight-radians(60)*mDip));
}

Pose Move::travoltaHeadNicker(double movePercentage) {

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,0.625+globalPhaseShift));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 1.375+globalPhaseShift));

	return Pose(Point(0,30*mBase,bodyHeight + 50.0*mBase),Rotation (0,-radians(30)*mUpDown,radians(20)*mUpDown));
}


Pose Move::enhancedTravoltaHeadNicker(double movePercentage) {

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,0.5+globalPhaseShift));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 1.25+globalPhaseShift));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 4.0, 0.5+globalPhaseShift));

	return Pose(Point(0,30*mBase,bodyHeight + 50.0*mBase),Rotation (-radians(20)*mSwing,-radians(30)*mSwing,radians(45)*mUpDown));
}


Pose Move::diagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.75 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, -0.25 + globalPhaseShift)));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,0,0));
}

Pose Move::dippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.75 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, -0.25 + globalPhaseShift)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.00, -0.25 + globalPhaseShift));

	return Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 50.0*mDip),Rotation (0,0,radians(30)*mEndDip));
}

Pose Move::rolledDippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.25 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, -0.25 + globalPhaseShift)));
	double mEndDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, -0.25 + globalPhaseShift)));
	double mRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, -0.25 + globalPhaseShift));

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
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.50 + globalPhaseShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + globalPhaseShift)));
	double mHeadMove = baseCurveFatCos(scaleMove(movePercentage, 1.0, 1.50 + globalPhaseShift));

	return Pose(Point(0,50.0*mBase,bodyHeight + 20.0*mDip),Rotation (radians(45)*mHeadMove, 0,0));
}


Pose Move::doubleBollywoodHeadMove(double movePercentage) {
	double startPhase = globalPhaseShift-0.750;
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 0.375 + globalPhaseShift));
	double mHeadMove = baseCurveFatCos(scaleMove(movePercentage, 4.0, startPhase + 0.25 ));

	return Pose(Point(0,20*mHeadMove,bodyHeight + 0.0*mDip),Rotation (0, 0,radians(30)*mDip));
}

Pose Move::swingDoubleBollywoodHeadMove(double movePercentage) {
	double startPhase = 2.25 + globalPhaseShift;
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 0.5,startPhase));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0,     startPhase + 0.75));
	double mHeadMove = baseCurveCos(scaleMove(movePercentage, 4.0, startPhase - 1.0));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 2.0,    startPhase - 0.75));

	return Pose(Point(0,00.0*mBase + 20*mHeadMove,bodyHeight + 50.0*(1.0-fabs(mDip))),Rotation (0,radians(30)*mSwing,radians(30)*mDip));
}

Pose Move::bodyWaveMove(double movePercentage) {

	double phaseShift = globalPhaseShift+0.50;
	// used move curves
	double mBase = baseCurveFatCos(scaleMove(movePercentage, 2.0,phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift + 0.50 ));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift ));

	return Pose(Point(0,0,bodyHeight + 50.0*mBase),Rotation (0,-radians(20)*mWave,radians(30)*mLeftRight));
}


Pose Move::dipBodyWaveMove(double movePercentage) {

	double phaseShift = globalPhaseShift+0.5;
	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0, phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift - 0.25));
	double mDip= baseCurveDip(scaleMove(movePercentage, 1.0,   phaseShift + 1.25 ));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift ));

	return Pose(Point(0,0,bodyHeight + 50.0*mBase),Rotation (0,-radians(15)*mWave,radians(45)*mDip+radians(30)*mLeftRight));
}


Pose Move::sidedDipBodyWaveMove(double movePercentage) {
	double phaseShift = globalPhaseShift+0.825;

	// used move curves
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 2.0,phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift - 0.5));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift - 0.0));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift ));

	return Pose(Point(0,50.0*mSideHip,bodyHeight + 50.0*mBase),Rotation (0,-radians(15)*mWave,radians(30)*mLeftRight));
}

Pose Move::turnAndShowBack(double movePercentage) {
	double startPhase = 0.25 + globalPhaseShift;

	// used move curves
	double mHeadNicker= baseCurveFatCos(scaleMove(movePercentage, 4.0,startPhase));
	double mTurn= movePercentage/4.0;
	return Pose(Point(0,0,bodyHeight + 30.0*mHeadNicker), Rotation (0,0,radians(mTurn*180.0)));
}

Pose Move::twerk(double movePercentage) {
	double startPhase = 0.25 + globalPhaseShift;

	// used move curves
	double mAsWave = baseCurveCos(scaleMove(movePercentage, 6.0, startPhase));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.0 + globalPhaseShift));
	return Pose(Point(0,50.0*mSideHip,bodyHeight),Rotation (0,-radians(30)*mAsWave+radians(15),radians(180.0)));
}

Pose Move::turnBack(double movePercentage) {
	// used move curves
	double mHeadNicker = baseCurveFatCos(scaleMove(movePercentage, 4.0,globalPhaseShift));
	double mTurn= movePercentage/4.0;

	return Pose(Point(0,0,bodyHeight + 30.0*mHeadNicker), Rotation (0,0,radians(180.0-180.0*mTurn)));
}


Pose Move::move(double movePercentage) {
	switch (id) {
		case PHYSICISTS_HEAD_NICKER:return physicistsHeadNicker(movePercentage);break;
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

