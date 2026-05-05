#include "../include/CharacterRepository.h"

CharacterRepository::CharacterRepository(Database& db) : m_db(db) {}

void CharacterRepository::InitTable() {
    m_db.Execute(
        "CREATE TABLE IF NOT EXISTS characters ("
        "  user_id   INTEGER PRIMARY KEY,"
        "  gender    INTEGER NOT NULL DEFAULT 0,"
        "  skin_idx  INTEGER NOT NULL DEFAULT 0,"
        "  hair_idx  INTEGER NOT NULL DEFAULT 0,"
        "  top_idx   INTEGER NOT NULL DEFAULT 0,"
        "  pants_idx INTEGER NOT NULL DEFAULT 0,"
        "  feet_idx  INTEGER NOT NULL DEFAULT 0"
        ");"
    );
}

bool CharacterRepository::Save(const CharacterData& c) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "INSERT OR REPLACE INTO characters "
        "(user_id, gender, skin_idx, hair_idx, top_idx, pants_idx, feet_idx) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(m_db.GetHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, c.userId);
    sqlite3_bind_int(stmt, 2, c.gender);
    sqlite3_bind_int(stmt, 3, c.skinIdx);
    sqlite3_bind_int(stmt, 4, c.hairIdx);
    sqlite3_bind_int(stmt, 5, c.topIdx);
    sqlite3_bind_int(stmt, 6, c.pantsIdx);
    sqlite3_bind_int(stmt, 7, c.feetIdx);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool CharacterRepository::Load(int userId, CharacterData& out) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT gender, skin_idx, hair_idx, top_idx, pants_idx, feet_idx "
        "FROM characters WHERE user_id = ?;";

    if (sqlite3_prepare_v2(m_db.GetHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, userId);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out.userId   = userId;
        out.gender   = sqlite3_column_int(stmt, 0);
        out.skinIdx  = sqlite3_column_int(stmt, 1);
        out.hairIdx  = sqlite3_column_int(stmt, 2);
        out.topIdx   = sqlite3_column_int(stmt, 3);
        out.pantsIdx = sqlite3_column_int(stmt, 4);
        out.feetIdx  = sqlite3_column_int(stmt, 5);
        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

bool CharacterRepository::Exists(int userId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT 1 FROM characters WHERE user_id = ? LIMIT 1;";

    if (sqlite3_prepare_v2(m_db.GetHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, userId);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}
