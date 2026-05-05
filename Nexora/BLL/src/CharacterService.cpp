#include "../include/CharacterService.h"

CharacterService::CharacterService(CharacterRepository& repo) : m_repo(repo) {}

void CharacterService::Init() { m_repo.InitTable(); }

bool CharacterService::HasCharacter(int userId) {
    return userId > 0 && m_repo.Exists(userId);
}

bool CharacterService::Save(const CharacterData& c) {
    return c.userId > 0 && m_repo.Save(c);
}

bool CharacterService::Load(int userId, CharacterData& out) {
    return userId > 0 && m_repo.Load(userId, out);
}
