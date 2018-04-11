/*
 * ClockGenerator.h
 *
 *  Created on: Apr 11, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIO_CLOCKGENERATOR_H_
#define SRC_AUDIO_CLOCKGENERATOR_H_

#include <basics/util.h>
#include <queue>

using namespace std;

// class to delegate a call to a later point in time. Makes calls that are not equally distributed clock generated.
template <typename Call> class ClockGenerator {
public:
	ClockGenerator() {};
	virtual ~ClockGenerator() {};

	// push an invokation to the queue which is to be fired at the given processTime
	void push(double processTime, const Call & t) {
		// feed the queue
		CriticalBlock queueUpBlock(queueMutex);
		clockObjectQueue.push(t);
		clockTimeQueue.push(processTime);
	}

	// fires a call which has been pushed earlier
	bool isClockDue(double processTime, Call& t) {
		// check if the first entry in the queue is due
		CriticalBlock queueUpBlock(queueMutex);
		// cout << "check " << processTime << " depth=" << clockTimeQueue.size() << " front=" << clockTimeQueue.front()  << "s" << endl;
		if (!clockTimeQueue.empty() && (clockTimeQueue.front() <= processTime)) {
			t = clockObjectQueue.front();
			clockObjectQueue.pop();
			clockTimeQueue.pop();
			return true;
		}
		return false;
	}
private:
	queue<Call> clockObjectQueue;
	queue<double> clockTimeQueue;

	ExclusiveMutex queueMutex;
};

#endif /* SRC_AUDIO_CLOCKGENERATOR_H_ */
