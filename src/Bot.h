#pragma once
#include "stdafx.h"
#include "sdk/sdk.h"
#include "Types.h"

class ModuleBase;

class Bot {
public:
    Bot(const UserInfo& userinfo);

    bool OnPrivateMessage(const PrivateMessageData& data);

    bool OnGroupMessage(const GroupMessageData& data);

    bool OnEvent(const EventData& data);

    void set_user_info(UserInfo user_info) {
        user_info_ = user_info;
    }

    UserInfo user_info() const {
        return user_info_;
    }

private:
    bool Init();

    void RegModule();

private:
    UserInfo user_info_;

    std::vector<ModuleBase> module_list_;
};