#ifndef __RECOMMEND_FRAME_COMMON_UTIL_H_
#define __RECOMMEND_FRAME_COMMON_UTIL_H_


#include <iostream>
#include <map>
#include <vector>
#include <string>

#define FOR_EACH(it_name, container) for (typeof((container).begin()) it_name = (container).begin(), it_name##_end = (container).end(); it_name##_end != it_name; ++it_name)

namespace util_so_flow{
std::string Base64Decode(const std::string& encoded_string);

std::string CNEncode(const std::string& str);

unsigned char FromHex(unsigned char x);

bool IsUtf8(const std::string& src);

bool IsBase64(unsigned char c);

const std::vector<std::string> StringToTokens(const std::string& content, bool reserve_empty_token, char delim);

void RightTrim(std::string& src, char ch = ' ');

void LeftTrim(std::string& src, char ch = ' ');

void Trim(std::string& src, char ch = ' ');

void StringReplace(std::string& content, const std::string& src, const std::string& dst);

void StringReplace(std::string& content, const char src, const char dst);

void StringRemoveChars(std::string& content, const char remove);

unsigned char ToHex(unsigned char x);

void ToUpper(std::string& content);

void ToLower(std::string& content);

double String2Double (const std::string& str);

std::string UrlEncode(const std::string& src_url);

std::string UrlDecode(const std::string& src_url);

std::string UrlPrepare(const std::string& src_url);

bool ZlibUncompress(const std::string& content, std::string& unzip_content);

std::string OneUnicode2UTF8(const char* unicode_char,size_t unicode_char_length);
}
#endif /* INCLUDE_UTIL_H_ */
