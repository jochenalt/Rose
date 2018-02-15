/*
 * RythmDetector.h
 *
 *  Created on: Feb 15, 2018
 *      Author: jochenalt
 */

#ifndef SRC_RHYTHMDETECTOR_H_
#define SRC_RHYTHMDETECTOR_H_

class RhythmDetector {
public:
	RhythmDetector();
	virtual ~RhythmDetector();
	static RhythmDetector& getInstance();

	void setup();
	void loop(bool beat, double BPM);

	int getRhythmInQuarters() { return rhythmInQuarters; };
	int getBeatCount() { return (beatCount % (4/rhythmInQuarters)); };
	bool hasBeatStarted() { return beatStarted; };
	bool isFirstBeat();
	double getRythmPercentage() { return movePercentage; };
private:
	bool beatStarted;
	int beatCount;
	int rhythmInQuarters;
	double timeOfLastBeat;
	double movePercentage;
	bool firstBeat;
};

#endif /* SRC_RHYTHMDETECTOR_H_ */
