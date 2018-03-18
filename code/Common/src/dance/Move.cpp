/*
 * DanceMove.cpp
 *
 *  Created on: Feb 16, 2018
 *      Author: jochenalt
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <assert.h>
#include <dance/Dancer.h>
#include <dance/Move.h>
#include "Stewart/BodyKinematics.h"



std::vector<Move> Move::moveLibrary;
const double latencyShift = 0.1;


Move& Move::getMove(MoveType m) {
	setup();
	assert((int)m < moveLibrary.size());
	return moveLibrary[(int)m];
}

void Move::setup() {
	if (moveLibrary.size() == 0) {
		moveLibrary.resize((int)LAST_MOVE);

		moveLibrary[(int)PHYSICISTS_HEAD_NICKER] = Move(PHYSICISTS_HEAD_NICKER, "physicist's move", 8);
		moveLibrary[(int)TENNIS_HEAD_NICKER] = Move(TENNIS_HEAD_NICKER, "tennis spectators's move",8);
		moveLibrary[(int)WEASELS_MOVE] = Move(WEASELS_MOVE, "weasel's move",8);

		moveLibrary[(int)TRAVOLTA_HEAD_NICKER] = Move(TRAVOLTA_HEAD_NICKER, "travolta move",8);
		moveLibrary[(int)ENHANCED_TRAVOLTA_HEAD_NICKER] = Move(ENHANCED_TRAVOLTA_HEAD_NICKER, "enhanced travolta move",8);

		moveLibrary[(int)DIAGONAL_HEAD_SWING] = Move(DIAGONAL_HEAD_SWING, "diagonal head move",8);
		moveLibrary[(int)DIPPED_DIAGONAL_HEAD_SWING] = Move(DIPPED_DIAGONAL_HEAD_SWING, "dipped diagonal head move",8);
		moveLibrary[(int)ROLLED_DIPPED_DIAGONAL_HEAD_SWING] = Move(ROLLED_DIPPED_DIAGONAL_HEAD_SWING, "rolle dipped diagonal move",8);
		moveLibrary[(int)EYED_DIPPED_DIAGONAL_HEAD_SWING] = Move(EYED_DIPPED_DIAGONAL_HEAD_SWING, "eye-rolling diagonal move",8);

		moveLibrary[(int)BELLY_MOVE] = Move(BELLY_MOVE, "belly swinging move",8);
		moveLibrary[(int)BOLLYWOOD_HEAD_MOVE] = Move(BOLLYWOOD_HEAD_MOVE, "bollywood move",8);
		moveLibrary[(int)SWING_DOUBLE_BOLLYWOOD_MOVE] = Move(SWING_DOUBLE_BOLLYWOOD_MOVE, "swinging bollywood move",8);

		moveLibrary[(int)BODY_WAVE] = Move(BODY_WAVE, "body wave",8);
		moveLibrary[(int)DIPPED_BODY_WAVE] = Move(DIPPED_BODY_WAVE, "dipped body wave",8);
		moveLibrary[(int)SIDE_DIPPED_BODY_WAVE] = Move(SIDE_DIPPED_BODY_WAVE, "rolling body wave",8);

		moveLibrary[(int)SHIMMYS] = Move(SHIMMYS, "shimmys",8);
		moveLibrary[(int)TRIPPLE_SHIMMYS] = Move(TRIPPLE_SHIMMYS, "tripple shimmis",8);
		moveLibrary[(int)LEANING_TRIPPLE_SHIMMYS] = Move(LEANING_TRIPPLE_SHIMMYS, "leaning tripple shimmis",8);

		moveLibrary[(int)SHOULDER_MOVE] = Move(SHOULDER_MOVE, "shoulder move",8);
		moveLibrary[(int)SHOULDER_CIRCLE] = Move(SHOULDER_CIRCLE, "shoulder circle",8);
		moveLibrary[(int)SHOULDER_DIP] = Move(SHOULDER_DIP, "shoulder dip move",8);

		moveLibrary[(int)TURN_AND_SHOW_BACK] = Move(TURN_AND_SHOW_BACK, "show your back",2);
		moveLibrary[(int)TWERK] = Move(TWERK, "twerk",8);
		moveLibrary[(int)TURN_BACK] = Move(TURN_BACK, "show your front",2);
		moveLibrary[(int)NO_MOVE] = Move(NO_MOVE, "no move",0);
	}
}


double Move::scaleMove(double movePercentage, double speedFactor, double phase) {
	return fmod(movePercentage*speedFactor + phase, 4.0);
}

double Move::scaleAmbition(double value) {
	return Dancer::getInstance().getAmbition()*value;
}

double Move::baseCurveCos(double movePercentage) {
	return scaleAmbition(cos(movePercentage/4.0*2.0*M_PI));
}


double Move::baseCurveFatCos(double movePercentage) {
	double x = movePercentage/4.0*2.0*M_PI;
	return scaleAmbition((cos(2.0*x+M_PI)*0.25+1.25)*cos(x));
}


//
//      |\    /#include "Stewart/BodyKinematics.h"
//      |-\--/---
//      |  \/
//
double Move::baseCurveTriangle(double movePercentage) {
	double result;
	if (movePercentage <= 2)
		result = 1.0-movePercentage;
	else
		result = movePercentage-3.0;
	return scaleAmbition(result);
}

//
//      |\             /
//      |-------------
//      |      \/
//
double Move::baseCurveDip(double movePercentage) {
	return scaleAmbition(pow(baseCurveCos(movePercentage),21.0));
}

double Move::baseCurveFatDip(double movePercentage) {
	return scaleAmbition(pow(baseCurveCos(movePercentage),3.0));
}

//       __
//      |  \      /
//      |---\----/---
//      |    \__/
//
double Move::baseCurveTrapezoid(double movePercentage) {
	double result;
	if (movePercentage <= 1.0)
		result = 1.0;
	else
		if (movePercentage <= 2.0)
			result = 1.0-(movePercentage-1.0)*2.0;
		else
			if (movePercentage <= 3.0)
				result = -1.0;
			else
				result = -1.0 + (movePercentage-3.0)*2.0;
	return scaleAmbition(result);
}


TotalBodyPose Move::absHead (const Pose& bodyPose, const Pose& relHeadPose) {

	static bool firstCall = true;
	static Pose restrictedBodyPose;
	static Pose restrictedRelHeadPose ;
	static TimeSamplerStatic moveTimer;
	double dT = moveTimer.dT();
	if (firstCall) {
		restrictedBodyPose = bodyPose;
		restrictedRelHeadPose = relHeadPose;
	} else {
		// limit acceleration between two moves
		restrictedBodyPose.moveTo(bodyPose, dT, 100.0, 10.0);
		restrictedRelHeadPose.moveTo(relHeadPose, dT, 100.0, 10.0);
	}
	return TotalBodyPose(restrictedBodyPose,BodyKinematics::getInstance().computeHeadStewartPose(restrictedBodyPose, restrictedRelHeadPose));
}


TotalBodyPose Move::physicistsHeadNicker(double movePercentage) {
	double startPhase = latencyShift;
	double mUpDown = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase));

	return absHead(Pose(Point(0,0,bodyHeight + 15.0*mUpDown), Rotation (0,0,0)),
			       Pose(Point(-15.0*mUpDown,0,headHeight), Rotation(0,0,0)));
}

TotalBodyPose Move::tennisHeadNicker(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, startPhase));
	double mUpDown = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase));
	double mDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, startPhase + 0.75)));

	return absHead (
			Pose(Point(mDip*20,0,bodyHeight  +15.0*mUpDown),Rotation (0,-radians(15)*mDip,-radians(15)*mBase)),
			Pose(Point(-mDip*40,0,headHeight),Rotation (0,-radians(20)*mDip,-radians(20)*mBase)));

}

TotalBodyPose Move::weaselsMove(double movePercentage) {
	double startPhase =  latencyShift;

	double mLeftRight = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, startPhase));
	double mNick = baseCurveCos(scaleMove(movePercentage, 4.0, startPhase));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.0 + startPhase));

	return absHead (
			Pose(Point(0,0,bodyHeight + 15.0*mNick),Rotation (0,-radians(0)*mDip,-radians(20)*mLeftRight)),
			Pose(Point(0,25.0*mLeftRight,headHeight),Rotation (0,0,radians(40)*mDip)));
}

TotalBodyPose Move::travoltaHeadNicker(double movePercentage) {
	double startPhase =  latencyShift;

	// used move curves
	double mBase = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 1.0 + startPhase));

	return absHead (
				Pose(Point(mUpDown*20.0,15*mBase,bodyHeight + 10.0*mBase),Rotation (0,-radians(10)*mUpDown,radians(10)*mUpDown)),
				Pose(Point(-mUpDown*30.0,0,headHeight),Rotation (0,-radians(15)*mUpDown,radians(15)*mUpDown)));

}


TotalBodyPose Move::enhancedTravoltaHeadNicker(double movePercentage) {
	double startPhase =  latencyShift;

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,0.5+startPhase));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 1.0 + startPhase));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 4.0, 1.0+startPhase));

	return absHead (
				Pose(Point(0,15*mBase,bodyHeight + 10.0*mBase),Rotation (-radians(10)*mSwing,radians(10)*mSwing,radians(10)*mUpDown)),
				Pose(Point(0,0,headHeight),Rotation (radians(15)*mSwing,-radians(15)*mSwing,radians(15)*mUpDown)));
}


TotalBodyPose Move::diagonalSwing(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, startPhase));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0,  startPhase)));

	return absHead (
				Pose(Point(-20.0*mBase,20.0*mBase,bodyHeight - 10.0 +20.0*mDip),Rotation (radians(15)*mBase,-radians(15)*mBase,0)),
				Pose(Point(0,0,headHeight),Rotation (0,0,0)));

}

TotalBodyPose Move::dippedDiagonalSwing(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, startPhase));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.00, 0.25 + startPhase)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.00, 1.0 + startPhase));

	return absHead (
			Pose(Point(-20.0*mBase,20.0*mBase,bodyHeight + 20.0*mDip),Rotation (radians(15)*mBase,-radians(15)*mBase,0)),
			Pose(Point(0,0,headHeight),Rotation (0,0,radians(30)*mEndDip)));
}

TotalBodyPose Move::rolledDippedDiagonalSwing(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, startPhase));
	double mRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, -0.25 + startPhase));

	return absHead (
			Pose(Point(-20.0*mBase,20.0*mBase,bodyHeight),Rotation (radians(15)*mBase,-radians(15)*mBase,0)),
			Pose(Point(0,0,headHeight),Rotation (-radians(10)*mRoll,0, 0)));
}


TotalBodyPose Move::eyedDippedDiagonalSwing(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 1.25 + startPhase));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 0.25 + startPhase)));
	double mRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, -0.25 + startPhase));

	return absHead (
			Pose(Point(-20.0*mBase,20.0*mBase,bodyHeight + 20.0*mHipDip),Rotation (radians(15)*mRoll,-radians(15)*mBase,0)),
			Pose(Point(0,0,headHeight),Rotation (-radians(10)*mRoll,0,0)));
}



TotalBodyPose Move::bellySwingingMove(double movePercentage) {
	double startPhase = latencyShift;
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.50 + startPhase));
	double mHeadTurn = baseCurveFatCos(scaleMove(movePercentage, 1.0, 1.50 + startPhase));
	double mDip  = fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 0.875 + startPhase)));
	double mHeadUp = baseCurveCos(scaleMove(movePercentage, 2.0,1.1 + startPhase));
	double mHeadMove = baseCurveFatCos(scaleMove(movePercentage, 4.0, 1.0 + startPhase ));

	return absHead(
			Pose(Point(0,20.0*mBase,bodyHeight + 20.0*mDip),Rotation (radians(15)*mBase,0,0)),
			Pose(Point(0,mHeadMove*0.0,headHeight+mHeadUp*5.0),Rotation (radians(10)*mHeadTurn,0,0)));
}

TotalBodyPose Move::bollywoodHeadMove(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.50 + startPhase));
	double mDip  = fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 0.875 + startPhase)));
	double mHeadTurn = baseCurveFatCos(scaleMove(movePercentage, 1.0, 1.50 + startPhase));
	double mHeadMove  = baseCurveFatCos(scaleMove(movePercentage, 1.0,2.8+startPhase));
	double mHeadUp = baseCurveCos(scaleMove(movePercentage, 2.0,1.1 + startPhase));

	return  absHead (
				Pose(Point(0,30.0*mBase,bodyHeight + 10.0*mDip),Rotation (radians(10)*mHeadTurn, 0,0)),
				Pose(Point(0,mHeadMove*30.0,headHeight+mHeadUp*5.0),Rotation (radians(30)*mHeadTurn,0,0)));
}


TotalBodyPose Move::swingDoubleBollywoodHeadMove(double movePercentage) {
	double startPhase = latencyShift;
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.50 + startPhase));
	double mDip  = fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.5 + startPhase)));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 2.0,    startPhase - 0.75));

	return  absHead (
				Pose(Point(0,30.0*mBase, bodyHeight + 20.0-20.0*mDip),Rotation (0,radians(20)*mSwing,0)),
				Pose(Point(0,0,headHeight),Rotation (0,0,0)));
}

TotalBodyPose Move::bodyWaveMove(double movePercentage) {

	double phaseShift = latencyShift;
	// used move curves
	double mBase = baseCurveFatCos(scaleMove(movePercentage, 2.0,phaseShift + 0.5));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift + 0.50 ));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift + 0.5 ));

	return absHead (
			Pose(Point(0,0,bodyHeight + 10.0*mBase),Rotation (0,-radians(15)*mWave,radians(15)*mLeftRight)),
			Pose(Point(0,0,headHeight + 5.0*mBase),Rotation (0,0,0)));
}


TotalBodyPose Move::dipBodyWaveMove(double movePercentage) {

	double phaseShift = latencyShift;
	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0, 0.5 + phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, 0.5 + phaseShift ));
	double mDip= baseCurveDip(scaleMove(movePercentage, 1.0,   0.5 + phaseShift ));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.5 + phaseShift ));

	return absHead (
			Pose(Point(0,0,bodyHeight + 10.0*mBase),Rotation (0,-radians(15)*mWave,radians(15)*mLeftRight)),
			Pose(Point(0,0,headHeight+5.0*mBase),Rotation (0,0,-radians(45)*mDip)));
}


TotalBodyPose Move::sidedDipBodyWaveMove(double movePercentage) {
	double phaseShift = latencyShift;

	// used move curves
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 2.0,phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift - 0.5));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.5 + phaseShift ));

	return absHead (
			Pose(Point(0,30.0*mSideHip,bodyHeight + 10.0*mBase),Rotation (0,-radians(15)*mWave,radians(15)*mLeftRight)),
			Pose(Point(0,0,headHeight+5.0*mBase),Rotation (0,0,0)));
}

TotalBodyPose Move::shimmys(double movePercentage) {
	double phaseShift = latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + phaseShift));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.25 + phaseShift)));
	double mShoulder  = baseCurveFatCos(scaleMove(movePercentage, 2.0, 0.50+phaseShift));

	return absHead
			(Pose(Point(0,10.0*mBase,bodyHeight + 20.0*mHipDip),Rotation (0,0,radians(20)*mShoulder)),
			 Pose(Point(0,0,headHeight-10.0*mHipDip),Rotation (0,0,0)));
}

TotalBodyPose Move::trippleShimmys(double movePercentage) {
	double phaseShift = latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + phaseShift));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.125 + phaseShift)));
	double mShoulder  = baseCurveCos(scaleMove(movePercentage, 6.0,  0.25+phaseShift));

	return absHead (
			 Pose(Point(0,10.0*mBase,bodyHeight + 20.0*mHipDip),Rotation (0,0,radians(15)*mShoulder)),
			 Pose(Point(0,0,headHeight-10.0*mHipDip),Rotation (0,0,0)));
}

TotalBodyPose Move::leaningTrippleShimmys(double movePercentage) {
	double phaseShift = latencyShift;

	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.125 + phaseShift)));
	double mShoulder  = baseCurveCos(scaleMove(movePercentage, 6.0,  0.25+phaseShift));
	double mLean    = baseCurveCos(scaleMove(movePercentage, 1.0,  0.25+phaseShift));

	return absHead (
			 Pose(Point(20.0*mLean,0,bodyHeight + 20.0*mHipDip),Rotation (0,radians(5) * mLean,radians(15)*mShoulder)),
			 Pose(Point(10*mLean,0,headHeight-10.0*mHipDip),Rotation (0,radians(20)*mLean,radians(20)*mLean)));
}


TotalBodyPose Move::shoulderMove(double movePercentage) {
	double startPhase =  latencyShift;
	double mShoulderMove  = baseCurveFatCos(scaleMove(movePercentage, 1.0,2.8+startPhase));
	return absHead (
			Pose(Point(0,mShoulderMove*10.0,bodyHeight),Rotation (radians(10)*mShoulderMove,0,0)),
			Pose(Point(0,-mShoulderMove*20.0,headHeight),Rotation (0,0,0)));
}

TotalBodyPose Move::shoulderCircle(double movePercentage) {
	double startPhase =  latencyShift;
	double mShoulderY = baseCurveCos(scaleMove(movePercentage, 2.0,2.8+startPhase));
	double mShoulderX = baseCurveCos(scaleMove(movePercentage, 2.0,1.0 + 2.8+startPhase));

	return absHead (
			Pose(Point(0,0,bodyHeight),Rotation (radians(10)*mShoulderY,radians(10)*mShoulderX,0)),
			Pose(Point(-mShoulderX*10.0,mShoulderY*10.0,headHeight),Rotation (0,0,0)));
}

TotalBodyPose Move::shoulderDipMove(double movePercentage) {
	double startPhase =  latencyShift;

	double mShoulderMove  = baseCurveFatCos(scaleMove(movePercentage, 2.0,2.8+startPhase));
	double mDip  = (baseCurveDip(scaleMove(movePercentage, 1.0, startPhase + 0.75)));
	double mDipAbs  = fabs(mDip);

	return absHead (
			Pose(Point(0,mDip*20.0,bodyHeight+10.0-mDipAbs*20.0),Rotation (radians(10)*mShoulderMove,0,0)),
			Pose(Point(0,-mShoulderMove*20.0,headHeight),Rotation (0,0,0)));
}

TotalBodyPose Move::turnAndShowBack(double movePercentage) {
	double startPhase = 0.25 + latencyShift;

	// used move curves
	double mHeadNicker= baseCurveFatCos(scaleMove(movePercentage, 4.0,startPhase));
	double mTurn= movePercentage/4.0;
	return absHead (
			Pose(Point(0,0,bodyHeight + 20.0*mHeadNicker), Rotation (0,0,radians(mTurn*180.0))),
			getDefaultHeadPose());
}

TotalBodyPose Move::twerk(double movePercentage) {
	double startPhase = 0.25 + latencyShift;

	// used move curves
	double mAsWave = baseCurveCos(scaleMove(movePercentage, 6.0, startPhase));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.0 + latencyShift));
	return absHead (
			Pose(Point(0,50.0*mSideHip,bodyHeight),Rotation (0,-radians(30)*mAsWave+radians(15),radians(180.0))),
			getDefaultHeadPose());
}

TotalBodyPose Move::turnBack(double movePercentage) {
	// used move curves
	double mHeadNicker = baseCurveFatCos(scaleMove(movePercentage, 4.0,latencyShift));
	double mTurn= movePercentage/4.0;

	return absHead (
			Pose(Point(0,0,bodyHeight + 30.0*mHeadNicker), Rotation (0,0,radians(180.0-180.0*mTurn))),
			getDefaultHeadPose());
}


TotalBodyPose Move::move(double movePercentage) {
	switch (id) {
		case PHYSICISTS_HEAD_NICKER:return physicistsHeadNicker(movePercentage);break;
		case TENNIS_HEAD_NICKER:return tennisHeadNicker(movePercentage);break;
		case TRAVOLTA_HEAD_NICKER:return  travoltaHeadNicker(movePercentage);break;
		case ENHANCED_TRAVOLTA_HEAD_NICKER:return enhancedTravoltaHeadNicker(movePercentage);break;
		case WEASELS_MOVE:return weaselsMove(movePercentage);break;
		case DIAGONAL_HEAD_SWING: return diagonalSwing(movePercentage); break;
		case DIPPED_DIAGONAL_HEAD_SWING: return dippedDiagonalSwing(movePercentage); break;
		case ROLLED_DIPPED_DIAGONAL_HEAD_SWING: return rolledDippedDiagonalSwing(movePercentage); break;
		case EYED_DIPPED_DIAGONAL_HEAD_SWING: return eyedDippedDiagonalSwing(movePercentage); break;
		case BELLY_MOVE: return  bellySwingingMove(movePercentage); break;
		case BOLLYWOOD_HEAD_MOVE: return bollywoodHeadMove(movePercentage); break;
		case SWING_DOUBLE_BOLLYWOOD_MOVE: return swingDoubleBollywoodHeadMove(movePercentage); break;
		case BODY_WAVE: return bodyWaveMove(movePercentage); break;
		case DIPPED_BODY_WAVE: return dipBodyWaveMove(movePercentage); break;
		case SIDE_DIPPED_BODY_WAVE: return sidedDipBodyWaveMove(movePercentage); break;

		case SHIMMYS: return shimmys(movePercentage); break;
		case TRIPPLE_SHIMMYS: return trippleShimmys(movePercentage); break;
		case LEANING_TRIPPLE_SHIMMYS: return leaningTrippleShimmys(movePercentage); break;

		case SHOULDER_MOVE: return shoulderMove( movePercentage);
		case SHOULDER_CIRCLE: return shoulderCircle( movePercentage);
		case SHOULDER_DIP: return shoulderDipMove( movePercentage);


		case TURN_AND_SHOW_BACK:return turnAndShowBack(movePercentage); break;
		case TWERK:			return twerk(movePercentage); break;
		case TURN_BACK:		return turnBack(movePercentage); break;
		default:
			return TotalBodyPose(Pose(Point(0,0,bodyHeight), Rotation(0,0,0)), getDefaultHeadPose());
	}
}

