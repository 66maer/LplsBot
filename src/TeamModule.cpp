#include "TeamModule.h"

using namespace std;

TeamModule::TeamModule() : ModuleBase(true, 50) {
    if (!Init()) {
        set_enable(false);
    }
}

bool TeamModule::ProcGroupMess(const GroupMessageData& data) {
    string mess = data.MessageContent;
    if (Trigger(mess, { "开团" })) {
        CreateTeamProc(data);
    }
    else if (Trigger(mess, { "报名", "登记报名", "老板报名", "老板登记报名" })) {
        //BaominProc(data);
    }
    else if (Trigger(mess, { "查团" }) || Trigger(mess, { "有团吗", "有团咩", "有团么", "有本吗", "有本咩", "有本么", "打本吗" }, false)) {
        //ChaTuanProc(data);
    }
    else if (Trigger(mess, { "取消开团" })) {
        //CancelKaituanProc(data);
    }
    else if (Trigger(mess, { "取消报名" })) {
        //CancelBaominProc(data);
    }
    else {
        return false;
    }
    return true;
}

bool TeamModule::Init() {

    // 这里应该要初始化数据库之类的

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
