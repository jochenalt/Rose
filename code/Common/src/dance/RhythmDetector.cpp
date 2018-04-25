/*
 * RythmDetector.cpp
 *
 *  Created on: Feb 15, 2018
 *      Author: jochenalt
 */

#include <queue>
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
	firstBeat = false;
	loopsSinceBeat = 0;
	loopProcessSpeed = 0;
}


void RhythmDetector::loop(double latency, double processTime, bool beat, double BPM) {

	double now = processTime;
	double timePerBeat = (60.0/BPM); 				// [s]
	double timeSinceBeat = now - timeOfLastBeat; 	// [s]

	if (beat) {
		beatsPerMinute = BPM;

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

			// start with shy head nicker
			Dancer::getInstance().setCurrentMove(Move::PHYSICISTS_HEAD_NICKER);
		}

		double currentBeatProgress = timeSinceBeat / timePerBeat;

		// compute deviation to decide if we compensate by moving forward quicker or slowing down.
		double currentMoveProgress = fmod(movePercentage,rhythmInQuarters);
		if (currentMoveProgress < rhythmInQuarters/2.0)
			currentMoveProgress = ((currentMoveProgress+rhythmInQuarters)/rhythmInQuarters); 	// too slow, hitRatio < 1.0, so accelerate
		else
			currentMoveProgress = (currentMoveProgress/rhythmInQuarters);						// too fast, hit Ratio > 1.0, so slow down

		// compute the deviation from actual beat timing to move timing
		// returns a number > 1 if beat is beat is ahead move and <1 if beat is behind move
		// double deviationMoveBeat = currentPercentageInRhythm/currentMovePercentage;
		cout << std::fixed << std::setprecision(4) << "%beat=" << currentBeatProgress << " move%=" << currentMoveProgress << endl;

		double loopSpeed = currentBeatProgress  / currentMoveProgress;

		// low pass the process speed with the predicted progress plus a correction hitRatio
		loopProcessSpeed = loopSpeed / (float)loopsSinceBeat;

		// compute latency compensation, i.e. the time we need to delay
		// the dance even more such that the beat meets the next one
		double secondsPerBeat = timePerBeat*rhythmInQuarters;
		// number of beats we need to overtake
		int numOfDelayedBeats = (int)(latency / secondsPerBeat + 1.0);
		sourceLatency = latency;
		latencyCompensationDelay = fmod(numOfDelayedBeats*secondsPerBeat-latency,secondsPerBeat); // [s]
		latencyPercentage = (latency/timePerBeat);

		// cout << "hitratio=" << hitRatio << " processTime=" << processTime << " percentagePerLoop=" << percentagePerLoop << " loopProcessSpeed=" << loopProcessSpeed
		//	 << " latencyPercentage=" << latencyPercentage << endl;

		// cout << " timeSinceBeat=" << timeSinceBeat << " loopsSinceBeat=" << loopsSinceBeat << " %/l=" << percentagePerLoop
		//	 << " fmod(filteredMove)=" << fmod(filterMovePercentage,rhythmInQuarters) << " hitRatio = " << hitRatio << endl;
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
		loopsSinceBeat++; // do not count the loop that gives a beat

	// wait 4 beats to detect the rhythm
	if (beatCount > 3) {
		// make progress in the move process
		movePercentage += loopProcessSpeed;

		// movePercentage = (beatCount % (4/rhythmInQuarters) )*rhythmInQuarters + timeSinceBeat/timePerBeat;
		// cout << "movePercentage=" << movePercentage << endl;
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
	return fmod(movePercentage,4.0);
};

double RhythmDetector::getLatencyCompensatedRythmPercentage() {
	// static TimeSampler l;
	// if (l.isDue(50))
	// 	cout << "move=" << movePercentage+latencyPercentage << endl;
	return fmod(movePercentage+latencyPercentage,4.0);
};


void RhythmDetector::queueUpBeatInvocation(double processTime, bool beat, double bpm) {
	BeatInvocation b;
	b.processTime = processTime;
	b.beat = beat;
	b.bpm = bpm;
	beatQueue.push(b);
}
