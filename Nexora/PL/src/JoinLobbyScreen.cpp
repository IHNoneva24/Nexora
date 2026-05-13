#include "../include/JoinLobbyScreen.h"
#include "../include/CharacterRenderer.h"

void JoinLobbyScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot = assetRoot;
    m_font      = font;
}

void JoinLobbyScreen::Unload() {
    CharacterRenderer::UnloadLayers(m_charLayers);
    CharacterRenderer::UnloadLayers(m_remoteCharLayers);
}

void JoinLobbyScreen::Enter(AuthService& auth, CharacterService& charSvc,
                             const std::string& gameName, NetworkManager& net) {
    m_gameName       = gameName;
    m_myUsername     = auth.GetUsername();
    m_remoteUsername.clear();

    CharacterRenderer::UnloadLayers(m_charLayers);
    CharacterRenderer::UnloadLayers(m_remoteCharLayers);

    CharacterData data;
    if (charSvc.Load(auth.GetUserId(), data)) {
        m_myCharData = data;
        m_charLayers = CharacterRenderer::LoadLayers(data, m_assetRoot);
        net.SendCharacterData(data);
        net.SendUsername(m_myUsername);
    }
}

void JoinLobbyScreen::DrawPlatform(float cx, float y, float w, float h) {
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

ScreenID JoinLobbyScreen::Tick(float dt, NetworkManager& net) {
    (void)dt;
    int   sw = GetScreenWidth(), sh = GetScreenHeight();
    float cx = (float)sw * .5f;

    DrawRectangle(0, 0, sw, sh, { 8, 5, 2, 245 });

    // Title
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

    // Detect host disconnect — return to main menu
    if (net.PollDisconnected()) {
        net.Shutdown();
        return ScreenID::MainMenu;
    }

    // Receive host's character data when it arrives
    CharacterData remoteData;
    if (net.PollRemoteCharacterData(remoteData)) {
        m_remoteCharData = remoteData;
        CharacterRenderer::UnloadLayers(m_remoteCharLayers);
        m_remoteCharLayers = CharacterRenderer::LoadLayers(remoteData, m_assetRoot);
    }

    // Receive host's username
    std::string incomingUser;
    if (net.PollRemoteUsername(incomingUser)) {
        m_remoteUsername = incomingUser;
    }

    // Left — host character
    DrawPlatform(leftCX, platY, platW, platH);
    if (!m_remoteCharLayers.empty()) {
        CharacterRenderer::Draw(m_remoteCharLayers, leftCX, platY, charH);
    } else {
        // Waiting for host character data — faint silhouette
        float gH = charH * .9f, gW = CharacterRenderer::FRAME_W / CharacterRenderer::FRAME_H * gH;
        DrawRectangle((int)(leftCX - gW * .5f), (int)(platY - gH), (int)gW, (int)gH,
                      { 180, 160, 100, 60 });
    }
    std::string hostLabel = m_remoteUsername.empty() ? "Host" : m_remoteUsername;
    UI::LabelC(hostLabel, leftCX, platY + platH + 8.f, 16.f, UI::C_TEXT_DIM, m_font);

    // Right — our character
    DrawPlatform(rightCX, platY, platW, platH);
    CharacterRenderer::Draw(m_charLayers, rightCX, platY, charH);
    UI::LabelC(m_myUsername, rightCX, platY + platH + 8.f, 16.f, UI::C_TEXT_DIM, m_font);

    DrawLineEx({ cx, areaTop + 10.f }, { cx, areaBottom - 10.f }, 1, { 80,60,20,120 });

    // Waiting message
    UI::LabelC("Waiting for host to start...", cx, (float)sh * .88f, 20.f, UI::C_TEXT_DIM, m_font);

    // Disconnect
    float btnH = 50.f;
    if (UI::Button({ 20.f, (float)sh - btnH - 20.f, 160.f, btnH }, "DISCONNECT", m_font, 20.f)) {
        net.Shutdown();
        return ScreenID::MainMenu;
    }

    // Host pressed START GAME — both players transition to question creation
    if (net.PollStartGame())
        return ScreenID::QuestionCreate;

    return ScreenID::JoinLobby;
}
