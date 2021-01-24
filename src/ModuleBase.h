#pragma once
#include "stdafx.h"
#include "Types.h"

class ModuleBase {
public:
    ModuleBase(std::uint32_t handle, int lv);

    virtual void init() = 0;

    virtual bool run(std::any data) = 0;

    void set_enable(bool enable) {
        enable_ = enable;
    }

    bool enable() const {
        return enable_;
    }

    std::uint32_t handle() const {
        return handle_;
    }

    int lv() const {
        return lv_;
    }

private:
    
    bool enable_;

    std::uint32_t handle_;  // 模块处理是信息类型

    int lv_;  // 模块优先级[-100~100]
};