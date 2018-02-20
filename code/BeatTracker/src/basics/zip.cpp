#include "basics/miniz.h"
#include "basics/base64.h"
#include <basics/zip.h>


std::string compressAndEncode(const std::string & s) {
	long unsigned int compressedLen = s.length();
	unsigned char compressed[s.length()+32];
    int status = compress(compressed, &compressedLen, (const unsigned char *)s.c_str(), s.length());
    if (status == Z_OK) {
    	std::string result = base64_encode(compressed,compressedLen);
    	return result;
    }
    return "";
}

std::string decodeAndUncompress(const std::string & s, int maxBufferSize) {
	std::string decoded = base64_decode(s);
	unsigned char uncompressed[maxBufferSize+32];
	long unsigned int uncompressedLen = maxBufferSize;

	int status = uncompress(uncompressed, &uncompressedLen, (const unsigned char*)decoded.c_str(), decoded.length());
    if (status == Z_OK) {
    	std::string result;
    	result.append((char*)uncompressed,uncompressedLen);
    	return result;
    }
    return "";
}

