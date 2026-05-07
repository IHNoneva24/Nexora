#include "../include/JoinLobbyScreen.h"
#include "../include/CharacterRenderer.h"
<<<<<<< HEAD

void JoinLobbyScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot  = assetRoot;
    m_font       = font;
    m_background = LoadTexture((assetRoot + "/background3.png").c_str());
}

void JoinLobbyScreen::Unload() {
    UnloadTexture(m_background);
=======
#include <cmath>

void JoinLobbyScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot = assetRoot;
    m_font      = font;
}

void JoinLobbyScreen::Unload() {
>>>>>>> 3fe0748 (Add multiplayer)
    CharacterRenderer::UnloadLayers(m_charLayers);
    CharacterRenderer::UnloadLayers(m_remoteCharLayers);
}

void JoinLobbyScreen::Enter(AuthService& auth, CharacterService& charSvc,
                             const std::string& gameName, NetworkManager& net) {
<<<<<<< HEAD
<<<<<<< HEAD
    m_gameName       = gameName;
    m_myUsername     = auth.GetUsername();
    m_remoteUsername.clear();
=======
    m_gameName = gameName;
>>>>>>> 3fe0748 (Add multiplayer)
=======
    m_gameName       = gameName;
    m_myUsername     = auth.GetUsername();
    m_remoteUsername.clear();
>>>>>>> 2e1294f (Fix bugs)

    CharacterRenderer::UnloadLayers(m_charLayers);
    CharacterRenderer::UnloadLayers(m_remoteCharLayers);

    CharacterData data;
    if (charSvc.Load(auth.GetUserId(), data)) {
<<<<<<< HEAD
        m_myCharData = data;
        m_charLayers = CharacterRenderer::LoadLayers(data, m_assetRoot);
        net.SendCharacterData(data);
        net.SendUsername(m_myUsername);
=======
        m_charLayers = CharacterRenderer::LoadLayers(data, m_assetRoot);
        // Send our character data and username to the host immediately
        net.SendCharacterData(data);
<<<<<<< HEAD
>>>>>>> 3fe0748 (Add multiplayer)
=======
        net.SendUsername(m_myUsername);
>>>>>>> 2e1294f (Fix bugs)
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

<<<<<<< HEAD
    if (m_background.id != 0)
        DrawTexturePro(m_background,
            { 0, 0, (float)m_background.width, (float)m_background.height },
            { 0, 0, (float)sw, (float)sh }, { 0, 0 }, 0.f, WHITE);
    else
        DrawRectangle(0, 0, sw, sh, { 8, 5, 2, 255 });
=======
    DrawRectangle(0, 0, sw, sh, { 8, 5, 2, 245 });
>>>>>>> 3fe0748 (Add multiplayer)

    // Title
    UI::LabelShadow(m_gameName, cx, 28.f, 42.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 180.f, 80.f, 360.f);

<<<<<<< HEAD
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
=======
    float areaTop    = 110.f;
    float areaBottom = (float)sh - 100.f;
    float areaH      = areaBottom - areaTop;
    float platW      = (float)sw * .28f;
    float platH      = 30.f;
    float platY      = areaTop + areaH * .72f;
    float charH      = areaH * .50f;
    float leftCX     = (float)sw * .25f;
    float rightCX    = (float)sw * .75f;
>>>>>>> 3fe0748 (Add multiplayer)

    // Detect host disconnect — return to main menu
    if (net.PollDisconnected()) {
        net.Shutdown();
        return ScreenID::MainMenu;
    }

    // Receive host's character data when it arrives
    CharacterData remoteData;
    if (net.PollRemoteCharacterData(remoteData)) {
<<<<<<< HEAD
        m_remoteCharData = remoteData;
=======
>>>>>>> 3fe0748 (Add multiplayer)
        CharacterRenderer::UnloadLayers(m_remoteCharLayers);
        m_remoteCharLayers = CharacterRenderer::LoadLayers(remoteData, m_assetRoot);
    }

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 2e1294f (Fix bugs)
    // Receive host's username
    std::string incomingUser;
    if (net.PollRemoteUsername(incomingUser)) {
        m_remoteUsername = incomingUser;
    }

<<<<<<< HEAD
    // Left — host character
    {
        bool  lFemale = (m_remoteCharData.gender == 1);
        float lYOff   = lFemale ? -14.f : 0.f;
        if (!m_remoteCharLayers.empty()) {
            CharacterRenderer::Draw(m_remoteCharLayers, leftCX, platY, charH,
                                    false, lFemale ? 2 : -1, 5.f, lYOff);
        } else {
            float gH = charH * .9f, gW = CharacterRenderer::FRAME_W / CharacterRenderer::FRAME_H * gH;
            DrawRectangle((int)(leftCX - gW * .5f), (int)(platY - gH), (int)gW, (int)gH,
                          { 180, 160, 100, 60 });
        }
        std::string hostLabel = m_remoteUsername.empty() ? "Host" : m_remoteUsername;
        DrawNameTag(hostLabel, leftCX, platY + lYOff - charH);
    }

    // Right — our character
    {
        bool  isFemale = (m_myCharData.gender == 1);
        float yOff     = isFemale ? -14.f : 0.f;
        CharacterRenderer::Draw(m_charLayers, rightCX, platY, charH,
                                false, isFemale ? 2 : -1, 5.f, yOff);
        DrawNameTag(m_myUsername, rightCX, platY + yOff - charH);
    }

    DrawLineEx({ cx, 110.f }, { cx, (float)sh - 100.f }, 1, { 80,60,20,120 });

    // Waiting message
    UI::LabelC("Waiting for host to start...", cx, (float)sh * .88f, 20.f, UI::C_TEXT_DIM, m_font);
=======
=======
>>>>>>> 2e1294f (Fix bugs)
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
    float pulse = (float)(0.65 + 0.35 * sin(GetTime() * 1.6));
    Color waitCol = { 200, 180, 100, (unsigned char)(pulse * 230) };
    UI::LabelC("Waiting for host to start...", cx, (float)sh * .88f, 20.f, waitCol, m_font);
>>>>>>> 3fe0748 (Add multiplayer)

    // Disconnect
    float btnH = 50.f;
    if (UI::Button({ 20.f, (float)sh - btnH - 20.f, 160.f, btnH }, "DISCONNECT", m_font, 20.f)) {
        net.Shutdown();
        return ScreenID::MainMenu;
    }

<<<<<<< HEAD
    // Host pressed START GAME — both players transition to question creation
    if (net.PollStartGame())
        return ScreenID::QuestionCreate;
=======
    // Host pressed START GAME — both players transition
    if (net.PollStartGame())
        return ScreenID::Game;
>>>>>>> 3fe0748 (Add multiplayer)

    return ScreenID::JoinLobby;
}
