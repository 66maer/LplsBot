#pragma once
#include "stdafx.h"

// 这里写所有的共用类型

struct UserInfo {
    std::string              userQQ;
    std::string              sdkQQ;
    std::string              name;
    std::vector<std::string> groupQQs;
    std::vector<std::string> managerQQs;
    time_t                   start_time;
    time_t                   end_time;
};

enum Handle {
    HandlePrivateMessage = 0x0001,
    HandleGroupMessage   = 0x0002,
};

enum XinfaID {
    Xinfa_DaXia,
    Xinfa_HuaJian,
    Xinfa_NaiHua,
    Xinfa_BingXin,
    Xinfa_NaiXiu,
    Xinfa_YiJin,
    Xinfa_XiSui,
    Xinfa_QiChun,
    Xinfa_JianChun,
    Xinfa_AoXue,
    Xinfa_TieLao,
    Xinfa_CangJian,
    Xinfa_DuJing,
    Xinfa_NaiDu,
    Xinfa_Jingyu,
    Xinfa_TianLuo,
    Xinfa_FenYing,
    Xinfa_MingZun,
    Xinfa_GaiBang,
    Xinfa_FenShan,
    Xinfa_TieGu,
    Xinfa_MoWen,
    Xinfa_NaiGe,
    Xinfa_BaDao,
    Xinfa_PengLai,
    Xinfa_LingXue,
    Xinfa_YanTian,
    Xinfa_All
};