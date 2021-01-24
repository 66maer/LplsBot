#pragma once
#include "ModuleBase.h"

class TeamModule final : public ModuleBase {
public:
    TeamModule();

    virtual void init() override;

    virtual bool run(std::any data) override;

private:
};