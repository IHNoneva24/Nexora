#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
#include "NetworkManager.h"
#include "GameContext.h"
#include "CharacterRenderer.h"
#include "../../BLL/include/QuestionData.h"
#include <string>
#include <vector>

class GameScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();
    void Enter(const GameContext& ctx, NetworkManager& net);
    ScreenID Tick(float dt, NetworkManager& net);

private:
    // ── Sub-states ────────────────────────────────────────────────────────────
    enum class Phase {
        Answering,      // waiting for both players to pick
        WaitRemote,     // local answer sent, waiting for remote
        ShowResult,     // reveal + play attack animation
        NextDelay,      // brief pause before next question
        GameOver        // all questions done, scoreboard
    };

    // ── Anim states ──────────────────────────────────────────────────────────
    enum class AnimState { Idle, Lunge, Impact, Return };

    void BuildQuestionList(const GameContext& ctx, NetRole role);
    void StartRound();
    void EvaluateRound();
    void DrawBackground(int sw, int sh) const;
    void DrawScorebar(int sw) const;
    void DrawQuestion(int sw, int sh) const;
    void DrawAnswerChoices(int sw, int sh, bool locked);
    void DrawCharacters(int sw, int sh) const;
    void DrawResultOverlay(int sw, int sh) const;
    void DrawGameOver(int sw, int sh, NetworkManager& net);
    Color CharTint(int playerIdx) const; // 0=left, 1=right
    void DrawPlatform(float cx, float y, float w, float h) const;

    Font        m_font      = {};
    std::string m_assetRoot;

    // Questions — interleaved so both players see same order
    std::vector<QuestionData> m_questions; // 20 total
    int  m_qIdx      = 0;
    int  m_scoreLeft = 0;   // host (left side)
    int  m_scoreRight= 0;   // client (right side)

    // Per-round answer state
    int  m_myAnswer     = -1; // -1 = not answered
    int  m_remoteAnswer = -1;
    bool m_mySent       = false;

    Phase     m_phase     = Phase::Answering;
    AnimState m_animState = AnimState::Idle;
    float     m_timer     = 0.f; // general phase/anim timer

    // Result for current round: -1=draw, 0=left wins, 1=right wins
    int m_roundWinner = -1;

    // Am I the host (left) or client (right)?
    NetRole m_role = NetRole::None;

    // Character rendering
    std::vector<Texture2D> m_leftLayers;
    std::vector<Texture2D> m_rightLayers;
    Texture2D              m_leftSword  = {};
    Texture2D              m_rightSword = {};

    // Animation offsets (pixels)
    float m_leftOffsetX  = 0.f;
    float m_rightOffsetX = 0.f;
    float m_shakeTimer   = 0.f;
    int   m_shakeTarget  = -1; // 0=left, 1=right

    // Layout constants
    static constexpr float CHAR_H      = 160.f;
    static constexpr float PLAT_H      = 18.f;
    static constexpr float LUNGE_DIST  = 80.f;
    static constexpr float ANIM_SPEED  = 260.f; // px/s during lunge

    static constexpr float PHASE_LUNGE      = 0.30f;
    static constexpr float PHASE_IMPACT     = 0.55f;
    static constexpr float PHASE_RETURN     = 0.85f;
    static constexpr float PHASE_SHOW_RESULT= 3.0f;
    static constexpr float PHASE_NEXT_DELAY = 0.6f;
};
