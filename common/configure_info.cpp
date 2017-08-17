/*
 * configure_info.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: zhaoyanbin
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "ini.h"
#include "configure_info.h"

ConfigureInfo::ConfigureInfo(){
}

ConfigureInfo::~ConfigureInfo() {
}

bool ConfigureInfo::Init(const std::string& config_file) {
    m_config_file = config_file;

    m_ini = ini_init(m_config_file.data());
    if (NULL == m_ini) {
        return false;
    }

    m_deamon = (0 == strcasecmp("true", ini_read(m_ini, "Server", "deamon")));
    return true;
}

std::string ConfigureInfo::GetItemValue(const std::string& section_name, const std::string& key_name) {
    char *ini_read(INI *ini, const char *sect, const char *key);
    void ini_read(INI *ini, const char *sect, std::map<char *, char *> &kv);
    if (m_ini) {
        char* ret = ini_read(m_ini, section_name.data(), key_name.data());
        if (NULL != ret) {
            return ret;
        }
    }

    return "";
}

void ConfigureInfo::GetItemValue(const std::string& section_name, std::map<std::string, std::string>& key_values) {
    if (m_ini) {
        std::map<char*, char*> ret;
        ini_read(m_ini, section_name.data(), ret);
        key_values.clear();
        for (std::map <char*, char*>::iterator it = ret.begin(); it != ret.end(); it ++) {
            key_values[it->first] = it->second;
        }
    }
}

