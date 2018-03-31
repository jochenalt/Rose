/*
 * AudioUtils.h
 *
 *  Created on: Mar 31, 2018
 *      Author: jochenalt
 */

#ifndef SRC_AUDIO_AUDIOUTILS_H_
#define SRC_AUDIO_AUDIOUTILS_H_



#include <iostream>
#include <vector>
#include <assert.h>
#include <string>

using namespace std;

struct SoundCard {
	string name;
	string descr;
	int index;
};
class SoundCardUtils {
public:
	SoundCardUtils();
	virtual ~SoundCardUtils();
	static SoundCardUtils& getInstance() {
		static SoundCardUtils instance;
		return instance;
	}
	void setup();
	void printSoundCards();
	vector<SoundCard> getInputDevice() { return inputDevices; };
	vector<SoundCard> getOutputDevice() { return outputDevices; };

	SoundCard getInputDevice(int index);
	SoundCard getOutputDevice(int index);

	void setOutputDevice(const SoundCard& output) { outputDeviceIndex = output.index; };
	void setInputDevice(const SoundCard& input) { outputDeviceIndex = input.index; };

	SoundCard getDefaultInputDevice() { return inputDevices[intputDeviceIndex]; };
	SoundCard getDefaultOutputDevice() { return outputDevices[outputDeviceIndex]; };

private:
	vector<SoundCard> outputDevices;
	vector<SoundCard> inputDevices;

	int outputDeviceIndex = 0;
	int intputDeviceIndex = 0;

};

#endif /* SRC_AUDIO_AUDIOUTILS_H_ */
