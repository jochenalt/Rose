/*
 * RythmDetector.cpp
 *
 *  Created on: Feb 15, 2018
 *      Author: jochenalt
 */

#include "basics/util.h"

#include <RhythmDetector.h>

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
	firstBeat = false;

}


void RhythmDetector::loop(bool beat, double BPM) {
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

		if (beatStarted == false)
			firstBeat = true;

		beatStarted = true;
		beatCount++;

	}

	// wait 4 beats to detect the rhythm
	if (beatCount > 3) {
		// compute elapsed time since last beat
		timeSinceBeat = secondsSinceEpoch() - timeOfLastBeat;

		movePercentage = (beatCount % (4/rhythmInQuarters) )*rhythmInQuarters + timeSinceBeat/(60.0/BPM);
		// cout << std::fixed << std::setprecision(2) << "(" << (beatCount % (4/rhythmInQuarters) ) << ") t=" << timeSinceBeat << "s " << " 60/BPM=" << 60.0/BPM <<"s rhyt=" << rhythmInQuarters << "% =" << movePercentage << " "  << endl;

	}
}

bool RhythmDetector::isFirstBeat() {

	bool bufferFirstBeat = firstBeat;
	firstBeat = false;
	return bufferFirstBeat;
}
