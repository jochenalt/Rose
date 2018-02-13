#include <stdlib.h>
#include <chrono>
#include <unistd.h>
#include <thread>
#include <iomanip>
#include <cstdarg>
#include <algorithm>

#include "types.h"
#include "stringhelper.h"


bool fileExists(const string& fileName) {
    ifstream file;

    file.open(fileName.c_str());
    if(file.is_open()) {
    	file.close();
    	return true;
    }
    return false;
}


void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}// trim from start (in place)

void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
} // trim from end (in place)

void trim(std::string &s) {
	ltrim(s);
    rtrim(s);
}



string upcase(string str) {
	string result(str);
	std::transform(result.begin(), result.end(),result.begin(), ::toupper);
	return result;
}

string dncase(string str) {
	string result(str);
	std::transform(result.begin(), result.end(),result.begin(), ::tolower);
	return result;
}

bool string_starts_with(string s, string start) {
	return (start.compare(0,start.length(), s) == 0);
}

string toString(double number, int precision) {
	return
			static_cast< std::ostringstream & >(
					(std::ostringstream() << std::setprecision(precision) <<  number)
			).str();
}

int stringToInt (const string &str, bool& ok) {
	try {
		int num = std::stoi(str);
		return num;
	}
	catch (...) {
		ok = false;
		return -1;
	}
}

double stringToFloat (const string& str, bool&ok) {
	try {
		double num = std::stod(str);
		return num;
	}
	catch (...) {
		ok = false;
		return qnan;
	}
}


bool jsonStringToBool (const string& str, bool&ok) {
	if (str.compare("true") == 0)
		return true;
	if (str.compare("false") == 0)
		return false;

	ok = false;
	return true;
}

string intToString(int x) {
	string r;
	stringstream s;
	s << x;
	r = s.str();
	return r;
}

string boolToJSonString(bool x) {
	string r;
	stringstream s;
	s << (x?"true":"false");
	return s.str();
}

string stringToJSonString(string x) {
	string result ="\"";
	for (unsigned i = 0;i<x.length();i++) {
		char c = x[i];
		switch (c) {
			case '\b': result += "\\b"; break;
			case '\f': result += "\\f"; break;
			case '\n': result += "\\n"; break;
			case '\r': result += "\\r"; break;
			case '\t': result += "\\t"; break;
			case '\\': result += "\\\\"; break;
			case '\"': result += "\\\""; break;
			default:
				result += c;
		}
	}
	result +="\"";
	return result;
}

string floatToString(double x) {
	return floatToString(x, 5);
}

string floatToString(double x, int decimalPlaces) {
    stringstream s;
    if (abs(x) < pow(10.0, -decimalPlaces))
    	s << "0";
    else
   		s << std::setprecision(decimalPlaces) << std::fixed << x;
    return s.str();
}


string currentTimeToString()
{
    time_t nowTime = time(0);
    tm *ltm = localtime(&nowTime);
    auto epoch = std::chrono::high_resolution_clock::from_time_t(0);
    auto now   = std::chrono::high_resolution_clock::now();

    string r;stringstream s;
    s << string("") + ((ltm->tm_hour<10)?"0":"") <<  ltm->tm_hour << ":"
      << string("") + ((ltm->tm_min<10)?"0":"") << ltm->tm_min << ":"
      << string("") + ((ltm->tm_sec<10)?"0":"") << ltm->tm_sec << ","
      << std::chrono::duration_cast<std::chrono::milliseconds>(now - epoch).count() % 1000;

    r = s.str();
    return r;
}


string urlEncode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

void urldecode_c(char *dst, const char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit(a) && isxdigit(b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else if (*src == '+') {
                        *dst++ = ' ';
                        src++;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}


string urlDecode(string input) {
	char target[input.length()*3];
	urldecode_c(&target[0], input.c_str());
	return string(target);
}

std::string stringFormat(const std::string &fmt, ...) {
    int size=100;
    std::string str;
    va_list ap;

    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf(&str[0], size, fmt.c_str(), ap);
        va_end(ap);

        if (n > -1 && n < size) {
            string result;
            result.append(str,0,n);
            return result;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
}



string getPath(string uri) {
	int idx = upcase(uri).find("?");
	if (idx >= 0)
		return uri.substr(0,idx);

	return "";
}

struct HTMLReplace {
  string match;
  string replace;
} codes[] = {
  {"&", "&amp;"},
  {"\"","&quot;"},
  {"'", "&apos;"},
  {"<", "&lt;"},
  {">", "&gt;"},
  {"\t", "&#9;"}
};



#define array_length(array) (sizeof (array) / sizeof (array)[0])

string htmlEncode( const string s )
{
  string rs = s;
  // Replace each matching token in turn
  for ( size_t i = 0; i < array_length( codes ); i++ ) {
    // Find the first match
    const string& match = codes[i].match;
    const string& repl = codes[i].replace;
    string::size_type start = rs.find_first_of( match );
    // Replace all matches
    while ( start != string::npos ) {
      rs.replace( start, match.size(), repl );
      // Be sure to jump forward by the replacement length
      start = rs.find_first_of( match, start + repl.size() );
    }
  }
  return rs;
}

string htmlDecode( string s )
{
  string rs = s;
  // Replace each matching token in turn
  for ( size_t i = 0; i < array_length( codes ); i++ ) {
    // Find the first match
    const string& match = codes[i].replace;
    const string& repl = codes[i].match;
    string::size_type start = rs.find_first_of( match );
    // Replace all matches
    while ( start != string::npos ) {
      rs.replace( start, match.size(), repl );
      // Be sure to jump forward by the replacement length
      start = rs.find_first_of( match, start + repl.size() );
    }
  }
  return rs;
}

std::string intToHex(int i)
{
    std::stringbuf buf;
    std::ostream os(&buf);

    os << "0x" << std::setfill('0') << std::setw(sizeof(int) * 2)
       << std::hex << i;

    return buf.str().c_str();
}

long hexToInt(const std::string s) {
	 char * p;
	long n = strtol( s.c_str(), &p, 16 );
	if ( * p != 0 ) {
       return 0;
    }
	return n;
}

std::string stringToHex(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    std::string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}



std::string hexToString(const std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();
    if (len & 1) throw std::invalid_argument("odd length");

    std::string output;
    output.reserve(len / 2);
    for (size_t i = 0; i < len; i += 2)
    {
        char a = input[i];
        const char* p = std::lower_bound(lut, lut + 16, a);
        if (*p != a) throw std::invalid_argument("not a hex digit");

        char b = input[i + 1];
        const char* q = std::lower_bound(lut, lut + 16, b);
        if (*q != b) throw std::invalid_argument("not a hex digit");

        output.push_back(((p - lut) << 4) | (q - lut));
    }
    return output;
}
