#include "obtainData_Ssdb.h"
#include "ssdb_wrapper.h"
#include "ini.h"
#include "log.h"
#include "util.h"
#include "md5.h"


ObtainData_Ssdb::ObtainData_Ssdb(){
    
}
ObtainData_Ssdb::ObtainData_Ssdb(const std::string& logPath, 
                                const std::string& ssdbServer,
                                int ssdbPort,
                                const std::string& tableName,
                                double simFilter){
    m_ssdbServer = ssdbServer;
    m_ssdbPort = ssdbPort;
    m_tableName = tableName;
    m_simFilter = simFilter;
    log_init(logPath.c_str(), LOG_YEAR_DAY_HOUR, LOG_DEBUG, true); 
}

int ObtainData_Ssdb::Init(const std::string& config){ 
    m_config = config;
    INI* iniObj = ini_init (m_config.c_str()); 
    if (! iniObj){
        return 1; // Ini error 
    } 
    char* ret = ini_read (iniObj, "log", "path"); 
    if (NULL != ret){
        log_init(ret, LOG_YEAR_DAY_HOUR, LOG_DEBUG, true); 
    }
    return 0;
}
int ObtainData_Ssdb::readConfig(){
    return 0;
}
int ObtainData_Ssdb::readSsdb(const std::string originQuery, void* ret){
    SSDBWrapper* ssdb_client = new SSDBWrapper(m_ssdbServer.c_str(), m_ssdbPort);
    if (!ssdb_client->Connect()){
        log (LOG_ERROR, "readSsdb ssdb connect error"); 
        return 1;
    }
    log (LOG_NOTICE, "query: %s", originQuery.c_str());
    std::string retStr="none";
    if (!ssdb_client->HGet (m_tableName, originQuery, retStr)){
        log (LOG_ERROR, "readSsdb ssdb Hget error"); 
        return 1;
    }
    log (LOG_DEBUG, "ObtainData_Ssdb::readSsdb originQuery: %s, ret:%s", originQuery.c_str(), ((std::string*)ret)->c_str());
    *(std::string*)ret = retStr;
    return 0;
} 
bool ObtainData_Ssdb::isExist(const std::vector<Item>& itemSet, const Item& obj){
    FOR_EACH (r, itemSet){
        if (obj.data.query == (r->data).query){
            return true; 
        }
    }
    return false;
}
bool ObtainData_Ssdb::isEn (const char* str){
    int i;
    for (i = 0; str[i] != '\0'; i ++){
        char ch = str[i];
        if ((ch <= 'Z' && ch >= 'A') || (ch <= 'z' && ch >= 'a') || ch == '.'){
            continue; 
        }
        return false;
    }
    return true;
}
int ObtainData_Ssdb::filter(){
    std::vector<Item> responseValueBuf;
    responseValueBuf.clear();
    responseValueBuf.swap (qStruct.responseValue);
    FOR_EACH (r, responseValueBuf){
        // sim
        if ((r->dataInfo).sim < m_simFilter){
            log (LOG_DEBUG, "ObtainData_Ssdb::filter [m_simFilter] origin: %s, g_query: %s sim:%lf", qStruct.originQuery.c_str(), (r->data).query.c_str(), (r->dataInfo).sim);
            continue; 
        } 
        // en spam
        if (!isEn(qStruct.originQuery.c_str()) && isEn((r->data).query.c_str())){
            log (LOG_DEBUG, "ObtainData_Ssdb::filter [en_spam] origin: %s, g_query: %s", qStruct.originQuery.c_str(), (r->data).query.c_str());
            continue; 
        }
        else{
            log (LOG_DEBUG, "ObtainData_Ssdb::filter [un en_spam] origin: %s, g_query: %s", qStruct.originQuery.c_str(), (r->data).query.c_str());
        }
        // uniq
        if (isExist (qStruct.responseValue, *r)){
            log (LOG_DEBUG, "ObtainData_Ssdb::isExist: origin: %s, g_query: %s", qStruct.originQuery.c_str(), (r->data).query.c_str());
            continue; 
        }
        log (LOG_DEBUG, "ObtainData_Ssdb::filter origin: %s, g_query: %s sim:%lf is OK", qStruct.originQuery.c_str(), (r->data).query.c_str(), (r->dataInfo).sim);
        qStruct.responseValue.push_back (*r);
    }
    std::string str = qStruct.originQuery;
    FOR_EACH (r, qStruct.responseValue){
        str += "\t" + (r->data).query;
        std::cout << (r->data).query << std::endl;
    }
    log (LOG_NOTICE, "originQuery and generatorQuery: %s", str.c_str()); 
    return 0;
}
int ObtainData_Ssdb::getJsonValue (const Object& jsonObj, const std::string& fieldName, Value* ret){
    for (Object::size_type i = 0; i != jsonObj.size(); ++i){
        const Pair& jsonPair = jsonObj[i];
        const std::string strValue = jsonPair.name_;

        if (fieldName == strValue){
            *ret = jsonPair.value_; 
            return 0;
        }
    }
    return 1;
}
int ObtainData_Ssdb::jsonParse (const std::string str){
    Value jsonValue;    
    if (!read (str, jsonValue)){
        log (LOG_ERROR, "read json from string error");
        return 1;
    }
    Value queryValue;
    if (getJsonValue(jsonValue.get_obj(), "querys", &queryValue)){
        log (LOG_ERROR, "getJsonValue (field: querys) Error"); 
        return 2;
    }
    const Array& queryArray = queryValue.get_array();
    int len = queryArray.size();
    log (LOG_DEBUG, "ObtainData_Ssdb::jsonPase, array.size(): %d", len);
    for( unsigned int i = 0; i < len; ++i ){
        std::string targetQuery;
        double targetSim;
        Object obj = queryArray[i].get_obj();   
        Value targetValue;
        // read query
        if (getJsonValue(obj, "query", &targetValue)){
            log (LOG_ERROR, "getJsonValue (field: query) Error"); 
            continue;
        }
        targetQuery = targetValue.get_str();
        log (LOG_DEBUG, "ObtainData_Ssdb::jsonPase query:%s", targetQuery.c_str());
        
        // read sim
        if (getJsonValue(obj, "sim", &targetValue)){
            log (LOG_ERROR, "getJsonValue (field: query) Error"); 
            continue;
        }
        targetSim = targetValue.get_real();
        log (LOG_DEBUG, "ObtainData_Ssdb::jsonPase sim:%lf", targetSim);
        Item itemNode;
        itemNode.data.query = targetQuery;
        itemNode.dataInfo.sim = targetSim;
        log (LOG_NOTICE, "ObtainData_Ssdb::jsonPase query: %s, sim: %lf", targetQuery.c_str(), targetSim);
        qStruct.responseValue.push_back(itemNode);
    }
   return 0;
}
int ObtainData_Ssdb::Run(const std::string& originQuery, void* ret){
    qStruct.clear();
    qStruct.originQuery = originQuery;
    std::string queryMd5 = MD5(originQuery).toString();
    log (LOG_NOTICE, "ObtainData_Ssdb::Run originQuery: %s, md5sum: %s", originQuery.c_str(), queryMd5.c_str());
    std::string retStr;
    if (readSsdb(queryMd5, &retStr)){
        log (LOG_ERROR, "read ssdb error");
        return 1;
    }
    if (jsonParse (retStr)){
        log (LOG_ERROR, "jsonParse error"); 
    }

    if (filter()){
        log (LOG_ERROR, "filter error"); 
    }
    *(QueryStruct*)ret = qStruct;
    return 0;
}
int ObtainData_Ssdb::ObtainQuerys (void *ret){
    std::vector<std::string> querys;
    FOR_EACH (r, qStruct.responseValue){
        querys.push_back ((r->data).query);
    }
    *(std::vector<std::string>*)ret = querys;
}
