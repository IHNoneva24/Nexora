#include "../include/LoginScreen.h"

void LoginScreen::Load(const std::string& assetRoot, Font font) {
    m_font = font;
    m_bg.Load(assetRoot +
        "/GandalfHardcore Background layers/Normal BG");
    Reset();
}
void LoginScreen::Unload() { m_bg.Unload(); }
void LoginScreen::Reset() {
    m_username = m_password = m_errorMsg = m_successMsg = "";
    m_focus = 0; m_msgTimer = 0.f; m_loggedIn = false;
}

ScreenID LoginScreen::Tick(float dt, AuthService& auth) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    float cx = (float)sw * .5f;

    m_bg.Update(dt);
    if (m_msgTimer > 0.f) m_msgTimer -= dt;

    // Auto-navigate after successful login display
    if (m_loggedIn && m_msgTimer <= 0.f) { Reset(); return ScreenID::MainMenu; }

    // Layout
    const float pW = 430.f, pH = 380.f;
    const float pX = cx - pW*.5f, pY = (float)sh*.5f - pH*.5f;
    const float fW = pW - 60.f, fX = pX + 30.f, fH = 44.f;
    const Rectangle rUser = { fX, pY + 105.f, fW, fH };
    const Rectangle rPass = { fX, pY + 190.f, fW, fH };

    // ── Draw BG ───────────────────────────────────────────────────────────────
    m_bg.Draw(sw, sh);

    // ── Panel ─────────────────────────────────────────────────────────────────
    UI::DrawPanel({ pX, pY, pW, pH }, 3);
    UI::LabelC("LOGIN", cx, pY + 18.f, 30.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(pX + 30.f, pY + 58.f, pW - 60.f);

    // Field labels
    UI::Label("Username", fX, pY + 84.f,  16.f, UI::C_TEXT_DIM, m_font);
    UI::Label("Password", fX, pY + 169.f, 16.f, UI::C_TEXT_DIM, m_font);

    // Click-to-focus
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if      (CheckCollisionPointRec(m, rUser)) m_focus = 0;
        else if (CheckCollisionPointRec(m, rPass)) m_focus = 1;
    }
    if (IsKeyPressed(KEY_TAB)) m_focus = (m_focus + 1) % 2;

    // Input fields
    bool enterU = UI::InputField(rUser, m_username, m_focus == 0, false, m_font);
    bool enterP = UI::InputField(rPass, m_password, m_focus == 1, true,  m_font);
    if (enterU) m_focus = 1;

    // Login action
    auto doLogin = [&]() {
        if (m_loggedIn) return;
        std::string err = auth.Login(m_username, m_password);
        if (err.empty()) {
            m_successMsg = "Welcome back, " + auth.GetUsername() + "!";
            m_errorMsg   = "";
            m_msgTimer   = 2.f;
            m_loggedIn   = true;
        } else {
            m_errorMsg   = err;
            m_successMsg = "";
            m_msgTimer   = 3.f;
        }
    };
    if (enterP) doLogin();

    // Buttons
    const float bW = 180.f, bH = 46.f;
    const float bY = pY + pH - 64.f;

    if (UI::Button({ cx - bW - 10.f, bY, bW, bH }, "LOGIN",    m_font, 22.f)) doLogin();
    if (UI::Button({ cx + 10.f,      bY, bW, bH }, "REGISTER", m_font, 22.f)) {
        Reset(); return ScreenID::Register;
    }
    if (UI::Button({ pX, pY - 48.f, 120.f, 36.f }, "< BACK", m_font, 18.f)) {
        Reset(); return ScreenID::MainMenu;
    }

    // Messages
    UI::Toast(m_errorMsg,   cx, pY + 252.f, 16.f, UI::C_TEXT_ERR, m_font, m_msgTimer);
    UI::Toast(m_successMsg, cx, pY + 252.f, 16.f, UI::C_TEXT_OK,  m_font, m_msgTimer);

    // Hint
    UI::LabelC("Don't have an account? Click REGISTER",
               cx, pY + pH - 18.f, 13.f, UI::C_TEXT_DIM, m_font);

    return ScreenID::Login;
}
