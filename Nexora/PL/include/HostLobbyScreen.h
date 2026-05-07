#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
<<<<<<< HEAD
<<<<<<< HEAD
#include "../../BLL/include/NetworkManager.h"
#include "../../BLL/include/GameContext.h"
=======
#include "NetworkManager.h"
#include "GameContext.h"
>>>>>>> a71f47d (Add game)
#include "../../BLL/include/AuthService.h"
#include "../../BLL/include/CharacterService.h"
=======
#include "NetworkManager.h"
#include "../../BLL/include/AuthService.h"
#include "../../BLL/include/CharacterService.h"
#include "../../DL/include/CharacterData.h"
>>>>>>> 3fe0748 (Add multiplayer)
#include <string>
#include <vector>

class HostLobbyScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    void Enter(AuthService& auth, CharacterService& charSvc);
    ScreenID Tick(float dt, NetworkManager& net);

<<<<<<< HEAD
    void FillGameContext(GameContext& ctx) const {
        ctx.myChar     = m_myCharData;
        ctx.remoteChar = m_remoteCharData;
    }

=======
>>>>>>> 3fe0748 (Add multiplayer)
private:
    enum class SubState { Naming, Lobby };

    bool     DrawNamingScreen(int sw, int sh, NetworkManager& net); // returns true → go back
    ScreenID DrawLobbyScreen (int sw, int sh, NetworkManager& net);
    void DrawPlatform(float cx, float y, float w, float h);

<<<<<<< HEAD
    Font        m_font       = {};
    Texture2D   m_background = {};
=======
    Font        m_font     = {};
>>>>>>> 3fe0748 (Add multiplayer)
    std::string m_assetRoot;
    std::string   m_hostName;

    SubState      m_state      = SubState::Naming;
    std::string   m_gameName;
    bool          m_nameActive = true;

    CharacterData          m_myCharData     = {};
<<<<<<< HEAD
    CharacterData          m_remoteCharData = {};
    bool                   m_charDataSent   = false;
    std::string            m_remoteUsername;    // received from client
<<<<<<< HEAD
=======
    bool                   m_charDataSent   = false;
>>>>>>> 3fe0748 (Add multiplayer)
=======
>>>>>>> 2e1294f (Fix bugs)
    std::vector<Texture2D> m_charLayers;       // host's own layers
    std::vector<Texture2D> m_remoteCharLayers; // client's layers (received over network)
};
