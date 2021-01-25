#include "TeamModule.h"

TeamModule::TeamModule() : ModuleBase(true, 50) {
    if (!Init()) {
        set_enable(false);
    }
}

bool TeamModule::ProcGroupMess(const GroupMessageData& data) {

    // 模组相关逻辑

    return false;
}

bool TeamModule::Init() {

    // 这里应该要初始化数据库之类的

    return false;
}
