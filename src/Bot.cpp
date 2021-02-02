#include "Bot.h"
#include "ModuleBase.h"
#include "TeamModule.h"

using namespace std;

Bot::Bot(const UserInfo& userinfo, sqlite3* base_db) : user_info_(userinfo), base_db_(base_db) {
    int rc = sqlite3_open((userinfo.name + userinfo.userQQ + ".db").c_str(), &db_);
    RegModule();
}

Bot::~Bot() {
    sqlite3_close(db_);
    db_ = nullptr;
}

bool Bot::OnPrivateMessage(const PrivateMessageData& data) {
    for (auto& mod : module_list_) {
        if (mod->enable()) {
            if (mod->ProcPrivateMess(data)) {
                return true;  // 表示消息由当前模组处理完毕且终结，不会再被其他低优先级模组处理
            }
        }
    }
    return false;
}

bool Bot::OnGroupMessage(const GroupMessageData& data) {
    for (auto& mod : module_list_) {
        if (mod->enable()) {
            if (mod->ProcGroupMess(data)) {
                return true;  // 表示消息由当前模组处理完毕且终结，不会再被其他低优先级模组处理
            }
        }
    }
    return false;
}

bool Bot::OnEvent(const EventData& data) {
    for (auto& mod : module_list_) {
        if (mod->enable()) {
            if (mod->ProcEvent(data)) {
                return true;  // 表示消息由当前模组处理完毕且终结，不会再被其他低优先级模组处理
            }
        }
    }
    return false;
}

bool Bot::Init() {
    return false;
}

void Bot::RegModule() {
    // 新加模块需要在这里注册
    module_list_.emplace_back(make_shared<TeamModule>(base_db_, db_));  // 开团排坑模块
    // module_list_.emplace_back(骚话); // 骚话模块
    // module_list_.emplace_back(查询); // 查询模块
    // module_list_.emplace_back(网络查询); // 如果写了爬虫的话
    // ...

    // 按照优先级排序模组
    sort(module_list_.begin(), module_list_.end(), [](auto a, auto b) { return a->lv() < b->lv(); });
}
