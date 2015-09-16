#ifndef _BASE64_UTIL_H_
#define _BASE64_UTIL_H_

#include <string>

/*
 * Base64Util only used for string data encode and decode.
 */

class Base64Util
{
public:
    static std::string Base64Encode(std::string strInput);
    static std::string Base64Decode(std::string strInput);
public:
    Base64Util();
    ~Base64Util();
};

#endif//_BASE64_UTIL_H_
