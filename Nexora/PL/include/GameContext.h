#pragma once
#include "../../DL/include/CharacterData.h"
#include "../../BLL/include/QuestionData.h"
#include <vector>

struct GameContext {
    CharacterData myChar;
    CharacterData remoteChar;
    std::vector<QuestionData> myQuestions;     // questions I created
    std::vector<QuestionData> remoteQuestions; // questions remote created
};
