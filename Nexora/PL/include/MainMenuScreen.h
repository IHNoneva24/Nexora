#pragma once
#include "raylib.h"
#include "enum.h"
#include "ParallaxBackground.h"
#include "UIHelpers.h"
#include "../../BLL/include/AuthService.h"
#include <string>
#include <vector>

// ─────────────────────────────────────────────
//  PRESENTATION LAYER – MainMenuScreen
//  Tick() = update + draw in one BeginDrawing pass.
// ─────────────────────────────────────────────
class MainMenuScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    // Called between BeginDrawing / EndDrawing.
    // Returns the next ScreenID to navigate to.
    ScreenID Tick(float dt, AuthService& auth);

private:
    void DrawBackground(int sw, int sh);
    void DrawTitle(float cx, int sh);

    ParallaxBackground m_bg;
    Texture2D          m_castle = {};
    Font               m_font   = {};
    std::string        m_assetRoot;

    // Floating particle runes (ambient decoration)
    struct Particle { float x, y, speed, alpha, size; int sym; };
    std::vector<Particle> m_particles;
    void InitParticles(int sw, int sh);
    void UpdateParticles(float dt, int sw, int sh);
    void DrawParticles() const;
};
