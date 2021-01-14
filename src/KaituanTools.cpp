#include "KaituanTools.h"
#include "sdk/sdk.h"
using namespace std;
using namespace cimg_library;

KaituanTools::KaituanTools() {
    InitDataBase();
    InitDefineData();
    InitDefineReply();
    chatuan_cd = 0;
    group_QQs_.push_back("184161352");
    //group_QQs_.push_back("933469537");
}

bool KaituanTools::Run(const GroupMessageData& data) {
    bool cando = false;
    for (auto& qq : group_QQs_) {
        if (to_string(data.MessageGroupQQ) == qq) {
            cando = true;
            break;
        }
    }
    if (!cando)
        return false;
    string mess = data.MessageContent;
    if (Trigger(mess, { "开团" })) {
        KaituanProc(data);
    }
    else if (Trigger(mess, { "报名", "登记报名", "老板报名", "老板登记报名" })) {
        BaominProc(data);
    }
    else if (Trigger(mess, { "查团" }) || Trigger(mess, { "有团吗", "有团咩", "有团么", "有本吗", "有本咩", "有本么", "打本吗" }, false)) {
        ChaTuanProc(data);
    }
    else if (Trigger(mess, { "取消开团" })) {
        CancelKaituanProc(data);
    }
    else if (Trigger(mess, { "取消报名", "咕咕咕" })) {
        CancelBaominProc(data);
    }
    else {
        return false;
    }
    return true;
}

bool KaituanTools::InitDataBase() {
    char*  cErrMsg;
    string sql_str;
    sql_str = "create table if not exists fuben_table("
              "fuben_name varchar(50) not null,"
              "fuben_config varchar(100) not null,"
              "fuben_nickname varchar(100) not null,"
              "fuben_size int not null,"
              "PRIMARY KEY(fuben_name)"
              ");";
    int res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("创建 副本表 失败:" + string(cErrMsg));
        return false;
    }

    sql_str = "create table if not exists team_table("
              "tuan_id INTEGER PRIMARY KEY not null,"
              "team_name varchar(50) not null,"
              "fuben_name varchar(50) not null,"
              "start_time varchar(30) not null,"
              "build_time varchar(30) not null,"
              "status varchar(10) default null"
              ");";
    res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("创建 团表 失败:" + string(cErrMsg));
        return false;
    }

    sql_str = "create table if not exists sign_up_table ("
              "id INTEGER PRIMARY KEY not null,"
              "tuan_id int not null,"
              "QQ varchar(20) not null,"
              "game_name varchar(30) not null,"
              "xinfa_id int not null,"
              "status varchar(10) not null"
              ");";
    res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("创建 报名表 失败:" + string(cErrMsg));
        return false;
    }

    sql_str = "create table if not exists xinfa_table ("
              "xinfa_id int not null,"
              "xinfa_name varchar(10) not null,"
              "xinfa_nicknames varchar(100) not null,"
              "xinfa_type_1 varchar(10) not null,"
              "xinfa_type_2 varchar(10) not null,"
              "xinfa_type_3 varchar(10) not null,"
              "zhenyan_score varchar(10) not null,"
              "rgb varchar(20) not null,"
              "PRIMARY KEY(xinfa_id)"
              ");";
    res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("创建 心法表 失败:" + string(cErrMsg));
        return false;
    }

    sql_str = "create table if not exists huifu_table ("
              "huifu_key varchar(100) not null,"
              "huifu_values varchar(300)  not null,"
              "Probability int not null"
              "); ";
    res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("创建 回复表 失败:" + string(cErrMsg));
        return false;
    }

    // sql_str = "create table if not exists member_table ("
    //          "QQ varchar(20) not null,"
    //          "go_times int not null,"
    //          "gu_times int not null,"
    //          "PRIMARY KEY(QQ)"
    //          "); ";
    // res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    // if (res != SQLITE_OK) {
    //    api->OutputLog("创建 成员表 失败:" + string(cErrMsg));
    //    return false;
    //}

    api->OutputLog("数据库表准备完毕！");
    return true;
}

bool KaituanTools::InitDefineData() {
    AddXinfaData({ Xinfa_DaXia, "大侠", { "大侠" }, "大侠", "大侠", "大侠", 0, 0, 0, 0 });
    AddXinfaData({ Xinfa_HuaJian, "花间", { "花间", "花间游", "万花", "花花", "fafa", "发发" }, "dps", "内功", "元气", 40, 159, 31, 223 });
    AddXinfaData({ Xinfa_NaiHua, "奶花", { "奶花", "花奶", "离经易道", "离经", "奶花花", "奶发", "奶发发" }, "奶", "奶", "奶", 0, 159, 31, 223 });
    AddXinfaData({ Xinfa_BingXin, "冰心", { "冰心", "冰心诀", "七秀", "dps秀", "秀秀" }, "dps", "内功", "根骨", 50, 223, 31, 255 });
    AddXinfaData({ Xinfa_NaiXiu, "奶秀", { "奶秀", "秀奶", "云裳心经", "云裳" }, "奶", "奶", "奶", 0, 223, 31, 255 });
    AddXinfaData({ Xinfa_YiJin, "易筋", { "易筋", "易筋经", "大师", "秃子", "和尚", "少林", "秃驴", "灯泡", "圣僧", "易经", "佛祖" }, "dps", "内功", "元气", 100, 191, 159, 31 });
    AddXinfaData({ Xinfa_XiSui, "洗髓", { "洗髓", "洗髓经", "大师T", "大师t", "秃T", "秃t", "和尚T", "和尚t" }, "T", "T", "T", 80, 191, 159, 31 });
    AddXinfaData({ Xinfa_QiChun, "气纯", { "气纯", "气咩", "紫霞功", "渣男" }, "dps", "内功", "根骨", 90, 63, 159, 223 });
    AddXinfaData({ Xinfa_JianChun, "剑纯", { "剑纯", "剑咩", "太虚剑意", "备胎" }, "dps", "外功", "身法", 100, 63, 159, 223 });
    AddXinfaData({ Xinfa_AoXue, "傲血", { "傲血", "傲雪", "傲血", "傲血战意", "天策" }, "dps", "外功", "力道", 70, 222, 28, 28 });
    AddXinfaData({ Xinfa_TieLao, "铁牢", { "铁牢", "铁牢律", "天策T", "天策t", "策T", "策t", "狗T", "狗t" }, "T", "T", "T", 90, 222, 28, 28 });
    AddXinfaData({ Xinfa_CangJian, "藏剑", { "藏剑", "黄鸡", "黄叽", "问水诀", "问水", "藏剑", "山居剑意", "山居" }, "dps", "外功", "身法", 70, 223, 223, 63 });
    AddXinfaData({ Xinfa_DuJing, "毒经", { "毒经", "五毒", "dj", "DJ" }, "dps", "内功", "根骨", 60, 31, 95, 255 });
    AddXinfaData({ Xinfa_NaiDu, "奶毒", { "奶毒", "补天", "毒奶", "补天诀", "补天" }, "奶", "奶", "奶", 0, 31, 95, 255 });
    AddXinfaData({ Xinfa_Jingyu, "惊羽", { "惊羽", "鲸鱼", "惊羽诀" }, "dps", "外功", "力道", 50, 94, 167, 51 });
    AddXinfaData({ Xinfa_TianLuo, "田螺", { "田螺", "天罗", "天罗诡道" }, "dps", "内功", "元气", 80, 94, 167, 51 });
    AddXinfaData({ Xinfa_FenYing, "焚影", { "焚影", "明教", "焚影圣诀", "喵喵" }, "dps", "内功", "元气", 50, 223, 95, 31 });
    AddXinfaData({ Xinfa_MingZun, "明尊", { "明尊", "喵T", "喵t", "明尊琉璃体", "明尊玻璃体", "明教T", "明教t" }, "T", "T", "T", 90, 223, 95, 31 });
    AddXinfaData({ Xinfa_GaiBang, "丐帮", { "丐帮", "丐丐", "笑尘诀", "笑尘决", "最强心法", "最强职业" }, "dps", "外功", "力道", 70, 191, 159, 63 });
    AddXinfaData({ Xinfa_FenShan, "分山", { "分山", "苍云", "分山劲", "岔劲", "苍云dps", "岔劲" }, "dps", "外功", "身法", 60, 162, 11, 11 });
    AddXinfaData({ Xinfa_TieGu, "铁骨", { "铁骨", "铁骨", "苍云T", "苍云t", "铁骨衣", "苍wulingzhid云wifi", "wifi" }, "T", "T", "T", 50, 162, 11, 11 });
    AddXinfaData({ Xinfa_MoWen, "莫问", { "莫问", "长歌", "鸽子", "咕咕" }, "dps", "内功", "根骨", 90, 63, 223, 95 });
    AddXinfaData({ Xinfa_NaiGe, "奶歌", { "奶歌", "歌奶", "相知", "奶鸽", "鸽奶", "乳鸽", "奶咕", "奶咕咕" }, "奶", "奶", "奶", 0, 63, 223, 95 });
    AddXinfaData({ Xinfa_BaDao, "霸刀", { "霸刀", "霸小刀", "北傲诀", "北傲决" }, "dps", "外功", "力道", 40, 31, 31, 127 });
    AddXinfaData({ Xinfa_PengLai, "蓬莱", { "蓬莱", "伞伞", "凌海诀", "凌海决" }, "dps", "外功", "身法", 40, 143, 231, 218 });
    AddXinfaData({ Xinfa_LingXue, "凌雪", { "凌雪", "凌雪阁", "0雪", "零雪" }, "dps", "外功", "身法", 90, 91, 12, 12 });
    AddXinfaData({ Xinfa_YanTian, "衍天", { "衍天", "衍天宗", "太玄经", "演员", "衍员" }, "dps", "内功", "元气", 40, 159, 12, 159 });
    LoadXinfaData();

    AddFubenData({ "英雄达摩洞",
                   { 2, 4, 19, 19, 19, 0, { 0, 5, 1, 5, 1, 5, 0, 5, 5, 5, 1, 5, 10, 1, 5, 5, 10, 1, 5, 5, 0, 5, 1, 5, 5, 5, 10 } },
                   { "英雄达摩洞", "YX达摩洞", "yx达摩洞", "yxdmd", "YXDMD", "英雄dmd", "英雄DMD", "YXdmd" },
                   25 });
    AddFubenData({ "普通达摩洞",
                   { 2, 3, 20, 20, 20, 6, { 0, 5, 2, 5, 2, 5, 2, 5, 5, 5, 2, 5, 10, 2, 5, 5, 10, 2, 5, 5, 2, 5, 2, 5, 5, 5, 10 } },
                   { "普通达摩洞", "PT达摩洞", "pt达摩洞", "ptdmd", "PTDMD", "普通dmd", "普通DMD", "PTdmd" },
                   25 });
    LoadFubenData();

    return true;
}

bool KaituanTools::InitDefineReply() {
    vector<Reply> replys;
    if (ChaReplyData("@#FLAG#@", &replys) && replys.empty()) {
        AddReplyData("开团格式错误", "开团格式错误!开团格式为：“开团 这一车名字 副本名 备注信息”\n比如“开团 晚上 YX达摩洞 8:30”");
        AddReplyData("开团格式错误", "开团格式错误!开团格式为：“开团 这一车名字 副本名 备注信息”\n比如“开团 周六 YX达摩洞 13:20”");
        AddReplyData("开团格式错误", "开团格式错误!开团格式为：“开团 这一车名字 副本名 备注信息”\n比如“开团 第一车 YX达摩洞 20:45”");
        AddReplyData("开团格式错误", "开团格式错误!开团格式为：“开团 这一车名字 副本名 备注信息”\n比如“开团 第十八车 YX达摩洞 9老板”");
        AddReplyData("未找到副本", "没有找到“{0}”的相关配置信息。");
        AddReplyData("未找到副本", "你确定“{0}”是一个副本？");
        AddReplyData("未找到副本", "副本“{0}”不太能理解是啥意思");
        AddReplyData("未找到副本", "并不能开启一个叫“{0}”的副本");
        AddReplyData("团名重复", "已经有一个叫“{0}”的团啦，换个名字（标识符）试试？");
        AddReplyData("团名重复", "开团失败，叫做“{0}”的这一车已经开启过报名了");
        AddReplyData("团名重复", "“{0}”团名重复，是不是上一车没有结束？");
        AddReplyData("团名重复", "已经开启过“{0}”的团了，尝试 查团 看看详细信息");
        AddReplyData("开团成功", "开团成功！\n“{0}”，{1}，{2}");
        AddReplyData("未找到心法", "“{0}”不是一个有效的心法名称");
        AddReplyData("未找到心法", "你确定“{0}”是一个心法的名字？");
        AddReplyData("未找到心法", "要么是你格式写错了，要么“{0}”不是个心法名字。");
        AddReplyData("报名格式错误", "报名格式错误！报名格式为：“报名 <哪一车> 心法 游戏ID”\n比如 “报名 晚上 丐帮 拔苗苗”\n*如果只有一车的话，可以省略车名");
        AddReplyData("需要指定车名", "目前开放报名的有{0}车，需要指定车名。目前开放报名的团有：{1}");
        AddReplyData("目前没有团", "团长很懒，目前没有开团");
        AddReplyData("目前没有团", "目前没团，试试圈一下团长问问今天开团吗");
        AddReplyData("目前没有团", "目前没有开放报名的团。");
        AddReplyData("目前没有团", "目前还没开团，问问团长怎么说---");
        AddReplyData("查无此团", "查无此团！");
        AddReplyData("查无此团", "没有找到叫“{0}”的团");
        AddReplyData("查无此团", "你确定你输入对了？“{0}”");
        AddReplyData("车满", "坑满啦！下一次见！");
        AddReplyData("车满", "这一车没坑啦！");
        AddReplyData("T满", "T坑满啦！打输出-他-不香么？");
        AddReplyData("T满", "T坑满啦！玩奶妈-他-不香么？");
        AddReplyData("T满", "T坑满啦！去换一个爆炸的dps试试！");
        AddReplyData("T满", "T坑满啦！去换一个温柔可爱的奶妈试试！");
        AddReplyData("奶满", "奶坑满啦！打输出-他-不香么？");
        AddReplyData("奶满", "奶坑满啦！玩T-他-不香么？");
        AddReplyData("奶满", "奶坑满啦！去换一个爆炸的dps试试！");
        AddReplyData("奶满", "奶坑满啦！去换一个坚若磐石的T试试！");
        AddReplyData("奶满", "奶坑满啦！去换一个一碰就cei的T试试！", "10");
        AddReplyData("dps满", "dps坑满啦！玩T-他-不香么？");
        AddReplyData("dps满", "dps坑满啦！玩奶妈-他-不香么？");
        AddReplyData("dps满", "dps坑满啦！去换一个温柔可爱的奶妈试试！");
        AddReplyData("dps满", "dps坑满啦！去换一个坚若磐石的T试试！");
        AddReplyData("dps满", "dps坑满啦！去换一个一碰就cei的T试试！", "10");
        AddReplyData("内满", "内功坑满啦！换一个试试！");
        AddReplyData("外满", "外功坑满啦！换一个试试！");
        AddReplyData("老板满", "老板坑满啦！");
        AddReplyData("该心法满", "{0}坑满啦！换个心法试试？");
        AddReplyData("反思心法", "好好反思反思，这心法能进这本吗？{0}");
        AddReplyData("人名重复", "你已经报过名啦！\n{0} {1}");
        AddReplyData("人名重复", "已经有一个叫“{0}”的报名信息啦！\n{0} {1}");
        AddReplyData("报名成功", "报名成功！\n{0} {1}");
        AddReplyData("ID太长", "ID太长啦！{0}");
        AddReplyData("非管理员", "[@{0}]不是管理员您还想{1}？");
        AddReplyData("非管理员", "[@{0}]只有管理员才可以{1}哟~");
        AddReplyData("非管理员", "[@{0}]您没有权限{1}。");
        AddReplyData("非管理员", "[@{0}]还是圈一下团长{1}吧--");
        AddReplyData("非管理员", "[@{0}]不！");
        AddReplyData("非管理员", "[@{0}]{1}？宁配吗？");
        AddReplyData("查团", "目前有{0}个团开放了报名。排坑表生成中...");
        AddReplyData("查团图片", "{0}");
        AddReplyData("取消开团格式错误", "取消开团格式为“取消开团 哪一车”\n比如：取消开团 晚上");
        AddReplyData("取消开团成功", "取消开团成功！");
        AddReplyData("取消报名格式错误", "取消报名格式为“取消报名 <哪一车> <游戏ID>”\n比如：取消开团 晚上 拔苗苗");
        AddReplyData("需要指定ID", "您有{0}个报名的ID，需要指定ID\n{1}");
        AddReplyData("取消报名成功", "取消报名成功!{0} {1}");
        AddReplyData("未报名", "您还没有报名! {0}");
        AddReplyData("未报名", "报名表中没有你，这波啊，这波叫无中生有! {0}");
        AddReplyData("未报名", "不报名，就取消？咕。就硬咕呗！ {0}");
        AddReplyData("未报名", "咕了？等等？你报名了吗？ {0}");
        AddReplyData("不能取消他人报名", "你不能取消他人报名，\n{0},报名人是[@{1}]");
        AddReplyData("CD中", "尚在调息之中，静待片刻方可。");

        AddReplyData("@#FLAG#@", "@#FLAG#@");
    }
    return true;
}

bool KaituanTools::LoadXinfaData() {
    return ChaXinfaData(&xinfa_list_);
}

bool KaituanTools::LoadFubenData() {
    return ChaFubenData(&fuben_list_);
}

bool KaituanTools::IsManager(const GroupMessageData& data) {
    auto gly_str = api->GetAdministratorList(data.ThisQQ, data.MessageGroupQQ);
    auto glys    = Mtools::Split(gly_str, "\r\n");
    auto glyqq   = to_string(data.SenderQQ);
    for (auto gly : glys) {
        if (gly == glyqq) {
            return true;
        }
    }
    return false;
}

void KaituanTools::ReplyMessage(const GroupMessageData& data, const string& key, std::initializer_list<std::string> args = {}) {
    vector<Reply> replys;
    if (ChaReplyData(key, &replys) && !replys.empty()) {
        Reply reply;
        int   sum = 0;
        for (auto& r : replys) {
            sum += r.Probability;
        }
        int p = rand() % sum;
        sum   = 0;
        for (auto& r : replys) {
            sum += r.Probability;
            if (sum > p) {
                reply = r;
                break;
            }
        }
        int idx = 0;
        for (auto& arg : args) {
            string s = "{" + to_string(idx++) + "}";
            for (int pos = 0; pos != -1;)
                if (pos = reply.mess.find(s, pos); pos != -1) {
                    reply.mess = reply.mess.replace(pos = reply.mess.find(s, pos), s.size(), arg);
                }
        }
        api->SendGroupMessage(data.ThisQQ, data.MessageGroupQQ, reply.mess);
    }
    return;
    api->OutputLog("没有对应的回复内容：" + key);
}

bool KaituanTools::FindBuben(const std::string& fb_str, Fuben* fuben) {
    for (auto& fb : fuben_list_) {
        for (auto& s : fb.fuben_nickname) {
            if (fb_str == s) {
                *fuben = fb;
                return true;
            }
        }
    }
    return false;
}

bool KaituanTools::FindXinfa(const std::string& xf_str, Xinfa* xinfa) {
    for (auto& xf : xinfa_list_) {
        for (auto& s : xf.xinfa_nicknames) {
            if (xf_str == s) {
                *xinfa = xf;
                return true;
            }
        }
    }
    return false;
}

bool KaituanTools::Trigger(const std::string& mess, const std::vector<std::string>& keys, bool first) {
    for (auto& key : keys) {
        if (first) {
            if (mess.substr(0, key.size()) == key) {
                return true;
            }
        }
        else {
            if (mess.find(key) != -1) {
                return true;
            }
        }
    }
    return false;
}

void KaituanTools::KaituanProc(const GroupMessageData& data) {
    Tuan new_tuan;
    if (KaituanFormat(data, &new_tuan)) {
        if (IsManager(data)) {
            Tuan tuan;
            if (ChaKaituanData(new_tuan.team_name, &tuan)) {
                ReplyMessage(data, "团名重复", { new_tuan.team_name });
                return;
            }
            AddKaituanData(new_tuan);
            ReplyMessage(data, "开团成功", { new_tuan.team_name, new_tuan.fuben.fuben_name, new_tuan.start_time });
        }
        else {
            ReplyMessage(data, "非管理员", { to_string(data.SenderQQ), "开团" });
        }
    }
}

bool KaituanTools::KaituanFormat(const GroupMessageData& data, Tuan* tuan) {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 4) {
        if (FindBuben(strs[2], &(tuan->fuben))) {
            tuan->team_name  = strs[1];
            tuan->build_time = to_string(data.MessageReceiveTime);
            tuan->start_time = strs[3];
            tuan->status     = "报名中";
            return true;
        }
        else {
            ReplyMessage(data, "未找到副本", { strs[2] });
            return false;
        }
    }
    else {
        ReplyMessage(data, "开团格式错误");
        return false;
    }
}

void KaituanTools::CancelKaituanProc(const GroupMessageData& data) {
    string team_name;
    if (CancelKaituanFormat(data, &team_name)) {
        if (IsManager(data)) {
            Tuan tuan;
            if (!ChaKaituanData(team_name, &tuan)) {
                ReplyMessage(data, "查无此团", { team_name });
                return;
            }
            CancelKaituanData(tuan.tuan_id);
            ReplyMessage(data, "取消开团成功");
        }
        else {
            ReplyMessage(data, "非管理员", { to_string(data.SenderQQ), "取消开团" });
        }
    }
}

bool KaituanTools::CancelKaituanFormat(const GroupMessageData& data, std::string* team_name) {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 2) {
        *team_name = strs[1];
        return true;
    }
    else {
        ReplyMessage(data, "取消开团格式错误");
        return false;
    }
    return false;
}

void KaituanTools::BaominProc(const GroupMessageData& data) {
    Ren    new_ren;
    string team_name;
    if (BaominFormat(data, &new_ren, &team_name)) {
        Tuan tuan;
        if (team_name.empty()) {
            std::vector<Tuan> tuans;
            ChaKaituanData(&tuans);
            if (tuans.empty()) {
                ReplyMessage(data, "目前没有团");
                return;
            }
            else if (tuans.size() == 1) {
                tuan = tuans.front();
            }
            else {
                string reply;
                for (auto& t : tuans) {
                    reply += "\n“" + t.team_name + "” " + t.fuben.fuben_name + " " + t.start_time;
                }
                ReplyMessage(data, "需要指定车名", { to_string(tuans.size()), reply });
                return;
            }
        }
        else {
            if (!ChaKaituanData(team_name, &tuan)) {
                ReplyMessage(data, "查无此团", { team_name });
                return;
            }
        }
        if (int pos = new_ren.status.find("登记"); pos != -1 || BaominCheckCnfg(data, tuan, new_ren)) {
            for (auto& ren : tuan.rens) {
                if (ren.game_name == new_ren.game_name) {
                    ReplyMessage(data, "人名重复", { ren.game_name, ren.game_xinfa.xinfa_name });
                    return;
                }
            }
            AddBaomingData(tuan.tuan_id, new_ren);
            ReplyMessage(data, "报名成功", { new_ren.game_name, new_ren.game_xinfa.xinfa_name });
        }
    }
}

bool KaituanTools::BaominFormat(const GroupMessageData& data, Ren* ren, std::string* team_name) {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    string name = "一二三四五六";
    if (strs.size() == 4) {
        *team_name = strs[1];
        strs.erase(strs.begin() + 1);
    }
    if (strs.size() == 3) {
        if (FindXinfa(strs[1], &(ren->game_xinfa))) {
            ren->QQ = to_string(data.SenderQQ);
            if (strs[2].size() > name.size()) {
                ReplyMessage(data, "ID太长", { strs[2] });
                return false;
            }
            ren->game_name = strs[2];
            if (strs[0] == "登记报名")
                ren->status = "登记";
            if (strs[0] == "老板报名")
                ren->status = "老板";
            if (strs[0] == "老板登记报名")
                ren->status = "老板登记";
            return true;
        }
        else {
            ReplyMessage(data, "未找到心法", { strs[1] });
            return false;
        }
    }
    else {
        ReplyMessage(data, "报名格式错误");
        return false;
    }
    return false;
}

bool KaituanTools::BaominCheckCnfg(const GroupMessageData& data, const Tuan& tuan, const Ren& ren) {
    auto& cnfg    = tuan.fuben.fuben_config;
    auto& now     = tuan.now_cnfg;
    auto& me      = ren.game_xinfa;
    int   sum_ren = now.T + now.nai + now.dps + now.laoban;
    if (sum_ren >= tuan.fuben.fuben_size) {
        ReplyMessage(data, "车满");
        return false;
    }
    if (ren.status == "老板" && now.laoban >= cnfg.laoban) {
        ReplyMessage(data, "老板满");
        return false;
    }
    else {
        if (me.xinfa_type_1 == "T" && now.T >= cnfg.T) {
            ReplyMessage(data, "T满");
            return false;
        }
        if (me.xinfa_type_1 == "奶" && now.nai >= cnfg.nai) {
            ReplyMessage(data, "奶满");
            return false;
        }
        if (me.xinfa_type_1 == "dps" && now.dps >= cnfg.dps) {
            ReplyMessage(data, "dps满");
            return false;
        }
        if (me.xinfa_type_2 == "内功" && now.nei_dps >= cnfg.nei_dps) {
            ReplyMessage(data, "内满");
            return false;
        }
        if (me.xinfa_type_2 == "外功" && now.wai_dps >= cnfg.wai_dps) {
            ReplyMessage(data, "外满");
            return false;
        }
        if (cnfg.xinfas[me.xinfa_id] == 0) {
            ReplyMessage(data, "反思心法", { me.xinfa_name });
            return false;
        }
        if (now.xinfas[me.xinfa_id] >= cnfg.xinfas[me.xinfa_id]) {
            ReplyMessage(data, "该心法满", { me.xinfa_name });
            return false;
        }
    }
    return true;
}

void KaituanTools::CancelBaominProc(const GroupMessageData& data) {
    string team_name, game_id;
    string QQ = to_string(data.SenderQQ);
    if (CancelBaominFormat(data, &team_name, &game_id)) {
        Tuan tuan;
        if (team_name.empty()) {
            std::vector<Tuan> tuans;
            ChaKaituanData(&tuans);
            if (tuans.empty()) {
                ReplyMessage(data, "目前没有团");
                return;
            }
            else if (tuans.size() == 1) {
                tuan = tuans.front();
            }
            else {
                string reply;
                for (auto& t : tuans) {
                    reply += "\n“" + t.team_name + "” " + t.fuben.fuben_name + " " + t.start_time;
                }
                ReplyMessage(data, "需要指定车名", { to_string(tuans.size()), reply });
                return;
            }
        }
        else {
            if (!ChaKaituanData(team_name, &tuan)) {
                ReplyMessage(data, "查无此团", { team_name });
                return;
            }
        }
        if (game_id.empty()) {
            vector<Ren> qqRen;
            for (auto& ren : tuan.rens) {
                if (ren.QQ == QQ) {
                    qqRen.emplace_back(ren);
                }
            }
            if (qqRen.size() == 1) {
                game_id = qqRen.front().game_name;
            }
            else {
                string reply;
                for (auto& ren : qqRen) {
                    reply += ren.game_name + " " + ren.game_xinfa.xinfa_name;
                }
                ReplyMessage(data, "需要指定ID", { to_string(qqRen.size()), reply });
                return;
            }
        }
        for (auto& ren : tuan.rens) {
            if (ren.game_name == game_id) {
                if (ren.QQ == QQ || IsManager(data)) {
                    DeleteBaomingData(tuan.tuan_id, game_id);
                    ReplyMessage(data, "取消报名成功", { game_id, ren.game_xinfa.xinfa_name });
                    return;
                }
                else {
                    ReplyMessage(data, "不能取消他人报名", { game_id, ren.QQ });
                    return;
                }
            }
        }
        ReplyMessage(data, "未报名", { game_id });
        return;
    }
}

bool KaituanTools::CancelBaominFormat(const GroupMessageData& data, std::string* tuan_name, std::string* game_id) {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 3) {
        *tuan_name = strs[1];
        *game_id   = strs[2];
        return true;
    }
    else if (strs.size() == 2) {
        *game_id = strs[1];
        return true;
    }
    else if (strs.size() == 1) {
        return true;
    }
    else {
        ReplyMessage(data, "取消报名格式错误");
        return false;
    }
    return false;
}

void KaituanTools::ChaTuanProc(const GroupMessageData& data) {
    if (time(0) - chatuan_cd < 10) {
        ReplyMessage(data, "CD中");
        return;
    }
    chatuan_cd = time(0);
    std::vector<Tuan> tuans;
    ChaKaituanData(&tuans);
    if (tuans.empty()) {
        ReplyMessage(data, "目前没有团");
        return;
    }
    vector<string> img_codes;
    ReplyMessage(data, "查团", { to_string(tuans.size()) });
    for (auto& tuan : tuans) {
        std::vector<uint8_t> img_s;
        BuildXiaoduiImg(tuan, &img_s);
        auto img_code = api->UploadGroupImage(data.ThisQQ, data.MessageGroupQQ, img_s.data(), img_s.size(), false);
        img_codes.emplace_back(img_code);
    }
    for (auto& img_code : img_codes) {
        ReplyMessage(data, "查团图片", { img_code });
    }
}

void initFreetype(FT_Library& ftlib, FT_Face& face, const std::string& fontFullName) {
    FT_Error fterr;
    FT_Init_FreeType(&ftlib);
    FT_New_Face(ftlib, fontFullName.c_str(), 0, &face);
}
void closeFreetype(FT_Library& ftlib, FT_Face& face) {
    FT_Done_Face(face);
    FT_Done_FreeType(ftlib);
}
void drawGlyph(FT_GlyphSlot& glyphSlot, CImg<unsigned char>& image, const int& shiftX, const int& shiftY, unsigned char fontColor[] = NULL) {
    unsigned char buff[] = { 255, 255, 255 };
    if (fontColor == NULL) {
        fontColor = buff;
    }
    float alpha = 0;
    for (int y = 0; y < glyphSlot->bitmap.rows; ++y) {
        for (int x = 0; x < glyphSlot->bitmap.width; ++x) {
            unsigned char glyphValue = glyphSlot->bitmap.buffer[y * glyphSlot->bitmap.width + x];
            alpha                    = (255.0f - glyphValue) / 255.0f;
            cimg_forC(image, c) {
                unsigned char value              = ( float )glyphValue * fontColor[c] / (255.0f);
                image(x + shiftX, y + shiftY, c) = alpha * image(x + shiftX, y + shiftY, c) + (1.0 - alpha) * value;
            }
        }
    }
}
void drawText(FT_Face& face, CImg<unsigned char>& image, const int& heightText, const std::wstring& text, const int& leftTopX, const int& leftTopY,
              unsigned char fontColor[] = NULL, const int separeteGlyphWidth = 1) {
    FT_Set_Pixel_Sizes(face, 0, heightText);
    FT_GlyphSlot glyphSlot = face->glyph;
    int          shiftX    = leftTopX;
    int          shiftY    = 0;
    for (int numberSymbol = 0; numberSymbol < text.length(); ++numberSymbol) {
        shiftY           = leftTopY;
        bool     isSpace = false;
        FT_ULong symbol  = text.at(numberSymbol);
        if (symbol == ' ') {
            symbol  = 'a';
            isSpace = true;
        }
        FT_Load_Char(face, symbol, FT_LOAD_RENDER);
        float shiftFactor = static_cast<int>(glyphSlot->bitmap.rows) - glyphSlot->bitmap_top;
        shiftY += shiftFactor;
        shiftY += (heightText > glyphSlot->bitmap.rows) ? heightText - glyphSlot->bitmap.rows : 0;
        if (!isSpace) {
            drawGlyph(glyphSlot, image, shiftX, shiftY, fontColor);
        }
        shiftX += glyphSlot->bitmap.width + separeteGlyphWidth;
    }
}

bool KaituanTools::BuildXiaoduiImg(const Tuan& tuan, std::vector<uint8_t>* img_s) {
    CImg<unsigned char> img(640, 480, 1, 3);
    vector<vector<Ren>> xiaodui;
    BuildXiaoduiTable(tuan, &xiaodui);
    int        posx = 10, posy = 70;
    FT_Library ftlib;
    FT_Face    face;
    if (xiaodui[5].empty()) {
        img = CImg<unsigned char>(640, 400, 1, 3);
    }
    initFreetype(ftlib, face, "simfang.ttf");
    unsigned char black[] = { 0, 0, 0 };
    wstring       title   = UTF8ToWideChar(tuan.team_name + " " + tuan.fuben.fuben_name + " " + tuan.start_time);
    drawText(face, img, 30, title, 20, 10, black);
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            unsigned char colorB[] = { 255, 255, 255 };
            unsigned char colorF[] = { 0, 0, 0 };
            if (j < xiaodui[i].size()) {
                auto& rgb = xiaodui[i][j].game_xinfa;
                colorB[0] = rgb.r;
                colorB[1] = rgb.g;
                colorB[2] = rgb.b;
                img.draw_rectangle(posx, posy, posx + 120, posy + 50, colorB, 0.5);
            }
            else {
                img.draw_rectangle(posx, posy, posx + 120, posy + 50, colorB);
            }
            if (j < xiaodui[i].size()) {
                auto&   ren     = xiaodui[i][j];
                wstring xf_name = UTF8ToWideChar(ren.game_xinfa.xinfa_name + ren.status);
                wstring id_name = UTF8ToWideChar(ren.game_name);
                drawText(face, img, 15, xf_name, posx + 20, posy + 5, colorF);
                drawText(face, img, 20, id_name, posx + 10, posy + 25, colorF);
            }
            posy += 52;
        }
        posx += 122;
        posy = 70;
    }
    if (!xiaodui[5].empty()) {
        drawText(face, img, 15, L"登记：", 10, 340, black);
        posx = 60;
        posy = 340;
        for (auto& ren : xiaodui[5]) {
            wstring xf_name = UTF8ToWideChar(ren.game_xinfa.xinfa_name);
            wstring id_name = UTF8ToWideChar(ren.game_name);
            drawText(face, img, 15, xf_name, posx, posy, black);
            drawText(face, img, 15, id_name, posx + 45, posy, black);
            posx += 100;
            if (posx >= 550)
                posy += 25;
        }
    }
    img.save_bmp("img.bmp");
    ifstream file("img.bmp", ios::binary);
    int      img_size = img.size() + 54;
    img_s->resize(img_size);
    file.read(( char* )(img_s->data()), img_size);
    file.close();
    return true;
}

bool KaituanTools::BuildXiaoduiTable(const Tuan& tuan, std::vector<std::vector<Ren>>* xiaodui) {
    xiaodui->resize(6);
    auto tryadd = [xiaodui](vector<int> d, const Ren& ren) {
        for (int i : d) {
            if ((*xiaodui)[i].size() < 5) {
                (*xiaodui)[i].emplace_back(ren);
                return;
            }
        }
    };
    for (auto& ren : tuan.rens) {
        if (int pos = ren.status.find("登记"); pos != -1) {
            (*xiaodui)[5].push_back(ren);
        }
        else if (ren.status == "老板") {
            tryadd({ 3, 1, 2, 0, 4 }, ren);
        }
        else if (ren.game_xinfa.xinfa_type_1 == "T") {
            tryadd({ 4, 3, 1, 2, 0 }, ren);
        }
        else if (ren.game_xinfa.xinfa_type_1 == "奶") {
            tryadd({ 4, 1, 0, 3, 2 }, ren);
        }
        else if (ren.game_xinfa.xinfa_type_2 == "内功") {
            tryadd({ 0, 1, 3, 2, 4 }, ren);
        }
        else if (ren.game_xinfa.xinfa_type_2 == "外功") {
            tryadd({ 2, 3, 1, 0, 4 }, ren);
        }
    }
    return true;
}

bool KaituanTools::AddXinfaData(const Xinfa& xinfa) {
    string sql_str = "insert into xinfa_table select '";
    sql_str += to_string(xinfa.xinfa_id) + "','" + xinfa.xinfa_name + "','";
    for (auto& xf_nk : xinfa.xinfa_nicknames) {
        sql_str += xf_nk + " ";
    }
    sql_str += "','";
    sql_str += xinfa.xinfa_type_1 + "','";
    sql_str += xinfa.xinfa_type_2 + "','";
    sql_str += xinfa.xinfa_type_3 + "','";
    sql_str += to_string(xinfa.zhenyan_score) + "','";
    sql_str += to_string(xinfa.r) + " ";
    sql_str += to_string(xinfa.g) + " ";
    sql_str += to_string(xinfa.b);
    sql_str += "'where not exists(select xinfa_id from xinfa_table where xinfa_id =";
    sql_str += to_string(xinfa.xinfa_id) + ");";

    char* cErrMsg;
    int   res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("添加心法信息错误:" + xinfa.xinfa_name + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::ChaXinfaData(std::vector<Xinfa>* xinfas) {
    string sql_str = "select * from xinfa_table;";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        db, sql_str.c_str(),
        [](auto p, auto cnt, auto val, auto name) {
            auto  xfs = static_cast<vector<Xinfa>*>(p);
            Xinfa xf;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "xinfa_id")
                    xf.xinfa_id = static_cast<XinfaID>(stoi(val[i]));
                else if (s == "xinfa_name")
                    xf.xinfa_name = val[i];
                else if (s == "xinfa_nicknames")
                    xf.xinfa_nicknames = Mtools::Split(val[i], " ");
                else if (s == "xinfa_type_1")
                    xf.xinfa_type_1 = val[i];
                else if (s == "xinfa_type_2")
                    xf.xinfa_type_2 = val[i];
                else if (s == "xinfa_type_3")
                    xf.xinfa_type_3 = val[i];
                else if (s == "zhenyan_score")
                    xf.zhenyan_score = stoi(val[i]);
                else if (s == "rgb") {
                    auto rgb = Mtools::Split(val[i], " ");
                    xf.r     = stoi(rgb[0]);
                    xf.g     = stoi(rgb[1]);
                    xf.b     = stoi(rgb[2]);
                }
            }
            xfs->emplace_back(xf);
            return 0;
        },
        xinfas, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找心法信息错误:" + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::AddFubenData(const Fuben& fuben) {
    string sql_str = "insert into fuben_table select '";
    sql_str += fuben.fuben_name + "','";
    auto& cnfg = fuben.fuben_config;
    sql_str += "T " + to_string(cnfg.T) + ";";
    sql_str += "奶 " + to_string(cnfg.nai) + ";";
    sql_str += "dps " + to_string(cnfg.dps) + ";";
    sql_str += "内功 " + to_string(cnfg.nei_dps) + ";";
    sql_str += "外功 " + to_string(cnfg.wai_dps) + ";";
    sql_str += "老板 " + to_string(cnfg.laoban) + ";";
    for (int i = 0; i < cnfg.xinfas.size(); ++i) {
        sql_str += xinfa_list_[i].xinfa_name + " " + to_string(cnfg.xinfas[i]) + ";";
    }
    sql_str += "','";
    for (auto& nk : fuben.fuben_nickname) {
        sql_str += nk + " ";
    }
    sql_str += +"','" + to_string(fuben.fuben_size);
    sql_str += "'where not exists(select fuben_name from fuben_table where fuben_name = '" + fuben.fuben_name + "');";

    char* cErrMsg;
    int   res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("添加副本信息错误:" + fuben.fuben_name + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::ChaFubenData(std::vector<Fuben>* fubens) {
    string sql_str = "select * from fuben_table;";
    struct Fubens_db {
        vector<Fuben>* fubens;
        vector<string> cnfgs;
    } fubens_db{ fubens };
    char* cErrMsg;
    int   res = sqlite3_exec(
        db, sql_str.c_str(),
        [](auto p, auto cnt, auto val, auto name) {
            auto   fbs = static_cast<Fubens_db*>(p);
            Fuben  fb;
            string cnfg_str;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "fuben_name")
                    fb.fuben_name = val[i];
                else if (s == "fuben_config")
                    cnfg_str = val[i];
                else if (s == "fuben_nickname")
                    fb.fuben_nickname = Mtools::Split(val[i], " ");
                else if (s == "fuben_size")
                    fb.fuben_size = stoi(val[i]);
            }
            fbs->fubens->emplace_back(fb);
            fbs->cnfgs.emplace_back(cnfg_str);
            return 0;
        },
        &fubens_db, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找副本信息错误:" + string(cErrMsg));
        return false;
    }
    for (int i = 0; i < fubens->size(); ++i) {
        auto& cnfg      = (*fubens)[i].fuben_config;
        auto  cnfg_strs = Mtools::Split(fubens_db.cnfgs[i], ";");
        int   xinfa_id  = 0;
        for (auto& cnfg_str : cnfg_strs) {
            auto cs = Mtools::Split(cnfg_str, " ");
            if (cs[0] == "T") {
                cnfg.T = stoi(cs[1]);
            }
            else if (cs[0] == "奶")
                cnfg.nai = stoi(cs[1]);
            else if (cs[0] == "dps")
                cnfg.dps = stoi(cs[1]);
            else if (cs[0] == "内功")
                cnfg.nei_dps = stoi(cs[1]);
            else if (cs[0] == "外功")
                cnfg.wai_dps = stoi(cs[1]);
            else if (cs[0] == "老板")
                cnfg.laoban = stoi(cs[1]);
            else
                for (auto& xf : xinfa_list_) {
                    if (cs[0] == xf.xinfa_name) {
                        cnfg.xinfas[xf.xinfa_id] = stoi(cs[1]);
                    }
                }
        }
    }
    return true;
}

bool KaituanTools::AddReplyData(const std::string& key, const std::string& content, const std::string& p) {
    string sql_str = "insert into huifu_table select '" + key + "','" + content + +"','" + p + "';";

    char* cErrMsg;
    int   res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("添加回复信息错误:" + key + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::ChaReplyData(const std::string& key, std::vector<Reply>* contents) {
    string sql_str = "select * from huifu_table where huifu_key = '" + key + "';";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        db, sql_str.c_str(),
        [](auto p, auto cnt, auto val, auto name) {
            auto  strs = static_cast<vector<Reply>*>(p);
            Reply re;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "huifu_values")
                    re.mess = val[i];
                else if (s == "Probability")
                    re.Probability = stoi(val[i]);
            }
            strs->push_back(re);
            return 0;
        },
        contents, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找回复信息错误:" + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::AddKaituanData(const Tuan& tuan) {
    string sql_str = "insert into team_table(team_name,fuben_name,start_time,build_time,status) values ('";
    sql_str += tuan.team_name + "','";
    sql_str += tuan.fuben.fuben_name + "','";
    sql_str += tuan.start_time + "','";
    sql_str += tuan.build_time + "','";
    sql_str += tuan.status + "');";

    char* cErrMsg;
    int   res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("添加开团信息错误:" + tuan.team_name + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::CancelKaituanData(int team_id) {
    // string sql_str = " delete from team_table where team_name = '" + team_name + "';";
    string sql_str = "update team_table set status = '已取消' where tuan_id = " + to_string(team_id) + ";";
    char*  cErrMsg;
    int    res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("删除开团信息错误:" + team_id + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::ChaKaituanData(std::vector<Tuan>* tuans) {
    string sql_str = "select * from team_table where status = '报名中';";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        db, sql_str.c_str(),
        [](auto p, auto cnt, auto val, auto name) {
            auto tuans = static_cast<vector<Tuan>*>(p);
            Tuan tuan;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "tuan_id")
                    tuan.tuan_id = stoi(val[i]);
                else if (s == "team_name")
                    tuan.team_name = val[i];
                else if (s == "fuben_name")
                    tuan.fuben.fuben_name = val[i];
                else if (s == "start_time")
                    tuan.start_time = val[i];
                else if (s == "build_time")
                    tuan.build_time = val[i];
                else if (s == "status")
                    tuan.status = val[i];
            }
            tuans->push_back(tuan);
            return 0;
        },
        tuans, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找开团信息错误:" + string(cErrMsg));
        return false;
    }
    for (auto& tuan : *tuans) {
        FindBuben(tuan.fuben.fuben_name, &(tuan.fuben));
        ChaBaomingData(tuan.tuan_id, &tuan.rens);
        UpdateKaituanNowcnfg(&tuan);
    }
    return true;
}

bool KaituanTools::ChaKaituanData(const std::string& team_name, Tuan* tuan) {
    string       sql_str = "select * from team_table where status = '报名中' and team_name ='" + team_name + "';";
    vector<Tuan> tuans;
    char*        cErrMsg;
    int          res = sqlite3_exec(
        db, sql_str.c_str(),
        [](auto p, auto cnt, auto val, auto name) {
            auto tuans = static_cast<vector<Tuan>*>(p);
            Tuan tuan;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "tuan_id")
                    tuan.tuan_id = stoi(val[i]);
                if (s == "team_name")
                    tuan.team_name = val[i];
                else if (s == "fuben_name")
                    tuan.fuben.fuben_name = val[i];
                else if (s == "start_time")
                    tuan.start_time = val[i];
                else if (s == "build_time")
                    tuan.build_time = val[i];
                else if (s == "status")
                    tuan.status = val[i];
            }
            tuans->push_back(tuan);
            return 0;
        },
        &tuans, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找开团信息错误:" + string(cErrMsg));
        return false;
    }
    if (tuans.empty())
        return false;
    *tuan = tuans.front();
    FindBuben(tuan->fuben.fuben_name, &(tuan->fuben));
    ChaBaomingData(tuan->tuan_id, &tuan->rens);
    UpdateKaituanNowcnfg(tuan);
    return true;
}

bool KaituanTools::UpdateKaituanNowcnfg(Tuan* tuan) {
    for (auto& ren : tuan->rens) {
        if (int pos = ren.status.find("登记"); pos == -1) {
            if (ren.game_xinfa.xinfa_type_1 == "T")
                tuan->now_cnfg.T++;
            else if (ren.game_xinfa.xinfa_type_1 == "奶")
                tuan->now_cnfg.nai++;
            else if (ren.game_xinfa.xinfa_type_1 == "dps")
                tuan->now_cnfg.dps++;
            else if (ren.game_xinfa.xinfa_type_2 == "内功")
                tuan->now_cnfg.nei_dps++;
            else if (ren.game_xinfa.xinfa_type_2 == "外功")
                tuan->now_cnfg.wai_dps++;
            else if (ren.status == "老板")
                tuan->now_cnfg.laoban++;
            tuan->now_cnfg.xinfas[ren.game_xinfa.xinfa_id]++;
        }
    }
    return true;
}

bool KaituanTools::AddBaomingData(int tuan_id, const Ren& ren) {
    string sql_str = "insert into sign_up_table(tuan_id,QQ,game_name,xinfa_id,status) values (";
    sql_str += to_string(tuan_id) + ",'";
    sql_str += ren.QQ + "','";
    sql_str += ren.game_name + "',";
    sql_str += to_string(ren.game_xinfa.xinfa_id) + ",'";
    sql_str += ren.status + "');";
    char* cErrMsg;
    int   res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("添加报名信息错误:" + ren.game_name + string(cErrMsg));
        return false;
    }
    return true;
}

bool KaituanTools::ChaBaomingData(int tuan_id, std::vector<Ren>* rens) {
    string sql_str = "select * from sign_up_table where tuan_id = " + to_string(tuan_id) + ";";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        db, sql_str.c_str(),
        [](auto p, auto cnt, auto val, auto name) {
            auto rens = static_cast<vector<Ren>*>(p);
            Ren  ren;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "QQ")
                    ren.QQ = val[i];
                else if (s == "game_name")
                    ren.game_name = val[i];
                else if (s == "xinfa_id")
                    ren.game_xinfa.xinfa_id = static_cast<XinfaID>(stoi(val[i]));
                else if (s == "status")
                    ren.status = val[i];
            }
            rens->push_back(ren);
            return 0;
        },
        rens, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找报名信息错误:" + string(cErrMsg));
        return false;
    }
    for (auto& ren : *rens) {
        ren.game_xinfa = xinfa_list_[ren.game_xinfa.xinfa_id];
    }
    return true;
}

bool KaituanTools::DeleteBaomingData(int team_id, const std::string& game_id) {

    string sql_str = " delete from sign_up_table where tuan_id = '" + to_string(team_id) + "' and game_name = '" + game_id + "';";
    char*  cErrMsg;
    int    res = sqlite3_exec(db, sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("删除报名信息错误:" + game_id + string(cErrMsg));
        return false;
    }
    return true;
}
