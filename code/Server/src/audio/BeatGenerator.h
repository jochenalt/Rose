/*
 * BeatGenerator.h
 *
 *  Created on: Apr 25, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIO_BEATGENERATOR_H_
#define SRC_AUDIO_BEATGENERATOR_H_

// generator that produces a constant beat with a defined bpm
class BeatGenerator {
public:
	BeatGenerator() {};
	virtual ~BeatGenerator() {};

	void setup(double processTime, double lastBeatTime, double BPM, int rhythm);

	// BPM from call of setup
	double getBPM(double processTime);

	// latched function returning true if a beat is pending
	bool getLatchedBeat(double processTime);
private:
	double startProcessTime = 0;
	double BPM = 0;
	double timeShift = 0;
	int rhythm = 0;
	double lastBeat = 0;
	double timeBetweenBeats = 0;
};

#endif /* SRC_AUDIO_BEATGENERATOR_H_ */
