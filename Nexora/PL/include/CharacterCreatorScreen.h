#pragma once
#include "raylib.h"
#include "enum.h"
#include "UIHelpers.h"
#include "../../BLL/include/CharacterService.h"
#include <string>
#include <vector>

// ─────────────────────────────────────────────
//  PRESENTATION LAYER – CharacterCreatorScreen
//  Lets the player build a character by layering
//  sprites from the GandalfHardcore asset pack.
// ─────────────────────────────────────────────
class CharacterCreatorScreen {
public:
    void Load(const std::string& assetRoot, Font font);
    void Unload();

    // Called once when navigating to this screen.
    void Enter(int userId, CharacterService& charSvc);

    // Called between BeginDrawing / EndDrawing.
    ScreenID Tick(float dt, CharacterService& charSvc);

private:
    struct LayerSet {
        std::vector<Texture2D>   textures;
        std::vector<std::string> names;
        int index = 0;

        Texture2D* Current() { return textures.empty() ? nullptr : &textures[index]; }
    };

    void LoadTex(LayerSet& ls, const std::string& path, const std::string& name);
    void UnloadSet(LayerSet& ls);

    // Draw a selector row: [<] NAME [>]  — returns true if index changed
    bool Selector(float cx, float y, float bSize, LayerSet& ls, const std::string& label);
    void DrawLayeredPreview(float cx, float py, float maxH) const;

    Font        m_font       = {};
    Texture2D   m_background = {};
    std::string m_assetRoot;
    int         m_gender   = 0;   // 0 = Male, 1 = Female
    int         m_userId   = 0;
    bool        m_saved    = false;  // shows confirmation message after save

    // ── Male layers ──────────────────────────
    LayerSet m_maleSkin;
    LayerSet m_maleHair;
    LayerSet m_maleTop;
    LayerSet m_malePants;
    LayerSet m_maleFeet;

    // ── Female layers ────────────────────────
    LayerSet m_femaleSkin;
    LayerSet m_femaleHair;
    LayerSet m_femaleTop;
    LayerSet m_femaleSkirt;
    LayerSet m_femaleFeet;
    bool     m_showSkirt = false;
};
