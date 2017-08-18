#ifndef __RECOMMEND_FRAME_BUSI_OBTAINDATA_SSDB__
#define __RECOMMEND_FRAME_BUSI_OBTAINDATA_SSDB__

#include <string>
#include "common.h"
#include "json_spirit.h"

using namespace json_spirit; 

class ObtainData_Ssdb{

public:
    ObtainData_Ssdb();
    ObtainData_Ssdb(const std::string& logPath, 
                                const std::string& ssdbServer,
                                int ssdbPort,
                                const std::string& tableName,
                                double simFilter);
 
    int Init(const std::string& config);
    int Run(const std::string& originQuery, void* queryList);
    int ObtainQuerys (void *ret);

private:
    int getJsonValue(const Object& value, const std::string& fieldName, Value* ret);
    bool isEn(const char* str);
    bool isExist(const std::vector<Item>& itemSet, const Item& obj);
private:
    int jsonParse (const std::string str, int);
    int jsonParse(const std::string str);
    int readConfig();
    int readSsdb(const std::string originQuery, void* ret);
    int jsonParseTableData (const Value& tableData, std::vector<Item>* tableInfo);
    int filter();
private:
    std::string m_config;
    std::string m_ssdbServer;
    int m_ssdbPort;
    std::string m_tableName;
    std::vector<std::string> m_tableList;
    double m_simFilter;
    QueryStruct qStruct;
};

#endif
