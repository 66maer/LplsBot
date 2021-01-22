#pragma once
#include "stdafx.h"
#include <shared_mutex>
#include "Bot.h"
class BotBase {
public:
    BotBase();

    ~BotBase();

    std::shared_ptr<Bot> GetBot(std::string sdkQQ, std::string groupQQ) const;

    void UpdateBotList();

private:
    struct UserInfo {
        std::string userQQ;
        std::string               sdkQQ;
        std::vector<std::string>  groupQQs;
        std::vector<std::string>  managerQQs;
        time_t                    start_time;
        time_t                    end_time;
    };

    std::vector<UserInfo> DB_SelectBotList();

    sqlite3* baseDB_;

    mutable std::shared_mutex mux_;

    std::map<std::string, std::shared_ptr<Bot>> bot_list_;  // ÈººÅ-»úÆ÷ÈË
};