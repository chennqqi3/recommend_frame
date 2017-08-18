#include "thrift/transport/TSocket.h"
#include "thrift/transport/TSimpleFileTransport.h"

#include "common.h"
#include "http_server_transport.h"
#include "zion_http_handler.h"
#include "data_center.h"
//#include <malloc.h>
#include <algorithm>

using namespace std;
using namespace spider;
using namespace boost;
using namespace apache::thrift::transport;
using namespace serializer;

ZionHttpIf* ZionHttpHandlerFactory::getHandler(const ::apache::thrift::TConnectionInfo& connection_info) {
    boost::shared_ptr<TSocket> cur_socket = dynamic_pointer_cast<TSocket>(connection_info.transport);
    ZionHttpHandler* handler = new ZionHttpHandler();
    if (cur_socket) {
        socklen_t len = 0;
        sockaddr_in* client_addr = (sockaddr_in*) cur_socket->getCachedAddress(&len);
        //int ip = client_addr->sin_addr.s_addr;

        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr->sin_addr.s_addr), addr_str, len);

        int port = (int) ntohs(client_addr->sin_port);

        handler->SetClientInfo(addr_str, port);

        if (DataCenter::GetInstance()->IsDisallowedIp(addr_str)) {
            cur_socket->close();
        }
    }
    return handler;
}

void ZionHttpHandlerFactory::releaseHandler(ZionHttpIf* handler) {
    delete handler;
}

bool ZionHttpHandler::HandleHttpRequest(shared_ptr<HttpServerTransport> http_server_transport, std::string& detail) {
    //http_server_transport->SetResponseBody(http_server_transport->DumpRequest());
    DEBUGL(http_server_transport->DumpRequest());
    string path = http_server_transport->GetRequestPath();

    if (path.empty()) 
        path = "/";
    size_t start_pos = path.find('?');

    STR_MAP kvs;
    string script_name = "";
    if (string::npos != start_pos) {
        script_name = path.substr(1, start_pos-1);
        size_t end_pos = path.find('#');
        const vector<string>& params = StringToTokens(string::npos == end_pos ? path.substr(start_pos + 1) : path.substr(start_pos + 1, end_pos - start_pos - 1), false, '&');
        FOR_EACH(it, params) {
            const vector<string>& param_kv = StringToTokens(*it, true, '=');
            if (param_kv.empty()) {
                continue;
            } else if (param_kv.size() > 1) {
                kvs[param_kv[0]] = UrlEncDecProcessor::UrlDecode(param_kv[1]);
            } else {
                kvs[param_kv[0]] = "";
            }
        }
    } else {
        script_name = path.substr(1);
    }
    DEBUGL("call script_name=" << script_name);

    if ("" == script_name) {
        std::string body;
        body += "usage:\n";
        body += "    status\n";
        body += "    ugc.php\n";
        http_server_transport->SetResponseBody(body);
        return true;
    } else if ("status" == script_name || "status.html" == script_name) {
        http_server_transport->SetResponseCode(200);
        http_server_transport->SetResponseBody("ok\n");
        return true;
    } else if ("report" == script_name) {
        std::string dict_name = GetValue(kvs, "dict", "");
        int num = DataCenter::GetInstance()->m_query_piper.GetDictKeyCount(dict_name);

        http_server_transport->SetResponseCode(200);
        http_server_transport->SetResponseBody(AllToString(num));
        return true;
    } else if ("ugc.php" == script_name || "ugc_test.php" == script_name) {
        std::string body = "";

        serializer::Object root = serializer::Object::map();
        ProcUgcRequest(kvs, root);

        if ("ugc.php" == script_name) {
            serializer::PHPSerializer php_s;
            php_s.Serialize(root, body);
        } else {
            serializer::JsonSerializer json_s;
            json_s.Serialize(root, body);
        }

        http_server_transport->SetResponseCode(200);
        http_server_transport->SetResponseBody(body);

        // log info
        std::string ugc_log = GetUgcLog(root);
        detail += "ugc_log:" + ugc_log;
        std::string content_log = GetContentLog(root);
        detail += "\tcontext:" + content_log;

        return true;
    } else if ("ugc_recom" == script_name) {
        std::string body = "";

        serializer::Object root = serializer::Object::map();
        ProcUserRecom(kvs, root);

        int ret_error_no = GetErrorNo(root);
        bool is_debug = GetValue(kvs, "debug", "no") == "yes";
        if (!is_debug && ret_error_no != UGC_ERR_OK) {      // 信息流，如果数据有问题，则返回空字符串
            body = "";
        } else {
            serializer::JsonSerializer json_s;
            json_s.Serialize(root, body);
        }

        http_server_transport->SetResponseCode(200);
        http_server_transport->SetResponseBody(body);

        // log info
        std::string ugc_log = GetUgcLog(root);
        detail += "ugc_log:" + ugc_log;
        std::string content_log = GetContentLog(root);
        detail += "\tcontext:" + content_log;

        return true;
    } else if ("get_news" == script_name) {
        std::string body = "";
        std::string title = GetValue(kvs, "title", "");
        body = ProcNews(title);
        http_server_transport->SetResponseCode(200);
        http_server_transport->SetResponseBody(body);
        return true;
    } else if ("redirect" == script_name) {
        http_server_transport->SetResponseCode(302);
        http_server_transport->SetResponseHeader("Location", UrlEncDecProcessor::UrlDecode(path.substr(strlen("/redirect?"))));
    } else {
        //printf("thread:%lu, path:%s, body:%s, request:{%s}\n", pthread_self(), path.data(), http_server_transport->GetRequestBody().data(), http_server_transport->DumpRequest().data());
        std::string body;
        bool is_file_ok = false;
        try {
            TSimpleFileTransport file(DataCenter::GetInstance()->GetHttpRootDir() + path);
            if (file.isOpen()) {
                is_file_ok = true;
                uint32_t rd_count = 0;
                uint8_t buf[1024];
                while ((rd_count = file.read(buf, ARRAYSIZE(buf))) > 0) {
                    body.append(reinterpret_cast<const char*>(buf), rd_count);
                }
            }
        } catch (...) {
        }

        if (!is_file_ok) {
            http_server_transport->SetResponseCode(404);
            return false;
        }
        http_server_transport->SetResponseBody(body);

        const vector<string>& path_break_down = StringToTokens(path, false, '/');
        if (!path_break_down.empty()) {
            const string& basename = *path_break_down.rbegin();
            if (string::npos != path.find("/js/")) {
                http_server_transport->SetResponseHeader("Content-Type", "application/x-javascript");
            } else if (string::npos != path.find("/css/")) {
                http_server_transport->SetResponseHeader("Content-Type", "text/css");
            } else if (string::npos != path.find("/image/")) {
                if (string::npos != basename.find(".jpg") || string::npos != basename.find(".jpeg")) {
                    http_server_transport->SetResponseHeader("Content-Type", "image/jpeg");
                } else if (string::npos != basename.find(".gif")) {
                    http_server_transport->SetResponseHeader("Content-Type", "image/gif");
                } else if (string::npos != basename.find(".png")) {
                    http_server_transport->SetResponseHeader("Content-Type", "image/png");
                } else if (string::npos != basename.find(".ico")) {
                    http_server_transport->SetResponseHeader("Content-Type", "image/x-icon");
                } else {
                    http_server_transport->SetResponseHeader("Content-Type", "image/");
                }
            }
        }
    }

    return true;
}

/*
 * 返回值含义：
 *  0. 成功
 *  4. 未命中
 *  5. 不在控制词表中
 *  6. 查db失败
 *  9. 数据格式错误，解json失败
 *
 */
bool ZionHttpHandler::ProcUgcRequest(const STR_MAP& kvs, serializer::Object& root) {
    ///////////////////////  debug start
    //while(1) {
    //    ::malloc_stats();
    //    sleep(1);
    //}
    ///////////////////////  debug end
    //UrlEncDecProcessor::UrlEncode(q)
    std::string q = GetValue(kvs, "k", "");   // 原始query
    std::string key = "";     // 泛化之后的key
    bool is_debug = GetValue(kvs, "debug", "no") == "yes";

    if (q.empty()) {
        std::string ugc_log = "q:" + q + ";error:empty_query";

        root.insert("errno", UGC_ERR_EMPTY_QUERY);
        root.insert("ugc_log", ugc_log);
        return false;
    }

    if (is_debug) {
        return ProcDebug(q, GetValue(kvs, "type", ""), root);
    }

    // 获取query控制项
    bool is_q_black, is_q_recom;
    DataCenter::GetInstance()->m_query_piper.GetQueryControl(q, is_q_black, is_q_recom);

    // 判断是否黑名单
    if (is_q_black) {
        std::string ugc_log = "q:" + q + ";error:black_query";

        root.insert("errno", UGC_ERR_BLACK_QUERY);
        root.insert("ugc_log", ugc_log);
        return false;
    }

    // 处理普通泛化流程
    return ProcExtend(q, root);

    /*
    // 处理用户推荐流程
    if (is_q_recom) {
        std::string guid = GetValue(kvs, "guid", "");
        std::size_t g_pos = guid.find(".");
        if (g_pos != std::string::npos) {
            guid = guid.substr(0, g_pos);
        }
        if (!guid.empty())   {
            // reset root
            root = serializer::Object::map();
            return ProcUserRecom(q, guid, root);
        }
    }

    return false;
    */
}

int ZionHttpHandler::GetErrorNo(const serializer::Object& root) {
    try{
        Object::iterator iter = root.find("errno");
        if (iter != root.end())
            return iter.value().GetInt();
    } catch (...) {
        return -1;
    }

    return -1;
}

std::string ZionHttpHandler::GetUgcLog(const serializer::Object& root) {
    try{
        Object::iterator iter = root.find("ugc_log");
        if (iter == root.end())
            return "";
        return iter.value().GetString();
    } catch (...) {
        return "";
    }
}

std::string ZionHttpHandler::GetContentLog(const serializer::Object& root) {
    std::string datetime = "";
    int text_num = 0;
    int img_num = 0;
    int video_num = 0;

    try{
        Object::iterator iter = root.find("datetime");
        if (iter == root.end())
            return "";
        datetime = iter.value().GetString();

        iter = root.find("content");
        if (iter == root.end())
            return "";
        text_num = iter.value().size();

        iter = root.find("extra_content");
        if (iter == root.end())
            return "";

        for(Object::iterator sub = iter.value().begin(); sub != iter.value().end(); ++sub) {
            Object& obj = sub.value();
            if (obj.empty() || !obj.IsArray())
                continue;

            Object::iterator img_iter = obj.find("imgs");
            if (img_iter == obj.end())
                continue;
            if (img_iter.value().empty() || !img_iter.value().IsArray())
                continue;

            Object::iterator item_iter = img_iter.value().begin();
            if (item_iter == img_iter.value().end())
                continue;

            Object::iterator is_video_img_iter = item_iter.value().find("is_video_img");
            if (is_video_img_iter == item_iter.value().end())
                continue;
            if (is_video_img_iter.value().IsBool()) {
                bool is_video_img = is_video_img_iter.value().GetBool();
                if (is_video_img)
                    ++video_num;
                else
                    ++img_num;
            }
        }
    } catch (...) {
        return "";
    }

    char buff[1024] = "\0";
    sprintf(buff, "text:%d;img:%d;video:%d;uptime:%s", text_num, img_num, video_num, datetime.c_str());

    return std::string(buff);
}

bool ZionHttpHandler::ProcDebug(const std::string& q, const std::string& qtype, serializer::Object& root) {
    std::string ugc_log = "q:" + q;

    ugc_log += ";type:" + qtype;
    if (!GetJsonWithTypeInUgcDb(q, qtype, root)) {
        ugc_log += ";error:not_indb";

        root.insert("errno", UGC_ERR_NOT_IN_DB);
        root.insert("ugc_log", ugc_log);
        return false;
    }

    return true;
}

bool ZionHttpHandler::ProcExtend(const std::string& q, serializer::Object& root) {
    std::string ugc_log = "q:" + q;

    std::string key = "";
    std::string qtype = "";

    // 泛化query
    RecordItem trans_item;
    bool trans_ok = DataCenter::GetInstance()->m_query_piper.Translate(trans_item, q);
    if (trans_ok) {
        key = trans_item.index_value;
        qtype = trans_item.type;
        log(LOG_DEBUG, "action:query_piper\tquery:%s->%s\ttype:%s\tsource:%s", q.c_str(), key.c_str(), qtype.c_str(), trans_item.source.c_str());
        ugc_log += ";is_extend:1;k:" + key + ";src:" + trans_item.source;
    } else {
        key = q;
        qtype = "";
        log(LOG_DEBUG, "action:query_piper\tquery:%s\tresult:no_trans", q.c_str());
        ugc_log += ";is_extend:0";
    }

    // 查询ugc控制词表，查询是否命中
    bool is_ugc_control_match = false;
    if (qtype.empty()) {        // 未识别出query类型，则从词表控制表中匹配个类型，并赋值给qtype
        is_ugc_control_match = DataCenter::GetInstance()->m_query_piper.MatchUgcControl(key, "*", qtype);
    } else {        // 有query类型的，去词表控制表中查询一把是否命中
        std::string _matched_type = "";
        is_ugc_control_match = DataCenter::GetInstance()->m_query_piper.MatchUgcControl(key, qtype, _matched_type);
    }
    ugc_log += ";type:" + qtype;
    if (!is_ugc_control_match) {
        log(LOG_DEBUG, "action:query_piper\tmatch_ugc_dict:miss");
        ugc_log += ";error:not_white";

        root.insert("errno", UGC_ERR_NOT_WHITE);
        root.insert("ugc_log", ugc_log);
        return false;
    }

    // 获取词条数据
    if (!GetJsonWithTypeInUgcDb(key, qtype, root)) {
        ugc_log += ";error:notin_db";

        root.insert("errno", UGC_ERR_NOT_IN_DB);
        root.insert("ugc_log", ugc_log);
        return false;
    }

    // get datelog if exists
    Object::iterator datalog_iter = root.find("datalog");
    if (datalog_iter != root.end()) {
        if (datalog_iter.value().IsString()) {
            ugc_log += ";" + datalog_iter.value().GetString();
        }
    }

    root.insert("errno", UGC_ERR_OK);
    root.insert("ugc_log", ugc_log);

    return true;
}

bool ZionHttpHandler::ProcUserRecom(const STR_MAP& kvs, serializer::Object& root) {
    std::string q = GetValue(kvs, "q", "");
    std::string guid = GetValue(kvs, "guid", "");
    bool is_check_control = GetValue(kvs, "need_check", "") == "yes";
    std::size_t g_pos = guid.find(".");
    if (g_pos != std::string::npos) {
        guid = guid.substr(0, g_pos);
    }

    std::string ugc_log = "q:" + q + ";guid:" + guid;
    root.insert("q", q);
    root.insert("guid", guid);

    // 获取query控制项
    bool is_q_black, is_q_recom;
    DataCenter::GetInstance()->m_query_piper.GetQueryControl(q, is_q_black, is_q_recom);
    // 判断是否黑名单
    if (is_q_black) {
        ugc_log += ";error:black_query";

        root.insert("errno", UGC_ERR_BLACK_QUERY);
        root.insert("ugc_log", ugc_log);
        return false;
    }
    // 判断是否推荐控制
    if (is_check_control) {
        if (!is_q_recom) {
            ugc_log += ";error:query_not_recom";

            root.insert("errno", UGC_ERR_EMPTY_RECOM);
            root.insert("ugc_log", ugc_log);
            return false;
        }
    }

    // 选取query的泛化结果，用于从推荐里面扣除
    std::string q_key = q;
    RecordItem trans_item;
    bool trans_ok = DataCenter::GetInstance()->m_query_piper.Translate(trans_item, q);
    if (trans_ok) {
        q_key = trans_item.index_value;
    }

    // 获取词条
    std::map<std::string, std::string> key_type_dict;
    std::vector<STR_PAIR> key_type_inputs;

    std::vector<std::string> selected_sites;        // 已选择的site列表，用于数据多样化

    int guid_key_num = 0;
    int hot_key_num = 0;
    serializer::Object res = serializer::Object::vector();
    // 先根据guid获取
    // guid列表中只有key，无type
    key_type_inputs.clear();
    if (1){
        std::vector<STR_PAIR> key_type_from_guid;
        UserRecomItem recom_item;
        DataCenter::GetInstance()->m_query_piper.GetRecomInfo(recom_item, guid);
        FOR_EACH(iter, recom_item.keys) {
            std::string k = *iter;
            if (k == q_key)         // 不能出泛化结果
                continue;
            if (key_type_dict.count(k) > 0)
                continue;

            std::string qtype = "";
            if (!DataCenter::GetInstance()->m_query_piper.MatchUgcControl(k, "*", qtype))
                continue;

            key_type_dict[k] = qtype;
            key_type_from_guid.push_back(std::make_pair(k, qtype));
        }

        // shuf & select
        // 最多占2/3
        std::random_shuffle(key_type_from_guid.begin(),key_type_from_guid.end());
        if (key_type_from_guid.size() > RECOM_AT_LEAST_NUM*2/3)
            key_type_inputs.assign(key_type_from_guid.begin(), key_type_from_guid.begin()+RECOM_AT_LEAST_NUM*2/3);
        else
            key_type_inputs.assign(key_type_from_guid.begin(), key_type_from_guid.end());

        log(LOG_DEBUG, "RecomFromGuid\tkeys:%s", TokensToString(key_type_inputs, ",").c_str());
        STR_MAP params;
        params["stag"] = "from_guid";
        guid_key_num = GetJsonMixInUgcDb(key_type_inputs, params, (key_type_from_guid.size()>RECOM_AT_LEAST_NUM*2/3)?RECOM_AT_LEAST_NUM*2/3:key_type_from_guid.size(), selected_sites, res);
    }

    // 若不足，则从热门中获取
    // 热门list中有key有type
    key_type_inputs.clear();
    if (guid_key_num < RECOM_AT_LEAST_NUM) {
        std::vector<STR_PAIR> hot_out;
        int sel_hot_buff_num = RECOM_AT_LEAST_NUM > 3 ? 15 : 10;    // ugc_dict中的数据，可能在db中不存在，故留了buffer，多取一点
        DataCenter::GetInstance()->m_query_piper.GetRandomHotItems(hot_out, sel_hot_buff_num);
        FOR_EACH(iter, hot_out) {
            std::string k = iter->first;
            std::string qtype = iter->second;
            if (k == q_key)
                continue;
            if (key_type_dict.count(k) > 0)
                continue;

            key_type_dict[k] = qtype;
            key_type_inputs.push_back(std::make_pair(k, qtype));
        }

        log(LOG_DEBUG, "RecomFromHot\tkeys:%s", TokensToString(key_type_inputs, ",").c_str());
        STR_MAP params;
        params["stag"] = "from_hot";
        hot_key_num = GetJsonMixInUgcDb(key_type_inputs, params, RECOM_AT_LEAST_NUM - guid_key_num, selected_sites, res);
    }
    ugc_log += ";is_extend:2;guid_knum:" + AllToString(guid_key_num) + ";hot_knum:" + AllToString(hot_key_num) +  ";source:user_trace;tracever:3.0";

    // 获取词条数据
    if (guid_key_num + hot_key_num < RECOM_AT_LEAST_NUM) {
        ugc_log += ";error:not_enough_recom";

        root.insert("errno", UGC_ERR_NOTENOUGH_RECOM);
        root.insert("ugc_log", ugc_log);
        return false;
    }

    serializer::Object data = serializer::Object::map();
    data.insert("res", res);

    root.insert("data", data);
    root.insert("guid", guid);
    root.insert("q", q);
    root.insert("errno", UGC_ERR_OK);
    root.insert("ugc_log", ugc_log);

    return true;
}


std::string ZionHttpHandler::ProcNews(const std::string& title) {
    std::string key = DataCenter::GetInstance()->m_query_piper.GetTmpTrieInfo(title);
    if (key.empty())
        return "";

    // 查询ugc控制词表，查询是否命中
    std::string qtype = "";
    if (!DataCenter::GetInstance()->m_query_piper.MatchUgcControl(key, "*", qtype)) {
        log(LOG_DEBUG, "action:query_piper\tkey:%s\tmatch_ugc_dict:miss", key.c_str());
        return "";
    }

    serializer::Object root = serializer::Object::map();
    if(!GetJsonWithTypeInUgcDb(key, qtype, root)) {
        log(LOG_INFO, "action:query_news\ttitle:%s\tkey:%s\ttype:%s\treason:GetJsonWithTypeInUgcDb fail", title.c_str(), key.c_str(), qtype.c_str());
        return "";
    }

    std::vector<std::string> out_arr;
    Object::iterator iter = root.find("extra_content");
    if (iter != root.end()) {
        for (Object::iterator item = iter.value().begin(); item != iter.value().end(); ++item) {
            if (out_arr.size() >= 10)
               break;

            STR_MAP parse_d;
            ParseExtraItemFromUgcObj(parse_d, item.value());

            std::string title = GetValue(parse_d, "title", "");
            std::string url = GetValue(parse_d, "url", "");
            std::string site = GetValue(parse_d, "site", "");
            std::string img = GetValue(parse_d, "img", "");
            std::string type = GetValue(parse_d, "type", "");

            if (title.empty() || (site != "toutiao.com" && site != "163.com"))
                continue;
            out_arr.push_back(title + "\t" + img);
        }
    }

    iter = root.find("content");
    if (iter != root.end()) {
        for (Object::iterator item = iter.value().begin(); item != iter.value().end(); ++item) {
            if (out_arr.size() >= 10)
               break;

            STR_MAP parse_d;
            ParseExtraItemFromUgcObj(parse_d, item.value());

            std::string title = GetValue(parse_d, "title", "");
            std::string url = GetValue(parse_d, "url", "");
            std::string site = GetValue(parse_d, "site", "");
            std::string img = GetValue(parse_d, "img", "");
            std::string type = GetValue(parse_d, "type", "");

            if (title.empty() || (site != "toutiao.com" && site != "163.com"))
                continue;
            out_arr.push_back(title + "\t" + img);
        }
    }

    std::random_shuffle(out_arr.begin(),out_arr.end());     // shuf

    static std::string s_prefix = "V20150612kernel_testugc\t相关新闻\t";
    static std::string s_sufix = "\t{}";

    std::string out_str = "";
    for (std::vector<std::string>::iterator iter = out_arr.begin();
            out_arr.end() != iter;
            ++iter) {
        if (!out_str.empty())
            out_str += "\n";
        out_str += s_prefix + *iter + s_sufix;

    }

    log(LOG_INFO, "action:query_news\ttitle:%s\tkey:%s\tcontent_size:%d", title.c_str(), key.c_str(), out_arr.size());
    return out_str;
}


bool ZionHttpHandler::GetJsonWithTypeInUgcDb(const std::string& key, const std::string& qtype, serializer::Object& root) {
    pthread_t tid = pthread_self();
    std::string ugc_redis_ret;
    if (!DataCenter::GetInstance()->m_ugc_redis_pool.GetDBRecordWithHkey(tid, key, qtype, ugc_redis_ret)) {
        return false;
    }
    if (ugc_redis_ret.size() < 2)
        return false;
    serializer::JsonUnserializer json_u;
    bool ret = json_u.Unserialize(ugc_redis_ret.c_str(), ugc_redis_ret.size(), root);

    return ret;
}

// 获取多个key的mix结果
int ZionHttpHandler::GetJsonMixInUgcDb(const std::vector<STR_PAIR>& key_type_inputs, const STR_MAP& params, const int limit_num, std::vector<std::string>& selected_sites, serializer::Object& res) {
    pthread_t tid = pthread_self();

    int select_num = 0;
    serializer::JsonUnserializer json_u;
    FOR_EACH(iter, key_type_inputs) {
        if (limit_num > 0 && select_num >= limit_num)
            break;

        std::string k = iter->first;
        std::string qtype = iter->second;
        std::string ugc_redis_ret = "";
        if (!DataCenter::GetInstance()->m_ugc_redis_pool.GetDBRecordWithHkey(tid, k, qtype, ugc_redis_ret)) {
            continue;
        }

        serializer::Object item = serializer::Object::map();
        if (!json_u.Unserialize(ugc_redis_ret.c_str(), ugc_redis_ret.size(), item))
            continue;

        std::string item_k = "";
        if (item.find("key") != item.end())
            item_k = item.find("key").value().GetString();

        Object::iterator extra_iter = item.find("extra_content");
        if (extra_iter != item.end()) {
            int num = extra_iter.value().size();

            // check & get items
            for (int ind = 0; ind < num; ++ind) {
                std::string site = GetUgcObjSite(extra_iter.value().at(ind));
                if (site == "buluo.qq.com")        // 不使用buluo的数据
                    continue;

                // 数据多样化
                if ( (selected_sites.size() > 0 && selected_sites[selected_sites.size()-1] == site))
                    continue;

                STR_MAP parse_d;
                ParseExtraItemFromUgcObj(parse_d, extra_iter.value().at(ind));

                // 格式化输出
                std::string title = GetValue(parse_d, "title", "");
                std::string url = GetValue(parse_d, "url", "");
                site = GetValue(parse_d, "site", "");
                std::string source_zh = GetValue(parse_d, "source", site);
                std::string img = GetValue(parse_d, "img", "");
                std::string type = GetValue(parse_d, "type", "");
                int cmt_num = atoi(GetValue(parse_d, "comment_num", "0").c_str());      // 评论数

                if (title.empty() || url.empty() || img.empty())
                    continue;

                serializer::Object record = serializer::Object::map();
                record.insert("url", url);
                record.insert("pcurl", url);
                record.insert("rawurl", url);
                record.insert("t", title);
                record.insert("f", source_zh);
                record.insert("c", type);
                record.insert("i", img);
                record.insert("cmt_num", cmt_num);
                record.insert("k", item_k);

                FOR_EACH(p_it, params) {
                    record.insert(p_it->first, p_it->second);
                }

                res.push_back(record);
                selected_sites.push_back(site);

                ++select_num;

                break;
            }
        }
    }

    return select_num;
}

std::string ZionHttpHandler::GetUgcObjSite(const serializer::Object& item) {
    Object::iterator iter = item.find("site");
    if (iter != item.end())
        return iter.value().GetString();
    return "";
}

void ZionHttpHandler::ParseExtraItemFromUgcObj(STR_MAP& out, const serializer::Object& item) {
    Object::iterator iter = item.find("title");
    if (iter != item.end())
        out["title"] = iter.value().GetString();

    iter = item.find("href");
    if (iter != item.end())
        out["url"] = iter.value().GetString();

    iter = item.find("type");
    if (iter != item.end())
        out["type"] = iter.value().GetString();

    iter = item.find("site");
    if (iter != item.end())
        out["site"] = iter.value().GetString();

    iter = item.find("source");
    if (iter != item.end())
        out["source"] = iter.value().GetString();

    iter = item.find("comment_num");
    if (iter != item.end())
        out["comment_num"] = iter.value().GetInt();

    iter = item.find("imgs");
    if (iter != item.end()) {
        for (Object::iterator img_iter = iter.value().begin(); img_iter != iter.value().end(); ++img_iter) {
            Object& img = img_iter.value();
            std::string img_addr = "";

//            if (img.find("is_video_img") != img.end()) {        // 视频的图片不要
//                if (img.find("is_video_img").value().GetBool())
//                    continue;
//            }

            if (img.find("qhimgsrc") != img.end()) {
                img_addr = img.find("qhimgsrc").value().GetString();
            }
            std::string width = "";
            if (img.find("width") != img.end()) {
                width = AllToString(img.find("width").value().GetInt());
            }
            std::string height = "";
            if (img.find("height") != img.end()) {
                height = AllToString(img.find("height").value().GetInt());
            }
            if (!width.empty() && !height.empty() && !img_addr.empty())
                img_addr += "?size=" + width + "x" + height;

            out["img"] = img_addr;
            break;
        }
    }
}

