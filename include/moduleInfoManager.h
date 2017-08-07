#ifndef __RECOMMEND_FRAME_MODULE_INFO_MANAGER__
#define __RECOMMEND_FRAME_MODULE_INFO_MANAGER__
class ModuleInfoManager{

    
private:
    struct ModuleInfo{
        std::string fatherModuleId;
        std::string inputField;
    };
private:
    std::map<std::string, ModuleInfo>
};

#endif
