#pragma once
#include "raylib.h"
#include "enum.h"
#include "ParallaxBackground.h"
#include "UIHelpers.h"
#include "../../BLL/include/NetworkManager.h"
#include "../../BLL/include/AuthService.h"
#include "../../BLL/include/CharacterService.h"
#include <string>

// ─────────────────────────────────────────────
//  PRESENTATION LAYER – MainMenuScreen
//  Tick() = update + draw in one BeginDrawing pass.
// ─────────────────────────────────────────────
class MainMenuScreen {
public:
    // Represents a discoverable hosted game (populated by the network layer).
    struct GameEntry {
        std::string gameName;
        std::string hostName;
    };

    void Load(const std::string& assetRoot, Font font);
    void Unload();

    // Called between BeginDrawing / EndDrawing.
    // Returns the next ScreenID to navigate to.
    ScreenID Tick(float dt, AuthService& auth, CharacterService& charSvc, NetworkManager& net);

    const std::string& GetJoinedGameName() const { return m_joinedGameName; }

private:
    // Returns true when a game was successfully joined (navigates to JoinLobby).
    bool DrawJoinPopup(int sw, int sh, NetworkManager& net);

    ParallaxBackground m_bg;
    Texture2D          m_castle = {};
    Font               m_font   = {};
    std::string        m_assetRoot;

    bool        m_showJoinPopup   = false;
    bool        m_discoveryActive = false;
    int         m_joinScroll      = 0;
    std::string m_connectError;
    std::string m_joinedGameName;

};
