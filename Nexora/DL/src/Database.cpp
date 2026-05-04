#include "Database.h"
#include <stdexcept>
#include <iostream>
#include <filesystem>

Database::Database(const std::string& path) {
    // Ensure the parent directory exists before SQLite tries to create the file
    std::filesystem::path fsPath(path);
    if (fsPath.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(fsPath.parent_path(), ec);
        if (ec) {
            std::cerr << "[DB] Cannot create directory '"
                      << fsPath.parent_path().string()
                      << "': " << ec.message() << "\n";
        }
    }

    int rc = sqlite3_open(path.c_str(), &m_db);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Cannot open '" << path << "': "
                  << sqlite3_errmsg(m_db) << "\n";
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

Database::~Database() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool Database::Execute(const std::string& sql) {
    if (!m_db) return false;
    char* err = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Execute error: " << (err ? err : "unknown") << "\n";
        sqlite3_free(err);
        return false;
    }
    return true;
}
