#ifndef __RECOMMEND_FRAME_MODULE_INFO_MANAGER__
#define __RECOMMEND_FRAME_MODULE_INFO_MANAGER__

#include "common.h"

class ModuleInfoManager{

public:
    ModuleInfoManager(){
        m_moduleInfoMap.clear();
        m_moduleNextList.clear();
    }
    int insert (const std::string& moduleId, const StrMap& p){
        // save module info
        m_moduleInfoMap.insert (std::pair<std::string, ModuleInfo>(moduleId, ModuleInfo(p)));

        // add to next list
        

        return 0;
    }
    int erase (const std::string& moduledId){
        m_moduleInfoMap.erase (moduleId); 
        return 0;
    } 
private:
    struct ModuleInfo{
        std::string fatherModuleId;
        std::string inputField;
        ModuleInfo(){
            fatherModuleId = "";
            inputField = "";
        }
        ModuleInfo (std::string fModuleId, std::iField):fatherModuleId (fModuleId), inputField (iField){
        
        }
        ModuleInfo (const StrMap& p){
            fatherModuleId = p["fatherModuleId"]; 
            inputField = p["inputField"];
        }
    };
private:
    std::map<std::string, ModuleInfo> m_moduleInfoMap;
    std::map<std::string, std::vector<std::string>> m_moduleNextList;
};

#endif
