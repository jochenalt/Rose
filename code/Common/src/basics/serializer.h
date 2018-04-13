/*
 * JSon serialization of primitive types and arrays
 *
 * Author: JochenAlt
 */

#ifndef SERIALIZER_H_
#define SERIALIZER_H_

#include <stdio.h>
#include "math.h"
#include <stdint.h>
#include <iostream>
#include <string.h>
#include <vector>


using namespace std;

class Serializable {
public:
	virtual std::ostream& serialize(std::ostream &out) const = 0;
	virtual std::istream& deserialize(std::istream &in, bool &ok) = 0;
	virtual ~Serializable() {};
	string toString();
};

void parseCharacter(istream& in, char ch, bool &ok);
string parseString(istream& in, bool &ok);

int parseInt(istream& in, bool &ok);

double parseFloat(istream& in, bool &ok);
bool parseBool(istream& in, bool &ok);


void parseWhiteSpace(istream& in);
string parseUntilWhiteSpace(istream& in, bool &ok);

std::ostream& serializePrim(std::ostream &in, bool item);
std::ostream& serializePrim(std::ostream &in, double item);
std::ostream& serializePrim(std::ostream &in, const string& item);
std::ostream& serializePrim(std::ostream &in, int item);

std::istream& deserializePrim(std::istream &in, bool &item, bool &ok);
std::istream& deserializePrim(std::istream &in, double &item, bool &ok);
std::istream& deserializePrim(std::istream &in, string &item, bool &ok);
std::istream& deserializePrim(std::istream &in, int &item, bool &ok);


template<class T>
std::ostream& serializeArrayOfPrimitives(T list[], int len, std::ostream &out) {
	out << "[";
	for (int i = 0;i<len;i++) {
		if (i>0)
			out << ",";
		T item = list[i];
		serializePrim(out, item);
	}
	out << "]";
	return out;
}

template<class T>
std::ostream& serializeVectorOfPrimitives(vector<T> list, std::ostream &out) {
	out << "[";
	for (unsigned int i = 0;i<list.size();i++) {
		if (i>0)
			out << ",";
		T item = list[i];
		serializePrim(out, item);
	}
	out << "]";
	return out;
}

template<class T>
std::ostream& serializeArrayOfSerializable(T list[], int len, std::ostream &out) {
	out << "[";
	for (int i = 0;i<len;i++) {
		if (i>0)
			out << ",";
		T item = list[i];
		item.serialize(out);
	}
	out << "]";
	return out;
}


template<class T>
std::ostream& serializeVectorOfSerializable(vector<T> list, std::ostream &out) {
	out << "[";
	for (unsigned int i = 0;i<list.size();i++) {
		if (i>0)
			out << ",";
		T item = list[i];
		item.serialize(out);
	}
	out << "]";
	return out;
}

template<class T>
std::istream& deserializeArrayOfSerializable(std::istream &in, T list[], int &len, bool& ok) {
	if (in) {
		len = 0;
		parseCharacter(in, '[', ok);
		bool endOfList = false;
		do {

			bool endFound = true;
			parseCharacter(in, ']', endFound);
			if (endFound) {
				endOfList = true;
			} else {
				T item;
				item.deserialize(in, ok);
				if (ok) {
					list[len++] = item;
				}
				bool commaFound = true;
				parseCharacter(in, ',', commaFound);
				if (!commaFound) {
					parseCharacter(in, ']', ok);
					endOfList = true;
				}
			}
		}
		while (!endOfList);
	}
	return in;
}

template<class T>
std::istream& deserializeVectorOfSerializable(std::istream &in, vector<T>& list, bool& ok) {
	if (in) {
		parseCharacter(in, '[', ok);
		bool endOfList = false;
		list.clear();
		do {

			bool endFound = true;
			parseCharacter(in, ']', endFound);
			if (endFound) {
				endOfList = true;
			} else {
				T item;
				item.deserialize(in, ok);
				if (ok) {
					list.push_back(item);
				}
				bool commaFound = true;
				parseCharacter(in, ',', commaFound);
				if (!commaFound) {
					parseCharacter(in, ']', ok);
					endOfList = true;
				}
			}
		}
		while (!endOfList);
	}
	return in;
}

template<class T>
std::istream& deserializeArrayOfPrimitives(std::istream &in, T list[], int &len, bool& ok) {
	if (in) {
		len = 0;
		parseCharacter(in, '[', ok);
		bool endOfList = false;
		do {

			bool endFound = true;
			parseCharacter(in, ']', endFound);
			if (endFound) {
				endOfList = true;
			} else {
				T item;
				deserializePrim(in, item, ok);
				if (ok) {
					list[len++] = item;
				}
				bool commaFound = true;
				parseCharacter(in, ',', commaFound);
				if (!commaFound) {
					parseCharacter(in, ']', ok);
					endOfList = true;
				}
			}
		}
		while (!endOfList);
	}
	return in;
}

template<class T>
std::istream& deserializeVectorOfPrimitives(std::istream &in, std::vector<T> &list, bool& ok) {
	if (in) {
		parseCharacter(in, '[', ok);
		bool endOfList = false;
		do {

			bool endFound = true;
			parseCharacter(in, ']', endFound);
			if (endFound) {
				endOfList = true;
			} else {
				T item;
				deserializePrim(in, item, ok);
				if (ok) {
					list.push_back(item);
				}
				bool commaFound = true;
				parseCharacter(in, ',', commaFound);
				if (!commaFound) {
					parseCharacter(in, ']', ok);
					endOfList = true;
				}
			}
		}
		while (!endOfList);
	}
	return in;
}

#endif
