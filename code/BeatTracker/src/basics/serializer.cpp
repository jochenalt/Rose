#include "serializer.h"
#include "stringhelper.h"
#include <ctype.h>

using namespace std;

bool isseparator(char c) {
	return (c == '[') || (c == '{') || (c == ']') || (c == '}') || (c == ',') || (c == ';') || (c == ':') || isspace(c);
}

void parseWhiteSpace(istream& in) {
	bool isWhiteSpace;
	do {
		int c = in.peek();
		char dummy;
		isWhiteSpace = isspace(c);
		if (isWhiteSpace && !in.eof())
			in.get(dummy);
	}
	while (isWhiteSpace);
}


void parseCharacter(istream& in, char ch, bool &ok) {
	parseWhiteSpace(in);
	char c;
	if (in && !in.eof()) {
		int cInt = in.peek();
		if ((char)cInt == ch) {
			in.get(c);
		} else
			ok = false;
	} else
		ok = false;
}


string parseString(istream& in, bool &ok) {
	string result;
	if (in && ok) {
		parseWhiteSpace(in);
		parseCharacter(in, '\"', ok);
		bool endOfString = false;
		bool endOfStream = false;
		do {
			char c;
			endOfStream = in.eof();
			in.get(c);
			// escaped character?
			if (c == '\\') {
				in.get(c);
				switch (c) {
					case 'b': c = '\b'; break;
					case 'f': c = '\f'; break;
					case 'n': c = '\n'; break;
					case 'r': c = '\r'; break;
					case 't': c = '\t'; break;
					case '\\': c = '\\'; break;
					case '"': c = '\"'; break;
				}
			} else {
				endOfString = (c == '\"');
			}
			if (!endOfStream && !endOfString)
				result += c;
		}
		while (!endOfStream && !endOfString);
		ok = ok && endOfString;
	} else
		ok = false;
	return result;
}

string parseUntilWhiteSpace(istream& in, bool &ok) {
	string result;
	if (in) {
		parseWhiteSpace(in);
		bool endOfStream = false;
		bool endOfToken = false;
		do {
			char c;
			endOfStream =  in.eof();
			if (endOfStream)
				endOfToken = true;
			else {
				c = in.peek();
				endOfToken = isseparator(c);
				if (!endOfToken) {
					in.get(c);
					result += c;
				}
			}
		}
		while (!endOfStream && !endOfToken);
		ok = ok && endOfToken;
	} else
		ok = false;
	return result;
}

int parseInt(istream& in, bool &ok) {
	string s = parseUntilWhiteSpace(in, ok);
	return stringToInt(s, ok);
}

double parseFloat(istream& in, bool &ok) {
	string s = parseUntilWhiteSpace(in, ok);
	return stringToFloat(s, ok);
}

bool parseBool(istream& in, bool &ok) {
	string s = parseUntilWhiteSpace(in, ok);
	return s.compare("true") == 0;
}


string boolToString(const string& tag, bool x) {
	stringstream str;
	str.precision(3);
	str << tag << "=" << (int)(x?1:0) << " ";
	return str.str();
}




std::istream& deserializePrim(std::istream &in, bool &item, bool &ok) {
	if (in) {
		item = parseBool(in, ok);
	}
	return in;
}

std::istream& deserializePrim(std::istream &in, double &item, bool &ok) {
	if (in) {
		item = parseFloat(in, ok);
	}
	return in;
}

std::istream& deserializePrim(std::istream &in, string &item, bool &ok) {
	if (in) {
		item = parseString(in, ok);
	}
	return in;
}

std::istream& deserializePrim(std::istream &in, int &item, bool &ok) {
	if (in && ok) {
		item = parseInt(in, ok);
	}
	return in;
}

std::ostream& serializePrim(std::ostream &in, bool item) {
	in << boolToJSonString(item);
	return in;
}

std::ostream& serializePrim(std::ostream &in, double item) {
	in << floatToString(item);
	return in;
};
std::ostream& serializePrim(std::ostream &in, const string& item) {
	in << "\"" << item << "\"";
	return in;
};
std::ostream& serializePrim(std::ostream &in, int item) {
	in << intToString(item);
	return in;
};

string Serializable::toString() {
	ostringstream out;
	serialize(out);
	return out.str();
}


