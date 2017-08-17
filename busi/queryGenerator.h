#ifndef __RECOMMEND_FRAME_BUSI_QUERY_GENERATOR__
#define __RECOMMEND_FRAME_BUSI_QUERY_GENERATOR__

#include <string>
#include <vector>
#include "common.h"

class QueryGenerator{

public:
    QueryGenerator();
    int Run(const std::string& originQuery, void* ret);
    int Init(const std::string& config, const std::string& logPath);
private:
    int readConfig();
private:
   QueryStruct                  qStruct; 
   std::string                  m_config; 
   std::string                  m_logPath;
   std::vector<std::string>     m_sourceConfig;
   std::vector<std::string>     m_serverConfig;
   std::vector<int>             m_portConfig;
   std::vector<std::string>     m_tableConfig;
   std::vector<double>          m_simConfig; 
};

#endif
