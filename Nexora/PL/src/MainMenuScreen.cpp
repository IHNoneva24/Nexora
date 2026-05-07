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
ScreenID MainMenuScreen::Tick(float dt, AuthService& auth, CharacterService& charSvc,
                              NetworkManager& net) {
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

    bool loggedIn = auth.IsLoggedIn();
    bool hasChar  = loggedIn && charSvc.HasCharacter(auth.GetUserId());
    bool canPlay  = loggedIn && hasChar;

    Rectangle hostRect = { bX, sY,         bW, bH };
    Rectangle joinRect = { bX, sY + gap,   bW, bH };
    Rectangle charRect = { bX, sY + gap*2, bW, bH };
    Rectangle htpRect  = { bX, sY + gap*3, bW, bH };
    Rectangle exitRect = { bX, sY + gap*4, bW, bH };

    // HOST GAME
    if (canPlay) {
        if (UI::Button(hostRect, "HOST GAME", m_font, 24.f)) next = ScreenID::HostLobby;
    } else {
        UI::ButtonDisabled(hostRect, "HOST GAME", m_font, 24.f);
        if (!loggedIn)
            UI::Tooltip(hostRect, "Sign in to play", m_font);
        else
            UI::Tooltip(hostRect, "Create a character first", m_font);
    }

    // JOIN GAME
    if (canPlay) {
        if (UI::Button(joinRect, "JOIN GAME", m_font, 24.f)) {
            m_showJoinPopup   = true;
            m_joinScroll      = 0;
            m_connectError    = "";
            if (!m_discoveryActive) {
                net.StartDiscovery();
                m_discoveryActive = true;
            }
        }
    } else {
        UI::ButtonDisabled(joinRect, "JOIN GAME", m_font, 24.f);
        if (!loggedIn)
            UI::Tooltip(joinRect, "Sign in to play", m_font);
        else
            UI::Tooltip(joinRect, "Create a character first", m_font);
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
        if (auth.IsLoggedIn()) { net.StopSessionBroadcast(); auth.Logout(); }
        else                   next = ScreenID::Login;
    }

    // Tooltip
    if (auth.IsLoggedIn() && CheckCollisionPointRec(GetMousePosition(), siRect)) {
        DrawRectangle((int)siRect.x - 60, (int)(siRect.y + siH + 6), 140, 22, {10,6,2,210});
        UI::Label("Click to log out",
                  siRect.x - 56.f, siRect.y + siH + 8.f,
                  14.f, UI::C_TEXT_DIM, m_font);
    }

    // ── Join-game popup (drawn on top of everything) ──────────────────────────
    if (m_showJoinPopup) {
        if (DrawJoinPopup(sw, sh, net))
            next = ScreenID::JoinLobby;
    }

    return next;
}

// ── Join popup ────────────────────────────────────────────────────────────────
bool MainMenuScreen::DrawJoinPopup(int sw, int sh, NetworkManager& net) {
    DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 160 });

    float popW = 600.f, popH = 460.f;
    float popX = (float)sw * .5f - popW * .5f;
    float popY = (float)sh * .5f - popH * .5f;
    Rectangle panel = { popX, popY, popW, popH };
    UI::DrawPanel(panel);

    float cx = (float)sw * .5f;
    UI::LabelShadow("JOIN A GAME", cx, popY + 18.f, 32.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 150.f, popY + 58.f, 300.f);

    // Close (X)
    Rectangle closeRect = { popX + popW - 38.f, popY + 10.f, 28.f, 28.f };
    auto closePopup = [&]() {
        m_showJoinPopup   = false;
        m_connectError    = "";
        if (m_discoveryActive) { net.StopDiscovery(); m_discoveryActive = false; }
    };
    if (UI::Button(closeRect, "X", m_font, 16.f)) { closePopup(); return false; }
    if (IsKeyPressed(KEY_ESCAPE))                  { closePopup(); return false; }

    // Error message
    if (!m_connectError.empty())
        UI::LabelC(m_connectError, cx, popY + 66.f, 14.f, UI::C_TEXT_ERR, m_font);

    // ── Game list ─────────────────────────────────────────────────────────────
    const auto& games = net.GetDiscoveredGames();
    float listX  = popX + 20.f;
    float listY  = popY + 88.f;
    float listW  = popW - 40.f;
    float rowH   = 56.f;
    int   visRows = 5;

    if (games.empty()) {
        float pulse = (float)(0.6 + 0.4 * sin(GetTime() * 1.8));
        Color dim = { 160, 140, 100, (unsigned char)(pulse * 200) };
        UI::LabelC("Searching for games on your network...", cx, listY + rowH * 1.2f, 18.f, dim, m_font);
        UI::LabelC("Ask your friend to host a game first.", cx, listY + rowH * 1.2f + 28.f,
                   15.f, UI::C_TEXT_DIM, m_font);
    } else {
        int maxScroll = (int)games.size() - visRows;
        if (maxScroll < 0) maxScroll = 0;
        if (m_joinScroll > maxScroll) m_joinScroll = maxScroll;

        if (m_joinScroll > 0) {
            if (UI::Button({ cx - 16.f, listY - 24.f, 32.f, 20.f }, "^", m_font, 14.f))
                m_joinScroll--;
        }

        for (int i = 0; i < visRows; ++i) {
            int idx = m_joinScroll + i;
            if (idx >= (int)games.size()) break;

            const auto& g = games[idx];
            Rectangle row = { listX, listY + rowH * i, listW, rowH - 6.f };

            bool hov = CheckCollisionPointRec(GetMousePosition(), row);
            DrawRectangleRec(row, hov ? UI::C_BTN_HOVER : UI::C_BTN_NORMAL);
            DrawRectangleLinesEx(row, 1, hov ? UI::C_TEXT_GOLD : UI::C_BORDER_DARK);

            UI::Label(g.gameName, row.x + 14.f, row.y + 8.f,  20.f, UI::C_TEXT_GOLD,  m_font);
            UI::Label("Host: " + g.hostName, row.x + 14.f, row.y + 30.f, 14.f, UI::C_TEXT_DIM, m_font);

            Rectangle joinBtn = { row.x + row.width - 90.f, row.y + 10.f, 80.f, rowH - 26.f };
            if (UI::Button(joinBtn, "JOIN", m_font, 16.f)) {
                std::string err;
                if (net.Connect(g.hostIP, g.port, err)) {
                    m_joinedGameName  = g.gameName;
                    m_showJoinPopup   = false;
                    m_discoveryActive = false; // discovery socket closed by Connect internals? No — stop manually
                    // UDP discovery socket is separate; stop it now that we have a TCP connection
                    net.StopDiscovery();
                    return true;
                } else {
                    m_connectError = err;
                }
            }
        }

        if (m_joinScroll < maxScroll) {
            if (UI::Button({ cx - 16.f, listY + rowH * visRows + 4.f, 32.f, 20.f }, "v", m_font, 14.f))
                m_joinScroll++;
        }
    }

    return false;
}
