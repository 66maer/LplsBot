#pragma once
#include "ModuleBase.h"
#include "Types.h"

class TeamModule final : public ModuleBase {
public:
    TeamModule(sqlite3* base_db, sqlite3* db);

    virtual bool ProcGroupMess(const GroupMessageData& data) override;

private:
    bool Init();

    bool IsManager(const GroupMessageData& data) const;

    void ReplyMess(const GroupMessageData& data, const std::string& key, std::initializer_list<std::string> args) const;

    bool Trigger(const std::string& mess, const std::vector<std::string>& keys, bool first = true) const;

    void CreateTeamProc(const GroupMessageData& data) const;

    bool CheckCreatTeamFormat(const GroupMessageData& data, Team* team) const;

    void OpenTeamProc(const GroupMessageData& data) const;

    bool CheckOpenTeamFormat(const GroupMessageData& data, std::string* team_name) const;

    void CancelTeamProc(const GroupMessageData& data) const;

    bool CheckCancelTeamFormat(const GroupMessageData& data, std::string* team_name) const;

    void FinishTeamProc(const GroupMessageData& data) const;

    bool CheckFinishTeamFormat(const GroupMessageData& data, Team* team) const;

    void SelectTeamProc(const GroupMessageData& data);

    bool BuildTeamImg(const Team& team, std::vector<uint8_t>* img_s);

    bool BuildTeamTable(const Team& team, std::vector<std::vector<Worker>>* group);

    void SignUpProc(const GroupMessageData& data) const;

    bool CheckSignUpFormat(const GroupMessageData& data, Worker* worker, std::string* team_name) const;

    bool CheckTeamCnfg(const GroupMessageData& data, const Team& team, const Worker& worker, Worker* swap_worker) const;

    void CancelSignUpProc(const GroupMessageData& data) const;

    bool CancelSignUpFormat(const GroupMessageData& data, std::string* team_name, std::string* game_id) const;

    void GuGuGuSignUpProc(const GroupMessageData& data) const;

    bool GuGuGuSignUpFormat(const GroupMessageData& data, std::string* team_name, std::string* game_id) const;

    std::vector<VSInfo> GetGuGuGuValue(const std::string& QQ) const;

    void SelectGuGuGuProc(const GroupMessageData& data);

    bool CheckSelectGuGuGuFormat(const GroupMessageData& data, std::string* QQ) const;

    bool SelectZone(const std::string& zone_nkname, Zone* zone) const;

    bool SelectXinfa(const std::string& xinfa_nkname, Xinfa* xinfa) const;

    void UpdateTeamNowcnfg(Team* team) const;

    bool DB_Exec(const std::string& sql) const;

    bool DB_SelectZone();

    bool DB_SelectXinfa();

    bool DB_InsertTeam(const Team& team) const;

    bool DB_SelectTeam(std::vector<Team>* teams) const;

    bool DB_SelectTeam(const std::string& zone_name, Team* team) const;

    bool DB_UpdateTeam(int team_id, const Team& team) const;

    bool DB_InsertSignUp(int team_id, const Worker& worker) const;

    bool DB_SelectSignUp(const int team_id, std::vector<Worker>* workers) const;

    bool DB_SelectSignUp(const std::string& ownerQQ, std::vector<Worker>* workers) const;

    bool DB_UpdateSignUpStatus(int team_id, const std::string& game_id, WorkerStatus status) const;

    bool DB_SelectReplyMessage(const std::string& key, std::vector<ReMess>* replys) const;

private:
    std::vector<Xinfa> xinfa_list_;

    std::vector<Zone> zone_list_;

    time_t select_team_cd_;
};