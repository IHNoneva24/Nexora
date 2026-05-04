#include "UserRepository.h"
#include <iostream>

UserRepository::UserRepository(Database& db) : m_db(db) {}

std::string UserRepository::HashPassword(const std::string& password) {
    // djb2 – fast but NOT cryptographically secure.
    // Replace with bcrypt / Argon2 for a real product.
    unsigned long hash = 5381;
    for (unsigned char c : password)
        hash = ((hash << 5) + hash) ^ c;
    return std::to_string(hash);
}

void UserRepository::InitTable() {
    const std::string sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username      TEXT NOT NULL UNIQUE,"
        "  password_hash TEXT NOT NULL"
        ");";
    m_db.Execute(sql);
}

bool UserRepository::CreateUser(const std::string& username,
                                const std::string& password) {
    if (UsernameExists(username)) return false;

    sqlite3_stmt* stmt = nullptr;
    const std::string sql =
        "INSERT INTO users (username, password_hash) VALUES (?, ?);";

    if (sqlite3_prepare_v2(m_db.GetHandle(), sql.c_str(), -1, &stmt, nullptr)
            != SQLITE_OK)
        return false;

    const std::string hash = HashPassword(password);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash.c_str(),     -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool UserRepository::VerifyPassword(const std::string& username,
                                    const std::string& password) {
    auto user = FindByUsername(username);
    return user.has_value() && user->passwordHash == HashPassword(password);
}

std::optional<User> UserRepository::FindByUsername(const std::string& username) {
    sqlite3_stmt* stmt = nullptr;
    const std::string sql =
        "SELECT id, username, password_hash FROM users WHERE username = ?;";

    if (sqlite3_prepare_v2(m_db.GetHandle(), sql.c_str(), -1, &stmt, nullptr)
            != SQLITE_OK)
        return std::nullopt;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id           = sqlite3_column_int(stmt, 0);
        u.username     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        result         = u;
    }

    sqlite3_finalize(stmt);
    return result;
}

bool UserRepository::UsernameExists(const std::string& username) {
    return FindByUsername(username).has_value();
}
