#include "../include/MainMenuScreen.h"

void MainMenuScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot = assetRoot;
    m_font      = font;

    std::string bgPath = assetRoot + "/main-menu-background.png";
    m_bgImage = LoadTexture(bgPath.c_str());
    if (m_bgImage.id != 0)
        SetTextureFilter(m_bgImage, TEXTURE_FILTER_BILINEAR);
}

void MainMenuScreen::Unload() {
    if (m_bgImage.id != 0) UnloadTexture(m_bgImage);
}

// ── Tick ──────────────────────────────────────────────────────────────────────
ScreenID MainMenuScreen::Tick(float dt, AuthService& auth, CharacterService& charSvc,
                              NetworkManager& net) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    float cx = (float)sw * .5f;

    // ── Draw background ───────────────────────────────────────────────────────
    if (m_bgImage.id != 0) {
        float scaleX = (float)sw / (float)m_bgImage.width;
        float scaleY = (float)sh / (float)m_bgImage.height;
        float scale  = (scaleX > scaleY) ? scaleX : scaleY;
        float drawW  = (float)m_bgImage.width * scale;
        float drawH  = (float)m_bgImage.height * scale;
        DrawTexturePro(m_bgImage,
                       { 0, 0, (float)m_bgImage.width, (float)m_bgImage.height },
                       { (sw - drawW) * 0.5f, (sh - drawH) * 0.5f, drawW, drawH },
                       { 0, 0 }, 0.f, WHITE);
    }

    // ── Title ─────────────────────────────────────────────────────────────────
    const char* title = "NEXORA";
    float titleY = (float)sh * .18f;
    UI::LabelShadow(title, cx, titleY, 66.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 170.f, titleY + 74.f, 340.f);

    // ── Main buttons ──────────────────────────────────────────────────────────
    float bW = 264.f, bH = 54.f;
    float bX = cx - bW * .5f;
    float sY = (float)sh * .40f;
    float gap = 56.f;

    ScreenID next = ScreenID::MainMenu;

    bool popupOpen = m_showSettings || m_showJoinPopup;
    bool loggedIn = auth.IsLoggedIn();
    bool hasChar  = loggedIn && charSvc.HasCharacter(auth.GetUserId());
    bool canPlay  = loggedIn && hasChar;

    Rectangle spRect   = { bX, sY,           bW, bH };
    Rectangle hostRect = { bX, sY + gap,     bW, bH };
    Rectangle joinRect = { bX, sY + gap*2,   bW, bH };
    Rectangle charRect = { bX, sY + gap*3,   bW, bH };
    Rectangle exitRect = { bX, sY + gap*4,   bW, bH };

    // SINGLE PLAYER
    if (canPlay) {
        if (UI::Button(spRect, "SINGLE PLAYER", m_font, 24.f) && !popupOpen)
            next = ScreenID::SinglePlayerGame;
    } else {
        UI::ButtonDisabled(spRect, "SINGLE PLAYER", m_font, 24.f);
        if (!popupOpen) {
            if (!loggedIn)
                UI::Tooltip(spRect, "Sign in to play", m_font);
            else
                UI::Tooltip(spRect, "Create a character first", m_font);
        }
    }

    // HOST GAME
    if (canPlay) {
        if (UI::Button(hostRect, "HOST GAME", m_font, 24.f) && !popupOpen)
            next = ScreenID::HostLobby;
    } else {
        UI::ButtonDisabled(hostRect, "HOST GAME", m_font, 24.f);
        if (!popupOpen) {
            if (!loggedIn)
                UI::Tooltip(hostRect, "Sign in to play", m_font);
            else
                UI::Tooltip(hostRect, "Create a character first", m_font);
        }
    }

    // JOIN GAME
    if (canPlay) {
        if (UI::Button(joinRect, "JOIN GAME", m_font, 24.f) && !popupOpen) {
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
        if (!popupOpen) {
            if (!loggedIn)
                UI::Tooltip(joinRect, "Sign in to play", m_font);
            else
                UI::Tooltip(joinRect, "Create a character first", m_font);
        }
    }

    // CHARACTER
    if (loggedIn) {
        if (UI::Button(charRect, "CHARACTER", m_font, 24.f) && !popupOpen)
            next = ScreenID::CharacterCreate;
    } else {
        UI::ButtonDisabled(charRect, "CHARACTER", m_font, 24.f);
        if (!popupOpen) UI::Tooltip(charRect, "Sign in first", m_font);
    }

    if (UI::Button(exitRect, "EXIT", m_font, 24.f) && !popupOpen)
        next = ScreenID::Exit;

    // ── Sign In / Username (top right) ────────────────────────────────────────
    std::string siLabel = auth.IsLoggedIn() ? auth.GetUsername() : "Sign In";
    float siW = 160.f, siH = 38.f;
    Rectangle siRect = { (float)sw - siW - 16.f, 22.f, siW, siH };

    if (UI::Button(siRect, siLabel, m_font, 18.f) && !popupOpen) {
        if (auth.IsLoggedIn()) { net.StopSessionBroadcast(); auth.Logout(); }
        else                   next = ScreenID::Login;
    }

    // Tooltip
    if (!popupOpen && auth.IsLoggedIn() && CheckCollisionPointRec(GetMousePosition(), siRect)) {
        DrawRectangle((int)siRect.x - 60, (int)(siRect.y + siH + 6), 140, 22, {10,6,2,210});
        UI::Label("Click to log out",
                  siRect.x - 56.f, siRect.y + siH + 8.f,
                  14.f, UI::C_TEXT_DIM, m_font);
    }

    // ── Settings button (top-left) ─────────────────────────────────────────────
    Rectangle settingsBtn = { 16.f, 22.f, 100.f, 38.f };
    if (UI::Button(settingsBtn, "SETTINGS", m_font, 16.f) && !m_showJoinPopup)
        m_showSettings = !m_showSettings;

    // ── Settings popup ──────────────────────────────────────────────────────────
    if (m_showSettings) {
        DrawRectangle(0, 0, sw, sh, { 0, 0, 0, 140 });
        float popW = 400.f, popH = 320.f;
        float popX = (float)sw * 0.5f - popW * 0.5f;
        float popY = (float)sh * 0.5f - popH * 0.5f;
        Rectangle panel = { popX, popY, popW, popH };
        UI::DrawPanel(panel);

        UI::LabelShadow("SETTINGS", (float)sw * 0.5f, popY + 16.f, 30.f, UI::C_TEXT_GOLD, m_font);
        UI::Divider((float)sw * 0.5f - 100.f, popY + 54.f, 200.f);

        UI::LabelC("WINDOW SIZE", (float)sw * 0.5f, popY + 72.f, 16.f, UI::C_TEXT_DIM, m_font);

        struct SizeOption { const char* label; int w; int h; };
        SizeOption sizes[] = {
            { "1280 x 720",  1280, 720 },
            { "1366 x 768",  1366, 768 },
            { "1600 x 900",  1600, 900 },
            { "1920 x 1080", 1920, 1080 },
            { "MAXIMIZED",   0,    0 }
        };

        float btnW2 = 200.f, btnH2 = 36.f;
        float startY2 = popY + 96.f;
        for (int i = 0; i < 5; ++i) {
            Rectangle r = { (float)sw * 0.5f - btnW2 * 0.5f, startY2 + i * 42.f, btnW2, btnH2 };
            if (UI::Button(r, sizes[i].label, m_font, 16.f)) {
                if (sizes[i].w == 0) {
                    MaximizeWindow();
                } else {
                    if (IsWindowMaximized()) RestoreWindow();
                    SetWindowSize(sizes[i].w, sizes[i].h);
                    SetWindowPosition(
                        (GetMonitorWidth(GetCurrentMonitor()) - sizes[i].w) / 2,
                        (GetMonitorHeight(GetCurrentMonitor()) - sizes[i].h) / 2);
                }
                m_showSettings = false;
            }
        }

        // Close button
        Rectangle closeBtn = { popX + popW - 38.f, popY + 10.f, 28.f, 28.f };
        if (UI::Button(closeBtn, "X", m_font, 16.f)) m_showSettings = false;
        if (IsKeyPressed(KEY_ESCAPE)) m_showSettings = false;
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
        UI::LabelC("Searching for games on your network...", cx, listY + rowH * 1.2f, 18.f, UI::C_TEXT_DIM, m_font);
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
                try {
                    std::string err;
                    if (net.Connect(g.hostIP, g.port, err)) {
                        m_joinedGameName  = g.gameName;
                        m_showJoinPopup   = false;
                        m_discoveryActive = false;
                        net.StopDiscovery();
                        return true;
                    } else {
                        m_connectError = err;
                    }
                } catch (...) {
                    m_connectError = "An unexpected error occurred.";
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
