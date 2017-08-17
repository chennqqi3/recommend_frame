#ifndef __RECOMMEND_FRAME_BUSI_OBTAINURL_HTTP_
#define __RECOMMEND_FRAME_BUSI_OBTAINURL_HTTP_

#include "common.h"
#include "json_spirit.h"

using namespace json_spirit; 

class ObtainUrl_Http{

public:
    ObtainUrl_Http();
    ObtainUrl_Http(const std::string& logPath);
    ObtainUrl_Http (const QueryStruct& qStruct, const std::string& server, int quota, const std::string& logPath);
    int Run (const std::string& query, void* ret);
    int jsonParse (const std::string& fromQuery, const std::string& str, void *ret);
private:
    double getSimQ2Q (const std::string& fromQuery);
    int getJsonValue (const Object& jsonObj, const std::string& fieldName, Value* ret);
    int readHttp(const std::string& query, void* ret);
    int sortUrlStructbySim();
    int jsonParse (const std::string& fromQuery, const std::string& str);
    int filter ();   

    
private:
    int m_quota;
    int m_limit;
    QueryStruct queryStruct;
    QueryStruct urlStruct;
};

#endif
