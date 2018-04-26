/*
 * RythmDetector.h
 *
 *  Created on: Feb 15, 2018
 *      Author: jochenalt
 */

#ifndef SRC_RHYTHMDETECTOR_H_
#define SRC_RHYTHMDETECTOR_H_

#include <queue>
#include "basics/util.h"
#include "assert.h"

struct BeatInvocation {
	double processTime;
	bool beat;
	double bpm;
	int rhythmInQuarters;
};

class RhythmDetector {
public:
	RhythmDetector();
	virtual ~RhythmDetector();
	static RhythmDetector& getInstance();

	void setup();
	void loop(double latency, double processTime, bool beat, double BPM, int rhythmInQuarters);

	int getBeatCount(int rhythmInQuarters) { return (beatCount % (4/rhythmInQuarters)); };
	int getAbsoluteBeatCount() { return (beatCount); };
	double bpm() { return beatsPerMinute; };

	bool hasBeatStarted() { return beatStarted; };
	bool isFirstBeat();

	double getRythmPercentage();
	double getLatencyCompensatedRythmPercentage();

	double getSourceLatency() { return sourceLatency; };
	double getLatencyCompensationDelay() { return latencyCompensationDelay; };
	double getLatencyCompensationPercentage() { return latencyPercentage; };


private:
	RhythmDetector(const RhythmDetector& x) { assert(false); };
	void operator=(const RhythmDetector& x) {assert (false); };
	void operator=(const RhythmDetector& x) const {assert (false); };

	void queueUpBeatInvocation(double processTime, bool beat, double bpm);

	bool beatStarted = false;
	int beatCount = 0;
	double timeOfLastBeat = 0;
	double movePercentage = 0;
	bool firstBeat = false;
	double beatsPerMinute;

	int loopsSinceBeat = 0;
	double loopProcessSpeed;
	double latencyCompensationDelay = 0;
	double latencyPercentage = 0;
	double sourceLatency = 0;
	queue<BeatInvocation> beatQueue;
};

#endif /* SRC_RHYTHMDETECTOR_H_ */
