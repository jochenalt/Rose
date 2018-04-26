/*
 * BeatGenerator.cpp
 *
 *  Created on: Apr 25, 2018
 *      Author: jochenalt
 */

#include <audio/BeatGenerator.h>

void BeatGenerator::setup(double processTime, double lastBeatTime, double newBPM, int newRhythm) {
	BPM = newBPM;
	startProcessTime = processTime;
	timeShift = processTime-lastBeatTime;
	rhythm = newRhythm;
	timeBetweenBeats = 60.0/newBPM*rhythm;
	lastBeat = lastBeatTime;
}

double BeatGenerator::getBPM(double processTime) {
	return BPM;
}

bool BeatGenerator::getLatchedBeat(double processTime) {
	if (processTime > lastBeat + timeBetweenBeats) {
		lastBeat += timeBetweenBeats;
		return true;
	}
	return false;
}
