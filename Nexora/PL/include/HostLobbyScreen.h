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

class HostLobbyScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    void Enter(AuthService& auth, CharacterService& charSvc);
    ScreenID Tick(float dt, NetworkManager& net);

    void FillGameContext(GameContext& ctx) const {
        ctx.myChar     = m_myCharData;
        ctx.remoteChar = m_remoteCharData;
    }

private:
    enum class SubState { Naming, Lobby };

    bool     DrawNamingScreen(int sw, int sh, NetworkManager& net); // returns true → go back
    ScreenID DrawLobbyScreen (int sw, int sh, NetworkManager& net);
    void DrawPlatform(float cx, float y, float w, float h);

    Font        m_font       = {};
    Texture2D   m_background = {};
    std::string m_assetRoot;
    std::string   m_hostName;

    SubState      m_state      = SubState::Naming;
    std::string   m_gameName;
    bool          m_nameActive = true;

    CharacterData          m_myCharData     = {};
    CharacterData          m_remoteCharData = {};
    bool                   m_charDataSent   = false;
    std::string            m_remoteUsername;    // received from client
    std::vector<Texture2D> m_charLayers;       // host's own layers
    std::vector<Texture2D> m_remoteCharLayers; // client's layers (received over network)
};
