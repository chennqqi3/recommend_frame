#include <stdio.h>
#include "ssdb_wrapper.h"
#include "tools.h"

using namespace std;
using ssdb::Status;

SSDBWrapper::SSDBWrapper(const string &host, const int port) {
    m_host = host;
    m_port = port;
    m_ssdb_client = NULL;
}

SSDBWrapper::~SSDBWrapper() {
    Close();
}

void SSDBWrapper::Close() {
    if (m_ssdb_client != NULL) {
        delete m_ssdb_client;
        m_ssdb_client = NULL;
    }
}

bool SSDBWrapper::Connect(int try_count) {
    m_messages[0] = '\0';
    if (m_ssdb_client != NULL) {
        delete m_ssdb_client;
        m_ssdb_client = NULL;
    }
    for (int loop = 0; try_count < 0 || loop < try_count; loop++) {
        m_ssdb_client = Client::connect(m_host, m_port);
        if (m_ssdb_client == NULL) {
            Sleep((loop+1) % 600);
        }
        else {
            break;
        }
    }
    if (m_ssdb_client == NULL) {
        return false;
    }
    return true;
}

bool SSDBWrapper::Set(const string &key, const string &val) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }
    Status s = m_ssdb_client->set(key, val);
    if (s.ok()){
        return true;
    }
    else {
        if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "set exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::Get(const string &key, string &val) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }

    Status s = m_ssdb_client->get(key, &val);
    if (s.ok()){
        return true;
    }
    else{
        if (s.not_found()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "get not found");
        }
        else if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "get exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::Del(const string &key) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }

    Status s = m_ssdb_client->del(key);
    if (s.ok()){
        return true;
    }
    else{
        if (s.not_found()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "del not found");
        }
        else if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "del exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::HSet(const string& name, const string &key, int64_t val) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }
    char buf[128] = {'\0'};
    snprintf(buf, 128, "%ld", val);
    Status s = m_ssdb_client->hset(name, key, string(buf));
    if (s.ok()){
        return true;
    }
    else {
        if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hset exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::HSet(const string& name, const string &key, const string &val) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }
    Status s = m_ssdb_client->hset(name, key, val);
    if (s.ok()){
        return true;
    }
    else {
        if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hset exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::HIncr(const string& name, const string &key, int64_t incr, int64_t* ret) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }
    int64_t final_value = 0;
    Status s = m_ssdb_client->hincr(name, key, incr, &final_value);
    if (s.ok()){
        if (ret != NULL) {
            *ret = final_value;
        }
        return true;
    }
    else {
        if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hincr exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::HGet(const string& name, const string &key, string &val) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }

    Status s = m_ssdb_client->hget(name, key, &val);
    if (s.ok()){
        return true;
    }
    else{
        if (s.not_found()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hget not found");
        }
        else if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hget exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::HScan(const string& name, const string &key_start, const string& key_end, uint64_t limit,
        vector<pair<string, string> >& values) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }

    vector<string> val;
    Status s = m_ssdb_client->hscan(name, key_start, key_end, limit, &val);
    if (s.ok()){
        for (size_t i = 0; i < val.size(); i++) {
            if ((i % 2) == 1) {
                values.push_back(make_pair(val[i-1], val[i]));
            }
        }
        return true;
    }
    else {
        if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hget exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::HScan2(const string& name, const string &key_start, const string& key_end, uint64_t limit,
        vector<pair<string, string> >& values) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }

    vector<string> val;
    Status s = m_ssdb_client->hscan(name, key_start, key_end, limit, &val);
    if (s.ok()){
        string k;
        for (size_t i = 0; i < val.size(); i++) {
            if ((i % 2) == 0) {
                size_t pos = val[i].find('_');
                if (pos != string::npos) {
                    k = val[i].substr(pos+1);
                }
                else {
                    k = val[i];
                }
            }
            else {
                values.push_back(make_pair(k, val[i]));
            }
        }
        return true;
    }
    else {
        if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hget exception:%s", s.code().c_str());
        }
        return false;
    }
}

bool SSDBWrapper::HDel(const string& name, const string &key) {
    m_messages[0] = '\0';
    if (m_ssdb_client == NULL) {
        snprintf(m_messages, COMMAND_BUFF_SIZE, "ssdb is not connected");
        return false;
    }

    Status s = m_ssdb_client->hdel(name, key);
    if (s.ok()){
        return true;
    }
    else{
        if (s.not_found()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hdel not found");
        }
        else if (s.error()) {
            snprintf(m_messages, COMMAND_BUFF_SIZE, "hdel exception:%s", s.code().c_str());
        }
        return false;
    }
}
