#pragma once
#define _USE_QTTOOLS_QTLOG
#define _USE_MATH_DEFINES
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include "MTools.h"
#include "library/sqlite3.h"
#include "library/CImg.h"
#include <ft2build.h>
#include FT_FREETYPE_H

extern sqlite3* db;

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
