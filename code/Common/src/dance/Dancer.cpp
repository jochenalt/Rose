/*
 * MoveMaker.cpp
 *
 *  Created on: Feb 13, 2018
 *      Author: jochenalt
 */

#include <basics/util.h>

#include <dance/Dancer.h>
#include <dance/RhythmDetector.h>
#include <stewart/BodyKinematics.h>


Dancer ::Dancer () {
}

Dancer ::~Dancer () {
}

Dancer & Dancer ::getInstance() {
	static Dancer  mm;
	return mm;
}


Pose Dancer ::getDefaultHeadPose() {
	return Pose(Point(0,0,headHeight), Rotation (0,0,0));
}

MouthPose Dancer ::getDefaultMouthPose() {
	return MouthPose(0,30,0);
}

void Dancer ::setup() {
	TotalBodyPose defaultPose = TotalBodyPose(getDefaultHeadPose());
	pose.head = defaultPose.head;

	prevMove = Move::NO_MOVE;
	currentMove = Move::NO_MOVE;
	passedBeatsInCurrentMove = 0;
	startAfterNBeats = 4;
}

void Dancer::createMove(double movePercentage) {

	// compute the current move and the previous move and
	// interpolate in between for a smooth transition.
	TotalBodyPose prevPose = Move::getMove(prevMove).move(movePercentage);
	TotalBodyPose newPose  = Move::getMove(currentMove).move(movePercentage);

	const double transitionDuration = 500; 												// [ms], duration of the transition between different moves
	double interpolationRatio = (float)(millis() - moveTransitionStartTime) / transitionDuration;	// [0..1], factor of transition
	if (interpolationRatio > 1.0)
		moveTransitionStartTime = 0;

	// if we are in a transition between two moves, interpolate between previous and current move
	if ( moveTransitionStartTime > 0 ) {
		newPose.head =  prevPose.head*(1.0-interpolationRatio) + newPose.head*interpolationRatio;
		newPose.mouth =  prevPose.mouth*(1.0-interpolationRatio) + newPose.mouth*interpolationRatio;
	}

	// moves are created in a different thread than the one fetching the result
	// encapsulate with a critical block
	uint32_t start = millis();
	CriticalBlock block(poseMutex);
	uint32_t duration = millis()-start;
	if (duration > 10)
		cerr << "createMove: Probably a bug:waiting at mutex for " << duration << "ms. " << endl;

	pose = newPose;

}

void Dancer::getThreadSafePose(TotalBodyPose& newPose) {
	uint32_t start = millis();
	CriticalBlock block(poseMutex);
	uint32_t duration = millis()-start;
	if (duration> 10)
		cerr << "createMove waiting on mutex for " << duration << "ms" << endl;

	newPose = pose;
};


void Dancer::imposeDanceParams(Move::MoveType newCurrentMove, double newAmbition,
		                       const TotalBodyPose& newPose) {
	CriticalBlock block(poseMutex);

	setCurrentMove(newCurrentMove);
	ambition = newAmbition;
	pose = newPose;
}

void Dancer::danceLoop(bool beat, double BPM, int rhythmInQarterts) {
	RhythmDetector& rhythmDetector = RhythmDetector::getInstance();

	if (beat) {


		// switch to next move after as soon as rhythm starts again
		passedBeatsInCurrentMove++;
		if ((sequenceMode == AUTOMATIC_SEQUENCE) &&
			(rhythmDetector.getAbsoluteBeatCount() > startAfterNBeats) &&
			(rhythmDetector.hasBeatStarted()) &&
			(passedBeatsInCurrentMove*rhythmInQarterts >= Move::getMove(currentMove).getLength()) &&
			(rhythmDetector.getBeatCount(rhythmInQarterts) == 0)) {
			doNewMove();
			passedBeatsInCurrentMove = 0;
		}
	}

	// wait 4 beats to detect the rhythm
	if ((rhythmDetector.getAbsoluteBeatCount() > startAfterNBeats) && (rhythmDetector.hasBeatStarted())) {
		// first move is the classical head nicker
		if (rhythmDetector.isFirstBeat()) {
			setCurrentMove(Move::MoveType::PHYSICISTS_HEAD_NICKER, true);
		}

		createMove(rhythmDetector.getLatencyCompensatedRythmPercentage());
	}
}

void Dancer::doNewMove() {

	if (!musicDetected) {
		setCurrentMove(Move::LISTENING);
		cout << "no music, listening mode" << endl;
		return;
	}

	// when all moves are shown, omit plain headnicker
	if (currentMove >= Move::SHOULDER_DIP)
		setCurrentMove((Move::MoveType)(Move::PHYSICISTS_HEAD_NICKER+1)); // skip physicists move and listening move
	else
		setCurrentMove((Move::MoveType)((int)currentMove + 1));

	// spead measurements consider one move only, so reset the data
	BodyKinematics::getInstance().resetSpeedMeasurement();

	// show me
	cout << "new move: " << Move::getMove(currentMove).getName() << "(" << (int)currentMove << ")" << endl;
}

void Dancer::setCurrentMove(Move::MoveType m, bool force /* = false */) {
	if (m != currentMove) {
		// save previous move and store current time for move transition (done in createMove)
		prevMove = currentMove;
		currentMove = m;
		moveTransitionStartTime = millis();
		passedBeatsInCurrentMove = 0;
	}
}

