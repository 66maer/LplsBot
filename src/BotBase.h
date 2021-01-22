#pragma once
#include "stdafx.h"
#include "Types.h"
#include <shared_mutex>

class Bot;

class BotBase {
public:
    BotBase();

    ~BotBase();

    std::shared_ptr<Bot> GetBot(std::string sdkQQ, std::string groupQQ) const;

    void UpdateBotList();

private:

    std::vector<UserInfo> DB_SelectBotList();

    sqlite3* baseDB_;

    mutable std::shared_mutex mux_;

    std::map<std::string, std::shared_ptr<Bot>> bot_list_;  // 群号-机器人
};