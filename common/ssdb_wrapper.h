#ifndef __DISPATCHER1_SSDB_WRAPPER_H__
#define __DISPATCHER1_SSDB_WRAPPER_H__

#include <SSDB_client.h>
#include <string>
#include <vector>

#define COMMAND_BUFF_SIZE 4096

using std::string;
using std::vector;
using std::pair;
using ssdb::Client;

class SSDBWrapper {
public:
    SSDBWrapper(const string &host, const int port);
    virtual ~SSDBWrapper();
    void Close();
    bool Connect(int try_count=3 /*<=0 denote infinite*/);
    
    // KV
    bool Set(const string &key, const string &val);
    bool Get(const string &key, string &val);
    bool Del(const string &key);

    // Hash
    bool HSet(const string& name, const string &key, int64_t val);
    bool HSet(const string& name, const string &key, const string &val);
    bool HIncr(const string& name, const string &key, int64_t incr, int64_t* ret=NULL);
    bool HGet(const string& name, const string &key, string &val);
    bool HScan(const string& name, const string &key_start, const string& key_end, uint64_t limit,
        vector<pair<string, string> >& values);
    bool HScan2(const string& name, const string &key_start, const string& key_end, uint64_t limit,
        vector<pair<string, string> >& values);
    bool HDel(const string& name, const string &key);

    // messages
    const char* ToString() { return m_messages; };

private:
    string m_host;
    int m_port;
    Client *m_ssdb_client;
    // ssdb messages
    char m_messages[COMMAND_BUFF_SIZE];
};

#endif
