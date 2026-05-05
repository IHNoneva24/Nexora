#pragma once
#include "../../DL/include/CharacterRepository.h"
#include "../../DL/include/CharacterData.h"

class CharacterService {
public:
    explicit CharacterService(CharacterRepository& repo);
    void Init();
    bool HasCharacter(int userId);
    bool Save(const CharacterData& c);
    bool Load(int userId, CharacterData& out);
private:
    CharacterRepository& m_repo;
};
