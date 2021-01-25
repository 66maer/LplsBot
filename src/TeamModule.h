#pragma once
#include "ModuleBase.h"

class TeamModule final : public ModuleBase {
public:
    TeamModule();
    
    virtual bool ProcGroupMess(const GroupMessageData& data) override;

private:
    bool Init();

    bool Trigger(const std::string& mess, const std::vector<std::string>& keys, bool first = true);

    void CreateTeamProc(const GroupMessageData& data);

};