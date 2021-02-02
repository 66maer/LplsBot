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

enum class XinfaID {
    DaXia,
    HuaJian,
    NaiHua,
    BingXin,
    NaiXiu,
    YiJin,
    XiSui,
    QiChun,
    JianChun,
    AoXue,
    TieLao,
    CangJian,
    DuJing,
    NaiDu,
    Jingyu,
    TianLuo,
    FenYing,
    MingZun,
    GaiBang,
    FenShan,
    TieGu,
    MoWen,
    NaiGe,
    BaDao,
    PengLai,
    LingXue,
    YanTian,
    All
};

enum class TeamStatus {
    SigningUp,   // 报名中
    Processing,  // 进行中
    Finish,      // 已完成
    Invalid,     // 已失效
};

enum class WorkerType {
    Normal,    // 正常
    Rich,      // 老板
    OverRich,  // 包团
};

enum class WorkerStatus {
    Normal,  // 正常
    Note,    // 仅登记
    Cancel,  // 取消
    Pigeon,  // 鸽了
};

struct TeamConfig {
    TeamConfig() {
        T = nai = dps = nei_dps = wai_dps = rich = {0,0};
        xinfas.resize(static_cast<int>(XinfaID::All));
    }
    std::pair<int, int>              T;
    std::pair<int, int>              nai;
    std::pair<int, int>              dps;
    std::pair<int, int>              nei_dps;
    std::pair<int, int>              wai_dps;
    std::pair<int, int>              rich;
    std::vector<std::pair<int, int>> xinfas;
};

struct Zone {
    Zone() : zone_size(0) {}
    std::string              zone_name;
    std::vector<std::string> zone_nickname;
    TeamConfig               zone_config;
    int                      zone_size;
};

struct Xinfa {
    Xinfa() {
        xinfa_id   = XinfaID::DaXia;
        core_score = 0;
        rgb        = { 0, 0, 0 };
    }
    XinfaID                   xinfa_id;
    std::string               xinfa_name;
    std::vector<std::string>  xinfa_nicknames;
    std::string               xinfa_type_1;
    std::string               xinfa_type_2;
    std::string               xinfa_type_3;
    std::tuple<int, int, int> rgb;
    int                       core_score;
};

struct Worker {
    std::string  sendQQ;
    std::string  ownerQQ;
    std::string  game_id;
    Xinfa        xinfa;
    WorkerType   type;
    WorkerStatus status;
};

struct Team {
    Team() : team_id(0), build_time(0), process_time(0), finish_time(0), status(TeamStatus::Invalid), glod(0) {}
    int                 team_id;
    std::string         team_name;
    Zone                zone;
    TeamConfig          now_cnfg;
    std::vector<Worker> worker_list;
    std::string         remark;
    time_t              build_time;
    time_t              process_time;
    time_t              finish_time;
    TeamStatus          status;
    std::string         debutQQ;
    int                 glod;
    std::string         goods;
};

struct ReMess {
    int         percent;
    std::string mess;
};