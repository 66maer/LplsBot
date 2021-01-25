#pragma once
#include "stdafx.h";
#include "sdk/sdk.h"
#include "Types.h"

struct Tuan;
struct Ren;
struct Xinfa;
struct TeamConfig;
struct Fuben;
struct Reply;

class KaituanTools {
public:
    KaituanTools();

    bool Run(const GroupMessageData& data);

private:
    bool InitDataBase();

    bool InitDefineData();

    bool InitDefineReply();

    bool LoadXinfaData();

    bool LoadFubenData();

    bool IsManager(const GroupMessageData& data);

    void ReplyMessage(const GroupMessageData& data, const std::string& key, std::initializer_list<std::string> args);

    bool FindBuben(const std::string& fb_str, Fuben* fuben);

    bool FindXinfa(const std::string& xf_str, Xinfa* xinfa);

    bool Trigger(const std::string& mess, const std::vector<std::string>& keys, bool first = true);

    void KaituanProc(const GroupMessageData& data);

    bool KaituanFormat(const GroupMessageData& data, Tuan* tuan);

    void CancelKaituanProc(const GroupMessageData& data);

    bool CancelKaituanFormat(const GroupMessageData& data, std::string* team_name);

    void BaominProc(const GroupMessageData& data);

    bool BaominFormat(const GroupMessageData& data, Ren* ren, std::string* team_name);

    bool BaominCheckCnfg(const GroupMessageData& data, const Tuan& tuan, const Ren& ren);

    void CancelBaominProc(const GroupMessageData& data);

    bool CancelBaominFormat(const GroupMessageData& data, std::string* tuan_name, std::string* game_id);

    void ChaTuanProc(const GroupMessageData& data);

    bool BuildXiaoduiImg(const Tuan& tuan, std::vector<uint8_t>* img_s);

    bool BuildXiaoduiTable(const Tuan& tuan, std::vector<std::vector<Ren>>* xiaodui);

    bool AddXinfaData(const Xinfa& xinfa);

    bool ChaXinfaData(std::vector<Xinfa>* xinfas);

    bool AddFubenData(const Fuben& fuben);

    bool ChaFubenData(std::vector<Fuben>* fubens);

    bool AddReplyData(const std::string& key, const std::string& content, const std::string& p = "100");

    bool ChaReplyData(const std::string& key, std::vector<Reply>* contents);

    bool AddKaituanData(const Tuan& tuan);

    bool CancelKaituanData(int team_id);

    bool ChaKaituanData(std::vector<Tuan>* tuans);

    bool ChaKaituanData(const std::string& team_name, Tuan* tuan);

    bool UpdateKaituanNowcnfg(Tuan* tuan);

    bool AddBaomingData(int tuan_id, const Ren& ren);

    bool ChaBaomingData(int tuan_id, std::vector<Ren>* rens);

    bool DeleteBaomingData(int team_id, const std::string& game_id);

    std::vector<Xinfa> xinfa_list_;
    std::vector<Fuben> fuben_list_;

    std::vector<std::string> group_QQs_;
    std::vector<std::string> Manager_QQs_;

    time_t chatuan_cd;
};

struct TeamConfig {
    TeamConfig() {
        T = nai = dps = nei_dps = wai_dps = laoban = 0;
        xinfas.resize(Xinfa_All);
    }
    TeamConfig(int t, int n, int d, int nei, int wai, int b, std::vector<int> vec) : T(t), nai(n), dps(d), nei_dps(nei), wai_dps(wai), laoban(b), xinfas(vec) {}
    int              T;
    int              nai;
    int              dps;
    int              nei_dps;
    int              wai_dps;
    int              laoban;
    std::vector<int> xinfas;
};

struct Fuben {
    Fuben() {
        fuben_size = 25;
    };
    Fuben(const std::string& fb_name, const TeamConfig& fb_config, const std::vector<std::string>& fb_nkname, int fb_size)
        : fuben_name(fb_name), fuben_config(fb_config), fuben_nickname(fb_nkname), fuben_size(fb_size) {}
    std::string              fuben_name;
    TeamConfig               fuben_config;
    std::vector<std::string> fuben_nickname;
    int                      fuben_size;
};

struct Xinfa {
    Xinfa() {
        xinfa_id      = Xinfa_DaXia;
        zhenyan_score = 0;
        r = g = b = 0;
    }
    Xinfa(XinfaID xf_id, const std::string& xf_name, const std::vector<std::string>& xf_nknames, const std::string& xf_t1, const std::string& xf_t2, const std::string& xf_t3,
          int zy_score, int R, int G, int B)
        : xinfa_id(xf_id), xinfa_name(xf_name), xinfa_nicknames(xf_nknames), xinfa_type_1(xf_t1), xinfa_type_2(xf_t2), xinfa_type_3(xf_t3), zhenyan_score(zy_score), r(R), g(G),
          b(B) {}
    XinfaID                  xinfa_id;
    std::string              xinfa_name;
    std::vector<std::string> xinfa_nicknames;
    std::string              xinfa_type_1;
    std::string              xinfa_type_2;
    std::string              xinfa_type_3;
    int                      zhenyan_score;
    int                      r;
    int                      g;
    int                      b;
};

struct Ren {
    std::string QQ;
    std::string game_name;
    Xinfa       game_xinfa;
    std::string status;
};

struct Tuan {
    int              tuan_id;
    std::string      team_name;
    Fuben            fuben;
    std::string      start_time;
    std::string      build_time;
    std::string      status;
    TeamConfig       now_cnfg;
    std::vector<Ren> rens;
};

struct Reply {
    int         Probability;
    std::string mess;
};