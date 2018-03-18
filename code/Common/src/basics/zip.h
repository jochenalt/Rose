#ifndef ZIP_H_
#define ZIP_H_

#include <string.h>

std::string compressAndEncode(const std::string & s);
std::string decodeAndUncompress(const std::string & s, int maxBufferSize);

#endif
