#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
#include "../../BLL/include/NetworkManager.h"
#include "../../BLL/include/AuthService.h"
#include <string>

// ─────────────────────────────────────────────
//  PRESENTATION LAYER – LoginScreen
// ─────────────────────────────────────────────
class LoginScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();
    void Reset();

    ScreenID Tick(float dt, AuthService& auth, NetworkManager& net);

private:
    Texture2D m_bgImage = {};
    Font      m_font   = {};

    std::string m_username;
    std::string m_password;
    int         m_focus     = 0;   // 0=user, 1=pass
    std::string m_errorMsg;
    std::string m_successMsg;
    float       m_msgTimer  = 0.f;
    bool        m_loggedIn  = false;   // triggers deferred return
    bool        m_checking  = false;   // waiting for duplicate-login check
};
