#include "../include/MainMenuScreen.h"
#include <cstdlib>
#include <cmath>

void MainMenuScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot = assetRoot;
    m_font      = font;

    m_bg.Load(assetRoot +
        "/GandalfHardcore Background layers/Normal BG");

    std::string castlePath =
        assetRoot +
        "/GandalfHardcore Background layers/Normal BG/Background Castle .png";
    m_castle = LoadTexture(castlePath.c_str());
    SetTextureFilter(m_castle, TEXTURE_FILTER_POINT);

    InitParticles(1280, 720);
}

void MainMenuScreen::Unload() {
    m_bg.Unload();
    UnloadTexture(m_castle);
}

// ── Particles ─────────────────────────────────────────────────────────────────
void MainMenuScreen::InitParticles(int sw, int sh) {
    m_particles.clear();
    for (int i = 0; i < 28; ++i) {
        Particle p;
        p.x     = (float)(rand() % sw);
        p.y     = (float)(rand() % sh);
        p.speed = 12.f + (rand() % 28);
        p.alpha = 50.f  + (rand() % 90);
        p.size  = 10.f  + (rand() % 10);
        p.sym   = rand() % 6;
        m_particles.push_back(p);
    }
}

void MainMenuScreen::UpdateParticles(float dt, int sw, int sh) {
    for (auto& p : m_particles) {
        p.y     -= p.speed * dt;
        p.alpha += dt * 30.f;
        if (p.y < -20.f) {
            p.y     = (float)(sh + 10);
            p.x     = (float)(rand() % sw);
            p.alpha = 50.f;
        }
        if (p.alpha > 130.f) p.alpha = 50.f;
    }
}

void MainMenuScreen::DrawParticles() const {
    const char* syms[] = { "*", "+", "o", "x", "~", "#" };
    for (auto& p : m_particles) {
        Color c = { 200, 160, 50, (unsigned char)p.alpha };
        DrawTextEx(m_font, syms[p.sym], { p.x, p.y }, p.size, 1, c);
    }
}

// ── Tick ──────────────────────────────────────────────────────────────────────
ScreenID MainMenuScreen::Tick(float dt, AuthService& auth, CharacterService& charSvc) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    float cx = (float)sw * .5f;

    // Update
    m_bg.Update(dt);
    UpdateParticles(dt, sw, sh);

    // ── Draw background ───────────────────────────────────────────────────────
    m_bg.Draw(sw, sh);

    if (m_castle.id != 0) {
        float scale = (float)sh / (float)m_castle.height * 0.68f;
        float cw    = (float)m_castle.width * scale;
        float ch    = (float)m_castle.height * scale;
        DrawTextureEx(m_castle,
                      { (sw - cw) * .5f, (float)sh - ch },
                      0.f, scale, { 255,255,255,170 });
    }

    DrawParticles();

    // ── Title ─────────────────────────────────────────────────────────────────
    const char* title = "REALM OF HEROES";
    float titleY = (float)sh * .18f;
    UI::LabelShadow(title, cx, titleY, 66.f, UI::C_TEXT_GOLD, m_font);
    UI::LabelC("~ A Fantasy Adventure ~", cx, titleY + 74.f, 22.f, UI::C_TEXT_DIM, m_font);
    UI::Divider(cx - 170.f, titleY + 104.f, 340.f);

    // ── Main buttons ──────────────────────────────────────────────────────────
    float bW = 264.f, bH = 54.f;
    float bX = cx - bW * .5f;
    float sY = (float)sh * .46f;
    float gap = 64.f;

    ScreenID next = ScreenID::MainMenu;

    bool loggedIn    = auth.IsLoggedIn();
    bool hasChar     = loggedIn && charSvc.HasCharacter(auth.GetUserId());
    bool canPlay     = loggedIn && hasChar;

    Rectangle startRect = { bX, sY,         bW, bH };
    Rectangle charRect  = { bX, sY + gap,   bW, bH };
    Rectangle htpRect   = { bX, sY + gap*2, bW, bH };
    Rectangle exitRect  = { bX, sY + gap*3, bW, bH };

    // START
    if (canPlay) {
        if (UI::Button(startRect, "START", m_font, 24.f)) next = ScreenID::Game;
    } else {
        UI::ButtonDisabled(startRect, "START", m_font, 24.f);
        if (!loggedIn)
            UI::Tooltip(startRect, "Sign in to play", m_font);
        else
            UI::Tooltip(startRect, "Create a character first", m_font);
    }

    // CHARACTER
    if (loggedIn) {
        if (UI::Button(charRect, "CHARACTER", m_font, 24.f)) next = ScreenID::CharacterCreate;
    } else {
        UI::ButtonDisabled(charRect, "CHARACTER", m_font, 24.f);
        UI::Tooltip(charRect, "Sign in first", m_font);
    }

    if (UI::Button(htpRect,  "HOW TO PLAY", m_font, 24.f)) next = ScreenID::HowToPlay;
    if (UI::Button(exitRect, "EXIT",         m_font, 24.f)) next = ScreenID::Exit;

    // ── Sign In / Username (top right) ────────────────────────────────────────
    std::string siLabel = auth.IsLoggedIn() ? auth.GetUsername() : "Sign In";
    float siW = 160.f, siH = 38.f;
    Rectangle siRect = { (float)sw - siW - 16.f, 22.f, siW, siH };

    if (UI::Button(siRect, siLabel, m_font, 18.f)) {
        if (auth.IsLoggedIn()) auth.Logout();
        else                   next = ScreenID::Login;
    }

    // Tooltip
    if (auth.IsLoggedIn() && CheckCollisionPointRec(GetMousePosition(), siRect)) {
        DrawRectangle((int)siRect.x - 60, (int)(siRect.y + siH + 6), 140, 22, {10,6,2,210});
        UI::Label("Click to log out",
                  siRect.x - 56.f, siRect.y + siH + 8.f,
                  14.f, UI::C_TEXT_DIM, m_font);
    }

    return next;
}
