#include <cmath>
#include <algorithm>
#include <curl/curl.h>
#include "common.h"
#include "log.h"
#include "obtainUrl_Http.h"

bool compareItem(const Item& a, const Item& b){
    return (a.dataInfo.sim > b.dataInfo.sim); 
}


ObtainUrl_Http::ObtainUrl_Http(){

}

ObtainUrl_Http::ObtainUrl_Http(const std::string& logPath){
    log_init(logPath.c_str(), LOG_YEAR_DAY_HOUR, LOG_NOTICE, true); 
}

ObtainUrl_Http::ObtainUrl_Http (const QueryStruct& qStruct, const std::string& server, int quota,const std::string& logPath){
    log_init(logPath.c_str(), LOG_YEAR_DAY_HOUR, LOG_NOTICE, true); 
    queryStruct.originQuery = qStruct.originQuery; 
    queryStruct.responseValue = qStruct.responseValue;
    urlStruct.originQuery = qStruct.originQuery;
    m_quota = quota;
}

static size_t httpRetHandler(void *buffer, size_t size, size_t nmemb, void *stream)
{
    std::string *buf = (std::string*)stream;
    buf->append ((char*)buffer, size * nmemb);
    return (size * nmemb); 
}

int ObtainUrl_Http::readHttp (const std::string& queryKey, void* ret){
    std::string query = "http://10.173.194.190:18101/mod_newsfeed/Search?eng_start=-1|0|0|-1|1|-1|-1&from=m.news&data_type=json&q=" + queryKey;
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    std::string outputStr;

    curl = curl_easy_init();
    if(curl) {
        /*
         * You better replace the URL with one that works!
         */
        curl_easy_setopt(curl, CURLOPT_URL,query.c_str());
        /* Define our callback to get called when there's data to be written */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, httpRetHandler);
        /* Set a pointer to our struct to pass to the callback */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outputStr);

        /* Switch on full protocol/debug output */
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);

        if(CURLE_OK != res) {
            /* we failed */
            log (LOG_ERROR, "http request error.", res);
            return 1;
        }
    }

    curl_global_cleanup();
    log (LOG_NOTICE, "query ret: %s.", outputStr.c_str());
    *(std::string*) ret = outputStr;
    return 0;
}

int ObtainUrl_Http::getJsonValue (const Object& jsonObj, const std::string& fieldName, Value* ret){
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

double ObtainUrl_Http::getSimQ2Q (const std::string& fromQuery){
    double ret = 1;  
    // use wenxun interface to get sim of q2q
    return ret;
}
int ObtainUrl_Http::jsonParse(const std::string& fromQuery, const std::string& str, void *ret){
    urlStruct.clear();
    jsonParse (fromQuery, str);
    *(QueryStruct*)ret = urlStruct;
}
int ObtainUrl_Http::jsonParse(const std::string& fromQuery, const std::string& str){
    // need do in urlMerger
    double simQ2Q = getSimQ2Q (fromQuery); 
    Value jsonValue;

    if (!read (str, jsonValue)){
        log (LOG_ERROR, "obtainUrl_Http::jsonParse read json from string Error") ;
        return 1;
    }
    
    Value itemsValue; // it is array
    if (getJsonValue (jsonValue.get_obj(), "items", &itemsValue)){
        log (LOG_ERROR, "ObtainUrl_Http::jsonParse read items error"); 
        return 2;
    }
   
    const Array& itemArray = itemsValue.get_array();
    unsigned int len = itemArray.size();
    for (unsigned int i = 0; i < len; i ++){
        Value targetField;     
        Item targetItem; 
        targetItem.dataInfo.source = fromQuery;
        if (getJsonValue (itemArray[i].get_obj(), "url", &targetField)){
            log (LOG_ERROR, "ObtainUrl_Http::jsonParse read url error"); 
        }  
        else{
            targetItem.data.url = targetField.get_str(); 
            log (LOG_NOTICE, "ObtainUrl_Http::jsonParse read url: %s", targetItem.data.url.c_str()); 
        }

        if (getJsonValue (itemArray[i].get_obj(), "title", &targetField)){
            log (LOG_ERROR, "ObtainUrl_Http::jsonParse read title error"); 
        }  
        else{
            targetItem.dataInfo.title = targetField.get_str(); 
            log (LOG_NOTICE, "ObtainUrl_Http::jsonParse read title: %s", targetItem.dataInfo.title.c_str()); 
        }
 
        if (getJsonValue (itemArray[i].get_obj(), "content", &targetField)){
            log (LOG_ERROR, "ObtainUrl_Http::jsonParse read content error"); 
        }  
        else{
            targetItem.dataInfo.content = targetField.get_str(); 
            log (LOG_NOTICE, "ObtainUrl_Http::jsonParse read content: %s", targetItem.dataInfo.content.c_str()); 
        }

        if (getJsonValue (itemArray[i].get_obj(), "from", &targetField)){
            log (LOG_ERROR, "ObtainUrl_Http::jsonParse read content error"); 
        }  
        else{
            targetItem.dataInfo.from_datatype= targetField.get_str(); 
            log (LOG_NOTICE, "ObtainUrl_Http::jsonParse read from: %s", targetItem.dataInfo.from_datatype.c_str()); 
        }
 
        if (getJsonValue (itemArray[i].get_obj(), "timestamp", &targetField)){
            log (LOG_ERROR, "ObtainUrl_Http::jsonParse read timestamp error"); 
        }  
        else{
            targetItem.dataInfo.timestamp = targetField.get_str(); 
            log (LOG_NOTICE, "ObtainUrl_Http::jsonParse read timestamp: %s", targetItem.dataInfo.timestamp.c_str()); 
        }

        if (getJsonValue (itemArray[i].get_obj(), "attr", &targetField)){
            log (LOG_ERROR, "ObtainUrl_Http::jsonParse attr timestamp error"); 
        }
        else{
            Value rptidObj;
            if (getJsonValue (targetField.get_obj(), "rptid", &rptidObj)){
                log (LOG_ERROR, "ObtainUrl_Http::jsonParse attr timestamp error"); 
            }     
            else{
                targetItem.dataInfo.rptid = rptidObj.get_str(); 
            }
        }
        log (LOG_NOTICE, "ObtainUrl_Http::jsonParse parseQuery: %s, url: %s title: %s, rptid: %s",fromQuery.c_str(), targetItem.data.url.c_str(), targetItem.dataInfo.title.c_str(), targetItem.dataInfo.rptid.c_str()); 

        targetItem.dataInfo.sim = 1; 

        urlStruct.responseValue.push_back (targetItem);
    }

    return 0;
}

int ObtainUrl_Http::sortUrlStructbySim (){
    std::vector <Item> responseValueBuf;
    responseValueBuf.clear();
    responseValueBuf.swap (urlStruct.responseValue);
    sort (responseValueBuf.begin(), responseValueBuf.end(), compareItem); 
    
    for (int i = 0; i < m_limit; i ++){
        urlStruct.responseValue.push_back (responseValueBuf.at (i));    
    }
    return 0;
}
int ObtainUrl_Http::filter(){
    // remove repeat
    // sort 
    sortUrlStructbySim ();
}
int ObtainUrl_Http::Run (const std::string& query, void* ret){
    // http query for URL 
    std::string urlJson = "";
    if (readHttp(query, &urlJson)){
        return 1; 
    }
    jsonParse (query, urlJson);   
    *(QueryStruct*) ret = urlStruct; 
} 
