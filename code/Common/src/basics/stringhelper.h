#ifndef STRING_HELPER_H_
#define STRING_HELPER_H_


#include <stdio.h>
#include "math.h"
#include <string>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <fstream>

bool endsWith(std::string const & value, std::string const & ending);

// old c-like printf
std::string stringFormat(const std::string &fmt, ...);

// returns true if file is there
bool fileExists(const std::string& filename);

// read complete content of a file
std::string readFileContent(const std::string& filename);


// converts a string to an integer, ok is set if successful
int stringToInt (const std::string &str, bool& ok);

// converts a string to an float, ok is set if successful
double stringToFloat (const std::string& str, bool&ok);

// converts a float to a string
std::string floatToString(double x);

// converts a float to a string using given number of decimal places
std::string floatToString(double x, int decimalPlaces);

// converts an integer to a string
std::string intToString(int x);

// converts a boolean to a json string, i.e. "true" or "false"
std::string boolToJSonString(bool x);

// converts a string to a json string, i.e. "<string>"
std::string stringToJSonString(std::string x);

// converts a json string to boolean, i.e. parses "true" or "false". Sets ok if successful
bool jsonStringToBool (const std::string& str, bool&ok);

// converts a string to hex string
std::string stringToHex(const std::string& input);

// converts a hexstring to a string
std::string hexToString(const std::string& input);

// converts an integer to a hex string
std::string intToHex(int i);

// converts a hex-string to a long
long hexToInt(const std::string s);

// returns the path of an uri, i.e. cuts of all parameters starting with ?
std::string getPath(std::string uri);

// escape all special characters that are not allowed in an URL
std::string urlDecode(std::string input);

// revert it
std::string urlEncode(const std::string &value);

// escape html characters like <, >, &
std::string htmlDecode(std::string input);

// revert it
std::string htmlEncode(std::string input);

// upcase of string
std::string upcase(std::string str);

// downcase string
std::string dncase(std::string str);

// retur, if string str starts with prefix
bool hasPrefix(std::string str, std::string prefix);

#endif
