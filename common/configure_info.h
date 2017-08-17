/*
 * configure_info.h
 *
 *  Created on: Nov 18, 2015
 *      Author: zhaoyanbin
 */

#ifndef CONFIGURE_INFO_H_
#define CONFIGURE_INFO_H_

#include <set>
#include <string>
#include <map>
#include <vector>
#include <iostream>
using namespace std;

using std::set;
using std::vector;
using std::string;
using std::vector;

class INI;

class ConfigureInfo {
public:
    bool Init(const std::string& config_file);

    bool IsDeamonServer() const {
        return m_deamon;
    }
    std::string GetItemValue(const std::string& section_name, const std::string& key_name);
    void GetItemValue(const std::string& section_name, std::map<std::string, std::string>& key_values);

    static ConfigureInfo& GetInstance() {
        static ConfigureInfo s_config_info;
        return s_config_info;
    }

private:
    ConfigureInfo();
    ~ConfigureInfo();
    ConfigureInfo(const ConfigureInfo&);
    ConfigureInfo& operator=(const ConfigureInfo&);

private:
    string          m_config_file;
    bool            m_deamon;
    INI*            m_ini;
};

#endif /* CONFIGURE_INFO_H_ */
