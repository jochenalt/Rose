/*
 * RythmDetector.h
 *
 *  Created on: Feb 15, 2018
 *      Author: jochenalt
 */

#ifndef SRC_RHYTHMDETECTOR_H_
#define SRC_RHYTHMDETECTOR_H_

#include "basics/util.h"
#include "assert.h"

class RhythmDetector {
public:
	RhythmDetector();
	virtual ~RhythmDetector();
	static RhythmDetector& getInstance();

	void setup();
	void loop(double processTime, bool beat, double BPM);

	int getRhythmInQuarters() { return rhythmInQuarters; };
	int getBeatCount() { return (beatCount % (4/rhythmInQuarters)); };
	int getAbsoluteBeatCount() { return (beatCount); };

	bool hasBeatStarted() { return beatStarted; };
	bool isFirstBeat();
	double getRythmPercentage();
private:
	RhythmDetector(const RhythmDetector& x) { assert(false); };
	void operator=(const RhythmDetector& x) {assert (false); };
	void operator=(const RhythmDetector& x) const {assert (false); };

	bool beatStarted = false;
	int beatCount = 0;
	int rhythmInQuarters = 1;
	double timeOfLastBeat = 0;
	double movePercentage = 0;
	bool firstBeat = false;

	int loopsSinceBeat = 0;
	LowPassFilterImpl loopProcessSpeed;
	double filterMovePercentage = 0;
};

#endif /* SRC_RHYTHMDETECTOR_H_ */
