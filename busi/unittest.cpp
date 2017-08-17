#include <iostream>
#include <openssl/md5.h> 
#include "obtainData_Ssdb.h"
#include "queryGenerator.h"
#include "obtainUrl_Http.h"
#include "json_spirit.h"
using namespace json_spirit; 
void parseJson(const std::string& str){
    Value value; 
    if (!read( str, value )){
        std::cout << "json parse error" << std::endl;
    }
    const Array& addr_array = value.get_array();
    for( unsigned int i = 0; i < addr_array.size(); ++i ){
        Object obj = addr_array[i].get_obj();   
        for( Object::size_type j = 0; j != obj.size(); ++j ){
            const Pair& pair = obj[j]; 
            std::string name  = pair.name_;
            const Value& objValue = pair.value_;
            std::cout << "obj." << j << ".name" << name.c_str() << std::endl;
            if (name == "querys"){
                const Array& childArray = objValue.get_array();
                for (unsigned int k = 0; k != childArray.size(); ++k){
                    Object obj1 = childArray[k].get_obj();
                    for( Object::size_type m = 0; m != obj1.size(); ++m ){
                        const Pair& p = obj1[m];
                        std::string name1=p.name_;
                        const Value& value1=p.value_;
                        std::cout << "obj." << m << ".name " << name1.c_str() << std::endl;
                        if (name1 == "query")
                            std::cout << "obj." << m << ".value " << value1.get_str() << std::endl;
                        else
                            std::cout << "obj." << m << ".value " << value1.get_real() << std::endl;
                    }
                }
            }
        }
    }
}
int main (){
    std::cout << "unit test" << std::endl;
    ObtainData_Ssdb query ("unittest.log", "page9.se.zzzc.qihoo.net", 18600, "q2q_nlp", 0.0);
    QueryGenerator queryG;
    queryG.Init ("queryGenerator.ini", "unittest.log");
    std::string q;
    std::string ret;
    QueryStruct qStruct;
    ObtainUrl_Http obtHttp (qStruct, "server", 10, "unittest.log");
    while (std::cin>>q){
        queryG.Run (q, &qStruct);
  //      obtHttp.Run (q, &qStruct);
    }
    return 0;
}
