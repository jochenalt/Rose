
#include <basics/stringhelper.h>
#include <basics/util.h>
#include <Configuration.h>

Configuration::Configuration() {
		filePath = getHomeDirectory() + "/.donnarc";
};

Configuration& Configuration::getInstance() {
	static Configuration instance;
	return instance;
}

void Configuration::print() {
	cout << "configuration values:" << endl
		 << "   microphoneLatency=" << microphoneLatency << "s" << endl
	     << "   microphoneSampleRate=" << microphoneSampleRate << " Hz" << endl
         << "   webserverPort=" << webserverPort << endl;

}

bool Configuration::load() {
	bool ok = fileExists(filePath);
	if (ok) {
		map<string,string> configItems = readConfigFile(filePath);
		readDouble(configItems, "microphoneLatency", microphoneLatency);
		readInt(configItems, "microphoneSampleRate", microphoneSampleRate);
		readInt(configItems, "webserverPort", webserverPort);

	}
	return ok;
}

void Configuration::save() {
	map<string,string> configItems;
	writeDouble(configItems, "microphoneLatency", microphoneLatency, 3);
	writeInt(configItems, "microphoneSampleRate", microphoneSampleRate);
	writeInt(configItems, "webserverPort", webserverPort);
	writeConfigFile(filePath,configItems);
};

void Configuration::readDouble(map<string,string>& configItems, string name, double &value) {
	if (configItems.find(name) == configItems.end()) {
		string tmp = configItems[name];
		bool ok = true;
		double i = stringToFloat(tmp, ok);
		if (ok)
			value = i;
	}
}
void Configuration::writeDouble(map<string,string>& configItems, string name, double value, int decimalPlaces) {
	configItems[name] = floatToString(value,decimalPlaces);
}
void Configuration::readInt(map<string,string>& configItems, string name, int &value) {
	if (configItems.find(name) == configItems.end()) {
		string tmp = configItems[name];
		bool ok = true;
		double i = stringToInt(tmp, ok);
		if (ok)
			value = i;
	}
}

void Configuration::writeInt(map<string,string>& configItems, string name, int value) {
	configItems[name] = intToString(value);
}

