#pragma once
#include "Database.h"
#include "CharacterData.h"

class CharacterRepository {
public:
    explicit CharacterRepository(Database& db);
    void InitTable();
    bool Save(const CharacterData& c);
    bool Load(int userId, CharacterData& out);
    bool Exists(int userId);
private:
    Database& m_db;
};
