#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct QuestionData {
    enum class Type : uint8_t { TrueFalse = 0, MultiChoice = 1 };

    Type        type       = Type::TrueFalse;
    std::string text;
    std::string choices[4]; // [0]=True/A, [1]=False/B, [2]=C, [3]=D
    int         correctIdx = 0;

    static constexpr int MAX_TEXT   = 128;
    static constexpr int MAX_CHOICE = 64;
};
