#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
#include "CharacterRenderer.h"
#include "../../DL/include/CharacterData.h"
#include "../../BLL/include/QuestionData.h"
#include <string>
#include <vector>

// ─────────────────────────────────────────────
//  Single Player – "Who Wants to Be a Millionaire?" style
//  15 questions with escalating prize money,
//  3 lifelines: 50:50, Phone-a-Friend, Ask the Audience.
//  Two safety nets at question 5 and 10.
// ─────────────────────────────────────────────
class SinglePlayerScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();
    void Enter(const CharacterData& charData);
    ScreenID Tick(float dt);

private:
    // ── Sub-states ────────────────────────────────────────────────────────────
    enum class Phase {
        Playing,        // answering questions
        Thinking,       // brief delay after selecting before reveal
        Correct,        // show correct animation
        Wrong,          // show wrong + game over
        WalkAway,       // player chose to walk away
        GameOver        // final result screen (win / loss / walk-away)
    };

    // ── Question loading ─────────────────────────────────────────────────────
    void LoadQuestions();

    // ── Gameplay drawing ──────────────────────────────────────────────────────
    void DrawBackground(int sw, int sh) const;
    void DrawMoneyLadder(int sw, int sh) const;
    void DrawQuestion(int sw, int sh) const;
    void DrawAnswerChoices(int sw, int sh);
    void DrawLifelines(int sw, int sh);
    void DrawCharacter(int sw, int sh) const;
    void DrawCorrectOverlay(int sw, int sh) const;
    void DrawWrongOverlay(int sw, int sh) const;
    bool DrawGameOver(int sw, int sh);
    void DrawPhoneAFriendPopup(int sw, int sh);
    void DrawAskAudiencePopup(int sw, int sh);

    void ShuffleQuestions();

    Font        m_font      = {};
    std::string m_assetRoot;
    Phase       m_phase     = Phase::Playing;

    // Questions
    static constexpr int TOTAL_QUESTIONS = 15;
    std::vector<QuestionData> m_questions;
    int  m_questionIdx = 0;

    // Money ladder (classic WWTBAM values)
    static constexpr int PRIZE_VALUES[TOTAL_QUESTIONS] = {
        100, 200, 300, 500, 1000,
        2000, 4000, 8000, 16000, 32000,
        64000, 125000, 250000, 500000, 1000000
    };
    static constexpr int SAFETY_NET_1 = 4;   // question 5 (index 4) = $1,000
    static constexpr int SAFETY_NET_2 = 9;   // question 10 (index 9) = $32,000

    int  m_selectedAnswer = -1;
    int  m_winnings       = 0;

    // Lifelines
    bool m_usedFiftyFifty    = false;
    bool m_usedPhoneAFriend  = false;
    bool m_usedAskAudience   = false;
    bool m_fiftyFiftyActive  = false; // currently applied to this question
    int  m_eliminatedChoices[2] = {-1, -1}; // indices removed by 50:50

    bool m_showPhonePopup    = false;
    bool m_showAudiencePopup = false;
    std::string m_phoneAdvice;
    int  m_audienceBars[4]   = {};

    // Timers / animation
    float m_timer       = 0.f;
    float m_thinkTime   = 1.5f;
    float m_correctTime = 2.0f;
    float m_wrongTime   = 3.0f;

    // Character
    std::vector<Texture2D> m_charLayers;
    CharacterData          m_charData = {};
};
