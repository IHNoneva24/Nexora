#include "../include/GameScreen.h"
#include <cmath>
#include <string>
#include <algorithm>

// ─── Helpers ──────────────────────────────────────────────────────────────────

static void FlipLayersHorizontal(std::vector<Texture2D>& layers) {
    for (auto& tex : layers) {
        if (tex.id == 0) continue;
        Image img = LoadImageFromTexture(tex);
        ImageFlipHorizontal(&img);
        UnloadTexture(tex);
        tex = LoadTextureFromImage(img);
        SetTextureFilter(tex, TEXTURE_FILTER_POINT);
        UnloadImage(img);
    }
}

static void DrawSword(Texture2D tex, float cx, float platformY, float charH,
                      bool flipped, float angle, Color tint) {
    if (tex.id == 0) return;
    float scale = charH / CharacterRenderer::FRAME_H;
    float w = CharacterRenderer::FRAME_W * scale;
    float h = CharacterRenderer::FRAME_H * scale;
    Rectangle src  = CharacterRenderer::FRAME_SRC;
    if (flipped) src.width = -src.width;
    Rectangle dest   = { cx - w * .5f, platformY - h, w, h };
    Vector2   origin = { w * .5f, h };
    DrawTexturePro(tex, src, dest, origin, angle, tint);
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

void GameScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot = assetRoot;
    m_font      = font;
}

void GameScreen::Unload() {
    CharacterRenderer::UnloadLayers(m_leftLayers);
    CharacterRenderer::UnloadLayers(m_rightLayers);
    if (m_leftSword.id)  { UnloadTexture(m_leftSword);  m_leftSword  = {}; }
    if (m_rightSword.id) { UnloadTexture(m_rightSword); m_rightSword = {}; }
}

void GameScreen::BuildQuestionList(const GameContext& ctx, NetRole role) {
    // Both sides agree: even slots = host questions, odd slots = client questions
    const auto& hostQ   = (role == NetRole::Host) ? ctx.myQuestions     : ctx.remoteQuestions;
    const auto& clientQ = (role == NetRole::Host) ? ctx.remoteQuestions : ctx.myQuestions;

    m_questions.clear();
    int n = (int)std::min(hostQ.size(), clientQ.size());
    for (int i = 0; i < n; ++i) {
        m_questions.push_back(hostQ[i]);
        m_questions.push_back(clientQ[i]);
    }
}

void GameScreen::Enter(const GameContext& ctx, NetworkManager& net) {
    m_role       = net.GetRole();
    m_qIdx       = 0;
    m_scoreLeft  = 0;
    m_scoreRight = 0;

    BuildQuestionList(ctx, m_role);

    CharacterRenderer::UnloadLayers(m_leftLayers);
    CharacterRenderer::UnloadLayers(m_rightLayers);

    m_leftLayers  = CharacterRenderer::LoadLayers(ctx.myChar,     m_assetRoot);
    m_rightLayers = CharacterRenderer::LoadLayers(ctx.remoteChar, m_assetRoot);

    // Swap for client: on client side myChar is on right, remoteChar on left
    if (m_role == NetRole::Client) {
        std::swap(m_leftLayers, m_rightLayers);
    }

    // Pre-flip left character so it faces right (toward opponent)
    FlipLayersHorizontal(m_leftLayers);

    // Load sword textures
    const std::string hand  = m_assetRoot + "/GandalfHardcore Character Asset Pack/";
    bool leftFemale  = (m_role == NetRole::Host) ? (ctx.myChar.gender == 1)
                                                  : (ctx.remoteChar.gender == 1);
    bool rightFemale = (m_role == NetRole::Host) ? (ctx.remoteChar.gender == 1)
                                                  : (ctx.myChar.gender == 1);
    if (m_leftSword.id)  { UnloadTexture(m_leftSword);  m_leftSword  = {}; }
    if (m_rightSword.id) { UnloadTexture(m_rightSword); m_rightSword = {}; }
    m_leftSword  = LoadTexture((hand + (leftFemale  ? "Female Hand/Female Sword.png"
                                                    : "Male Hand/Male Sword.png")).c_str());
    m_rightSword = LoadTexture((hand + (rightFemale ? "Female Hand/Female Sword.png"
                                                    : "Male Hand/Male Sword.png")).c_str());
    if (m_leftSword.id)  SetTextureFilter(m_leftSword,  TEXTURE_FILTER_POINT);
    if (m_rightSword.id) SetTextureFilter(m_rightSword, TEXTURE_FILTER_POINT);

    // Pre-flip left sword to match the pre-flipped character
    if (m_leftSword.id) {
        Image img = LoadImageFromTexture(m_leftSword);
        ImageFlipHorizontal(&img);
        UnloadTexture(m_leftSword);
        m_leftSword = LoadTextureFromImage(img);
        SetTextureFilter(m_leftSword, TEXTURE_FILTER_POINT);
        UnloadImage(img);
    }

    StartRound();
}

void GameScreen::StartRound() {
    m_myAnswer     = -1;
    m_remoteAnswer = -1;
    m_mySent       = false;
    m_phase        = Phase::Answering;
    m_animState    = AnimState::Idle;
    m_timer        = 0.f;
    m_leftOffsetX  = 0.f;
    m_rightOffsetX = 0.f;
    m_shakeTimer   = 0.f;
    m_shakeTarget  = -1;
    m_roundWinner  = -1;
}

void GameScreen::EvaluateRound() {
    if (m_qIdx >= (int)m_questions.size()) return;
    const QuestionData& q = m_questions[m_qIdx];

    // Determine who is left (host) and who is right (client)
    int leftAnswer  = (m_role == NetRole::Host) ? m_myAnswer     : m_remoteAnswer;
    int rightAnswer = (m_role == NetRole::Host) ? m_remoteAnswer : m_myAnswer;

    bool leftCorrect  = (leftAnswer  == q.correctIdx);
    bool rightCorrect = (rightAnswer == q.correctIdx);

    if (leftCorrect && !rightCorrect) {
        m_roundWinner = 0;
        m_scoreLeft++;
    } else if (rightCorrect && !leftCorrect) {
        m_roundWinner = 1;
        m_scoreRight++;
    } else {
        m_roundWinner = -1; // draw
    }

    m_phase     = Phase::ShowResult;
    m_animState = (m_roundWinner >= 0) ? AnimState::Lunge : AnimState::Idle;
    m_timer     = 0.f;
}

// ─── Drawing helpers ──────────────────────────────────────────────────────────

void GameScreen::DrawPlatform(float cx, float y, float w, float h) const {
    Rectangle r = { cx - w * .5f, y, w, h };
    DrawRectangleRec(r, { 55, 45, 30, 255 });
    DrawRectangleLinesEx(r, 2, UI::C_BORDER);
    DrawLineEx({ r.x + 4, r.y + 2 }, { r.x + r.width - 4, r.y + 2 }, 1, { 200,180,120,80 });
}

Color GameScreen::CharTint(int playerIdx) const {
    // Flash hit character red during impact
    if (m_animState == AnimState::Impact && m_shakeTarget == playerIdx) {
        float t = m_timer - PHASE_LUNGE;
        float frac = t / (PHASE_IMPACT - PHASE_LUNGE);
        unsigned char r = (unsigned char)(255);
        unsigned char g = (unsigned char)(255 * (1.f - frac * 0.7f));
        unsigned char b = (unsigned char)(255 * (1.f - frac * 0.7f));
        return { r, g, b, 255 };
    }
    return WHITE;
}

void GameScreen::DrawBackground(int sw, int sh) const {
    DrawRectangle(0, 0, sw, sh, { 6, 4, 2, 255 });
    // subtle arena floor gradient
    for (int i = 0; i < 80; ++i) {
        float alpha = (float)i / 80.f * 40.f;
        DrawRectangle(0, sh - 80 + i, sw, 1, { 60, 45, 20, (unsigned char)alpha });
    }
    // centre divider
    float cx = (float)sw * 0.5f;
    DrawLineEx({ cx, 90.f }, { cx, (float)sh - 60.f }, 1, { 80, 60, 20, 80 });
}

void GameScreen::DrawScorebar(int sw) const {
    float cx = (float)sw * 0.5f;
    std::string left  = "P1: " + std::to_string(m_scoreLeft);
    std::string right = "P2: " + std::to_string(m_scoreRight);
    std::string qnum  = "Q " + std::to_string(m_qIdx + 1) + "/" +
                         std::to_string((int)m_questions.size());

    UI::LabelC(left,  cx - 220.f, 8.f,  22.f, UI::C_TEXT_GOLD,  m_font);
    UI::LabelC(qnum,  cx,         8.f,  18.f, UI::C_TEXT_DIM,   m_font);
    UI::LabelC(right, cx + 220.f, 8.f,  22.f, UI::C_TEXT_GOLD,  m_font);
    DrawLineEx({ 0.f, 36.f }, { (float)sw, 36.f }, 1, UI::C_BORDER_DARK);
}

void GameScreen::DrawQuestion(int sw, int sh) const {
    if (m_qIdx >= (int)m_questions.size()) return;
    const QuestionData& q = m_questions[m_qIdx];

    float cx = (float)sw * 0.5f;
    float panelW = (float)sw * 0.52f;
    float panelY = 44.f;

    // Word-wrap simulation: just draw the text centered, it may overflow
    // For now draw the question text in the upper-center panel
    Rectangle panel = { cx - panelW * 0.5f, panelY, panelW, 68.f };
    UI::DrawPanel(panel);

    float fs = 18.f;
    // Simple single-line truncation for now
    Vector2 sz = MeasureTextEx(m_font, q.text.c_str(), fs, 1);
    while (sz.x > panelW - 24.f && fs > 10.f) {
        fs -= 1.f;
        sz = MeasureTextEx(m_font, q.text.c_str(), fs, 1);
    }
    DrawTextEx(m_font, q.text.c_str(),
               { cx - sz.x * 0.5f, panelY + (68.f - sz.y) * 0.5f }, fs, 1, UI::C_TEXT_LIGHT);
}

void GameScreen::DrawAnswerChoices(int sw, int sh, bool locked) {
    if (m_qIdx >= (int)m_questions.size()) return;
    const QuestionData& q = m_questions[m_qIdx];

    float cx = (float)sw * 0.5f;
    int   numChoices = (q.type == QuestionData::Type::TrueFalse) ? 2 : 4;
    float btnW = (numChoices == 2) ? 200.f : 180.f;
    float btnH = 44.f;
    float gap  = 14.f;
    float totalW = numChoices * btnW + (numChoices - 1) * gap;
    float startX = cx - totalW * 0.5f;
    float btnY   = (float)sh * 0.82f;

    const char* choiceLabels[4] = { "A", "B", "C", "D" };
    const char* tfLabels[2]     = { "TRUE", "FALSE" };

    for (int i = 0; i < numChoices; ++i) {
        float bx = startX + i * (btnW + gap);
        Rectangle r = { bx, btnY, btnW, btnH };

        bool selected = (m_myAnswer == i);
        bool correct  = (i == q.correctIdx);

        // After both answered: show correct/wrong coloring
        bool revealColors = (m_phase == Phase::ShowResult ||
                             m_phase == Phase::NextDelay);

        Color bg, border, tc;
        if (revealColors) {
            if (correct) {
                bg = { 20, 80, 20, 240 }; border = { 80, 210, 88, 255 }; tc = UI::C_TEXT_OK;
            } else if (selected) {
                bg = { 80, 20, 20, 240 }; border = { 248, 72, 52, 255 }; tc = UI::C_TEXT_ERR;
            } else {
                bg = { 18, 12, 4, 200 }; border = UI::C_BORDER_DARK; tc = UI::C_TEXT_DIM;
            }
        } else if (selected) {
            bg = { 50, 38, 10, 255 }; border = UI::C_TEXT_GOLD; tc = UI::C_TEXT_GOLD;
        } else {
            bg = UI::C_BTN_NORMAL; border = UI::C_BORDER; tc = UI::C_TEXT_LIGHT;
        }

        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, selected ? 2.f : 1.f, border);

        // Choice label prefix
        const char* prefix = (q.type == QuestionData::Type::TrueFalse)
                             ? tfLabels[i] : choiceLabels[i];

        std::string label;
        if (q.type == QuestionData::Type::TrueFalse) {
            label = prefix;
        } else {
            label = std::string(prefix) + ": " + q.choices[i];
        }

        // Truncate if too long
        float fs = 17.f;
        Vector2 sz = MeasureTextEx(m_font, label.c_str(), fs, 1);
        while (sz.x > btnW - 16.f && fs > 10.f) {
            fs -= 1.f;
            sz = MeasureTextEx(m_font, label.c_str(), fs, 1);
        }
        DrawTextEx(m_font, label.c_str(),
                   { r.x + (r.width  - sz.x) * 0.5f,
                     r.y + (r.height - sz.y) * 0.5f }, fs, 1, tc);

        // Click handler
        if (!locked && !revealColors &&
            CheckCollisionPointRec(GetMousePosition(), r) &&
            IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            m_myAnswer = i;
        }
    }
}

void GameScreen::DrawCharacters(int sw, int sh) const {
    float platY = (float)sh * 0.76f;
    float platW = (float)sw * 0.25f;
    float leftCX  = (float)sw * 0.20f + m_leftOffsetX;
    float rightCX = (float)sw * 0.80f + m_rightOffsetX;

    // Shake: use elapsed (not remaining) time so sin starts from 0 and oscillates visibly
    float shakeOff = 0.f;
    if (m_shakeTimer > 0.f) {
        float elapsed = 0.4f - m_shakeTimer;
        shakeOff = std::sin(elapsed * 55.f) * 9.f * (m_shakeTimer / 0.4f);
    }

    DrawPlatform((float)sw * 0.20f, platY, platW, PLAT_H);
    DrawPlatform((float)sw * 0.80f, platY, platW, PLAT_H);

    // Left character — pre-flipped at load time so it faces RIGHT (toward opponent)
    {
        float cx = leftCX + (m_shakeTarget == 0 ? shakeOff : 0.f);
        Color tint = CharTint(0);
        CharacterRenderer::Draw(m_leftLayers, cx, platY, CHAR_H);
        if (tint.r != 255 || tint.g != 255 || tint.b != 255) {
            float scale = CHAR_H / CharacterRenderer::FRAME_H;
            float destW = CharacterRenderer::FRAME_W * scale;
            float destH = CharacterRenderer::FRAME_H * scale;
            Rectangle dest = { cx - destW * .5f, platY - destH, destW, destH };
            DrawRectangleRec(dest, { 255, 60, 60, 80 });
        }
        // Sword drawn normally — texture was pre-flipped at load time
        if (m_animState == AnimState::Lunge || m_animState == AnimState::Impact) {
            if (m_roundWinner == 0) {
                DrawSword(m_leftSword, cx + 14.f, platY, CHAR_H, false, 20.f, WHITE);
            }
        }
    }

    // Right character — normal draw so it faces LEFT (toward opponent)
    {
        float cx = rightCX + (m_shakeTarget == 1 ? shakeOff : 0.f);
        Color tint = CharTint(1);
        CharacterRenderer::Draw(m_rightLayers, cx, platY, CHAR_H);
        if (tint.r != 255 || tint.g != 255 || tint.b != 255) {
            float scale = CHAR_H / CharacterRenderer::FRAME_H;
            float destW = CharacterRenderer::FRAME_W * scale;
            float destH = CharacterRenderer::FRAME_H * scale;
            Rectangle dest = { cx - destW * .5f, platY - destH, destW, destH };
            DrawRectangleRec(dest, { 255, 60, 60, 80 });
        }
        if (m_animState == AnimState::Lunge || m_animState == AnimState::Impact) {
            if (m_roundWinner == 1) {
                DrawSword(m_rightSword, cx - 14.f, platY, CHAR_H, false, -20.f, WHITE);
            }
        }
    }

    // Slash effect at moment of impact
    if (m_animState == AnimState::Impact) {
        float impactX = (m_roundWinner == 0)
                        ? (float)sw * 0.80f + m_rightOffsetX
                        : (float)sw * 0.20f + m_leftOffsetX;
        float impactY = platY - CHAR_H * 0.55f;
        float t       = (m_timer - PHASE_LUNGE) / (PHASE_IMPACT - PHASE_LUNGE);
        unsigned char alpha = (unsigned char)(220.f * (1.f - t));
        float sz = 28.f + t * 18.f;

        // Slash lines radiating outward
        Color slashCol = { 255, 220, 80, alpha };
        for (int k = 0; k < 6; ++k) {
            float angle = (float)k * (3.14159f / 3.f) + t * 0.4f;
            float ex = impactX + std::cos(angle) * sz;
            float ey = impactY + std::sin(angle) * sz * 0.7f;
            DrawLineEx({ impactX, impactY }, { ex, ey }, 3.f, slashCol);
        }
        // Star burst center
        DrawCircle((int)impactX, (int)impactY, 8.f * (1.f - t), { 255, 255, 200, alpha });
    }
}

void GameScreen::DrawResultOverlay(int sw, int sh) const {
    if (m_phase != Phase::ShowResult && m_phase != Phase::NextDelay) return;

    float cx = (float)sw * 0.5f;
    float cy = (float)sh * 0.50f;

    std::string msg;
    Color       col;
    if (m_roundWinner == 0)       { msg = "PLAYER 1 WINS!"; col = UI::C_TEXT_GOLD; }
    else if (m_roundWinner == 1)  { msg = "PLAYER 2 WINS!"; col = UI::C_TEXT_GOLD; }
    else                          { msg = "DRAW!";           col = UI::C_TEXT_DIM;  }

    // Fade in
    float alpha = std::min(1.f, m_timer / 0.25f);
    col.a = (unsigned char)(alpha * 255.f);

    float fs = 42.f;
    Vector2 sz = MeasureTextEx(m_font, msg.c_str(), fs, 1);
    float tx = cx - sz.x * 0.5f;
    float ty = cy - sz.y * 0.5f;

    // Shadow
    Color shadow = { 0, 0, 0, (unsigned char)(alpha * 160.f) };
    DrawTextEx(m_font, msg.c_str(), { tx + 3.f, ty + 3.f }, fs, 1, shadow);
    DrawTextEx(m_font, msg.c_str(), { tx, ty }, fs, 1, col);
}

void GameScreen::DrawGameOver(int sw, int sh, NetworkManager& net) {
    float cx = (float)sw * 0.5f;
    DrawRectangle(0, 0, sw, sh, { 6, 4, 2, 255 });

    UI::LabelShadow("GAME OVER", cx, (float)sh * 0.12f, 52.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 200.f, (float)sh * 0.12f + 60.f, 400.f);

    float py = (float)sh * 0.28f;
    UI::LabelC("FINAL SCORE", cx, py, 22.f, UI::C_TEXT_DIM, m_font);
    py += 36.f;

    auto scoreStr = [](const std::string& name, int score) {
        return name + "  " + std::to_string(score);
    };

    Color leftCol  = (m_scoreLeft  > m_scoreRight) ? UI::C_TEXT_OK  :
                     (m_scoreLeft  < m_scoreRight) ? UI::C_TEXT_ERR : UI::C_TEXT_DIM;
    Color rightCol = (m_scoreRight > m_scoreLeft)  ? UI::C_TEXT_OK  :
                     (m_scoreRight < m_scoreLeft)  ? UI::C_TEXT_ERR : UI::C_TEXT_DIM;

    UI::LabelC("PLAYER 1", cx - 160.f, py, 28.f, leftCol,  m_font);
    UI::LabelC("PLAYER 2", cx + 160.f, py, 28.f, rightCol, m_font);
    py += 38.f;

    UI::LabelC(std::to_string(m_scoreLeft),  cx - 160.f, py, 52.f, leftCol,  m_font);
    UI::LabelC(std::to_string(m_scoreRight), cx + 160.f, py, 52.f, rightCol, m_font);
    py += 74.f;

    std::string verdict;
    Color verdictCol;
    if      (m_scoreLeft  > m_scoreRight) { verdict = "PLAYER 1 WINS!"; verdictCol = UI::C_TEXT_GOLD; }
    else if (m_scoreRight > m_scoreLeft)  { verdict = "PLAYER 2 WINS!"; verdictCol = UI::C_TEXT_GOLD; }
    else                                  { verdict = "IT'S A TIE!";     verdictCol = UI::C_TEXT_DIM;  }
    UI::LabelShadow(verdict, cx, py, 36.f, verdictCol, m_font);

    float btnH = 50.f, btnW = 200.f;
    if (UI::Button({ cx - btnW * 0.5f, (float)sh - btnH - 30.f, btnW, btnH },
                   "MAIN MENU", m_font, 22.f)) {
        net.Shutdown();
        m_phase = Phase::Answering; // reset so we don't loop
        // The Tick return handles transition
    }
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

ScreenID GameScreen::Tick(float dt, NetworkManager& net) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    // ── Poll remote answer at any time ───────────────────────────────────────
    int remoteAns;
    if (net.PollRemoteAnswer(remoteAns))
        m_remoteAnswer = remoteAns;

    // ── Phase logic ──────────────────────────────────────────────────────────
    switch (m_phase) {
    case Phase::Answering:
        // Player clicks an answer → send and move to WaitRemote
        if (m_myAnswer >= 0 && !m_mySent) {
            net.SendAnswer(m_myAnswer);
            m_mySent = true;
            if (m_remoteAnswer >= 0)
                EvaluateRound();     // remote already came in
            else
                m_phase = Phase::WaitRemote;
        }
        break;

    case Phase::WaitRemote:
        if (m_remoteAnswer >= 0)
            EvaluateRound();
        break;

    case Phase::ShowResult:
        m_timer += dt;

        // ── Animation sub-state ──────────────────────────────────────────────
        if (m_roundWinner >= 0) {
            if (m_animState == AnimState::Lunge) {
                float dir = (m_roundWinner == 0) ? 1.f : -1.f;
                if (m_roundWinner == 0) m_leftOffsetX  += ANIM_SPEED * dt;
                else                   m_rightOffsetX -= ANIM_SPEED * dt;

                if (m_timer >= PHASE_LUNGE) {
                    m_animState = AnimState::Impact;
                    m_shakeTarget = (m_roundWinner == 0) ? 1 : 0;
                    m_shakeTimer  = 0.4f;
                }
            } else if (m_animState == AnimState::Impact) {
                m_shakeTimer = std::max(0.f, m_shakeTimer - dt);
                if (m_timer >= PHASE_IMPACT)
                    m_animState = AnimState::Return;
            } else if (m_animState == AnimState::Return) {
                if (m_roundWinner == 0) m_leftOffsetX  = std::max(0.f, m_leftOffsetX  - ANIM_SPEED * dt);
                else                   m_rightOffsetX = std::min(0.f, m_rightOffsetX + ANIM_SPEED * dt);

                if (m_timer >= PHASE_RETURN)
                    m_animState = AnimState::Idle;
            }
        }

        if (m_timer >= PHASE_SHOW_RESULT) {
            m_qIdx++;
            if (m_qIdx >= (int)m_questions.size()) {
                m_phase = Phase::GameOver;
            } else {
                m_phase = Phase::NextDelay;
                m_timer = 0.f;
            }
        }
        break;

    case Phase::NextDelay:
        m_timer += dt;
        if (m_timer >= PHASE_NEXT_DELAY)
            StartRound();
        break;

    case Phase::GameOver:
        break;
    }

    // ── Render ───────────────────────────────────────────────────────────────
    if (m_phase == Phase::GameOver) {
        DrawGameOver(sw, sh, net);
        // DrawGameOver sets up the "MAIN MENU" button which calls net.Shutdown()
        // We detect it via net.GetRole() becoming None
        if (net.GetRole() == NetRole::None)
            return ScreenID::MainMenu;
        return ScreenID::Game;
    }

    DrawBackground(sw, sh);
    DrawScorebar(sw);
    DrawQuestion(sw, sh);
    DrawCharacters(sw, sh);

    bool locked = (m_phase != Phase::Answering) || (m_myAnswer >= 0);
    DrawAnswerChoices(sw, sh, locked);

    // "Waiting for opponent..." hint while in WaitRemote
    if (m_phase == Phase::WaitRemote) {
        float pulse = (float)(0.55 + 0.45 * std::sin(GetTime() * 2.2));
        Color wc = { 180, 160, 100, (unsigned char)(pulse * 200) };
        UI::LabelC("Waiting for opponent...", (float)sw * 0.5f,
                   (float)sh * 0.73f, 18.f, wc, m_font);
    }

    DrawResultOverlay(sw, sh);

    return ScreenID::Game;
}
