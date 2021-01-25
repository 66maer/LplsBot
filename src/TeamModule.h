#pragma once
#include "ModuleBase.h"

class TeamModule final : public ModuleBase {
public:
    TeamModule();
    
    virtual bool ProcGroupMess(const GroupMessageData& data) override;

private:
    bool Init();

    bool IsManager(const GroupMessageData& data);

    bool Trigger(const std::string& mess, const std::vector<std::string>& keys, bool first = true) const;

    void CreateTeamProc(const GroupMessageData& data);

    bool CheckCreatTeamFormat(const GroupMessageData& data, Tuan* tuan);
};