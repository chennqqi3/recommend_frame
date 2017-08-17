#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "zlib.h"

#include "util.h"

using namespace std;
namespace util_so_flow{
static const unsigned char base64_chars[256] =
{
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
  52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
 255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
 255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
  49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
 255, 255, 255, 255
};

string Base64Decode(const string& encoded_str) {
    int in_len = encoded_str.size();
    string decoded_str(in_len, '\0');
    char* out_start = &decoded_str[0];
    char* out_dest = &decoded_str[0];
    const char* in_start = encoded_str.c_str();
    const char* in_end = in_start + in_len;
    unsigned char char_array_4[4] = {'\0'};
    size_t i = 0, j = 0;

    while (in_start < in_end && ('=' != *in_start) && IsBase64(*in_start)) {
        char_array_4[i++] = *in_start++;

        if (4 == i) {
            for (i = 0; i < 4; ++i) {
                char_array_4[i] = base64_chars[char_array_4[i]];
            }

            *out_dest++ = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            *out_dest++ = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            *out_dest++ = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            i = 0;
        }
    }

    if (0 != i) {
        for (j = i; j < 4; ++j) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; ++j) {
            char_array_4[j] = base64_chars[char_array_4[j]];
        }

        unsigned char char_array_3[3] = {'\0'};

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; j < i - 1; ++j) {
            *out_dest++ = char_array_3[j];
        }

    }
    decoded_str.resize(out_dest - out_start);

    return decoded_str;
}

bool IsBase64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

string CNEncode(const string& str) {
    string str_tmp(3 * str.size(), '\0');
    const char* src_start = str.c_str();
    const char* src_end = src_start + str.size();
    char *start = &str_tmp[0];
    char *dest = &str_tmp[0];

    while (src_start < src_end) {
        if (0x20 == (unsigned char)(*src_start)) {
            *dest++ = '%';
            *dest++ = '2';
            *dest++ = '0';
        } else if ((unsigned char)(*src_start) < 0x80) {
            *dest++ = *src_start;
        } else {
            *dest++ = '%';
            *dest++ = ToHex((unsigned char)(*src_start) >> 4);
            *dest++ = ToHex((unsigned char)(*src_start) % 16);
        }
        ++src_start;
    }
    str_tmp.resize(dest - start);

    return str_tmp;
}

unsigned char FromHex(unsigned char x) {
    unsigned char y = 0;

    switch (x) {
    case 'a' ... 'f':
        y = x - 'a' + 10;
        break;
    case 'A' ... 'F':
        y = x - 'A' + 10;
        break;
    case '0' ... '9':
        y = x - '0';
        break;
    default:
        ;
    }

    return y;
}

void LeftTrim(string& src, char ch) {
    src.erase(0, src.find_first_not_of(ch));
}

void RightTrim(string& src, char ch) {
    src.erase(src.find_last_not_of(ch) + 1);
}

void Trim(string& src, char ch) {
    LeftTrim(src, ch);
    RightTrim(src, ch);
}

bool IsUtf8(const string& src) {
    bool is_utf8 = true;
    bool all_asc = true;
    const unsigned char* start = (const unsigned char*)src.c_str();
    const unsigned char* end = start + src.size();

    while(start < end) {
        if (*start < 0x80) { //0xxxxxxx
            ++start;
        } else {
            all_asc = false;
            if (*start < 0xC0) {
                is_utf8 = false;
                break;
            } else if (*start < 0xE0) {
                if (start >= end - 1) {
                    break;
                }
                if (0x80 != (start[1] & 0xC0)) {
                    is_utf8 = false;
                    break;
                } else if (0x80 == (start[1] & 0xC0)) {
                    is_utf8 = false;
                    break;
                }
                start += 2;
            } else if (*start < 0xF0) {
                if (start >= end - 2) {
                    break;
                }

                if ((0x80 != (start[1] & 0xC0)) || (0x80 != (start[2] & 0xC0))) {
                    is_utf8 = false;
                    break;
                }
                start += 3;
            } else if (*start < 0xF8) {
                if (start >= end - 3) {
                    is_utf8 = false;
                    break;
                }

                if ((0x80 != (start[1] & 0xC0)) || (0x80 != (start[2] & 0xC0)) || (0x80 != (start[3] & 0xC0))) {
                    is_utf8 = false;
                    break;
                }
                start += 4;
            } else {
                is_utf8 = false;
                break;
            }
        }
    }

    return is_utf8 && (!all_asc);
}

void StringReplace(string& content, const string& src, const string& dst) {
    size_t pos = 0;
    size_t src_len = src.size();
    size_t dst_len = dst.size();
    pos = content.find(src.c_str(), pos, src_len);

    while (pos != string::npos) {
        content.replace(pos, src_len, dst);
        pos = content.find(src.c_str(), pos + dst_len, src_len);
    }
}

void StringReplace(string& content, const char src, const char dst) {
    char *start = &content[0];
    char *end = start + content.size();

    while (start < end) {
        if (*start == src) {
            *start = dst;
        }

        ++start;
    }
}

void StringRemoveChars(string& content, const char remove) {
    const char* start = content.c_str();
    const char* curr = content.c_str();
    char* dst = &content[0];
    const char* end = start + content.size();

    while (curr < end) {
        if (*curr != remove) {
            *dst++ = *curr;
        }

        ++curr;
    }
    content.resize(dst - start);
}

const vector<string> StringToTokens(const string& content, bool reserve_empty_token, char delim) {
    bool only_the_first = false;
    vector<string> tokens;
    const char* start = content.c_str();
    const char* end = start + content.size();
    const char* n = NULL;
    bool check_first = false;

    while (NULL != (n = strchr(start, delim)) && start < end && !check_first) {
        if (reserve_empty_token || n != start) {
            tokens.push_back(string(start, n - start));
        }

        start = n + 1;
        check_first = only_the_first;
    }

    if (*start || (reserve_empty_token && delim == *(start - 1))) {
        tokens.push_back(start);
    }

    return tokens;
}

unsigned char ToHex(unsigned char x) {
    unsigned char y = '0';

    switch (x) {
    case 0 ... 9:
        y = x + '0';
        break;
    case 10 ... 15:
        y = x - 10 + 'A';
        break;
    default:
        ;
    }

    return y;
}

void ToLower(string& src) {
    char* start = &src[0];
    char* end = start + src.size();

    while (start < end) {
        if (*start >= 'A' && *start <= 'Z') {
            *start = tolower(*start);
        }

        ++start;
    }
}

void ToUpper(string& src) {
    char* start = &src[0];
    char* end = start + src.size();

    while (start < end) {
        if (*start >= 'a' && *start <= 'z') {
            *start = toupper(*start);
        }

        ++start;
    }
}

string UrlDecode(const string& src_url) {
    std::string str_tmp(src_url.size(), '\0');
    const char* src_start = src_url.c_str();
    const char* src_end = src_start + src_url.size();
    char* start = &str_tmp[0];
    char* dest = &str_tmp[0];

    while (src_start < src_end) {
        if ('+' == *src_start) {
            *dest++ = ' ';
            ++src_start;
        } else if ('%' == *src_start) {
            if ((src_start + 2) > src_end) {
                return src_url;
            }
            ++src_start;
            unsigned char high = FromHex((unsigned char)(*src_start++));
            unsigned char low = FromHex((unsigned char)(*src_start++));
            *dest++ = high * 16 + low;
        } else {
            *dest++ = *src_start++;
        }
    }
    str_tmp.resize(dest - start);

    return str_tmp;
}

string UrlEncode(const string& src_url) {
    string str_tmp(3 * src_url.size(), '\0');
    const char* src_start = src_url.c_str();
    const char* src_end = src_start + src_url.size();
    char* start = &str_tmp[0];
    char* dest = &str_tmp[0];

    while (src_start < src_end) {
        if (
                isalnum((unsigned char)(*src_start)) ||
                '-' == (*src_start) ||
                '_' == (*src_start) ||
                '.' == (*src_start) ||
                '~' == (*src_start) ||
                ',' == (*src_start) ||
                ':' == (*src_start)
            )
        {
            *dest++ = *src_start++;
        } else {
            *dest++ = '%';
            *dest++ = ToHex((unsigned char)(*src_start) >> 4);
            *dest++ = ToHex((unsigned char)(*src_start) % 16);
            ++src_start;
        }
    }
    str_tmp.resize(dest - start);

    return str_tmp;
}

string UrlPrepare(const std::string& src_url) {
    std::string str_tmp(src_url.size(), '\0');
    const char* src_start = src_url.c_str();
    const char* src_end = src_start + src_url.size();
    char* start = &str_tmp[0];
    char* dest = &str_tmp[0];

    while (src_start < src_end) {
        if ('%' == *src_start) {
            if (src_start + 2 < src_end) {
                if ((
                        (((*(src_start + 1)) >= 'a') && ((*(src_start + 1)) <= 'f')) ||
                        (((*(src_start + 1)) >= 'A') && ((*(src_start + 1)) <= 'F')) ||
                        (((*(src_start + 1)) >= '0') && ((*(src_start + 1)) <= '9'))
                    ) &&
                    (
                        (((*(src_start + 2)) >= 'a') && ((*(src_start + 2)) <= 'f')) ||
                        (((*(src_start + 2)) >= 'A') && ((*(src_start + 2)) <= 'F')) ||
                        (((*(src_start + 2)) >= '0') && ((*(src_start + 2)) <= '9'))
                    ))
                {
                    *dest++ = *src_start++;
                    *dest++ = (char)toupper(*src_start++);
                    *dest++ = (char)toupper(*src_start++);
                } else {
                    *dest++ = *src_start++;
                }
            } else {
                *dest++ = *src_start++;
            }
        } else {
            *dest++ = *src_start++;
        }
    }
    str_tmp.resize(dest - start);

    return str_tmp;
}
double String2Double (const std::string& str){
    double ret;
    sscanf (str.c_str(), "%lf", &ret);
    return ret;
}
bool ZlibUncompress(const string& content, string& unzip_content) {
    uLongf unzip_size = 10485760;
    unsigned char* unzip = (unsigned char*)malloc(10485760);
    memset(unzip, '\0', unzip_size);
    int ret = uncompress(unzip, &unzip_size, (const Bytef*)content.data(), content.size());
    bool unzip_result = false;

    if (ret == Z_OK) {
        unzip_content.append((char*)unzip, unzip_size);
        unzip_result = true;
    }
    free(unzip);

    return unzip_result;
}

string OneUnicode2UTF8(const char* unicode_char,size_t unicode_char_length) {
    //unicode: 0x192->110010010 ,utf8:0xC692->1100011010010010
    int value = 0;
    string utf8_str(10, '\0');
    char* start = &utf8_str[0];
    char* dest = &utf8_str[0];
    for (size_t i = 0; i < unicode_char_length; i++) {
        value = value * 16 + FromHex(unicode_char[i]);
    }

    if (value >= 0x0000 && value <= 0x007F) {
        *dest++ = unicode_char[0];
    }
    else if (value >= 0x0080 && value <= 0x07FF) {
        *dest++ = ((value >> 6) | 0xC0);
        *dest++ = ((value & 0x3F) | 0x80);
    }
    else if (value >= 0x0800 && value <= 0xFFFF) {
        *dest++ = ((value >> 12) | 0xE0);
        *dest++ = ((value >> 6 & 0x3F) | 0x80);
        *dest++ = ((value & 0x3F) | 0x80);
    }
    else if (value >= 0x10000 && value <= 0x10FFFF) {
        *dest++ = (value >> 18 | 0xF0);
        *dest++ = ((value >> 12 & 0x3F) | 0x80);
        *dest++ = ((value >> 6 & 0x3F) | 0x80);
        *dest++ = ((value & 0x3F) | 0x80);
    }

    utf8_str.resize(dest - start);
    return utf8_str;
}
}
