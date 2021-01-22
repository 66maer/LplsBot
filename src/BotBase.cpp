#include "BotBase.h"
#include "sdk/sdk.h"

using namespace std;

BotBase::BotBase():baseDB_(nullptr) {
    int rc = sqlite3_open("LplsBaseDB.db", &baseDB_);
    thread([this] { // 每日更新用户列表
        while (true) {
            UpdateBotList();
            std::this_thread::sleep_for(std::chrono::hours(24));
        }
    }).detach();
}

BotBase::~BotBase() {
    sqlite3_close(db);
    db = nullptr;
}

std::shared_ptr<Bot> BotBase::GetBot(std::string sdkQQ, std::string groupQQ) const {
    shared_lock<shared_mutex> lk(mux_);
    if (auto it = bot_list_.find(groupQQ); it != bot_list_.end()) {
        return it->second;
    }
    return nullptr;
}

void BotBase::UpdateBotList() {
    auto users = DB_SelectBotList();
    unique_lock<shared_mutex> lk(mux_);
    bot_list_.clear();
    for (auto& user : users) {
        for (auto& gQQ : user.groupQQs) {
            bot_list_[gQQ] = make_shared<Bot>();
        }
    }
}

std::vector<BotBase::UserInfo> BotBase::DB_SelectBotList() {
    vector<UserInfo> rtn;
    auto             t       = to_string(time(0));
    string           sql_str = "select * from user_table where end_time < " + t + ";";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        baseDB_, sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto              p = static_cast<vector<UserInfo>*>(vp);
            BotBase::UserInfo q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "userQQ")
                    q.userQQ = val[i];
                else if (s == "sdkQQ")
                    q.sdkQQ = val[i];
                else if (s == "groupQQs")
                    q.groupQQs = Mtools::Split(val[i]," ");
                else if (s == "managerQQs")
                    q.managerQQs = Mtools::Split(val[i], " ");
                else if (s == "start_time")
                    q.start_time = stoll(val[i]);
                else if (s == "end_time")
                    q.end_time = stoll(val[i]);
            }
            p->push_back(q);
            return 0;
        },
        &rtn, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找报名信息错误:" + string(cErrMsg));
    }
    return rtn;
}
