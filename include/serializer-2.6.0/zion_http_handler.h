#ifndef ZionHttp_HANDLER_H_
#define ZionHttp_HANDLER_H_

#include "ZionHttp.h"
#include "http_server_transport.h"
#include "defines.h"
#include "serialize.h"

namespace spider {

class ZionHttpHandler: virtual public ZionHttpIf {
public:
    ZionHttpHandler() {
        // Your initialization goes here
    }

    void Echo(std::string& _return, const std::string& message) {
        // Your implementation goes here
        _return = message;
    }

    void SetClientInfo(const std::string& ip, int port) {
        m_client_ip = ip;
        m_client_port = port;
    }

    bool HandleHttpRequest(::boost::shared_ptr<HttpServerTransport> http_server_transport, std::string& detail);

protected:
    bool ProcUgcRequest(const STR_MAP& kvs, serializer::Object& root);

    bool ProcDebug(const std::string& q, const std::string& qtype, serializer::Object& root);
    bool ProcExtend(const std::string& q, serializer::Object& root);
    bool ProcUserRecom(const STR_MAP& kvs, serializer::Object& root);
    std::string ProcNews(const std::string& title);

    int GetErrorNo(const serializer::Object& root);
    std::string GetUgcLog(const serializer::Object& root);
    std::string GetContentLog(const serializer::Object& root);

    bool GetJsonWithTypeInUgcDb(const std::string& key, const std::string& qtype, serializer::Object& root);
    int GetJsonMixInUgcDb(const std::vector<STR_PAIR>& key_type_inputs, const STR_MAP& params, const int limit_num, std::vector<std::string>& selected_sites, serializer::Object& data);

private:
    void ParseExtraItemFromUgcObj(STR_MAP& out, const serializer::Object& item);
    std::string GetUgcObjSite(const serializer::Object& item);

public:
    std::string m_client_ip;
    int m_client_port;
};

class ZionHttpHandlerFactory: virtual public ZionHttpIfFactory {
public:
    virtual ZionHttpIf* getHandler(const ::apache::thrift::TConnectionInfo& connection_info);
    virtual void releaseHandler(ZionHttpIf* handler);
};

} //end of namespace spider

#endif //end of ZionHttp_HANDLER_H_
