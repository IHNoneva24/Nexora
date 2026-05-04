#pragma once
#include <string>
#include "sqlite3.h"

// ─────────────────────────────────────────────
//  DATA LAYER – Database
//  Manages the raw SQLite connection.
//  All SQL execution goes through this class.
// ─────────────────────────────────────────────
class Database {
public:
    explicit Database(const std::string& path);
    ~Database();

    // Non-copyable
    Database(const Database&)            = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* GetHandle() const { return m_db; }

    // Returns true if the DB opened successfully
    bool IsOpen() const { return m_db != nullptr; }

    // Run a statement that returns no rows (CREATE, INSERT, UPDATE, DELETE)
    bool Execute(const std::string& sql);

private:
    sqlite3* m_db = nullptr;
};
