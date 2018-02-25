/*
 * DanceMove.cpp
 *
 *  Created on: Feb 16, 2018
 *      Author: jochenalt
 */

#include <math.h>

#include <DanceMove.h>
#include "Stewart/BodyKinematics.h"


std::vector<Move> Move::moveLibrary;
const double latencyShift = 0.1;


Move& Move::getMove(MoveType m) {
	setup();
	return moveLibrary[(int)m];
}

void Move::setup() {
	if (moveLibrary.size() == 0) {
		moveLibrary.resize((int)NO_MOVE+1);

		moveLibrary[(int)PHYSICISTS_HEAD_NICKER] = Move(PHYSICISTS_HEAD_NICKER, "physicist's move", 8);
		moveLibrary[(int)TENNIS_HEAD_NICKER] = Move(TENNIS_HEAD_NICKER, "tennis spectators's move",8);
		moveLibrary[(int)WEASELS_MOVE] = Move(WEASELS_MOVE, "weasel's move",8);

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

		moveLibrary[(int)SHIMMYS] = Move(SHIMMYS, "shimmys",8);
		moveLibrary[(int)HIGH_SPEED_SHIMMYS] = Move(HIGH_SPEED_SHIMMYS, "tripple shimmis",8);

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


TotalBodyPose Move::absHead (const Pose& bodyPose, const Pose& relHeadPose) {
	return TotalBodyPose(bodyPose,BodyKinematics::getInstance().computeHeadPose(bodyPose, relHeadPose));
}


TotalBodyPose Move::physicistsHeadNicker(double movePercentage) {
	// used move curves
	double startPhase = latencyShift;
	double mUpDown = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase));

	return TotalBodyPose ( Pose(Point(0,0,bodyHeight + 15.0*mUpDown), Rotation (0,0,0)),
			               Pose(Point(-15.0*mUpDown,0,headHeight), Rotation(0,0,0)));
}

TotalBodyPose Move::tennisHeadNicker(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, startPhase));
	double mUpDown = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase));
	double mDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, startPhase + 0.75)));

	return TotalBodyPose (
			Pose(Point(mDip*20,0,bodyHeight + 20+20.0*mUpDown),Rotation (0,-radians(20)*mDip,-radians(20)*mBase)),
			Pose(Point(mDip*10,0,headHeight),Rotation (0,-radians(30)*mDip,-radians(20)*mBase)));

}

TotalBodyPose Move::weaselsMove(double movePercentage) {
	double startPhase =  latencyShift;

	double mLeftRight = baseCurveTrapezoid(scaleMove(movePercentage, 1.0, startPhase));
	double mNick = baseCurveCos(scaleMove(movePercentage, 4.0, startPhase));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 1.0 + startPhase));

	return TotalBodyPose (Pose(Point(0,0,bodyHeight + 15.0*mNick),Rotation (0,-radians(0)*mDip,-radians(20)*mLeftRight)),
			Pose(Point(0,25.0*mLeftRight,headHeight),Rotation (0,0,radians(40)*mDip)));
}

TotalBodyPose Move::travoltaHeadNicker(double movePercentage) {
	double startPhase =  latencyShift;

	// used move curves
	double mBase = baseCurveFatCos(scaleMove(movePercentage, 2.0,startPhase + 0.5));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 1.0 + startPhase));

	return TotalBodyPose (
				Pose(Point(0,15*mBase,bodyHeight + 10.0*mBase),Rotation (0,-radians(15)*mUpDown,radians(20)*mUpDown)),
				Pose(Point(0,0,headHeight),Rotation (0,-radians(20)*mUpDown,radians(25)*mUpDown)));

}


TotalBodyPose Move::enhancedTravoltaHeadNicker(double movePercentage) {
	double startPhase =  latencyShift;

	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0,0.5+startPhase));
	double mUpDown = baseCurveTrapezoid(scaleMove(movePercentage, 2.0, 1.0 + startPhase));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 4.0, 1.0+startPhase));

	return TotalBodyPose (
				Pose(Point(0,15*mBase,bodyHeight + 10.0*mBase),Rotation (-radians(15)*mSwing,0,radians(20)*mUpDown)),
				Pose(Point(0,0,headHeight),Rotation (radians(20)*mSwing,-radians(20)*mSwing,radians(20)*mUpDown)));
}


TotalBodyPose Move::diagonalSwing(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.50 + startPhase));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, -0.20 + startPhase)));

	return TotalBodyPose (
				Pose(Point(-20.0*mBase,20.0*mBase,bodyHeight + 20.0*mDip),Rotation (0,radians(10)*mBase,-radians(30)*mBase)),
				Pose(Point(0,0,headHeight),Rotation (0,-radians(10)*mBase,radians(30)*mBase)));

}

TotalBodyPose Move::dippedDiagonalSwing(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.5 + startPhase));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, -0.20 + startPhase)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.00, 0.8 + startPhase));

	return TotalBodyPose (Pose(Point(-25.0*mBase,25.0*mBase,bodyHeight + 20.0*mDip),Rotation (0,radians(10)*mBase,-radians(30)*mBase)),
			Pose(Point(0,0,headHeight),Rotation (0,-radians(10)*mBase,-radians(40)*mEndDip)));
}

TotalBodyPose Move::rolledDippedDiagonalSwing(double movePercentage) {
	double startPhase =  latencyShift;

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.5 + startPhase));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, -0.20 + startPhase)));
	double mEndDip  = fabs(baseCurveDip(scaleMove(movePercentage, 1.0, 0.8 + startPhase)));
	double mRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, -0.25 + startPhase));

	/*
	return TotalBodyPose (Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 20.0*mDip),Rotation (0,-radians(15)*mRoll,-radians(20)*mRoll + radians(30)*mEndDip)),
			Pose(Point(0,0,headHeight),Rotation (-radians(15)*mRoll,0,0)));
			*/
	return absHead(Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 20.0*mDip),Rotation (0,-radians(15)*mRoll,-radians(20)*mRoll + radians(30)*mEndDip)),
				   Pose(Point(0,0,headHeight),Rotation (0,0,0)));
}


TotalBodyPose Move::eyedDippedDiagonalSwing(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + latencyShift));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.0 + latencyShift)));
	double mEndDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 0.0 + latencyShift));
	double mEyeRoll  = baseCurveCos(scaleMove(movePercentage, 4.0, 1.25 + latencyShift));

	return TotalBodyPose (Pose(Point(-30.0*mBase,30.0*mBase,bodyHeight + 20.0*mHipDip),Rotation (radians(20)*mEyeRoll,0,radians(0)*mEndDip)),getDefaultHeadPose());
}

TotalBodyPose Move::bollywoodHeadMove(double movePercentage) {
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.50 + latencyShift));
	double mDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 0.875 + latencyShift)));
	double mHeadMove = baseCurveFatCos(scaleMove(movePercentage, 1.0, 1.50 + latencyShift));

	return  TotalBodyPose (Pose(Point(0,50.0*mBase,bodyHeight + 20.0*mDip),Rotation (radians(45)*mHeadMove, 0,0)),getDefaultHeadPose());
}


TotalBodyPose Move::doubleBollywoodHeadMove(double movePercentage) {
	double startPhase = latencyShift-0.750;
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0, 0.375 + latencyShift));
	double mHeadMove = baseCurveFatCos(scaleMove(movePercentage, 4.0, startPhase + 0.375 ));

	return TotalBodyPose(Pose(Point(0,20*mHeadMove,bodyHeight + 0.0*mDip),Rotation (0, 0,radians(30)*mDip)),getDefaultHeadPose());
}

TotalBodyPose Move::swingDoubleBollywoodHeadMove(double movePercentage) {
	double startPhase = 2.25 + latencyShift;
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 0.5,startPhase));
	double mDip  = baseCurveDip(scaleMove(movePercentage, 1.0,     startPhase + 1.625));
	double mHeadMove = baseCurveCos(scaleMove(movePercentage, 4.0, startPhase - 1.0));
	double mSwing = baseCurveCos(scaleMove(movePercentage, 2.0,    startPhase - 0.75));

	return  TotalBodyPose (Pose(Point(0,00.0*mBase + 20*mHeadMove,bodyHeight + 20.0*(1.0-fabs(mDip))),Rotation (0,radians(30)*mSwing,radians(30)*mDip)),getDefaultHeadPose());
}

TotalBodyPose Move::bodyWaveMove(double movePercentage) {

	double phaseShift = latencyShift+0.50;
	// used move curves
	double mBase = baseCurveFatCos(scaleMove(movePercentage, 2.0,phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift + 0.50 ));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift ));

	return TotalBodyPose (Pose(Point(0,0,bodyHeight + 20.0*mBase),Rotation (0,-radians(20)*mWave,radians(30)*mLeftRight)),getDefaultHeadPose());
}


TotalBodyPose Move::dipBodyWaveMove(double movePercentage) {

	double phaseShift = latencyShift+0.5;
	// used move curves
	double mBase = baseCurveCos(scaleMove(movePercentage, 2.0, phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift - 0.25));
	double mDip= baseCurveDip(scaleMove(movePercentage, 1.0,   phaseShift + 1.25 ));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift ));

	return TotalBodyPose (Pose(Point(0,0,bodyHeight + 20.0*mBase),Rotation (0,-radians(15)*mWave,radians(45)*mDip+radians(30)*mLeftRight)),
			Pose(Point(0,0,headHeight),Rotation (0,radians(15)*mWave,-radians(45)*mDip-radians(30)*mLeftRight)));
}


TotalBodyPose Move::sidedDipBodyWaveMove(double movePercentage) {
	double phaseShift = latencyShift+0.825;

	// used move curves
	double mBase = baseCurveTriangle(scaleMove(movePercentage, 2.0,phaseShift));
	double mWave = baseCurveCos(scaleMove(movePercentage, 4.0, phaseShift - 0.5));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift - 0.0));
	double mLeftRight= baseCurveTriangle(scaleMove(movePercentage, 1.0, phaseShift ));

	return TotalBodyPose (
			Pose(Point(0,50.0*mSideHip,bodyHeight + 20.0*mBase),Rotation (0,-radians(15)*mWave,radians(30)*mLeftRight)),
			Pose(Point(0,0,headHeight),Rotation (0,radians(15)*mWave,-radians(30)*mLeftRight)));
}

TotalBodyPose Move::shimmys(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + latencyShift));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.25 + latencyShift)));
	double mShoulder  = baseCurveFatCos(scaleMove(movePercentage, 2.0, 0.50+latencyShift));

	return TotalBodyPose
			(Pose(Point(0,30.0*mBase,bodyHeight + 20.0*mHipDip),Rotation (0,0,radians(20)*mShoulder)),
			 Pose(Point(0,0,headHeight-10.0*mHipDip),Rotation (0,0,-radians(20)*mShoulder)));
}

TotalBodyPose Move::highSpeedShimmys(double movePercentage) {

	double mBase = baseCurveTriangle(scaleMove(movePercentage, 1.0, 2.0 + latencyShift));
	double mHipDip  = 1.0-fabs(baseCurveCos(scaleMove(movePercentage, 1.0, 1.125 + latencyShift)));
	double mShoulder  = baseCurveCos(scaleMove(movePercentage, 6.0,  0.25+latencyShift));

	return TotalBodyPose (
			 Pose(Point(0,30.0*mBase,bodyHeight + 20.0*mHipDip),Rotation (0,0,radians(15)*mShoulder)),
			 Pose(Point(0,0,headHeight-10.0*mHipDip),Rotation (0,0,-radians(15)*mShoulder)));
}

TotalBodyPose Move::turnAndShowBack(double movePercentage) {
	double startPhase = 0.25 + latencyShift;

	// used move curves
	double mHeadNicker= baseCurveFatCos(scaleMove(movePercentage, 4.0,startPhase));
	double mTurn= movePercentage/4.0;
	return TotalBodyPose (Pose(Point(0,0,bodyHeight + 20.0*mHeadNicker), Rotation (0,0,radians(mTurn*180.0))),getDefaultHeadPose());
}

TotalBodyPose Move::twerk(double movePercentage) {
	double startPhase = 0.25 + latencyShift;

	// used move curves
	double mAsWave = baseCurveCos(scaleMove(movePercentage, 6.0, startPhase));

	double mSideHip  = baseCurveTriangle(scaleMove(movePercentage, 1.0, 0.0 + latencyShift));
	return TotalBodyPose (Pose(Point(0,50.0*mSideHip,bodyHeight),Rotation (0,-radians(30)*mAsWave+radians(15),radians(180.0))),getDefaultHeadPose());
}

TotalBodyPose Move::turnBack(double movePercentage) {
	// used move curves
	double mHeadNicker = baseCurveFatCos(scaleMove(movePercentage, 4.0,latencyShift));
	double mTurn= movePercentage/4.0;

	return TotalBodyPose (Pose(Point(0,0,bodyHeight + 30.0*mHeadNicker), Rotation (0,0,radians(180.0-180.0*mTurn))),getDefaultHeadPose());
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
		case BOLLYWOOD_HEAD_MOVE: return bollywoodHeadMove(movePercentage); break;
		case DOUBLE_BOLLYWOOD_MOVE: return  doubleBollywoodHeadMove(movePercentage); break;
		case SWING_DOUBLE_BOLLYWOOD_MOVE: return swingDoubleBollywoodHeadMove(movePercentage); break;
		case BODY_WAVE: return bodyWaveMove(movePercentage); break;
		case DIPPED_BODY_WAVE: return dipBodyWaveMove(movePercentage); break;
		case SIDE_DIPPED_BODY_WAVE: return sidedDipBodyWaveMove(movePercentage); break;
		case SHIMMYS: return shimmys(movePercentage); break;
		case HIGH_SPEED_SHIMMYS: return highSpeedShimmys(movePercentage); break;
		case TURN_AND_SHOW_BACK:return turnAndShowBack(movePercentage); break;
		case TWERK:			return twerk(movePercentage); break;
		case TURN_BACK:		return turnBack(movePercentage); break;
		default:
			return TotalBodyPose(Pose(Point(0,0,bodyHeight), Rotation(0,0,0)), getDefaultHeadPose());
	}
}

