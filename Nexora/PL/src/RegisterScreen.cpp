#include "../include/RegisterScreen.h"

void RegisterScreen::Load(const std::string& assetRoot, Font font) {
    m_font = font;
    m_bg.Load(assetRoot +
        "/GandalfHardcore Background layers/Normal BG");
    Reset();
}
void RegisterScreen::Unload() { m_bg.Unload(); }
void RegisterScreen::Reset() {
    m_username = m_password = m_confirm = m_errorMsg = m_successMsg = "";
    m_focus = 0; m_msgTimer = 0.f; m_done = false;
}

ScreenID RegisterScreen::Tick(float dt, AuthService& auth) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    float cx = (float)sw * .5f;

    m_bg.Update(dt);
    if (m_msgTimer > 0.f) m_msgTimer -= dt;

    if (m_done && m_msgTimer <= 0.f) { Reset(); return ScreenID::Login; }

    // Layout
    const float pW = 430.f, pH = 440.f;
    const float pX = cx - pW*.5f, pY = (float)sh*.5f - pH*.5f;
    const float fW = pW - 60.f, fX = pX + 30.f, fH = 44.f;
    const Rectangle rUser = { fX, pY + 105.f, fW, fH };
    const Rectangle rPass = { fX, pY + 195.f, fW, fH };
    const Rectangle rConf = { fX, pY + 285.f, fW, fH };

    // ── Draw ──────────────────────────────────────────────────────────────────
    m_bg.Draw(sw, sh);
    UI::DrawPanel({ pX, pY, pW, pH }, 3);
    UI::LabelC("REGISTER", cx, pY + 18.f, 30.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(pX + 30.f, pY + 58.f, pW - 60.f);

    UI::Label("Username",         fX, pY + 84.f,  16.f, UI::C_TEXT_DIM, m_font);
    UI::Label("Password",         fX, pY + 174.f, 16.f, UI::C_TEXT_DIM, m_font);
    UI::Label("Confirm Password", fX, pY + 264.f, 16.f, UI::C_TEXT_DIM, m_font);

    // Click-to-focus
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if      (CheckCollisionPointRec(m, rUser)) m_focus = 0;
        else if (CheckCollisionPointRec(m, rPass)) m_focus = 1;
        else if (CheckCollisionPointRec(m, rConf)) m_focus = 2;
    }
    if (IsKeyPressed(KEY_TAB)) m_focus = (m_focus + 1) % 3;

    bool enterU = UI::InputField(rUser, m_username, m_focus == 0, false, m_font);
    bool enterP = UI::InputField(rPass, m_password, m_focus == 1, true,  m_font);
    bool enterC = UI::InputField(rConf, m_confirm,  m_focus == 2, true,  m_font);

    if (enterU) m_focus = 1;
    if (enterP) m_focus = 2;

    // Register action
    auto doRegister = [&]() {
        try {
            if (m_done) return;
            std::string err = auth.Register(m_username, m_password, m_confirm);
            if (err.empty()) {
                m_successMsg = "Account created! You can now log in.";
                m_errorMsg   = "";
                m_msgTimer   = 2.5f;
                m_done       = true;
            } else {
                m_errorMsg   = err;
                m_successMsg = "";
                m_msgTimer   = 3.5f;
            }
        } catch (...) {
            m_errorMsg   = "An unexpected error occurred.";
            m_successMsg = "";
            m_msgTimer   = 3.5f;
        }
    };
    if (enterC) doRegister();

    // Buttons
    const float bW = 180.f, bH = 46.f;
    const float bY = pY + pH - 64.f;

    if (UI::Button({ cx - bW - 10.f, bY, bW, bH }, "CREATE ACCOUNT", m_font, 20.f)) doRegister();
    if (UI::Button({ cx + 10.f,      bY, bW, bH }, "LOGIN",          m_font, 22.f)) {
        Reset(); return ScreenID::Login;
    }
    if (UI::Button({ pX, pY - 48.f, 120.f, 36.f }, "< BACK", m_font, 18.f)) {
        Reset(); return ScreenID::MainMenu;
    }

    UI::Toast(m_errorMsg,   cx, pY + pH - 100.f, 15.f, UI::C_TEXT_ERR, m_font, m_msgTimer);
    UI::Toast(m_successMsg, cx, pY + pH - 100.f, 15.f, UI::C_TEXT_OK,  m_font, m_msgTimer);

    return ScreenID::Register;
}
