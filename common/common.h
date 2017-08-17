#ifndef __RECOMMEND_FRAME_COMMON__
#define __RECOMMEND_FRAME_COMMON__

#include <vector>
#include <string>
#include <stdio.h>
#include "util.h"

#define JSON_SPIRIT_VALUE_ENABLED

struct Data{
    std::string query;
    std::string url;
};

struct DataInfo{
    double sim;
    double simQ2Tile;
    std::string source; // FOR QUERY: url generator's source such as rec, nlp, ft
    std::string relatedQuery; // FOR URL: query2url's query 
    std::string title; // url's title
    std::string content; // url's content
    std::string timestamp;
    std::string rptid;
    std::string from_datatype;
};
struct RankValue{
    double v_sim;
    double v_time;
    double v_site;
    double v_score;
    RankValue():v_sim(0), v_time(0), v_site(0), v_score(0){
    
    }
    void clear (){
        v_sim = 0;
        v_time = 0;
        v_site = 0;
        v_score = 0;
        return ;
    }
};
struct Item{
    Data data;
    DataInfo dataInfo;
    RankValue rankValue;
    Item(){
        data.query.clear();
        data.url.clear();
        dataInfo.sim = 0;
        dataInfo.title.clear();
        dataInfo.content.clear();
        dataInfo.timestamp.clear();
        rankValue.clear();
    }
    void clear(){
        data.query.clear();
        data.url.clear();
        dataInfo.sim = 0;
        dataInfo.title.clear();
        dataInfo.content.clear();
        dataInfo.timestamp.clear();
        rankValue.clear();
    }
};

struct QueryStruct{
    std::string originQuery;
    std::vector<Item> responseValue;
    QueryStruct(){
        originQuery.clear(); 
        responseValue.clear();
    }
    void clear(){
        originQuery.clear(); 
        FOR_EACH(r, responseValue){
           r->clear();  
        } 
        responseValue.clear();
    }
};


#endif
