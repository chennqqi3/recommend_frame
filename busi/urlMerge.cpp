#include <algorithm>
#include "urlMerge.h" 
#include "log.h"

bool compareItem1(const Item& a, const Item& b){
    return (a.dataInfo.sim > b.dataInfo.sim); 
}

UrlMerger::UrlMerger(){
    m_obtainUrl_Http = ObtainUrl_Http();

}

UrlMerger::UrlMerger(const std::string& config, const std::string logPath){
    m_obtainUrl_Http = ObtainUrl_Http(logPath);
    log_init(logPath.c_str(), LOG_YEAR_DAY_HOUR, LOG_NOTICE, true); 
    m_quota = 3;
    m_limit = 20;
}
bool UrlMerger::isExistByTitle (const Item& obj){
    FOR_EACH (r, qStruct.responseValue){
        if ((r->dataInfo).title == obj.dataInfo.title){
            return true; 
        } 
    }
    return false;
} 
int UrlMerger::sortUrlStructbySim (){
    std::vector <Item> responseValueBuf;
    responseValueBuf.clear();
    responseValueBuf.swap (qStruct.responseValue);
    sort (responseValueBuf.begin(), responseValueBuf.end(), compareItem1); 
    
    for (int i = 0; i < m_limit; i ++){
        qStruct.responseValue.push_back (responseValueBuf.at (i));    
    }
    return 0;
}
int UrlMerger::filter (){
    // split by limit
    return 0;
}

int UrlMerger::Run(const std::map<std::string, std::string>& httpResponse, void* ret){
    QueryStruct urlStruct;
    FOR_EACH (r, httpResponse){
        log (LOG_NOTICE, "UrlMerger::Run query: %s, retJson: %s.", (r->first).c_str(), (r->second).c_str());
        m_obtainUrl_Http.jsonParse (r->first, r->second, &urlStruct);  
        int quota = m_quota;
        FOR_EACH (i, urlStruct.responseValue){
            if (!isExistByTitle (*i)){
                qStruct.responseValue.push_back (*i); 
     //           -- quota;
                if (0 == quota){
                    break; 
                }
            }
        } 
    }
//   sortUrlStructbySim ();
    filter();    
    *(QueryStruct*) ret = qStruct;
}

int UrlMerger::Run(const QueryStruct& qList, QueryStruct* ret){

    qStruct.clear();
    

//    m_obtainUrl_Http.Run ();

    return 0;
}

