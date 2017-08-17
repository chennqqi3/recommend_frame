#include <stdlib.h>
#include "queryGenerator.h"
#include "obtainData_Ssdb.h"
#include "log.h"
#include "ini.h"
#include "util.h"

QueryGenerator::QueryGenerator(){
    qStruct.clear();
}
int QueryGenerator::Init(const std::string & config, const std::string& logPath){
    m_config = config;
    m_logPath = logPath;
    log_init(logPath.c_str(), LOG_YEAR_DAY_HOUR, LOG_NOTICE, true); 
    log (LOG_NOTICE, "m_config: %s",m_config.c_str());
    readConfig();
    return 0;
}

int QueryGenerator::Run (const std::string& originQuery, void* ret){
    int i, len = m_sourceConfig.size();         
    qStruct.clear ();
    QueryStruct qRet; 
    for (i = 0; i < len; i ++){
        std::string source = m_sourceConfig.at(i); 
        std::string server = m_serverConfig.at (i);
        int port = m_portConfig.at (i);
        std::string table = m_tableConfig.at (i); 
        double sim = m_simConfig.at (i);
        log (LOG_NOTICE, "server: %s port:%d table:%s, sim: %lf.", server.c_str(), port, table.c_str(), sim);
        // should be a member
        ObtainData_Ssdb od_ssdb(m_logPath, server, port, table, sim);
        od_ssdb.Run (originQuery, &qRet);
        FOR_EACH(r, qRet.responseValue){
            (r->dataInfo).source = source; 
            qStruct.responseValue.push_back(*r);
            log (LOG_NOTICE, "QueryGenerator::Run source: %s, query: %s, sim: %lf", source.c_str(), (r->data).query.c_str(), (r->dataInfo).sim);
        }

        qRet.clear();
    } 
    *(QueryStruct*) ret = qStruct;
    return 0;
}

int QueryGenerator::readConfig(){
    INI* iniObj = ini_init (m_config.c_str()); 
    if (! iniObj){
        log (LOG_ERROR, "read config error");
        return 1; // Ini error 
    } 
    char* ret = ini_read (iniObj, "source", "from"); 
    std::vector<std::string> tokens;
    if (NULL != ret){
        log (LOG_NOTICE, "source: %s", ret);
        tokens.clear();
        tokens = util_so_flow::StringToTokens (ret, false, ';');
        m_sourceConfig.swap(tokens);
    }
    else{
        log (LOG_ERROR, "read source fail, exit"); 
        return 1;
    }
    FOR_EACH(s, m_sourceConfig){
        ret = ini_read (iniObj, s->c_str(), "server"); 
        if (NULL != ret){
            log (LOG_NOTICE, "source: %s, server: %s", s->c_str(), ret);
            m_serverConfig.push_back (ret);
        }      
        else{
            log (LOG_ERROR, "read source: %s server fail, use none as placeholder", s->c_str()); 
            m_serverConfig.push_back ("none");
        }

        ret = ini_read (iniObj, s->c_str(), "port"); 
        if (NULL != ret){
            log (LOG_NOTICE, "source: %s, port: %s", s->c_str(), ret);
            m_portConfig.push_back (atoi(ret));
        }      
        else{
            log (LOG_ERROR, "read source: %s port fail, use -1 as placeholder", s->c_str()); 
            m_portConfig.push_back (-1);
        }
        
        ret = ini_read (iniObj, s->c_str(), "table"); 
        if (NULL != ret){
            log (LOG_NOTICE, "source: %s, table: %s", s->c_str(), ret);
            m_tableConfig.push_back (ret);
        }      
        else{
            log (LOG_ERROR, "read source: %s port fail, use none as placeholder", s->c_str()); 
            m_tableConfig.push_back ("none");
        }

        ret = ini_read (iniObj, s->c_str(), "sim"); 
        if (NULL != ret){
            log (LOG_NOTICE, "source: %s, sim: %s", s->c_str(), ret);
            m_simConfig.push_back (atof (ret));
        }      
        else{
            log (LOG_ERROR, "read source: %s port fail, use -1 as placeholder", s->c_str()); 
            m_simConfig.push_back (-1);
        }

    }
    return 0;
}
