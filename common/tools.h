#ifndef __COMMON_TOOLS_H__
#define __COMMON_TOOLS_H__

#include "common.h"
#include "murmurhash2.h"
#include <ext/hash_map>

using std::string;
using __gnu_cxx::hash_map;

struct StringHashFunc
{
    size_t operator() (const string& s) const
    {
        return MurmurHash2(s.c_str(), s.size(), 0x1234567);
    }
};

string GetSTDTime(time_t timestamp = 0);
string GetMinuteTime(time_t timestamp = 0);
string GetHourTime(time_t timestamp = 0);
string GetDayTime(time_t timestamp = 0);
string GetMd5(const string& str);
void InitDaemon(void (*func)(int));
void Sleep(int seconds);
void Usleep(int microseconds);
string StringTrim(const string& str);

#endif
