#include "TeamModule.h"
#include <regex>

using namespace std;
using namespace cimg_library;

TeamModule::TeamModule(sqlite3* base_db, sqlite3* db) : ModuleBase(base_db, db, true, 50) {
    if (!Init()) {
        set_enable(false);
    }
}

bool TeamModule::ProcGroupMess(const GroupMessageData& data) {
    string mess = data.MessageContent;
    if (Trigger(mess, { "开团" })) {
        CreateTeamProc(data);
    }
    else if (Trigger(mess, { "报名", "登记报名", "老板报名", "老板登记报名" })) {
        SignUpProc(data);
    }
    else if (Trigger(mess, { "查团" }) || Trigger(mess, { "有团吗", "有团咩", "有团么", "有本吗", "有本咩", "有本么", "打本吗", "开团吗" }, false)) {
        SelectTeamProc(data);
    }
    else if (Trigger(mess, { "开组" })) {
        OpenTeamProc(data);
    }
    else if (Trigger(mess, { "取消开团" })) {
        CancelTeamProc(data);
    }
    else if (Trigger(mess, { "完成开团", "结束开团" })) {
        FinishTeamProc(data);
    }
    else if (Trigger(mess, { "取消报名" })) {
        CancelSignUpProc(data);
    }
    else if (Trigger(mess, { "咕咕咕" })) {
        GuGuGuSignUpProc(data);
    }
    else if (Trigger(mess, { "查鸽子值" })) {
        SelectGuGuGuProc(data);
    }
    else {
        return false;
    }
    return true;
}

bool TeamModule::Init() {
    // 这里应该要初始化数据库之类的
    ifstream file("teamModuleDB.sql");
    if (file.fail()) {
        cerr << "找不到文件:\""
             << "teamModuleDB.sql"
             << "\"" << endl;
        exit(-1);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    auto sql = buffer.str();
    file.close();
    DB_Exec(sql);
    select_team_cd_ = 0;
    DB_SelectXinfa();
    DB_SelectZone();
    return true;
}

bool TeamModule::IsManager(const GroupMessageData& data) const {
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

void TeamModule::ReplyMess(const GroupMessageData& data, const std::string& key, std::initializer_list<std::string> args = {}) const {
    vector<ReMess> replys;
    if (DB_SelectReplyMessage(key, &replys) && !replys.empty()) {
        ReMess reply;
        int    sum = 0;
        for (auto& r : replys) {
            sum += r.percent;
        }
        int p = rand() % sum;
        sum   = 0;
        for (auto& r : replys) {
            sum += r.percent;
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
        string s = "\\n";
        for (int pos = 0; pos != -1;)
            if (pos = reply.mess.find(s, pos); pos != -1) {
                reply.mess = reply.mess.replace(pos, s.size(), "\n");
            }
        api->SendGroupMessage(data.ThisQQ, data.MessageGroupQQ, reply.mess);
    }
    else {
        api->OutputLog("没有对应的回复内容：" + key);
    }
    return;
}

bool TeamModule::Trigger(const std::string& mess, const std::vector<std::string>& keys, bool first) const {
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

void TeamModule::CreateTeamProc(const GroupMessageData& data) const {
    Team new_team;
    if (CheckCreatTeamFormat(data, &new_team)) {
        if (IsManager(data)) {
            Team team;
            if (DB_SelectTeam(new_team.team_name, &team)) {
                ReplyMess(data, "团名重复", { new_team.team_name });
                return;
            }
            DB_InsertTeam(new_team);
            ReplyMess(data, "开团成功", { new_team.team_name, new_team.zone.zone_name, new_team.remark });
        }
        else {
            ReplyMess(data, "非管理员", { to_string(data.SenderQQ), "开团" });
        }
    }
}

bool TeamModule::CheckCreatTeamFormat(const GroupMessageData& data, Team* team) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 4) {
        SelectZone(strs[2], &(team->zone));
        if (team->zone.zone_size > 0) {
            team->team_name  = strs[1];
            team->build_time = data.MessageReceiveTime;
            team->remark     = strs[3];
            team->status     = TeamStatus::SigningUp;
            return true;
        }
        else {
            ReplyMess(data, "未找到副本", { strs[2] });
            return false;
        }
    }
    else {
        ReplyMess(data, "开团格式错误");
        return false;
    }
}

void TeamModule::OpenTeamProc(const GroupMessageData& data) const {
    string team_name;
    if (CheckOpenTeamFormat(data, &team_name)) {
        if (IsManager(data)) {
            Team team;
            if (!DB_SelectTeam(team_name, &team)) {
                ReplyMess(data, "查无此团", { team_name });
                return;
            }
            if (team.status == TeamStatus::SigningUp) {
                team.status       = TeamStatus::Processing;
                team.process_time = data.MessageReceiveTime;
                DB_UpdateTeam(team.team_id, team);
                ReplyMess(data, "通用字符串", { "开组成功！" });
            }
            else {
                ReplyMess(data, "通用字符串", { "正在开组中！" });
            }
        }
        else {
            ReplyMess(data, "非管理员", { to_string(data.SenderQQ), "开组" });
        }
    }
}

bool TeamModule::CheckOpenTeamFormat(const GroupMessageData& data, std::string* team_name) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 2) {
        *team_name = strs[1];
        return true;
    }
    else {
        ReplyMess(data, "开组格式错误");
        return false;
    }
    return false;
}

void TeamModule::CancelTeamProc(const GroupMessageData& data) const {
    string team_name;
    if (CheckCancelTeamFormat(data, &team_name)) {
        if (IsManager(data)) {
            Team team;
            if (!DB_SelectTeam(team_name, &team)) {
                ReplyMess(data, "查无此团", { team_name });
                return;
            }
            team.status = TeamStatus::Invalid;
            DB_UpdateTeam(team.team_id, team);
            ReplyMess(data, "通用字符串", { "取消开团成功！" });
        }
        else {
            ReplyMess(data, "非管理员", { to_string(data.SenderQQ), "取消开团" });
        }
    }
}

bool TeamModule::CheckCancelTeamFormat(const GroupMessageData& data, std::string* team_name) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 2) {
        *team_name = strs[1];
        return true;
    }
    else {
        ReplyMess(data, "取消开团格式错误");
        return false;
    }
    return false;
}

void TeamModule::FinishTeamProc(const GroupMessageData& data) const {
    Team finish_team;
    if (CheckFinishTeamFormat(data, &finish_team)) {
        if (IsManager(data)) {
            Team team;
            if (!DB_SelectTeam(finish_team.team_name, &team)) {
                ReplyMess(data, "查无此团", { finish_team.team_name });
                return;
            }
            string debut_name;
            team.debutQQ     = finish_team.debutQQ;
            team.glod        = finish_team.glod;
            team.goods       = finish_team.goods;
            team.finish_time = data.MessageSendTime;
            team.status      = TeamStatus::Finish;
            if (DB_UpdateTeam(team.team_id, team)) {
                for (auto& work : team.worker_list) {
                    if (work.ownerQQ == team.debutQQ) {
                        debut_name = work.game_id;
                        break;
                    }
                }
                if (!team.debutQQ.empty() && debut_name.empty()) {
                    debut_name = api->GetGroupNickname(data.ThisQQ, data.MessageGroupQQ, stoll(team.debutQQ));
                    if (debut_name.empty()) {
                        debut_name = api->GetNameForce(data.ThisQQ, stoll(team.debutQQ));
                    }
                }
                ReplyMess(data, "结束开团成功", { team.team_name, team.zone.zone_name, debut_name, to_string(team.glod), team.goods.empty() ? "无" : team.goods });
            }
        }
        else {
            ReplyMess(data, "非管理员", { to_string(data.SenderQQ), "结束开团" });
        }
    }
}

bool TeamModule::CheckFinishTeamFormat(const GroupMessageData& data, Team* team) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 2) {
        team->team_name = strs[1];
        return true;
    }
    else if (strs.size() >= 3) {
        team->team_name = strs[1];
        if (regex_match(strs[2], regex("\\[@[0-9]{5,14}\\]"))) {
            strs[2].pop_back();
            team->debutQQ = strs[2].substr(2);
        }
        else {
            ReplyMess(data, "结束开团格式错误");
            return false;
        }
        if (strs.size() >= 4) {
            if (regex_match(UTF8ToWideChar(strs[3]), wregex(L"[0-9]{1,10}砖?z?Z?[0-9]?"))) {
                int  z     = regex_search(UTF8ToWideChar(strs[3]), wregex(L"砖|z|Z")) ? 10000 : 1;
                auto glods = Mtools::Split(strs[3], "砖", true);
                if (glods.size() == 1) {
                    glods = Mtools::Split(strs[3], "zZ");
                }
                int glod = stoi(glods[0]) * z;
                if (glods.size() > 1) {
                    glod += stoi(glods[1]);
                }
                team->glod = glod;
            }
            else {
                ReplyMess(data, "结束开团格式错误");
                return false;
            }
        }
        if (strs.size() >= 5) {
            string s;
            for (int i = 4; i < strs.size(); ++i) {
                s += strs[i] + " ";
            }
            team->goods = s;
        }
    }
    else {
        ReplyMess(data, "结束开团格式错误");
        return false;
    }
    return true;
}

void TeamModule::SelectTeamProc(const GroupMessageData& data) {
    if (time(0) - select_team_cd_ < 10) {
        ReplyMess(data, "CD中");
        return;
    }
    select_team_cd_ = time(0);
    std::vector<Team> teams;
    DB_SelectTeam(&teams);
    if (teams.empty()) {
        ReplyMess(data, "目前没有团");
        return;
    }
    vector<string> img_codes;
    ReplyMess(data, "查团", { to_string(teams.size()) });
    for (auto& team : teams) {
        std::vector<uint8_t> img_s;
        if (BuildTeamImg(team, &img_s)) {
            auto img_code = api->UploadGroupImage(data.ThisQQ, data.MessageGroupQQ, img_s.data(), img_s.size(), false);
            img_codes.emplace_back(img_code);
        }
    }
    for (auto& img_code : img_codes) {
        ReplyMess(data, "查团图片", { img_code });
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

bool TeamModule::BuildTeamImg(const Team& team, std::vector<uint8_t>* img_s) {
    CImg<unsigned char>    img(640, 480, 1, 3);
    vector<vector<Worker>> group;
    BuildTeamTable(team, &group);
    int        posx = 10, posy = 70;
    FT_Library ftlib;
    FT_Face    face;
    if (group[5].empty()) {
        img = CImg<unsigned char>(640, 400, 1, 3);
    }
    initFreetype(ftlib, face, "simfang.ttf");
    unsigned char black[] = { 0, 0, 0 };
    wstring       title   = UTF8ToWideChar(team.team_name + " " + team.zone.zone_name + " " + team.remark);
    drawText(face, img, 30, title, 20, 10, black);
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            unsigned char colorB[] = { 255, 255, 255 };
            unsigned char colorF[] = { 0, 0, 0 };
            if (j < group[i].size()) {
                auto& [r, g, b] = group[i][j].xinfa.rgb;
                colorB[0]       = r;
                colorB[1]       = g;
                colorB[2]       = b;
                img.draw_rectangle(posx, posy, posx + 120, posy + 50, colorB, 0.6);
            }
            else {
                img.draw_rectangle(posx, posy, posx + 120, posy + 50, colorB);
            }
            if (j < group[i].size()) {
                auto&  worker = group[i][j];
                string suf;
                if (worker.type == WorkerType::Rich)
                    suf = "老板";
                else if (worker.type == WorkerType::OverRich)
                    suf = "包团";
                wstring xf_name = UTF8ToWideChar(worker.xinfa.xinfa_name + suf);
                wstring id_name = UTF8ToWideChar(worker.game_id);
                drawText(face, img, 15, xf_name, posx + 20, posy + 5, colorF);
                drawText(face, img, 20, id_name, posx + 10, posy + 25, colorF);
            }
            posy += 52;
        }
        posx += 122;
        posy = 70;
    }
    if (!group[5].empty()) {
        drawText(face, img, 15, L"登记：", 10, 340, black);
        posx = 60;
        posy = 340;
        for (auto& worker : group[5]) {
            string suf;
            if (worker.type == WorkerType::Rich)
                suf = "老板";
            else if (worker.type == WorkerType::OverRich)
                suf = "包团";
            wstring xf_name = UTF8ToWideChar(worker.xinfa.xinfa_name + suf);
            wstring id_name = UTF8ToWideChar(worker.game_id);
            drawText(face, img, 15, xf_name, posx, posy, black);
            drawText(face, img, 15, id_name, posx + 45, posy, black);
            posx += 130;
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
    if (img_s->empty())
        return false;
    return true;
}

bool TeamModule::BuildTeamTable(const Team& team, std::vector<std::vector<Worker>>* group) {
    group->resize(6);
    auto tryadd = [group](vector<int> d, const Worker& worker) {
        for (int i : d) {
            if ((*group)[i].size() < 5) {
                (*group)[i].emplace_back(worker);
                return;
            }
        }
    };
    for (auto& worker : team.worker_list) {
        if (worker.status == WorkerStatus::Note) {
            (*group)[5].push_back(worker);
        }
        else if (worker.type == WorkerType::Rich || worker.type == WorkerType::OverRich) {
            tryadd({ 3, 1, 2, 0, 4 }, worker);
        }
        else if (worker.xinfa.xinfa_type_1 == "T") {
            tryadd({ 4, 1, 3, 0, 2 }, worker);
        }
        else if (worker.xinfa.xinfa_type_1 == "nai") {
            tryadd({ 4, 3, 2, 1, 0 }, worker);
        }
        else if (worker.xinfa.xinfa_type_2 == "nei_dps") {
            tryadd({ 2, 3, 1, 0, 4 }, worker);
        }
        else if (worker.xinfa.xinfa_type_2 == "wai_dps") {
            tryadd({ 0, 1, 3, 2, 4 }, worker);
        }
    }
    return true;
}

void TeamModule::SignUpProc(const GroupMessageData& data) const {
    Worker new_worker;
    string team_name;
    if (CheckSignUpFormat(data, &new_worker, &team_name)) {
        Team team;
        if (team_name.empty()) {
            std::vector<Team> teams;
            DB_SelectTeam(&teams);
            if (teams.empty()) {
                ReplyMess(data, "目前没有团");
                return;
            }
            else if (teams.size() == 1) {
                team = teams.front();
            }
            else {
                string reply;
                for (auto& t : teams) {
                    reply += "\n“" + t.team_name + "” " + t.zone.zone_name + " " + t.remark;
                }
                ReplyMess(data, "需要指定车名", { to_string(teams.size()), reply });
                return;
            }
        }
        else {
            if (!DB_SelectTeam(team_name, &team)) {
                ReplyMess(data, "查无此团", { team_name });
                return;
            }
        }
        if (team.status == TeamStatus::Processing) {
            ReplyMess(data, "开组不能报名", { team_name, Mtools::GetTimeFmt(team.process_time) });
            return;
        }
        Worker swap_worker;
        if (new_worker.status == WorkerStatus::Note || CheckTeamCnfg(data, team, new_worker, &swap_worker)) {
            for (auto& wk : team.worker_list) {
                if ((new_worker.status != WorkerStatus::Note && new_worker.ownerQQ == wk.ownerQQ) || new_worker.game_id == wk.game_id) {
                    ReplyMess(data, "重复报名", { wk.game_id, wk.xinfa.xinfa_name, wk.ownerQQ, wk.sendQQ });
                    return;
                }
            }
            if (!swap_worker.game_id.empty()) {
                auto st = WorkerStatus::Note;
                DB_UpdateSignUpStatus(team.team_id, swap_worker.game_id, st);
                auto v = GetGuGuGuValue(swap_worker.ownerQQ);
                if (v[0].value >= 0) {
                    v[0].value = -0.00001;
                }
                ReplyMess(data, "顶坑", { swap_worker.ownerQQ, swap_worker.game_id, swap_worker.xinfa.xinfa_name, to_string(v[0].value) });
            }
            if (DB_InsertSignUp(team.team_id, new_worker))
                ReplyMess(data, "报名成功", { new_worker.game_id, new_worker.xinfa.xinfa_name });
        }
    }
}

bool TeamModule::CheckSignUpFormat(const GroupMessageData& data, Worker* worker, std::string* team_name) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() >= 3 && strs.size() <= 5) {
        worker->sendQQ  = to_string(data.SenderQQ);
        worker->ownerQQ = worker->sendQQ;
        if (regex_match(strs.back(), regex("\\[@[0-9]{5,14}\\]"))) {
            strs.back().pop_back();
            worker->ownerQQ = strs.back().substr(2);
            strs.pop_back();
        }
        else if (regex_match(strs.back(), regex("<[0-9]{5,14}>"))) {
            strs.back().pop_back();
            worker->ownerQQ = strs.back().substr(1);
            strs.pop_back();
        }
        if (strs.size() == 4) {
            *team_name = strs[1];
            strs.erase(strs.begin() + 1);
        }
        if (strs.size() == 3) {
            if (SelectXinfa(strs[1], &(worker->xinfa))) {
                if (!regex_match(UTF8ToWideChar(strs[2]), wregex(L"[\\u4e00-\\u9fa5]{2,6}"))) {
                    ReplyMess(data, "ID不合法", { strs[2] });
                    return false;
                }
                worker->game_id = strs[2];
                worker->type    = WorkerType::Normal;
                worker->status  = WorkerStatus::Normal;
                if (strs[0].find("老板") != -1) {
                    worker->type = WorkerType::Rich;
                }
                else if (strs[0].find("包团") != -1) {
                    worker->type = WorkerType::OverRich;
                }
                if (strs[0].find("登记") != -1) {
                    worker->status = WorkerStatus::Note;
                }
                return true;
            }
            else {
                ReplyMess(data, "未找到心法", { strs[1] });
                return false;
            }
        }
    }
    else {
        ReplyMess(data, "报名格式错误");
        return false;
    }
    return false;
}

bool TeamModule::CheckTeamCnfg(const GroupMessageData& data, const Team& team, const Worker& worker, Worker* swap_worker) const {
    auto& cnfg      = team.zone.zone_config;
    auto& now       = team.now_cnfg;
    auto& me        = worker.xinfa;
    auto  me_gu_val = GetGuGuGuValue(worker.ownerQQ)[0].value;
    int   sum       = now.T.first + now.nai.first + now.dps.first + now.rich.first;
    int   n         = 0;

    vector<pair<Worker, double>> gugugus;
    for (auto& wk : team.worker_list) {
        auto guvalue = GetGuGuGuValue(wk.ownerQQ);
        if (guvalue[0].value < 0) {
            gugugus.push_back({ wk, guvalue[0].value });
        }
    }
    auto tryReplace = [&gugugus, &me, &me_gu_val, &swap_worker](const string& xf_t2) {
        if (gugugus.empty() || me_gu_val < 0)
            return false;
        vector<pair<Worker, double>> swapwork;
        for (auto& [gu, guv] : gugugus) {
            if (guv < 0 && xf_t2 == me.xinfa_type_2 && gu.xinfa.xinfa_type_2 == me.xinfa_type_2) {
                swapwork.push_back({ gu, guv });
            }
        }
        double minv = 0;
        for (auto& [gu, guv] : swapwork) {
            if (guv < minv) {
                *swap_worker = gu;
                minv         = guv;
            }
        }
        if (swapwork.empty())
            return false;
        return true;
    };
    if (sum >= team.zone.zone_size) {
        if (!tryReplace(me.xinfa_type_2)) {
            ReplyMess(data, "车满");
            return false;
        }
    }
    if ((worker.type == WorkerType::Rich || worker.type == WorkerType::OverRich)) {
        if (now.rich.first >= cnfg.rich.second) {
            ReplyMess(data, "老板满");
            return false;
        }
        for (auto& wk : team.worker_list) {
            if ((wk.type == WorkerType::Rich || wk.type == WorkerType::OverRich) && wk.xinfa.xinfa_id == me.xinfa_id) {
                ReplyMess(data, "该心法老板满", { me.xinfa_name });
                return false;
            }
        }
    }
    if (me.xinfa_type_1 == "T" && now.T.first >= cnfg.T.second) {
        if (!tryReplace(me.xinfa_type_2)) {
            ReplyMess(data, "T满");
            return false;
        }
    }
    else if (me.xinfa_type_1 == "nai" && now.nai.first >= cnfg.nai.second) {
        if (!tryReplace(me.xinfa_type_2)) {
            ReplyMess(data, "奶满");
            return false;
        }
    }
    else {
        if (me.xinfa_type_1 == "dps" && now.dps.first >= cnfg.dps.second) {
            if (!tryReplace(me.xinfa_type_2)) {
                ReplyMess(data, "dps满");
                return false;
            }
        }
        else if (me.xinfa_type_2 == "nei_dps" && now.nei_dps.first >= cnfg.nei_dps.second) {
            if (!tryReplace(me.xinfa_type_2)) {
                ReplyMess(data, "内满");
                return false;
            }
        }
        else if (me.xinfa_type_2 == "wai_dps" && now.wai_dps.first >= cnfg.wai_dps.second) {
            if (!tryReplace(me.xinfa_type_2)) {
                ReplyMess(data, "外满");
                return false;
            }
        }
    }
    if (cnfg.xinfas[static_cast<int>(me.xinfa_id)].second == 0) {
        ReplyMess(data, "反思心法", { me.xinfa_name });
        return false;
    }
    if (now.xinfas[static_cast<int>(me.xinfa_id)].first >= cnfg.xinfas[static_cast<int>(me.xinfa_id)].second) {
        if (!tryReplace(me.xinfa_type_2)) {
            ReplyMess(data, "该心法满", { me.xinfa_name });
            return false;
        }
    }
    if (swap_worker->game_id.empty()) {
        if (me.xinfa_type_1 != "T" && sum >= team.zone.zone_size - cnfg.T.first + now.T.first) {
            ReplyMess(data, "缺T");
            return false;
        }
        if (me.xinfa_type_1 != "nai" && sum >= team.zone.zone_size - cnfg.nai.first + now.nai.first) {
            ReplyMess(data, "缺奶");
            return false;
        }
        if (me.xinfa_type_1 != "dps" && sum >= team.zone.zone_size - cnfg.dps.first + now.dps.first) {
            ReplyMess(data, "缺dps");
            return false;
        }
        if (me.xinfa_type_2 != "nei_dps" && sum >= team.zone.zone_size - cnfg.nei_dps.first + now.nei_dps.first) {
            ReplyMess(data, "缺内");
            return false;
        }
        if (me.xinfa_type_2 != "wai_dps" && sum >= team.zone.zone_size - cnfg.wai_dps.first + now.wai_dps.first) {
            ReplyMess(data, "缺外");
            return false;
        }
        for (int i = 0; i < cnfg.xinfas.size(); ++i) {
            if (static_cast<int>(me.xinfa_id) != i && sum >= team.zone.zone_size - cnfg.xinfas[i].first + now.xinfas[i].first) {
                ReplyMess(data, "缺此心法");
                return false;
            }
        }
    }
    return true;
}

void TeamModule::CancelSignUpProc(const GroupMessageData& data) const {
    string team_name, game_id;
    auto   QQ = to_string(data.SenderQQ);
    if (CancelSignUpFormat(data, &team_name, &game_id)) {
        Team team;
        if (team_name.empty()) {
            std::vector<Team> teams;
            DB_SelectTeam(&teams);
            if (teams.empty()) {
                ReplyMess(data, "目前没有团");
                return;
            }
            else if (teams.size() == 1) {
                team = teams.front();
            }
            else {
                string reply;
                for (auto& t : teams) {
                    reply += "\n“" + t.team_name + "” " + t.zone.zone_name + " " + t.remark;
                }
                ReplyMess(data, "需要指定车名", { to_string(teams.size()), reply });
                return;
            }
        }
        else {
            if (!DB_SelectTeam(team_name, &team)) {
                ReplyMess(data, "查无此团", { team_name });
                return;
            }
        }
        if (team.status == TeamStatus::Processing) {
            ReplyMess(data, "已经开组", { team.team_name, Mtools::GetTimeFmt(team.process_time) });
            return;
        }
        if (game_id.empty()) {
            for (auto& wk : team.worker_list) {
                if (wk.ownerQQ == QQ) {
                    game_id = wk.game_id;
                    break;
                }
            }
        }
        if (!game_id.empty()) {
            for (auto& wk : team.worker_list) {
                if (wk.game_id == game_id) {
                    if (wk.ownerQQ == QQ || wk.sendQQ == QQ || IsManager(data)) {
                        auto st = WorkerStatus::Cancel;
                        DB_UpdateSignUpStatus(team.team_id, game_id, st);
                        ReplyMess(data, "取消报名成功", { game_id, wk.xinfa.xinfa_name });
                        return;
                    }
                    else {
                        if (wk.ownerQQ == wk.sendQQ) {
                            ReplyMess(data, "不能取消他人报名", { game_id, wk.ownerQQ });
                            return;
                        }
                        else {
                            ReplyMess(data, "不能取消他人帮的报名", { game_id, wk.ownerQQ, wk.sendQQ });
                            return;
                        }
                    }
                }
            }
        }
        ReplyMess(data, "未报名", { game_id });
        return;
    }
}

bool TeamModule::CancelSignUpFormat(const GroupMessageData& data, std::string* team_name, std::string* game_id) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 3) {
        *team_name = strs[1];
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
        ReplyMess(data, "取消报名格式错误");
        return false;
    }
    return false;
}

void TeamModule::GuGuGuSignUpProc(const GroupMessageData& data) const {
    string team_name, game_id;
    auto   QQ = to_string(data.SenderQQ);
    if (CancelSignUpFormat(data, &team_name, &game_id)) {
        Team team;
        if (team_name.empty()) {
            std::vector<Team> teams;
            DB_SelectTeam(&teams);
            if (teams.empty()) {
                ReplyMess(data, "目前没有团");
                return;
            }
            else if (teams.size() == 1) {
                team = teams.front();
            }
            else {
                string reply;
                for (auto& t : teams) {
                    reply += "\n“" + t.team_name + "” " + t.zone.zone_name + " " + t.remark;
                }
                ReplyMess(data, "需要指定车名", { to_string(teams.size()), reply });
                return;
            }
        }
        else {
            if (!DB_SelectTeam(team_name, &team)) {
                ReplyMess(data, "查无此团", { team_name });
                return;
            }
        }
        if (team.status == TeamStatus::SigningUp) {
            ReplyMess(data, "只有开组才咕", { team.team_name });
            return;
        }
        if (game_id.empty()) {
            for (auto& wk : team.worker_list) {
                if (wk.ownerQQ == QQ) {
                    game_id = wk.game_id;
                    break;
                }
            }
        }
        if (!game_id.empty()) {
            for (auto& wk : team.worker_list) {
                if (wk.game_id == game_id) {
                    if (wk.ownerQQ == QQ || wk.sendQQ == QQ || IsManager(data)) {
                        auto st = WorkerStatus::Pigeon;
                        DB_UpdateSignUpStatus(team.team_id, game_id, st);
                        auto guinfo = GetGuGuGuValue(wk.ownerQQ);
                        ReplyMess(data, "咕咕咕成功", { wk.ownerQQ, game_id, wk.xinfa.xinfa_name, to_string(guinfo[0].value) });
                        return;
                    }
                    else {
                        ReplyMess(data, "非管理员", { to_string(data.SenderQQ), "帮别人咕咕咕" });
                        return;
                    }
                }
            }
        }
        ReplyMess(data, "未报名", { game_id });
        return;
    }
}

bool TeamModule::GuGuGuSignUpFormat(const GroupMessageData& data, std::string* team_name, std::string* game_id) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 3) {
        *team_name = strs[1];
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
        ReplyMess(data, "咕咕咕格式错误");
        return false;
    }
    return false;
}

std::vector<VSInfo> TeamModule::GetGuGuGuValue(const std::string& QQ) const {
    vector<VSInfo> info{ { 0, "" } };
    vector<Worker> workers;
    DB_SelectSignUp(QQ, &workers);
    if (!workers.empty()) {
        int    x   = workers.size();
        int    y   = 0;
        double gu  = 0;
        auto   now = time(0);
        for (auto& wk : workers) {
            if (wk.status == WorkerStatus::Pigeon) {
                y++;
                double day = (now - wk.update_time) / 172800.0 - 5;
                day        = day > 100 ? 100 : day;
                auto v     = 100 / ((1 + exp(day)) * x);
                gu += v;
                info.push_back({ v, Mtools::GetTimeFmt(wk.update_time) + " " + wk.game_id + " " + wk.xinfa.xinfa_name });
            }
        }
        info[0].value = log(x + 5) / log(1.2) - log(6) / log(1.2) - gu;
    }
    return info;
}

void TeamModule::SelectGuGuGuProc(const GroupMessageData& data) {
    string QQ;
    if (CheckSelectGuGuGuFormat(data, &QQ)) {
        auto v = GetGuGuGuValue(QQ);
        ReplyMess(data, "查鸽子值", { to_string(v[0].value) });
    }
}

bool TeamModule::CheckSelectGuGuGuFormat(const GroupMessageData& data, std::string* QQ) const {
    string mess = data.MessageContent;
    auto   strs = Mtools::Split(mess, " ");
    if (strs.size() == 2) {
        if (regex_match(strs.back(), regex("\\[@[0-9]{5,14}\\]"))) {
            strs.back().pop_back();
            *QQ = strs.back().substr(2);
            strs.pop_back();
        }
        else if (regex_match(strs.back(), regex("<[0-9]{5,14}>"))) {
            strs.back().pop_back();
            *QQ = strs.back().substr(1);
            strs.pop_back();
        }
        else {
            ReplyMess(data, "查鸽值格式错误", { strs.back() });
            return false;
        }
    }
    else {
        *QQ = to_string(data.SenderQQ);
    }
    return true;
}

bool TeamModule::SelectZone(const std::string& zone_nkname, Zone* zone) const {
    for (auto& z : zone_list_) {
        for (auto& s : z.zone_nickname) {
            if (zone_nkname == s) {
                *zone = z;
                return true;
            }
        }
    }
    return false;
}

bool TeamModule::SelectXinfa(const std::string& xinfa_nkname, Xinfa* xinfa) const {
    for (auto& xf : xinfa_list_) {
        for (auto& s : xf.xinfa_nicknames) {
            if (xinfa_nkname == s) {
                *xinfa = xf;
                return true;
            }
        }
    }
    return false;
}

void TeamModule::UpdateTeamNowcnfg(Team* team) const {
    for (auto& wk : team->worker_list) {
        if (wk.status == WorkerStatus::Normal) {
            if (wk.type != WorkerType::Normal) {
                team->now_cnfg.rich.first++;
            }
            else {
                if (wk.xinfa.xinfa_type_1 == "T")
                    team->now_cnfg.T.first++;
                else if (wk.xinfa.xinfa_type_1 == "nai")
                    team->now_cnfg.nai.first++;
                else if (wk.xinfa.xinfa_type_1 == "dps")
                    team->now_cnfg.dps.first++;
                if (wk.xinfa.xinfa_type_2 == "nei_dps")
                    team->now_cnfg.nei_dps.first++;
                else if (wk.xinfa.xinfa_type_2 == "wai_dps")
                    team->now_cnfg.wai_dps.first++;
                team->now_cnfg.xinfas[static_cast<int>(wk.xinfa.xinfa_id)].first++;
            }
        }
    }
}

bool TeamModule::DB_Exec(const std::string& sql) const {
    char* cErrMsg;
    int   res = sqlite3_exec(db(), sql.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("SQL执行失败:" + sql + string(cErrMsg));
        return false;
    }
    return true;
}

bool TeamModule::DB_SelectZone() {
    string sql_str = "select * from zone_template_config;";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        db(), sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto  tp  = static_cast<tuple<vector<Zone>*, std::vector<Xinfa>*>*>(vp);
            auto& p   = *(get<0>(*tp));
            auto& xfs = *(get<1>(*tp));
            Zone  q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "zone_name")
                    q.zone_name = val[i];
                else if (s == "zone_nickname")
                    q.zone_nickname = Mtools::Split(val[i], " ");
                else if (s == "zone_config") {
                    auto cnfg_strs = Mtools::Split(val[i], ";");
                    for (auto& cnfg_str : cnfg_strs) {
                        auto cs = Mtools::Split(cnfg_str, " ");
                        if (cs[0] == "T") {
                            q.zone_config.T.first  = stoi(cs[1]);
                            q.zone_config.T.second = stoi(cs[2]);
                        }
                        else if (cs[0] == "nai") {
                            q.zone_config.nai.first  = stoi(cs[1]);
                            q.zone_config.nai.second = stoi(cs[2]);
                        }
                        else if (cs[0] == "dps") {
                            q.zone_config.dps.first  = stoi(cs[1]);
                            q.zone_config.dps.second = stoi(cs[2]);
                        }
                        else if (cs[0] == "nei_dps") {
                            q.zone_config.nei_dps.first  = stoi(cs[1]);
                            q.zone_config.nei_dps.second = stoi(cs[2]);
                        }
                        else if (cs[0] == "wai_dps") {
                            q.zone_config.wai_dps.first  = stoi(cs[1]);
                            q.zone_config.wai_dps.second = stoi(cs[2]);
                        }
                        else if (cs[0] == "rich") {
                            q.zone_config.rich.first  = stoi(cs[1]);
                            q.zone_config.rich.second = stoi(cs[2]);
                        }
                        else
                            for (auto& xf : xfs) {
                                if (cs[0] == xf.xinfa_name) {
                                    q.zone_config.xinfas[static_cast<int>(xf.xinfa_id)].first  = stoi(cs[1]);
                                    q.zone_config.xinfas[static_cast<int>(xf.xinfa_id)].second = stoi(cs[2]);
                                }
                            }
                    }
                }
                else if (s == "zone_size")
                    q.zone_size = stoi(val[i]);
            }
            p.push_back(q);
            return 0;
        },
        &tuple{ &zone_list_, &xinfa_list_ }, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("获取zone_template_config数据错误:" + string(cErrMsg));
        return false;
    }
    return true;
}

bool TeamModule::DB_SelectXinfa() {
    string sql_str = "select * from xinfa_info;";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        base_db(), sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto  p = static_cast<std::vector<Xinfa>*>(vp);
            Xinfa q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "xinfa_id")
                    q.xinfa_id = static_cast<XinfaID>(stoi(val[i]));
                else if (s == "xinfa_name")
                    q.xinfa_name = val[i];
                else if (s == "xinfa_nickname")
                    q.xinfa_nicknames = Mtools::Split(val[i], " ");
                else if (s == "xinfa_first_type")
                    q.xinfa_type_1 = val[i];
                else if (s == "xinfa_second_type")
                    q.xinfa_type_2 = val[i];
                else if (s == "xinfa_third_type")
                    q.xinfa_type_3 = val[i];
                else if (s == "xinfa_zhenyan_score")
                    q.core_score = stoi(val[i]);
                else if (s == "xinfa_RGB") {
                    auto rgb = Mtools::Split(val[i], " ");
                    if (rgb.size() == 3)
                        q.rgb = {
                            stoi(rgb[0]),
                            stoi(rgb[1]),
                            stoi(rgb[2]),
                        };
                }
            }
            p->push_back(q);
            return 0;
        },
        &xinfa_list_, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("获取Xinfa数据错误:" + string(cErrMsg));
        return false;
    }
    return true;
}

bool TeamModule::DB_InsertTeam(const Team& team) const {
    string sql_str = "insert into team_create_info"
                     "(team_name,zone_name,remark,build_time,process_time,finish_time,group_status,debut_qq,glod,goods) values ('";
    sql_str += team.team_name + "','";
    sql_str += team.zone.zone_name + "','";
    sql_str += team.remark + "','";
    sql_str += to_string(team.build_time) + "','";
    sql_str += to_string(team.process_time) + "','";
    sql_str += to_string(team.finish_time) + "','";
    sql_str += to_string(static_cast<int>(team.status)) + "','";
    sql_str += team.debutQQ + "','";
    sql_str += to_string(team.glod) + "','";
    sql_str += team.goods + "');";

    char* cErrMsg;
    int   res = sqlite3_exec(db(), sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("添加开团信息错误:" + team.team_name + string(cErrMsg));
        return false;
    }
    return true;
}

bool TeamModule::DB_SelectTeam(std::vector<Team>* teams) const {
    string sql_str = "select * from team_create_info where group_status = " + to_string(static_cast<int>(TeamStatus::SigningUp));
    sql_str += " OR group_status =" + to_string(static_cast<int>(TeamStatus::Processing)) + ";";
    char* cErrMsg;
    int   res = sqlite3_exec(
        db(), sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto p = static_cast<std::vector<Team>*>(vp);
            Team q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "team_id")
                    q.team_id = stoi(val[i]);
                else if (s == "team_name")
                    q.team_name = val[i];
                else if (s == "zone_name")
                    q.zone.zone_name = val[i];
                else if (s == "remark")
                    q.remark = val[i];
                else if (s == "group_status")
                    q.status = static_cast<TeamStatus>(stoi(val[i]));
                else if (s == "build_time")
                    q.build_time = stoi(val[i]);
                else if (s == "process_time")
                    q.process_time = stoi(val[i]);
                else if (s == "finish_time")
                    q.finish_time = stoi(val[i]);
                else if (s == "debutQQ")
                    q.debutQQ = val[i];
                else if (s == "glod")
                    q.glod = stoi(val[i]);
                else if (s == "goods")
                    q.goods = val[i];
            }
            p->push_back(q);
            return 0;
        },
        teams, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找开团信息错误:" + string(cErrMsg));
        return false;
    }
    for (auto& team : *teams) {
        SelectZone(team.zone.zone_name, &(team.zone));
        DB_SelectSignUp(team.team_id, &team.worker_list);
        UpdateTeamNowcnfg(&team);
    }
    return true;
}

bool TeamModule::DB_SelectTeam(const std::string& zone_name, Team* team) const {
    string sql_str = "select * from team_create_info where (group_status = ";
    sql_str += to_string(static_cast<int>(TeamStatus::SigningUp));
    sql_str += " OR group_status =" + to_string(static_cast<int>(TeamStatus::Processing));
    sql_str += ") and team_name ='" + zone_name + "';";
    vector<Team> teams;
    char*        cErrMsg;
    int          res = sqlite3_exec(
        db(), sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto p = static_cast<std::vector<Team>*>(vp);
            Team q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "team_id")
                    q.team_id = stoi(val[i]);
                else if (s == "team_name")
                    q.team_name = val[i];
                else if (s == "zone_name")
                    q.zone.zone_name = val[i];
                else if (s == "remark")
                    q.remark = val[i];
                else if (s == "group_status")
                    q.status = static_cast<TeamStatus>(stoi(val[i]));
                else if (s == "build_time")
                    q.build_time = stoi(val[i]);
                else if (s == "process_time")
                    q.process_time = stoi(val[i]);
                else if (s == "finish_time")
                    q.finish_time = stoi(val[i]);
                else if (s == "debutQQ")
                    q.debutQQ = val[i];
                else if (s == "glod")
                    q.glod = stoi(val[i]);
                else if (s == "goods")
                    q.goods = val[i];
            }
            p->push_back(q);
            return 0;
        },
        &teams, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找开团信息错误:" + string(cErrMsg));
        return false;
    }
    if (teams.empty())
        return false;
    *team = teams.front();
    SelectZone(team->zone.zone_name, &(team->zone));
    DB_SelectSignUp(team->team_id, &team->worker_list);
    UpdateTeamNowcnfg(team);
    return true;
}

bool TeamModule::DB_UpdateTeam(int team_id, const Team& team) const {
    string sql_str = "UPDATE team_create_info SET ";
    sql_str += "team_name = '" + team.team_name + "',";
    sql_str += "zone_name = '" + team.zone.zone_name + "',";
    sql_str += "remark = '" + team.remark + "',";
    sql_str += "group_status = " + to_string(static_cast<int>(team.status)) + ",";
    sql_str += "build_time = " + to_string(team.build_time) + ",";
    sql_str += "process_time = " + to_string(team.process_time) + ",";
    sql_str += "finish_time = " + to_string(team.finish_time) + ",";
    sql_str += "debut_qq = '" + team.debutQQ + "',";
    sql_str += "glod = " + to_string(team.glod) + ",";
    sql_str += "goods = '" + team.goods + "' where team_id = " + to_string(team_id) + ";";
    char* cErrMsg;
    int   res = sqlite3_exec(db(), sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("更新开团状态错误:" + team_id + string(cErrMsg));
        return false;
    }
    return true;
}

bool TeamModule::DB_InsertSignUp(int team_id, const Worker& worker) const {
    string sql_str = "insert into player_signup_info "
                     "(team_id,sendQQ,ownerQQ,game_id,signup_type,signup_status,xinfa_id,create_datetime,update_datetime)"
                     " values (";
    auto now = time(0);
    sql_str += to_string(team_id) + ",'";
    sql_str += worker.sendQQ + "','";
    sql_str += worker.ownerQQ + "','";
    sql_str += worker.game_id + "',";
    sql_str += to_string(static_cast<int>(worker.type)) + ",";
    sql_str += to_string(static_cast<int>(worker.status)) + ",";
    sql_str += to_string(static_cast<int>(worker.xinfa.xinfa_id)) + ",";
    sql_str += to_string(now) + ",";
    sql_str += to_string(now) + ");";
    char* cErrMsg;
    int   res = sqlite3_exec(db(), sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("添加报名信息错误:" + worker.game_id + string(cErrMsg));
        return false;
    }
    return true;
}

bool TeamModule::DB_SelectSignUp(const int team_id, std::vector<Worker>* workers) const {
    string sql_str = "select * from player_signup_info where team_id = ";
    sql_str += to_string(team_id) + " and (signup_status = ";
    sql_str += to_string(static_cast<int>(WorkerStatus::Normal));
    sql_str += " or signup_status = " + to_string(static_cast<int>(WorkerStatus::Note)) + ");";
    char* cErrMsg;
    int   res = sqlite3_exec(
        db(), sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto   p = static_cast<std::vector<Worker>*>(vp);
            Worker q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "game_id")
                    q.game_id = val[i];
                else if (s == "sendQQ")
                    q.sendQQ = val[i];
                else if (s == "ownerQQ")
                    q.ownerQQ = val[i];
                else if (s == "signup_type")
                    q.type = static_cast<WorkerType>(stoi(val[i]));
                else if (s == "signup_status")
                    q.status = static_cast<WorkerStatus>(stoi(val[i]));
                else if (s == "xinfa_id")
                    q.xinfa.xinfa_id = static_cast<XinfaID>(stoi(val[i]));
            }
            p->push_back(q);
            return 0;
        },
        workers, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找报名信息错误:" + string(cErrMsg));
        return false;
    }
    for (auto& wk : *workers) {
        wk.xinfa = xinfa_list_[static_cast<int>(wk.xinfa.xinfa_id)];
    }
    return true;
}

bool TeamModule::DB_SelectSignUp(const std::string& ownerQQ, std::vector<Worker>* workers) const {
    string sql_str = "select * from player_signup_info where ownerQQ = '" + ownerQQ + "' and signup_type = ";
    sql_str += to_string(static_cast<int>(WorkerType::Normal)) + " and ( signup_status = ";
    sql_str += to_string(static_cast<int>(WorkerStatus::Normal));
    sql_str += " or signup_status = " + to_string(static_cast<int>(WorkerStatus::Pigeon)) + " );";
    char* cErrMsg;
    int   res = sqlite3_exec(
        db(), sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto   p = static_cast<std::vector<Worker>*>(vp);
            Worker q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "game_id")
                    q.game_id = val[i];
                else if (s == "sendQQ")
                    q.sendQQ = val[i];
                else if (s == "ownerQQ")
                    q.ownerQQ = val[i];
                else if (s == "signup_type")
                    q.type = static_cast<WorkerType>(stoi(val[i]));
                else if (s == "signup_status")
                    q.status = static_cast<WorkerStatus>(stoi(val[i]));
                else if (s == "xinfa_id")
                    q.xinfa.xinfa_id = static_cast<XinfaID>(stoi(val[i]));
                else if (s == "update_datetime")
                    q.update_time = stoll(val[i]);
            }
            p->push_back(q);
            return 0;
        },
        workers, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找报名信息错误:" + string(cErrMsg));
        return false;
    }
    for (auto& wk : *workers) {
        wk.xinfa = xinfa_list_[static_cast<int>(wk.xinfa.xinfa_id)];
    }
    return true;
}

bool TeamModule::DB_UpdateSignUpStatus(int team_id, const std::string& game_id, WorkerStatus status) const {
    string sql_str = "update player_signup_info set ";
    sql_str += "signup_status = " + to_string(static_cast<int>(status));
    sql_str += " , update_datetime = " + to_string(time(0));
    sql_str += " where game_id = '" + game_id;
    sql_str += "' and team_id=" + to_string(team_id) + ";";
    char* cErrMsg;
    int   res = sqlite3_exec(db(), sql_str.c_str(), 0, 0, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("更新报名状态错误:" + game_id + string(cErrMsg));
        return false;
    }
    return true;
}

bool TeamModule::DB_SelectReplyMessage(const std::string& key, vector<ReMess>* replys) const {
    string sql_str = "select * from reply_mess_table where key = '" + key + "';";
    char*  cErrMsg;
    int    res = sqlite3_exec(
        base_db(), sql_str.c_str(),
        [](auto vp, auto cnt, auto val, auto name) {
            auto   p = static_cast<std::vector<ReMess>*>(vp);
            ReMess q;
            for (int i = 0; i < cnt; ++i) {
                string s = name[i];
                if (s == "message")
                    q.mess = val[i];
                else if (s == "percent")
                    q.percent = stoi(val[i]);
            }
            p->push_back(q);
            return 0;
        },
        replys, &cErrMsg);
    if (res != SQLITE_OK) {
        api->OutputLog("查找回复信息错误:" + string(cErrMsg));
        return false;
    }
    return true;
}
