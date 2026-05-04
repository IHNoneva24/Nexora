#pragma once
#include "raylib.h"
#include "enum.h"
#include "ParallaxBackground.h"
#include "UIHelpers.h"
#include <string>
#include <vector>

// ─────────────────────────────────────────────
//  PRESENTATION LAYER – HowToPlayScreen
// ─────────────────────────────────────────────
class HowToPlayScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    ScreenID Tick(float dt);

private:
    ParallaxBackground m_bg;
    Font               m_font = {};

    struct Entry { std::string key; std::string desc; };
    std::vector<Entry> m_controls = {
        { "WASD / Arrow Keys",  "Move your character"           },
        { "Space",              "Jump"                          },
        { "Left Click",         "Attack / Interact"             },
        { "E",                  "Pick up item"                  },
        { "Shift",              "Sprint"                        },
        { "ESC",                "Pause / Return to main menu"   },
        { "F11",                "Toggle fullscreen"             },
    };
    float m_scroll = 0.f;
};
