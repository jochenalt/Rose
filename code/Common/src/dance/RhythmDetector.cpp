/*
 * RythmDetector.cpp
 *
 *  Created on: Feb 15, 2018
 *      Author: jochenalt
 */

#include "basics/util.h"

#include <dance/RhythmDetector.h>
#include <dance/Move.h>
#include <dance/Dancer.h>

RhythmDetector::RhythmDetector() {
}

RhythmDetector::~RhythmDetector() {
}


RhythmDetector& RhythmDetector::getInstance() {
	static RhythmDetector mm;
	return mm;
}

void RhythmDetector::setup() {
	beatStarted = false;
	beatCount = 0;
	rhythmInQuarters = 0;
	timeOfLastBeat = 0;
	movePercentage = 0;
	moveSpeed.init(10);
	firstBeat = false;
	loopsSinceBeat = 0;
	loopProcessSpeed.init(2);
}


void RhythmDetector::loop(double processTime, bool beat, double BPM) {

	double now = processTime;
	double timePerBeat = (60.0/BPM); 				// [s]
	double timeSinceBeat = now - timeOfLastBeat; 	// [s]


	if (beat) {
		// detect 1/1 or 1/2 rhythm
		rhythmInQuarters = 1;
		if (abs(timePerBeat - timeSinceBeat) > abs(2.0*timePerBeat - timeSinceBeat))
			rhythmInQuarters = 2;
		else
			rhythmInQuarters = 1;

		// start first move after some beats
		if (beatCount  == 3) {

			// detect 1/1 or 1/2 rhythm
			rhythmInQuarters = 1;
			if (abs(timePerBeat - timeSinceBeat) > abs(2.0*timePerBeat - timeSinceBeat))
				rhythmInQuarters = 2;
			cout << std::fixed << std::setprecision(2) << "Rhythm is 1/" << rhythmInQuarters << " BPM=" << BPM << "(" << BPM/rhythmInQuarters << ") s/beat=" <<  timePerBeat << "s(" << timeSinceBeat<< ")" << endl;

			// start with shy head nicker
			Dancer::getInstance().setCurrentMove(Move::PHYSICISTS_HEAD_NICKER);
		}


		if (!beatStarted) {
			loopProcessSpeed.sampler.dT();
		}
		else {
			double percentageInBeat = timeSinceBeat / (timePerBeat);
			double percentagePerLoop = percentageInBeat/ (float)loopsSinceBeat;

			// compute deviation
			double currentPercentageInRhythm = fmod(filterMovePercentage,rhythmInQuarters);
			double hitRatio = 0;
			if (currentPercentageInRhythm < rhythmInQuarters/2.0)
				hitRatio = (currentPercentageInRhythm+rhythmInQuarters)/rhythmInQuarters;
			else
				hitRatio = currentPercentageInRhythm/rhythmInQuarters;

			// low pass the process speed with the predicted progress plus a correction hitRatio
			loopProcessSpeed = percentagePerLoop / hitRatio;

			// cout << " timeSinceBeat=" << timeSinceBeat << " loopsSinceBeat=" << loopsSinceBeat << " %/l=" << percentagePerLoop
			//	 << " fmod(filteredMove)=" << fmod(filterMovePercentage,rhythmInQuarters) << " hitRatio = " << hitRatio << endl;
		}
		loopsSinceBeat = 0;

		timeOfLastBeat = now;
		timeSinceBeat = 0;

		if (beatStarted == false) {
			firstBeat = true;
		}

		beatStarted = true;
		beatCount++;
	}
	else
		loopsSinceBeat++;

	// wait 4 beats to detect the rhythm
	if (beatCount > 3) {
		// make progress in the move process
	    filterMovePercentage += loopProcessSpeed;

		// movePercentage = (beatCount % (4/rhythmInQuarters) )*rhythmInQuarters + timeSinceBeat/timePerBeat;
		// cout << "timeInBeat=" << timeInBeat << "  move%" << movePercentage << " fmove%=" << fmod(filterMovePercentage,4.0) << "loopProcessSpeed=" << loopProcessSpeed<<  endl;
		movePercentage = filterMovePercentage;
		// cout << std::fixed << std::setprecision(3) << "( tsb=" << timeSinceBeat
		//	 << "s bt=" << (beatCount % (4/rhythmInQuarters) )
		//	 << " tsb/(60/BPM)=" << timeSinceBeat/(60.0/BPM) <<"% rhyt=" << rhythmInQuarters << " =" << movePercentage << " "  << endl;
	}
}

bool RhythmDetector::isFirstBeat() {

	bool bufferFirstBeat = firstBeat;
	firstBeat = false;
	return bufferFirstBeat;
}

double RhythmDetector::getRythmPercentage() {
	return fmod(movePercentage,4.0);;
};
