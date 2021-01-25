#pragma once
#include "stdafx.h"
#include "sdk/sdk.h"

class ModuleBase {
public:
    ModuleBase() : enable_(false), lv_(0) {}

    ModuleBase(bool enable, int lv) : enable_(enable), lv_(){};

    virtual bool ProcPrivateMess(const PrivateMessageData& data) {
        return false;
    };

    virtual bool ProcGroupMess(const GroupMessageData& data) {
        return false;
    };

    virtual bool ProcEvent(const EventData& data) {
        return false;
    };

    void set_enable(bool enable) {
        enable_ = enable;
    }

    bool enable() const {
        return enable_;
    }

    int lv() const {
        return lv_;
    }

private:
    bool enable_;

    int lv_;  // 模块优先级[-100~100]
};