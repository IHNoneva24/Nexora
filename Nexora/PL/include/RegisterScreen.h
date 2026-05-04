#pragma once
#include "raylib.h"
#include "enum.h"
#include "ParallaxBackground.h"
#include "UIHelpers.h"
#include "../../BLL/include/AuthService.h"
#include <string>

// ─────────────────────────────────────────────
//  PRESENTATION LAYER – RegisterScreen
// ─────────────────────────────────────────────
class RegisterScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();
    void Reset();

    ScreenID Tick(float dt, AuthService& auth);

private:
    ParallaxBackground m_bg;
    Font               m_font = {};

    std::string m_username, m_password, m_confirm;
    int         m_focus     = 0;  // 0 user, 1 pass, 2 confirm
    std::string m_errorMsg, m_successMsg;
    float       m_msgTimer  = 0.f;
    bool        m_done      = false;  // registered → deferred nav to Login
};
