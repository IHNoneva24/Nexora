#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
<<<<<<< HEAD
#include "../../BLL/include/NetworkManager.h"
#include "../../BLL/include/GameContext.h"
=======
#include "NetworkManager.h"
#include "GameContext.h"
>>>>>>> a71f47d (Add game)
#include "../../BLL/include/AuthService.h"
#include "../../BLL/include/CharacterService.h"
#include <string>
#include <vector>

class JoinLobbyScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    void Enter(AuthService& auth, CharacterService& charSvc,
               const std::string& gameName, NetworkManager& net);
    ScreenID Tick(float dt, NetworkManager& net);

    void FillGameContext(GameContext& ctx) const {
        ctx.myChar     = m_myCharData;
        ctx.remoteChar = m_remoteCharData;
    }

private:
    void DrawPlatform(float cx, float y, float w, float h);

    Font        m_font       = {};
    Texture2D   m_background = {};
    std::string m_assetRoot;
    std::string m_gameName;
    std::string m_myUsername;      // our logged-in username
    std::string m_remoteUsername;  // host's username (received over network)

    CharacterData m_myCharData     = {};
    CharacterData m_remoteCharData = {};

    std::vector<Texture2D> m_charLayers;        // our own character
    std::vector<Texture2D> m_remoteCharLayers;  // host's character (received over network)
};
