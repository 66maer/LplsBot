#include "TeamModule.h"

TeamModule::TeamModule() : ModuleBase(true, 50) {
    if (!Init()) {
        set_enable(false);
    }
}

bool TeamModule::ProcGroupMess(const GroupMessageData& data) {

    // ģ������߼�

    return false;
}

bool TeamModule::Init() {

    // ����Ӧ��Ҫ��ʼ�����ݿ�֮���

    return false;
}
