/*
 * Configuration.h
 *
 *  Created on: Mar 20, 2018
 *      Author: jochenalt
 */

#ifndef SRC_CONFIGURATION_H_
#define SRC_CONFIGURATION_H_

#include <basics/stringhelper.h>
#include <basics/util.h>

class Configuration {
public:
	string filePath;
	Configuration();
	virtual ~Configuration() {};
	static Configuration& getInstance();
	void print();
	bool load();
	void save();

	double microphoneLatency = 0.0;   		// [s]
	int microphoneSampleRate = 44100; 		// [Hz]
	int webserverPort = 8080;				// port of webserver
	double microphoneBufferLength = 0.08; 	// [s] added latency that compensates the alsa microphone buffer pushing data blockwise
private:
	void readDouble(map<string,string>& configItems, string name, double &value);
	void writeDouble(map<string,string>& configItems, string name, double value, int decimalPlaces);
	void readInt(map<string,string>& configItems, string name, int &value);
	void writeInt(map<string,string>& configItems, string name, int value);
};
#endif
