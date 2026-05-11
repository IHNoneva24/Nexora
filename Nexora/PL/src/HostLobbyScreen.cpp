#include "../include/HostLobbyScreen.h"
#include "../include/CharacterRenderer.h"
#include <cmath>

void HostLobbyScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot = assetRoot;
    m_font      = font;
}

void HostLobbyScreen::Unload() {
    CharacterRenderer::UnloadLayers(m_charLayers);
    CharacterRenderer::UnloadLayers(m_remoteCharLayers);
}

void HostLobbyScreen::Enter(AuthService& auth, CharacterService& charSvc) {
    m_state        = SubState::Naming;
    m_gameName     = "";
    m_nameActive   = true;
    m_hostName     = auth.GetUsername();
    m_charDataSent = false;

    CharacterRenderer::UnloadLayers(m_charLayers);
    CharacterRenderer::UnloadLayers(m_remoteCharLayers);

    m_myCharData = {};
    CharacterData data;
    if (charSvc.Load(auth.GetUserId(), data)) {
        m_myCharData = data;
        m_charLayers = CharacterRenderer::LoadLayers(data, m_assetRoot);
    }
}

// ── Platform helper ───────────────────────────────────────────────────────────

void HostLobbyScreen::DrawPlatform(float cx, float y, float w, float h) {
    Rectangle r = { cx - w * .5f, y, w, h };
    DrawRectangleRec(r, { 55, 45, 30, 255 });
    DrawRectangleLinesEx(r, 2, UI::C_BORDER);
    DrawLineEx({ r.x + 4, r.y + 2 }, { r.x + r.width - 4, r.y + 2 }, 1, { 200,180,120,80 });
    for (int i = 1; i <= 2; ++i) {
        float ly = r.y + (r.height / 3.f) * i;
        DrawLineEx({ r.x, ly }, { r.x + r.width, ly }, 1, { 30,24,12,180 });
    }
    DrawLineEx({ cx, r.y }, { cx, r.y + r.height }, 1, { 30,24,12,180 });
}

// ── Naming screen ─────────────────────────────────────────────────────────────

bool HostLobbyScreen::DrawNamingScreen(int sw, int sh, NetworkManager& net) {
    float cx = (float)sw * .5f;
    DrawRectangle(0, 0, sw, sh, { 10, 6, 2, 240 });

    float panelW = 480.f, panelH = 230.f;
    Rectangle panel = { cx - panelW * .5f, (float)sh * .35f, panelW, panelH };
    UI::DrawPanel(panel);

    UI::LabelShadow("NAME YOUR GAME", cx, panel.y + 22.f, 34.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 140.f, panel.y + 66.f, 280.f);

    Rectangle field = { panel.x + 30.f, panel.y + 86.f, panelW - 60.f, 46.f };
    if (CheckCollisionPointRec(GetMousePosition(), field) &&
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        m_nameActive = true;

    bool enter = UI::InputField(field, m_gameName, m_nameActive, false, m_font, 22.f, 28);

    float btnY  = panel.y + panelH - 62.f;
    float btnH  = 44.f, btnW = 140.f, gap = 16.f;
    float totalW = btnW * 2 + gap;
    float startX = cx - totalW * .5f;

    // BACK
    if (UI::Button({ startX, btnY, btnW, btnH }, "BACK", m_font, 22.f) ||
        IsKeyPressed(KEY_ESCAPE))
        return true;

    // CREATE
    if (!m_gameName.empty()) {
        if (UI::Button({ startX + btnW + gap, btnY, btnW, btnH }, "CREATE", m_font, 22.f) || enter) {
            net.StartHost(m_gameName, m_hostName);
            m_charDataSent = false;
            m_state = SubState::Lobby;
        }
    } else {
        UI::ButtonDisabled({ startX + btnW + gap, btnY, btnW, btnH }, "CREATE", m_font, 22.f);
    }

    return false;
}

// ── Lobby screen ──────────────────────────────────────────────────────────────

ScreenID HostLobbyScreen::DrawLobbyScreen(int sw, int sh, NetworkManager& net) {
    float cx = (float)sw * .5f;
    DrawRectangle(0, 0, sw, sh, { 8, 5, 2, 245 });

    UI::LabelShadow(m_gameName, cx, 28.f, 42.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 180.f, 80.f, 360.f);

    float areaTop    = 110.f;
    float areaBottom = (float)sh - 100.f;
    float areaH      = areaBottom - areaTop;
    float platW      = (float)sw * .28f;
    float platH      = 30.f;
    float platY      = areaTop + areaH * .72f;
    float charH      = areaH * .50f;
    float leftCX     = (float)sw * .25f;
    float rightCX    = (float)sw * .75f;

    // Detect client disconnect — go back to waiting state
    if (net.PollDisconnected()) {
        CharacterRenderer::UnloadLayers(m_remoteCharLayers);
        m_charDataSent = false;
        m_remoteUsername.clear();
        // net already resumed broadcasting inside HandleDisconnect
    }

    bool joined = net.IsClientConnected();

    // When client first connects: exchange character data and username
    if (joined && !m_charDataSent) {
        net.SendCharacterData(m_myCharData);
        net.SendUsername(m_hostName);
        m_charDataSent = true;
    }

    // Receive client's username
    std::string incomingUser;
    if (net.PollRemoteUsername(incomingUser)) {
        m_remoteUsername = incomingUser;
    }

    // Receive client's character data as soon as it arrives
    CharacterData remoteData;
    if (net.PollRemoteCharacterData(remoteData)) {
        m_remoteCharData = remoteData;
        CharacterRenderer::UnloadLayers(m_remoteCharLayers);
        m_remoteCharLayers = CharacterRenderer::LoadLayers(remoteData, m_assetRoot);
    }

    // Left — host character
    DrawPlatform(leftCX, platY, platW, platH);
    CharacterRenderer::Draw(m_charLayers, leftCX, platY, charH);
    UI::LabelC(m_hostName, leftCX, platY + platH + 8.f, 16.f, UI::C_TEXT_DIM, m_font);

    // Right — client slot
    DrawPlatform(rightCX, platY, platW, platH);
    if (!joined) {
        float pulse = (float)(0.55 + 0.25 * sin(GetTime() * 2.0));
        Color ghost = { 180, 160, 100, (unsigned char)(pulse * 110) };
        float gH = charH * .9f, gW = CharacterRenderer::FRAME_W / CharacterRenderer::FRAME_H * gH;
        DrawRectangle((int)(rightCX - gW * .5f), (int)(platY - gH), (int)gW, (int)gH, ghost);
        UI::LabelC("Waiting for other player...", rightCX, platY - charH - 28.f,
                   18.f, UI::C_TEXT_DIM, m_font);
    } else {
        if (!m_remoteCharLayers.empty()) {
            CharacterRenderer::Draw(m_remoteCharLayers, rightCX, platY, charH);
        } else {
            // Character data not yet received — draw a faint silhouette
            float gH = charH * .9f, gW = CharacterRenderer::FRAME_W / CharacterRenderer::FRAME_H * gH;
            DrawRectangle((int)(rightCX - gW * .5f), (int)(platY - gH), (int)gW, (int)gH,
                          { 180, 160, 100, 60 });
        }
        std::string clientLabel = m_remoteUsername.empty() ? "Player 2" : m_remoteUsername;
        UI::LabelC(clientLabel, rightCX, platY + platH + 8.f, 16.f, UI::C_TEXT_DIM, m_font);
        UI::LabelC(clientLabel + " connected!", rightCX, platY - charH - 28.f,
                   18.f, UI::C_TEXT_OK, m_font);
    }

    DrawLineEx({ cx, areaTop + 10.f }, { cx, areaBottom - 10.f }, 1, { 80,60,20,120 });

    // Bottom buttons
    float btnH = 50.f, btnW = 180.f;
    float btnY = (float)sh - btnH - 20.f;

    if (UI::Button({ 20.f, btnY, 120.f, btnH }, "BACK", m_font, 22.f)) {
        net.StopHost();
        CharacterRenderer::UnloadLayers(m_remoteCharLayers);
        m_charDataSent = false;
        m_state = SubState::Naming;
    }

    if (joined) {
        if (UI::Button({ (float)sw - btnW - 20.f, btnY, btnW, btnH }, "START GAME", m_font, 20.f)) {
            net.SendStartGame();
            return ScreenID::QuestionCreate;
        }
    } else {
        UI::ButtonDisabled({ (float)sw - btnW - 20.f, btnY, btnW, btnH }, "START GAME", m_font, 20.f);
    }

    return ScreenID::HostLobby;
}

// ── Tick ──────────────────────────────────────────────────────────────────────

ScreenID HostLobbyScreen::Tick(float dt, NetworkManager& net) {
    (void)dt;
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    if (m_state == SubState::Naming) {
        if (DrawNamingScreen(sw, sh, net)) {
            net.Shutdown();
            return ScreenID::MainMenu;
        }
        return ScreenID::HostLobby;
    } else {
        return DrawLobbyScreen(sw, sh, net);
    }
}
