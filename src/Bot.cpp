#include "Bot.h"
#include "ModuleBase.h"
#include "TeamModule.h"

using namespace std;

Bot::Bot(const UserInfo& userinfo):user_info_(userinfo) {
    RegModule();
}

bool Bot::OnGroupMessage(const GroupMessageData& data) {

    for (auto& mod : module_list_) {
        if (mod.handle() & HandleGroupMessage) {
            return mod.run(data);
        }
    }
    return false;
}

void Bot::RegModule() {
    // 新加模块需要在这里注册
    module_list_.emplace_back(TeamModule()); // 开团排坑模块

    // 按照优先级排序模组
    sort(module_list_.begin(), module_list_.end(), [](auto a, auto b) { a.lv() < b.lv() });
}
