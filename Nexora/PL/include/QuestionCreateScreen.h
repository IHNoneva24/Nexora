#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
#include "NetworkManager.h"
#include "GameContext.h"
#include "../../BLL/include/QuestionData.h"
#include <string>
#include <vector>

class QuestionCreateScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload() {}
    void Enter(NetworkManager& net);
    ScreenID Tick(float dt, NetworkManager& net);

    const std::vector<QuestionData>& GetMyQuestions()     const { return m_myQuestions; }
    const std::vector<QuestionData>& GetRemoteQuestions() const { return m_remoteQuestions; }

    void FillGameContext(GameContext& ctx) const {
        ctx.myQuestions     = m_myQuestions;
        ctx.remoteQuestions = m_remoteQuestions;
    }

private:
    enum class SubState { Inputting, Waiting };

    void DrawInputting(int sw, int sh, NetworkManager& net);
    void DrawWaiting(int sw, int sh);
    bool ValidateCurrent() const;
    void CommitCurrent();

    Font        m_font  = {};
    std::string m_assetRoot;
    SubState    m_state = SubState::Inputting;

    static constexpr int TOTAL_QUESTIONS = 10;

    // Per-question working state
    QuestionData::Type m_curType     = QuestionData::Type::TrueFalse;
    std::string        m_curText;
    std::string        m_curChoices[4];
    int                m_curCorrect  = 0;
    int                m_activeField = 0; // 0=question, 1-4=choice fields

    int  m_questionIdx = 0; // 0-based, how many finalized
    bool m_submitted   = false;

    std::vector<QuestionData> m_myQuestions;
    std::vector<QuestionData> m_remoteQuestions;
};
