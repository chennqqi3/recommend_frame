#ifndef __RECOMMEND_FRAME_URL_MERGE__
#define __RECOMMEND_FRAME_URL_MERGE__

#include <string>
#include <map>
#include "common.h"
#include "obtainUrl_Http.h"


class UrlMerger{

public:
    UrlMerger ();
    UrlMerger(const std::string& config, const std::string logPath);
    int Run (const QueryStruct& qList, QueryStruct* ret);
    int Run (const std::map<std::string, std::string>& input, void* ret);
private:
    bool isExistByTitle(const Item&);
    int sortUrlStructbySim();
    int filter ();
    int readConfig();
    bool isValidQuerybySim(const std::string& originQuery, const std::string& genQuery);
private:
    int m_quota;
    int m_limit;
    double m_validSim;
    ObtainUrl_Http m_obtainUrl_Http;
    std::string m_config;
    std::string m_logPath;
    QueryStruct qStruct; 
};

#endif
