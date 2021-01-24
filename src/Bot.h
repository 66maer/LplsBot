#pragma once
#include "stdafx.h"
#include "sdk/sdk.h"

class ModuleBase;

class Bot {
public:
    Bot(const UserInfo& userinfo);

    bool OnGroupMessage(const GroupMessageData& data);

private:
    void RegModule();

private:
    UserInfo user_info_;

    std::vector<ModuleBase> module_list_;
};