#pragma once
#include "stdafx.h"
#include "sdk/sdk.h"

class ModuleBase {
public:
    ModuleBase(sqlite3* base_db, sqlite3* db) : base_db_(base_db), db_(db), enable_(false), lv_(0) {}

    ModuleBase(sqlite3* base_db, sqlite3* db, bool enable, int lv) : base_db_(base_db), db_(db), enable_(enable), lv_(){};

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

    void set_db(sqlite3* db) {
        db_ = db;
    }

    sqlite3* db() const {
        return db_;
    }

     sqlite3* base_db() const {
        return base_db_;
    }

private:
    bool enable_;

    int lv_;  // 模块优先级[-100~100]

    sqlite3* base_db_;

    sqlite3* db_;
};