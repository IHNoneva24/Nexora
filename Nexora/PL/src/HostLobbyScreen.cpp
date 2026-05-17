#include "../include/HostLobbyScreen.h"
#include "../include/CharacterRenderer.h"

void HostLobbyScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot  = assetRoot;
    m_font       = font;
    m_background = LoadTexture((assetRoot + "/background3.png").c_str());
}

void HostLobbyScreen::Unload() {
    UnloadTexture(m_background);
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
    if (m_background.id != 0)
        DrawTexturePro(m_background,
            { 0, 0, (float)m_background.width, (float)m_background.height },
            { 0, 0, (float)sw, (float)sh }, { 0, 0 }, 0.f, WHITE);
    else
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
            try {
                net.StartHost(m_gameName, m_hostName);
                m_charDataSent = false;
                m_state = SubState::Lobby;
            } catch (...) {}
        }
    } else {
        UI::ButtonDisabled({ startX + btnW + gap, btnY, btnW, btnH }, "CREATE", m_font, 22.f);
    }

    return false;
}

// ── Lobby screen ──────────────────────────────────────────────────────────────

ScreenID HostLobbyScreen::DrawLobbyScreen(int sw, int sh, NetworkManager& net) {
    float cx = (float)sw * .5f;
    if (m_background.id != 0)
        DrawTexturePro(m_background,
            { 0, 0, (float)m_background.width, (float)m_background.height },
            { 0, 0, (float)sw, (float)sh }, { 0, 0 }, 0.f, WHITE);
    else
        DrawRectangle(0, 0, sw, sh, { 8, 5, 2, 255 });

    UI::LabelShadow(m_gameName, cx, 28.f, 42.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 180.f, 80.f, 360.f);

    float platY   = (float)sh * 0.68f;
    float charH   = (float)sh * 0.38f;
    float leftCX  = (float)sw * 0.20f;
    float rightCX = (float)sw * 0.80f;

    // Helper: draw name badge above a character's head
    auto DrawNameTag = [&](const std::string& name, float cx, float charTopY) {
        const float fs = 16.f;
        Vector2 sz  = MeasureTextEx(m_font, name.c_str(), fs, 1.f);
        float   px  = 8.f, py = 3.f;
        float   ty  = charTopY - sz.y - py * 2.f - 6.f;
        Rectangle r = { cx - sz.x * .5f - px, ty, sz.x + px * 2.f, sz.y + py * 2.f };
        DrawRectangleRec(r, { 0, 0, 0, 150 });
        DrawRectangleLinesEx(r, 1.f, { 100, 80, 30, 180 });
        UI::LabelC(name, cx, ty + (r.height - sz.y) * .5f, fs, UI::C_TEXT_LIGHT, m_font);
    };

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
    {
        bool  isFemale = (m_myCharData.gender == 1);
        float yOff     = isFemale ? -14.f : 0.f;
        CharacterRenderer::Draw(m_charLayers, leftCX, platY, charH, false, isFemale ? 2 : -1, 5.f, yOff);
        DrawNameTag(m_hostName, leftCX, platY + yOff - charH);
    }

    // Right — client slot
    if (!joined) {
        Color ghost = { 180, 160, 100, 80 };
        float gH = charH * .9f, gW = CharacterRenderer::FRAME_W / CharacterRenderer::FRAME_H * gH;
        DrawRectangle((int)(rightCX - gW * .5f), (int)(platY - gH), (int)gW, (int)gH, ghost);
        UI::LabelC("Waiting for other player...", rightCX, platY - charH - 28.f,
                   18.f, UI::C_TEXT_DIM, m_font);
    } else {
        bool  rFemale = (m_remoteCharData.gender == 1);
        float rYOff   = rFemale ? -14.f : 0.f;
        if (!m_remoteCharLayers.empty()) {
            CharacterRenderer::Draw(m_remoteCharLayers, rightCX, platY, charH,
                                    false, rFemale ? 2 : -1, 5.f, rYOff);
        } else {
            float gH = charH * .9f, gW = CharacterRenderer::FRAME_W / CharacterRenderer::FRAME_H * gH;
            DrawRectangle((int)(rightCX - gW * .5f), (int)(platY - gH), (int)gW, (int)gH,
                          { 180, 160, 100, 60 });
        }
        std::string clientLabel = m_remoteUsername.empty() ? "Player 2" : m_remoteUsername;
        DrawNameTag(clientLabel, rightCX, platY + rYOff - charH);
        UI::LabelC(clientLabel + " connected!", rightCX, platY - charH - 28.f,
                   18.f, UI::C_TEXT_OK, m_font);
    }

    DrawLineEx({ cx, 110.f }, { cx, (float)sh - 100.f }, 1, { 80,60,20,120 });

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
