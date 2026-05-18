#include "../include/SinglePlayerScreen.h"
#include <cstdlib>
#include <algorithm>
#include <string>

constexpr int SinglePlayerScreen::PRIZE_VALUES[TOTAL_QUESTIONS];

void SinglePlayerScreen::DrawBackground(int sw, int sh) const {
    if (m_background.id != 0)
        DrawTexturePro(m_background,
            { 0, 0, (float)m_background.width, (float)m_background.height },
            { 0, 0, (float)sw, (float)sh }, { 0, 0 }, 0.f, WHITE);
    else
        DrawRectangle(0, 0, sw, sh, { 6, 4, 8, 255 });
}

// Stores the asset root path and font for later use
void SinglePlayerScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot  = assetRoot;
    m_font       = font;
    m_background = LoadTexture((assetRoot + "/background4.png").c_str());
    m_bgGameOver = LoadTexture((assetRoot + "/background1.png").c_str());
}

// Releases character layer textures from memory
void SinglePlayerScreen::Unload() {
    UnloadTexture(m_background);
    UnloadTexture(m_bgGameOver);
    CharacterRenderer::UnloadLayers(m_charLayers);
}

// Resets all game state and loads questions and the player's character for a new round
void SinglePlayerScreen::Enter(const CharacterData& charData) {
    m_phase        = Phase::Playing;
    m_questionIdx  = 0;
    m_selectedAnswer = -1;
    m_winnings     = 0;
    m_timer        = 0.f;

    m_usedFiftyFifty   = false;
    m_usedPhoneAFriend = false;
    m_usedAskAudience  = false;
    m_fiftyFiftyActive = false;
    m_eliminatedChoices[0] = m_eliminatedChoices[1] = -1;
    m_showPhonePopup    = false;
    m_showAudiencePopup = false;

    m_questions.clear();
    LoadQuestions();
    ShuffleQuestions();

    m_charData = charData;
    CharacterRenderer::UnloadLayers(m_charLayers);
    m_charLayers = CharacterRenderer::LoadLayers(charData, m_assetRoot);
}

// Randomly reorders the question list using Fisher-Yates
void SinglePlayerScreen::ShuffleQuestions() {
    for (int i = (int)m_questions.size() - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        std::swap(m_questions[i], m_questions[j]);
    }
}

// Populates the question list with all predefined multiple-choice questions
void SinglePlayerScreen::LoadQuestions() {
    struct QDef { const char* text; const char* a; const char* b; const char* c; const char* d; int correct; };
    static const QDef defs[TOTAL_QUESTIONS] = {
        {"What color are bananas when they are ripe?",
         "Red", "Yellow", "Blue", "Green", 1},
        {"How many legs does a spider have?",
         "6", "8", "10", "4", 1},
        {"What planet is known as the Red Planet?",
         "Venus", "Jupiter", "Mars", "Saturn", 2},
        {"What is the largest ocean on Earth?",
         "Atlantic", "Indian", "Arctic", "Pacific", 3},
        {"In what country would you find the Eiffel Tower?",
         "Italy", "Germany", "France", "Spain", 2},
        {"What is the chemical symbol for gold?",
         "Go", "Gd", "Au", "Ag", 2},
        {"Which artist painted the Mona Lisa?",
         "Michelangelo", "Leonardo da Vinci", "Raphael", "Donatello", 1},
        {"What is the smallest prime number?",
         "0", "1", "2", "3", 2},
        {"What gas do plants absorb from the atmosphere?",
         "Oxygen", "Nitrogen", "Carbon Dioxide", "Hydrogen", 2},
        {"In what year did the Titanic sink?",
         "1905", "1912", "1920", "1898", 1},
        {"What is the hardest natural substance on Earth?",
         "Titanium", "Diamond", "Quartz", "Graphene", 1},
        {"Which element has the atomic number 79?",
         "Silver", "Platinum", "Gold", "Copper", 2},
        {"What is the longest river in the world?",
         "Amazon", "Yangtze", "Mississippi", "Nile", 3},
        {"In computing, how many bits are in a byte?",
         "4", "8", "16", "32", 1},
        {"What is the speed of light in km/s (approx)?",
         "150,000", "300,000", "450,000", "600,000", 1},
    };

    for (int i = 0; i < TOTAL_QUESTIONS; ++i) {
        QuestionData q;
        q.type       = QuestionData::Type::MultiChoice;
        q.text       = defs[i].text;
        q.choices[0] = defs[i].a;
        q.choices[1] = defs[i].b;
        q.choices[2] = defs[i].c;
        q.choices[3] = defs[i].d;
        q.correctIdx = defs[i].correct;
        m_questions.push_back(q);
    }
}

// Draws the current question text inside a panel at the center of the screen
void SinglePlayerScreen::DrawQuestion(int sw, int sh) const {
    if (m_questionIdx >= TOTAL_QUESTIONS) return;
    const QuestionData& q = m_questions[m_questionIdx];

    float cx = (float)sw * 0.5f;
    float panelW = (float)sw * 0.60f;
    float panelY = (float)sh * 0.38f;

    Rectangle panel = { cx - panelW * 0.5f, panelY, panelW, 72.f };
    DrawRectangleRec(panel, { 10, 15, 40, 220 });
    DrawRectangleLinesEx(panel, 2.f, { 100, 140, 220, 200 });

    float fs = 20.f;
    Vector2 sz = MeasureTextEx(m_font, q.text.c_str(), fs, 1);
    while (sz.x > panelW - 30.f && fs > 12.f) {
        fs -= 1.f;
        sz = MeasureTextEx(m_font, q.text.c_str(), fs, 1);
    }
    DrawTextEx(m_font, q.text.c_str(),
               { cx - sz.x * 0.5f, panelY + (72.f - sz.y) * 0.5f },
               fs, 1, UI::C_TEXT_LIGHT);
}

// Draws the A/B/C/D answer buttons at the bottom and handles click selection
void SinglePlayerScreen::DrawAnswerChoices(int sw, int sh) {
    if (m_questionIdx >= TOTAL_QUESTIONS) return;
    const QuestionData& q = m_questions[m_questionIdx];

    float cx = (float)sw * 0.5f;
    int numChoices = (q.type == QuestionData::Type::TrueFalse) ? 2 : 4;

    float btnW = (float)sw * 0.22f;
    float btnH = 48.f;
    float gapX = 16.f;
    float gapY = 12.f;
    float gridW = btnW * 2 + gapX;
    float startX = cx - gridW * 0.5f;
    float startY = (float)sh - btnH * 2 - gapY - 24.f;

    const char* choiceLabels[] = { "A", "B", "C", "D" };
    const char* tfLabels[]     = { "TRUE", "FALSE" };

    bool locked = (m_phase != Phase::Playing) || (m_selectedAnswer >= 0);
    bool reveal = (m_phase == Phase::Correct || m_phase == Phase::Wrong);

    for (int i = 0; i < numChoices; ++i) {
        int col = (numChoices == 2) ? i : (i % 2);
        int row = (numChoices == 2) ? 0 : (i / 2);
        float bx = startX + col * (btnW + gapX);
        float by = startY + row * (btnH + gapY);

        if (numChoices == 2) {
            bx = cx - btnW - gapX * 0.5f + i * (btnW + gapX);
        }

        Rectangle r = { bx, by, btnW, btnH };

        bool selected  = (m_selectedAnswer == i);
        bool correct   = (i == q.correctIdx);
        bool eliminated = (m_fiftyFiftyActive &&
                          (i == m_eliminatedChoices[0] || i == m_eliminatedChoices[1]));

        if (eliminated) {
            DrawRectangleRec(r, { 10, 8, 15, 100 });
            DrawRectangleLinesEx(r, 1.f, { 40, 30, 20, 80 });
            continue;
        }

        Color bg, border, tc;
        if (reveal) {
            if (correct) {
                bg = { 15, 80, 15, 230 };
                border = { 60, 220, 60, 255 };
                tc = UI::C_TEXT_OK;
            } else if (selected) {
                bg = { 80, 15, 15, 230 };
                border = { 220, 60, 40, 255 };
                tc = UI::C_TEXT_ERR;
            } else {
                bg = { 10, 12, 30, 180 };
                border = { 50, 60, 90, 150 };
                tc = UI::C_TEXT_DIM;
            }
        } else if (selected) {
            bg = { 80, 55, 10, 220 };
            border = { 255, 180, 40, 255 };
            tc = UI::C_TEXT_GOLD;
        } else {
            bg = { 10, 15, 40, 200 };
            border = { 80, 110, 180, 200 };
            tc = UI::C_TEXT_LIGHT;
        }

        DrawRectangleRec(r, bg);
        DrawRectangleLinesEx(r, selected ? 2.f : 1.f, border);

        const char* prefix = (q.type == QuestionData::Type::TrueFalse)
                             ? tfLabels[i] : choiceLabels[i];
        std::string label;
        if (q.type == QuestionData::Type::TrueFalse) {
            label = prefix;
        } else {
            label = std::string(prefix) + ": " + q.choices[i];
        }

        float fs = 17.f;
        Vector2 sz = MeasureTextEx(m_font, label.c_str(), fs, 1);
        while (sz.x > btnW - 20.f && fs > 11.f) {
            fs -= 1.f;
            sz = MeasureTextEx(m_font, label.c_str(), fs, 1);
        }
        DrawTextEx(m_font, label.c_str(),
                   { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                   fs, 1, tc);

        try {
            if (!locked && !eliminated &&
                CheckCollisionPointRec(GetMousePosition(), r) &&
                IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                m_selectedAnswer = i;
            }
        } catch (...) {}
    }
}

// Draws the hint buttons (50:50, Phone, Audience) and Walk Away at the top center
void SinglePlayerScreen::DrawLifelines(int sw, int sh) {
    float btnSize = 50.f;
    float gap = 14.f;
    float totalW = btnSize * 3 + gap * 2;
    float startX = (float)sw * 0.5f - totalW * 0.5f;
    float llY = 14.f;

    bool canUse = (m_phase == Phase::Playing && m_selectedAnswer < 0);

    struct LifelineInfo {
        const char* label;
        const char* tooltip;
        bool used;
    };
    LifelineInfo lifelines[] = {
        { "50:50", "Remove two wrong answers", m_usedFiftyFifty },
        { "PHONE", "Phone a friend for advice", m_usedPhoneAFriend },
        { "AUDIENCE", "Ask the audience to vote", m_usedAskAudience }
    };

    for (int i = 0; i < 3; ++i) {
        float bx = startX + i * (btnSize + gap);
        Rectangle r = { bx, llY, btnSize, btnSize };

        if (lifelines[i].used) {
            DrawRectangleRec(r, { 15, 10, 8, 120 });
            DrawRectangleLinesEx(r, 1.f, { 60, 40, 20, 120 });
            Color dimTc = { 80, 60, 40, 140 };
            float fs = (i == 2) ? 11.f : 14.f;
            Vector2 sz = MeasureTextEx(m_font, lifelines[i].label, fs, 1);
            DrawTextEx(m_font, lifelines[i].label,
                       { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                       fs, 1, dimTc);
            DrawLineEx({ r.x + 4, r.y + 4 }, { r.x + r.width - 4, r.y + r.height - 4 },
                       2.f, { 200, 50, 30, 160 });
            DrawLineEx({ r.x + r.width - 4, r.y + 4 }, { r.x + 4, r.y + r.height - 4 },
                       2.f, { 200, 50, 30, 160 });
        } else if (canUse) {
            bool hov = CheckCollisionPointRec(GetMousePosition(), r);
            Color bg = hov ? Color{40, 55, 90, 230} : Color{15, 25, 55, 200};
            Color border = hov ? UI::C_TEXT_GOLD : Color{80, 110, 180, 200};
            DrawRectangleRec(r, bg);
            DrawRectangleLinesEx(r, hov ? 2.f : 1.f, border);
            Color tc = hov ? UI::C_TEXT_GOLD : UI::C_TEXT_LIGHT;
            float fs = (i == 2) ? 11.f : 14.f;
            Vector2 sz = MeasureTextEx(m_font, lifelines[i].label, fs, 1);
            DrawTextEx(m_font, lifelines[i].label,
                       { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                       fs, 1, tc);

            if (hov) {
                Vector2 tsz = MeasureTextEx(m_font, lifelines[i].tooltip, 13.f, 1);
                float tx = r.x + (r.width - tsz.x) * 0.5f;
                float ty = r.y + r.height + 6.f;
                DrawRectangle((int)tx - 6, (int)ty - 2, (int)tsz.x + 12, (int)tsz.y + 4,
                              { 10, 6, 2, 220 });
                DrawTextEx(m_font, lifelines[i].tooltip, { tx, ty }, 13.f, 1, UI::C_TEXT_DIM);
            }

            if (hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                try {
                    if (i == 0) {
                        m_usedFiftyFifty  = true;
                        m_fiftyFiftyActive = true;
                        const QuestionData& q = m_questions[m_questionIdx];
                        std::vector<int> wrongIndices;
                        int numC = (q.type == QuestionData::Type::TrueFalse) ? 2 : 4;
                        for (int j = 0; j < numC; ++j)
                            if (j != q.correctIdx) wrongIndices.push_back(j);
                        for (int j = (int)wrongIndices.size() - 1; j > 0; --j) {
                            int k = rand() % (j + 1);
                            std::swap(wrongIndices[j], wrongIndices[k]);
                        }
                        m_eliminatedChoices[0] = wrongIndices[0];
                        m_eliminatedChoices[1] = (wrongIndices.size() > 1) ? wrongIndices[1] : -1;
                    } else if (i == 1) {
                        m_usedPhoneAFriend = true;
                        m_showPhonePopup   = true;
                        const QuestionData& q = m_questions[m_questionIdx];
                        int friendAnswer;
                        if (rand() % 100 < 70) {
                            friendAnswer = q.correctIdx;
                        } else {
                            int numC = (q.type == QuestionData::Type::TrueFalse) ? 2 : 4;
                            friendAnswer = rand() % numC;
                        }
                        const char* letters[] = { "A", "B", "C", "D" };
                        if (q.type == QuestionData::Type::TrueFalse) {
                            m_phoneAdvice = "I'm fairly sure it's " +
                                            std::string(friendAnswer == 0 ? "True" : "False") + ".";
                        } else {
                            m_phoneAdvice = "I think it's " + std::string(letters[friendAnswer]) +
                                            ": " + q.choices[friendAnswer] + ".";
                        }
                    } else {
                        m_usedAskAudience   = true;
                        m_showAudiencePopup = true;
                        const QuestionData& q = m_questions[m_questionIdx];
                        int numC = (q.type == QuestionData::Type::TrueFalse) ? 2 : 4;
                        int correctPct = 50 + rand() % 21;
                        int remaining = 100 - correctPct;
                        for (int j = 0; j < 4; ++j) m_audienceBars[j] = 0;
                        m_audienceBars[q.correctIdx] = correctPct;
                        for (int j = 0; j < numC; ++j) {
                            if (j == q.correctIdx) continue;
                            int pct = (j < numC - 1 && j != q.correctIdx)
                                      ? (rand() % (remaining + 1)) : remaining;
                            m_audienceBars[j] = pct;
                            remaining -= pct;
                        }
                        if (remaining > 0) {
                            for (int j = numC - 1; j >= 0; --j) {
                                if (j != q.correctIdx) {
                                    m_audienceBars[j] += remaining;
                                    break;
                                }
                            }
                        }
                    }
                } catch (...) {}
            }
        } else {
            DrawRectangleRec(r, { 12, 15, 35, 160 });
            DrawRectangleLinesEx(r, 1.f, { 50, 60, 90, 120 });
            float fs = (i == 2) ? 11.f : 14.f;
            Vector2 sz = MeasureTextEx(m_font, lifelines[i].label, fs, 1);
            DrawTextEx(m_font, lifelines[i].label,
                       { r.x + (r.width - sz.x) * 0.5f, r.y + (r.height - sz.y) * 0.5f },
                       fs, 1, UI::C_TEXT_DIM);
        }
    }

    float llCX = (float)sw * 0.5f;
    float waW = 130.f, waH = 36.f;
    Rectangle waRect = { llCX - waW * 0.5f, llY + btnSize + 20.f, waW, waH };
    if (canUse) {
        if (UI::Button(waRect, "WALK AWAY", m_font, 16.f)) {
            try {
                m_winnings = (m_questionIdx > 0) ? PRIZE_VALUES[m_questionIdx - 1] : 0;
                if (m_questionIdx > SAFETY_NET_2)
                    m_winnings = std::max(m_winnings, PRIZE_VALUES[SAFETY_NET_2]);
                else if (m_questionIdx > SAFETY_NET_1)
                    m_winnings = std::max(m_winnings, PRIZE_VALUES[SAFETY_NET_1]);
                m_phase = Phase::WalkAway;
                m_timer = 0.f;
            } catch (...) {}
        }
    }
}

// Shows the Phone a Friend popup with the simulated friend's advice
void SinglePlayerScreen::DrawPhoneAFriendPopup(int sw, int sh) {
    if (!m_showPhonePopup) return;

    DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 140 });

    float popW = 500.f, popH = 200.f;
    float cx = (float)sw * 0.5f;
    float popX = cx - popW * 0.5f;
    float popY = (float)sh * 0.5f - popH * 0.5f;
    Rectangle panel = { popX, popY, popW, popH };
    UI::DrawPanel(panel);

    UI::LabelShadow("PHONE A FRIEND", cx, popY + 16.f, 28.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 120.f, popY + 52.f, 240.f);

    float fs = 18.f;
    Vector2 sz = MeasureTextEx(m_font, m_phoneAdvice.c_str(), fs, 1);
    while (sz.x > popW - 40.f && fs > 12.f) {
        fs -= 1.f;
        sz = MeasureTextEx(m_font, m_phoneAdvice.c_str(), fs, 1);
    }
    DrawTextEx(m_font, m_phoneAdvice.c_str(),
               { cx - sz.x * 0.5f, popY + 75.f }, fs, 1, UI::C_TEXT_LIGHT);

    float btnW = 100.f, btnH = 36.f;
    if (UI::Button({ cx - btnW * 0.5f, popY + popH - btnH - 16.f, btnW, btnH },
                   "OK", m_font, 18.f)) {
        m_showPhonePopup = false;
    }
}

// Shows the Ask the Audience popup with a bar chart of simulated vote percentages
void SinglePlayerScreen::DrawAskAudiencePopup(int sw, int sh) {
    if (!m_showAudiencePopup) return;

    DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 140 });

    float popW = 460.f, popH = 300.f;
    float cx = (float)sw * 0.5f;
    float popX = cx - popW * 0.5f;
    float popY = (float)sh * 0.5f - popH * 0.5f;
    Rectangle panel = { popX, popY, popW, popH };
    UI::DrawPanel(panel);

    UI::LabelShadow("ASK THE AUDIENCE", cx, popY + 14.f, 26.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 120.f, popY + 48.f, 240.f);

    const QuestionData& q = m_questions[m_questionIdx];
    int numC = (q.type == QuestionData::Type::TrueFalse) ? 2 : 4;
    float barW = 50.f;
    float barMaxH = 130.f;
    float barGap = 30.f;
    float barsW = numC * barW + (numC - 1) * barGap;
    float barStartX = cx - barsW * 0.5f;
    float barBaseY = popY + popH - 70.f;

    const char* labels[] = { "A", "B", "C", "D" };
    const char* tfLabels[] = { "T", "F" };

    for (int i = 0; i < numC; ++i) {
        float bx = barStartX + i * (barW + barGap);
        float barH = barMaxH * ((float)m_audienceBars[i] / 100.f);

        DrawRectangle((int)bx, (int)(barBaseY - barH), (int)barW, (int)barH,
                      { 60, 120, 200, 200 });
        DrawRectangleLinesEx({ bx, barBaseY - barH, barW, barH }, 1.f,
                             { 100, 160, 240, 200 });

        std::string pct = std::to_string(m_audienceBars[i]) + "%";
        UI::LabelC(pct, bx + barW * 0.5f, barBaseY - barH - 18.f, 14.f,
                   UI::C_TEXT_LIGHT, m_font);

        const char* lbl = (q.type == QuestionData::Type::TrueFalse) ? tfLabels[i] : labels[i];
        UI::LabelC(lbl, bx + barW * 0.5f, barBaseY + 4.f, 16.f, UI::C_TEXT_GOLD, m_font);
    }

    float btnW = 100.f, btnH = 36.f;
    if (UI::Button({ cx - btnW * 0.5f, popY + popH - btnH - 14.f, btnW, btnH },
                   "OK", m_font, 18.f)) {
        m_showAudiencePopup = false;
    }
}

// Draws the player's character sprite and its platform in the bottom-left corner
void SinglePlayerScreen::DrawCharacter(int sw, int sh) const {
    float platY = (float)sh * 0.92f;
    float charCX = 120.f;
    CharacterRenderer::Draw(m_charLayers, charCX, platY, 130.f);

    float platW = 100.f;
    Rectangle plat = { charCX - platW * 0.5f, platY, platW, 14.f };
    DrawRectangleRec(plat, { 40, 30, 15, 200 });
    DrawRectangleLinesEx(plat, 1.f, UI::C_BORDER_DARK);
}

void SinglePlayerScreen::DrawCorrectOverlay(int sw, int sh) const {
    if (m_phase != Phase::Correct) return;

    float cx = (float)sw * 0.5f;
    float cy = (float)sh * 0.48f;

    float alpha = std::min(1.f, m_timer / 0.3f);
    Color col = { 90, 220, 90, (unsigned char)(alpha * 255) };

    std::string msg = "CORRECT!";
    float fs = 48.f;
    Vector2 sz = MeasureTextEx(m_font, msg.c_str(), fs, 1);

    DrawTextEx(m_font, msg.c_str(), { cx - sz.x * 0.5f + 3, cy + 3 }, fs, 1,
               { 0, 0, 0, (unsigned char)(alpha * 180) });
    DrawTextEx(m_font, msg.c_str(), { cx - sz.x * 0.5f, cy }, fs, 1, col);
}

void SinglePlayerScreen::DrawWrongOverlay(int sw, int sh) const {
    if (m_phase != Phase::Wrong) return;

    float cx = (float)sw * 0.5f;
    float cy = (float)sh * 0.46f;

    float alpha = std::min(1.f, m_timer / 0.3f);
    Color col = { 255, 70, 50, (unsigned char)(alpha * 255) };

    std::string msg = "WRONG!";
    float fs = 48.f;
    Vector2 sz = MeasureTextEx(m_font, msg.c_str(), fs, 1);

    DrawTextEx(m_font, msg.c_str(), { cx - sz.x * 0.5f + 3, cy + 3 }, fs, 1,
               { 0, 0, 0, (unsigned char)(alpha * 180) });
    DrawTextEx(m_font, msg.c_str(), { cx - sz.x * 0.5f, cy }, fs, 1, col);

    const QuestionData& q = m_questions[m_questionIdx];
    const char* labels[] = { "A", "B", "C", "D" };
    std::string correctMsg;
    if (q.type == QuestionData::Type::TrueFalse) {
        correctMsg = "The answer was " + std::string(q.correctIdx == 0 ? "True" : "False");
    } else {
        correctMsg = "The answer was " + std::string(labels[q.correctIdx]) +
                     ": " + q.choices[q.correctIdx];
    }
    UI::LabelC(correctMsg, cx, cy + 52.f, 18.f,
               { 200, 180, 140, (unsigned char)(alpha * 220) }, m_font);
}

bool SinglePlayerScreen::DrawGameOver(int sw, int sh) {
    float cx = (float)sw * 0.5f;
    if (m_bgGameOver.id != 0)
        DrawTexturePro(m_bgGameOver,
            { 0, 0, (float)m_bgGameOver.width, (float)m_bgGameOver.height },
            { 0, 0, (float)sw, (float)sh }, { 0, 0 }, 0.f, WHITE);
    else
        DrawRectangle(0, 0, sw, sh, { 6, 4, 8, 255 });

    bool isWin = (m_questionIdx >= TOTAL_QUESTIONS);

    if (isWin) {
        UI::LabelShadow("CONGRATULATIONS!", cx, (float)sh * 0.10f, 52.f,
                        UI::C_TEXT_GOLD, m_font);
        UI::LabelC("YOU WON!", cx, (float)sh * 0.10f + 60.f, 36.f,
                   UI::C_TEXT_GOLD, m_font);
    } else if (m_phase == Phase::WalkAway) {
        UI::LabelShadow("YOU WALKED AWAY", cx, (float)sh * 0.12f, 46.f,
                        UI::C_TEXT_GOLD, m_font);
    } else {
        UI::LabelShadow("GAME OVER", cx, (float)sh * 0.12f, 52.f,
                        UI::C_TEXT_ERR, m_font);
    }

    UI::Divider(cx - 200.f, (float)sh * 0.30f, 400.f);

    std::string qMsg = "Questions answered: " + std::to_string(m_questionIdx) +
                       " / " + std::to_string(TOTAL_QUESTIONS);
    UI::LabelC(qMsg, cx, (float)sh * 0.40f, 18.f, UI::C_TEXT_DIM, m_font);

    DrawCharacter(sw, sh);

    float btnH = 50.f, btnW = 200.f;
    return UI::Button({ cx - btnW * 0.5f, (float)sh - btnH - 30.f, btnW, btnH },
                      "MAIN MENU", m_font, 22.f);
}

// Updates game phase logic each frame and draws the full gameplay scene
ScreenID SinglePlayerScreen::Tick(float dt) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    if (m_phase == Phase::GameOver || m_phase == Phase::WalkAway) {
        if (DrawGameOver(sw, sh))
            return ScreenID::MainMenu;
        return ScreenID::SinglePlayerGame;
    }

    switch (m_phase) {
    case Phase::Playing:
        if (m_selectedAnswer >= 0) {
            m_phase = Phase::Thinking;
            m_timer = 0.f;
        }
        break;

    case Phase::Thinking:
        m_timer += dt;
        if (m_timer >= m_thinkTime) {
            const QuestionData& q = m_questions[m_questionIdx];
            if (m_selectedAnswer == q.correctIdx) {
                m_phase = Phase::Correct;
            } else {
                if (m_questionIdx > SAFETY_NET_2)
                    m_winnings = PRIZE_VALUES[SAFETY_NET_2];
                else if (m_questionIdx > SAFETY_NET_1)
                    m_winnings = PRIZE_VALUES[SAFETY_NET_1];
                else
                    m_winnings = 0;
                m_phase = Phase::Wrong;
            }
            m_timer = 0.f;
        }
        break;

    case Phase::Correct:
        m_timer += dt;
        if (m_timer >= m_correctTime) {
            m_questionIdx++;
            if (m_questionIdx >= TOTAL_QUESTIONS) {
                m_winnings = PRIZE_VALUES[TOTAL_QUESTIONS - 1];
                m_phase = Phase::GameOver;
            } else {
                m_phase = Phase::Playing;
                m_selectedAnswer = -1;
                m_fiftyFiftyActive = false;
                m_eliminatedChoices[0] = m_eliminatedChoices[1] = -1;
                m_showPhonePopup = false;
                m_showAudiencePopup = false;
            }
            m_timer = 0.f;
        }
        break;

    case Phase::Wrong:
        m_timer += dt;
        if (m_timer >= m_wrongTime) {
            m_phase = Phase::GameOver;
            m_timer = 0.f;
        }
        break;

    default:
        break;
    }

    DrawBackground(sw, sh);
    DrawCharacter(sw, sh);
    DrawLifelines(sw, sh);
    DrawQuestion(sw, sh);
    DrawAnswerChoices(sw, sh);
    DrawCorrectOverlay(sw, sh);
    DrawWrongOverlay(sw, sh);

    DrawPhoneAFriendPopup(sw, sh);
    DrawAskAudiencePopup(sw, sh);

    return ScreenID::SinglePlayerGame;
}
