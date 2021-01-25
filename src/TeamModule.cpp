#include "TeamModule.h"

using namespace std;

TeamModule::TeamModule() : ModuleBase(true, 50) {
    if (!Init()) {
        set_enable(false);
    }
}

bool TeamModule::ProcGroupMess(const GroupMessageData& data) {
    string mess = data.MessageContent;
    if (Trigger(mess, { "����" })) {
        CreateTeamProc(data);
    }
    else if (Trigger(mess, { "����", "�ǼǱ���", "�ϰ屨��", "�ϰ�ǼǱ���" })) {
        //BaominProc(data);
    }
    else if (Trigger(mess, { "����" }) || Trigger(mess, { "������", "������", "����ô", "�б���", "�б���", "�б�ô", "����" }, false)) {
        //ChaTuanProc(data);
    }
    else if (Trigger(mess, { "ȡ������" })) {
        //CancelKaituanProc(data);
    }
    else if (Trigger(mess, { "ȡ������" })) {
        //CancelBaominProc(data);
    }
    else {
        return false;
    }
    return true;
}

bool TeamModule::Init() {

    // ����Ӧ��Ҫ��ʼ�����ݿ�֮���

    return false;
}

bool TeamModule::IsManager(const GroupMessageData& data) {
    auto gly_str = api->GetAdministratorList(data.ThisQQ, data.MessageGroupQQ);
    auto glys    = Mtools::Split(gly_str, "\r\n");
    auto glyqq   = to_string(data.SenderQQ);
    for (auto gly : glys) {
        if (gly == glyqq) {
            return true;
        }
    }
    return false;
}

bool TeamModule::Trigger(const std::string& mess, const std::vector<std::string>& keys, bool first) const {
    for (auto& key : keys) {
        if (first) {
            if (mess.substr(0, key.size()) == key) {
                return true;
            }
        }
        else {
            if (mess.find(key) != -1) {
                return true;
            }
        }
    }
    return false;
}

void TeamModule::CreateTeamProc(const GroupMessageData& data) {

}
