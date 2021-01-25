#pragma once
#include "ModuleBase.h"

class TeamModule final : public ModuleBase {
public:
    TeamModule();
    
    virtual bool ProcGroupMess(const GroupMessageData& data) override;

private:
    bool Init();
};