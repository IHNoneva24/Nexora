#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
#include "NetworkManager.h"
#include "../../BLL/include/AuthService.h"
#include "../../BLL/include/CharacterService.h"
#include "../../DL/include/CharacterData.h"
#include <string>
#include <vector>

class JoinLobbyScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    // Pass net so we can immediately send our character data on enter.
    void Enter(AuthService& auth, CharacterService& charSvc,
               const std::string& gameName, NetworkManager& net);
    ScreenID Tick(float dt, NetworkManager& net);

private:
    void DrawPlatform(float cx, float y, float w, float h);

    Font        m_font     = {};
    std::string m_assetRoot;
    std::string m_gameName;
    std::string m_myUsername;      // our logged-in username
    std::string m_remoteUsername;  // host's username (received over network)

    std::vector<Texture2D> m_charLayers;        // our own character
    std::vector<Texture2D> m_remoteCharLayers;  // host's character (received over network)
};
