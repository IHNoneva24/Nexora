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

class HostLobbyScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    void Enter(AuthService& auth, CharacterService& charSvc);
    ScreenID Tick(float dt, NetworkManager& net);

private:
    enum class SubState { Naming, Lobby };

    bool     DrawNamingScreen(int sw, int sh, NetworkManager& net); // returns true → go back
    ScreenID DrawLobbyScreen (int sw, int sh, NetworkManager& net);
    void DrawPlatform(float cx, float y, float w, float h);

    Font        m_font     = {};
    std::string m_assetRoot;
    std::string   m_hostName;

    SubState      m_state      = SubState::Naming;
    std::string   m_gameName;
    bool          m_nameActive = true;

    CharacterData          m_myCharData     = {};
    bool                   m_charDataSent   = false;
    std::vector<Texture2D> m_charLayers;       // host's own layers
    std::vector<Texture2D> m_remoteCharLayers; // client's layers (received over network)
};
