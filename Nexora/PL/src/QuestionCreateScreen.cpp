#include "../include/QuestionCreateScreen.h"
#include <cmath>
#include <string>

void QuestionCreateScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot = assetRoot;
    m_font      = font;
}

void QuestionCreateScreen::Enter(NetworkManager& /*net*/) {
    m_state      = SubState::Inputting;
    m_questionIdx = 0;
    m_submitted   = false;
    m_myQuestions.clear();
    m_remoteQuestions.clear();

    m_curType     = QuestionData::Type::TrueFalse;
    m_curText     = "";
    for (auto& c : m_curChoices) c = "";
    m_curCorrect  = 0;
    m_activeField = 0;
}

bool QuestionCreateScreen::ValidateCurrent() const {
    if (m_curText.empty()) return false;
    if (m_curType == QuestionData::Type::MultiChoice) {
        for (const auto& c : m_curChoices)
            if (c.empty()) return false;
    }
    return true;
}

void QuestionCreateScreen::CommitCurrent() {
    QuestionData q;
    q.type       = m_curType;
    q.text       = m_curText;
    q.correctIdx = m_curCorrect;
    if (m_curType == QuestionData::Type::TrueFalse) {
        q.choices[0] = "True";
        q.choices[1] = "False";
    } else {
        for (int i = 0; i < 4; ++i) q.choices[i] = m_curChoices[i];
    }
    m_myQuestions.push_back(q);
}

// ── Drawing helpers ───────────────────────────────────────────────────────────

static void DrawTypeToggle(float cx, float y, float btnW, float btnH, float gap,
                           QuestionData::Type& type, Font font) {
    const float totalW = btnW * 2 + gap;
    float x = cx - totalW * 0.5f;

    // TRUE/FALSE button
    {
        Rectangle r = { x, y, btnW, btnH };
        bool sel     = type == QuestionData::Type::TrueFalse;
        Color bg     = sel ? Color{50,38,10,255} : Color{18,12,4,220};
        Color border = sel ? UI::C_TEXT_GOLD : UI::C_BORDER_DARK;
        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, sel ? 2.f : 1.f, border);
        Color tc = sel ? UI::C_TEXT_GOLD : UI::C_TEXT_DIM;
        Vector2 sz = MeasureTextEx(font, "TRUE / FALSE", 17.f, 1);
        DrawTextEx(font, "TRUE / FALSE",
                   { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                   17.f, 1, tc);
        if (!sel && CheckCollisionPointRec(GetMousePosition(), r) &&
            IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            type = QuestionData::Type::TrueFalse;
    }

    // A/B/C/D button
    {
        Rectangle r = { x + btnW + gap, y, btnW, btnH };
        bool sel     = type == QuestionData::Type::MultiChoice;
        Color bg     = sel ? Color{50,38,10,255} : Color{18,12,4,220};
        Color border = sel ? UI::C_TEXT_GOLD : UI::C_BORDER_DARK;
        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, sel ? 2.f : 1.f, border);
        Color tc = sel ? UI::C_TEXT_GOLD : UI::C_TEXT_DIM;
        Vector2 sz = MeasureTextEx(font, "A / B / C / D", 17.f, 1);
        DrawTextEx(font, "A / B / C / D",
                   { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                   17.f, 1, tc);
        if (!sel && CheckCollisionPointRec(GetMousePosition(), r) &&
            IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            type = QuestionData::Type::MultiChoice;
    }
}

void QuestionCreateScreen::DrawInputting(int sw, int sh, NetworkManager& /*net*/) {
    float cx = (float)sw * 0.5f;

    // Background overlay
    DrawRectangle(0, 0, sw, sh, Color{8, 5, 2, 248});

    // Header
    std::string headerStr = "QUESTION " + std::to_string(m_questionIdx + 1) + " / " +
                            std::to_string(TOTAL_QUESTIONS);
    UI::LabelShadow(headerStr, cx, 22.f, 36.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 200.f, 68.f, 400.f);

    // Main panel
    float panelW = (float)sw * 0.72f;
    float panelH = (float)sh * 0.78f;
    Rectangle panel = { cx - panelW * 0.5f, 85.f, panelW, panelH };
    UI::DrawPanel(panel);

    float px = panel.x + 28.f;
    float pw = panelW - 56.f;
    float py = panel.y + 20.f;

    // Type toggle
    UI::LabelC("Question Type:", cx, py, 16.f, UI::C_TEXT_DIM, m_font);
    py += 22.f;

    float toggleW = 190.f, toggleH = 38.f, toggleGap = 18.f;

    // Save previous type so we can reset correctIdx on change
    QuestionData::Type prevType = m_curType;
    DrawTypeToggle(cx, py, toggleW, toggleH, toggleGap, m_curType, m_font);
    if (m_curType != prevType) {
        m_curCorrect  = 0;
        m_activeField = 0;
    }
    py += toggleH + 18.f;

    UI::Divider(px, py, pw);
    py += 12.f;

    // Question text field
    UI::LabelC("Question:", cx, py, 16.f, UI::C_TEXT_DIM, m_font);
    py += 20.f;
    Rectangle qField = { px, py, pw, 46.f };
    if (CheckCollisionPointRec(GetMousePosition(), qField) &&
        IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        m_activeField = 0;
    UI::InputField(qField, m_curText, m_activeField == 0, false, m_font, 18.f,
                   QuestionData::MAX_TEXT);
    py += 54.f;

    bool canAdvance = ValidateCurrent();

    if (m_curType == QuestionData::Type::TrueFalse) {
        // Correct answer selector
        UI::LabelC("Correct Answer:", cx, py, 16.f, UI::C_TEXT_DIM, m_font);
        py += 24.f;

        float bw = 160.f, bh = 50.f, gap = 30.f;
        float bx = cx - bw - gap * 0.5f;

        for (int i = 0; i < 2; ++i) {
            Rectangle r = { bx + i * (bw + gap), py, bw, bh };
            const char* lbl = (i == 0) ? "TRUE" : "FALSE";
            bool sel = m_curCorrect == i;
            Color bg     = sel ? Color{50,38,10,255} : Color{18,12,4,200};
            Color border = sel ? UI::C_TEXT_GOLD     : UI::C_BORDER_DARK;
            DrawRectangleRec(r, bg);
            DrawRectangleLinesEx(r, sel ? 2.f : 1.f, border);
            Color tc = sel ? UI::C_TEXT_GOLD : UI::C_TEXT_LIGHT;
            Vector2 sz = MeasureTextEx(m_font, lbl, 20.f, 1);
            DrawTextEx(m_font, lbl,
                       { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                       20.f, 1, tc);
            if (CheckCollisionPointRec(GetMousePosition(), r) &&
                IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                m_curCorrect = i;
        }
        py += bh + 10.f;

    } else {
        // 4 choice input fields
        const char* labels[] = { "A:", "B:", "C:", "D:" };
        float fieldH = 40.f, fieldGap = 10.f;
        float labelW = 22.f;

        for (int i = 0; i < 4; ++i) {
            float fy = py + i * (fieldH + fieldGap);
            DrawTextEx(m_font, labels[i], { px, fy + (fieldH - 16.f) * 0.5f },
                       16.f, 1, UI::C_TEXT_DIM);
            Rectangle r = { px + labelW + 6.f, fy, pw - labelW - 6.f, fieldH };
            if (CheckCollisionPointRec(GetMousePosition(), r) &&
                IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                m_activeField = i + 1;
            UI::InputField(r, m_curChoices[i], m_activeField == (i + 1), false,
                           m_font, 17.f, QuestionData::MAX_CHOICE);
        }
        py += 4 * (fieldH + fieldGap) + 8.f;

        // Correct answer radio row
        UI::LabelC("Correct Answer:", cx, py, 15.f, UI::C_TEXT_DIM, m_font);
        py += 22.f;

        float bw = (pw - 3 * 10.f) / 4.f, bh = 36.f;
        for (int i = 0; i < 4; ++i) {
            Rectangle r = { px + i * (bw + 10.f), py, bw, bh };
            const char* lbl = labels[i];
            bool sel = m_curCorrect == i;
            Color bg     = sel ? Color{50,38,10,255} : Color{18,12,4,200};
            Color border = sel ? UI::C_TEXT_GOLD     : UI::C_BORDER_DARK;
            DrawRectangleRec(r, bg);
            DrawRectangleLinesEx(r, sel ? 2.f : 1.f, border);
            Color tc = sel ? UI::C_TEXT_GOLD : UI::C_TEXT_LIGHT;
            Vector2 sz = MeasureTextEx(m_font, lbl, 18.f, 1);
            DrawTextEx(m_font, lbl,
                       { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                       18.f, 1, tc);
            if (CheckCollisionPointRec(GetMousePosition(), r) &&
                IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                m_curCorrect = i;
        }
        py += bh + 6.f;
    }

    // Validation hint
    if (!canAdvance) {
        std::string hint = (m_curType == QuestionData::Type::MultiChoice)
            ? "Fill in the question and all 4 choices to continue."
            : "Fill in the question to continue.";
        UI::LabelC(hint, cx, panel.y + panelH - 58.f, 14.f, UI::C_TEXT_DIM, m_font);
    }

    // Bottom: NEXT / SUBMIT button
    float btnH = 46.f, btnW = 160.f;
    float btnY = panel.y + panelH - btnH - 14.f;
    bool last = (m_questionIdx == TOTAL_QUESTIONS - 1);
    const std::string btnLabel = last ? "SUBMIT" : "NEXT";

    if (canAdvance) {
        if (UI::Button({ cx - btnW * 0.5f, btnY, btnW, btnH }, btnLabel, m_font, 22.f) ||
            IsKeyPressed(KEY_ENTER)) {
            CommitCurrent();
            m_questionIdx++;
            // Reset working state for next question
            m_curType     = QuestionData::Type::TrueFalse;
            m_curText     = "";
            for (auto& c : m_curChoices) c = "";
            m_curCorrect  = 0;
            m_activeField = 0;
        }
    } else {
        UI::ButtonDisabled({ cx - btnW * 0.5f, btnY, btnW, btnH }, btnLabel, m_font, 22.f);
    }

    // Progress dots
    float dotSpacing = 22.f;
    float dotStartX  = cx - (TOTAL_QUESTIONS - 1) * dotSpacing * 0.5f;
    float dotY = (float)sh - 22.f;
    for (int i = 0; i < TOTAL_QUESTIONS; ++i) {
        float dx = dotStartX + i * dotSpacing;
        Color col = (i < m_questionIdx)  ? UI::C_TEXT_GOLD :
                    (i == m_questionIdx) ? UI::C_TEXT_LIGHT : UI::C_TEXT_DIM;
        DrawCircle((int)dx, (int)dotY, (i == m_questionIdx) ? 5.f : 3.5f, col);
    }
}

void QuestionCreateScreen::DrawWaiting(int sw, int sh) {
    float cx = (float)sw * 0.5f;
    DrawRectangle(0, 0, sw, sh, Color{8, 5, 2, 248});

    UI::LabelShadow("QUESTIONS SUBMITTED", cx, (float)sh * 0.35f, 36.f,
                    UI::C_TEXT_GOLD, m_font);

    float pulse = (float)(0.6 + 0.4 * sin(GetTime() * 1.8));
    Color waitCol = { 200, 180, 100, (unsigned char)(pulse * 240) };
    UI::LabelC("Waiting for the other player...", cx, (float)sh * 0.50f,
               22.f, waitCol, m_font);

    // Animated dots
    int dots = ((int)(GetTime() * 2.0) % 4);
    std::string ellipsis(dots, '.');
    UI::LabelC(ellipsis, cx, (float)sh * 0.57f, 28.f, waitCol, m_font);
}

// ── Tick ──────────────────────────────────────────────────────────────────────

ScreenID QuestionCreateScreen::Tick(float dt, NetworkManager& net) {
    (void)dt;
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    if (m_state == SubState::Inputting) {
        DrawInputting(sw, sh, net);

        // All questions entered — send to remote and switch to waiting
        if (m_questionIdx >= TOTAL_QUESTIONS) {
            net.SendQuestions(m_myQuestions);
            m_submitted = true;
            m_state     = SubState::Waiting;
        }
    } else {
        DrawWaiting(sw, sh);
    }

    // Always poll for remote questions (they may arrive at any time)
    std::vector<QuestionData> incoming;
    if (net.PollRemoteQuestions(incoming)) {
        m_remoteQuestions = std::move(incoming);
    }

    // Transition to game when both sides are done
    if (m_submitted && !m_remoteQuestions.empty())
        return ScreenID::Game;

    return ScreenID::QuestionCreate;
}
