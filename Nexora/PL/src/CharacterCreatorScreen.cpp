#include "../include/CharacterCreatorScreen.h"
#include <cmath>

// Sprite sheet: 800x448, frame size 50x64.
// Frame 5 (x=250) is the most centred standing pose.
// Tight crop centred on the character's pixel bounds [21..37, 20..63] within that frame:
//   absolute src x = 250+21-6 = 265, width = 17+12 = 28
//   absolute src y = 20-6      = 14,  height = 43+6 = 49
static constexpr float FRAME_W  = 28.f;
static constexpr float FRAME_H  = 49.f;
static constexpr Rectangle FRAME_SRC = { 265.f, 14.f, FRAME_W, FRAME_H };

// ── helpers ──────────────────────────────────────────────────────────────────

void CharacterCreatorScreen::LoadTex(LayerSet& ls, const std::string& path, const std::string& name) {
    Texture2D t = LoadTexture(path.c_str());
    if (t.id != 0) {
        SetTextureFilter(t, TEXTURE_FILTER_POINT);
        ls.textures.push_back(t);
        ls.names.push_back(name);
    }
}

void CharacterCreatorScreen::UnloadSet(LayerSet& ls) {
    for (auto& t : ls.textures) UnloadTexture(t);
    ls.textures.clear();
    ls.names.clear();
    ls.index = 0;
}

// ── Load ─────────────────────────────────────────────────────────────────────

void CharacterCreatorScreen::Load(const std::string& assetRoot, Font font) {
    m_assetRoot  = assetRoot;
    m_font       = font;
    m_background = LoadTexture((assetRoot + "/character-backround.png").c_str());

    const std::string base = assetRoot + "/GandalfHardcore Character Asset Pack/";

    // ── Male ─────────────────────────────────────────────────────────────────
    const std::string mSkin  = base + "Character skin colors/";
    const std::string mHair  = base + "Male Hair/";
    const std::string mCloth = base + "Male Clothing/";

    for (int i = 1; i <= 5; ++i)
        LoadTex(m_maleSkin, mSkin + "Male Skin" + std::to_string(i) + ".png", "Skin " + std::to_string(i));

    for (int i = 1; i <= 5; ++i)
        LoadTex(m_maleHair, mHair + "Male Hair" + std::to_string(i) + ".png", "Style " + std::to_string(i));

    LoadTex(m_maleTop, mCloth + "Shirt.png",           "Shirt");
    LoadTex(m_maleTop, mCloth + "Shirt v2.png",        "Shirt Alt");
    LoadTex(m_maleTop, mCloth + "Blue Shirt v2.png",   "Blue Shirt");
    LoadTex(m_maleTop, mCloth + "Green Shirt v2.png",  "Green Shirt");
    LoadTex(m_maleTop, mCloth + "orange Shirt v2.png", "Orange Shirt");
    LoadTex(m_maleTop, mCloth + "Purple Shirt v2.png", "Purple Shirt");

    LoadTex(m_malePants, mCloth + "Pants.png",        "Pants");
    LoadTex(m_malePants, mCloth + "Blue Pants.png",   "Blue Pants");
    LoadTex(m_malePants, mCloth + "Green Pants.png",  "Green Pants");
    LoadTex(m_malePants, mCloth + "Orange Pants.png", "Orange Pants");
    LoadTex(m_malePants, mCloth + "Purple Pants.png", "Purple Pants");

    LoadTex(m_maleFeet, mCloth + "Boots.png", "Boots");
    LoadTex(m_maleFeet, mCloth + "Shoes.png", "Shoes");

    // ── Female ───────────────────────────────────────────────────────────────
    const std::string fSkin  = base + "Character skin colors/";
    const std::string fHair  = base + "Female Hair/";
    const std::string fCloth = base + "Female Clothing/";

    for (int i = 1; i <= 5; ++i)
        LoadTex(m_femaleSkin, fSkin + "Female Skin" + std::to_string(i) + ".png", "Skin " + std::to_string(i));

    for (int i = 1; i <= 5; ++i)
        LoadTex(m_femaleHair, fHair + "Female Hair" + std::to_string(i) + ".png", "Style " + std::to_string(i));

    LoadTex(m_femaleTop, fCloth + "Corset.png",           "Corset");
    LoadTex(m_femaleTop, fCloth + "Corset v2.png",        "Corset Alt");
    LoadTex(m_femaleTop, fCloth + "Blue Corset.png",      "Blue Corset");
    LoadTex(m_femaleTop, fCloth + "Blue Corset v2.png",   "Blue Corset Alt");
    LoadTex(m_femaleTop, fCloth + "Green Corset.png",     "Green Corset");
    LoadTex(m_femaleTop, fCloth + "Green Corset v2.png",  "Green Corset Alt");
    LoadTex(m_femaleTop, fCloth + "Orange Corset.png",    "Orange Corset");
    LoadTex(m_femaleTop, fCloth + "Orange Corset v2.png", "Orange Corset Alt");
    LoadTex(m_femaleTop, fCloth + "Purple Corset.png",    "Purple Corset");
    LoadTex(m_femaleTop, fCloth + "Purple Corset v2.png", "Purple Corset Alt");

    LoadTex(m_femaleSkirt, fCloth + "Skirt.png", "Skirt");

    LoadTex(m_femaleFeet, fCloth + "Boots.png",         "Boots");
    LoadTex(m_femaleFeet, fCloth + "Socks.png",         "Socks");
    LoadTex(m_femaleFeet, fCloth + "Green Socks.png",   "Green Socks");
    LoadTex(m_femaleFeet, fCloth + "Orange Socks.png",  "Orange Socks");
    LoadTex(m_femaleFeet, fCloth + "Purple Socks.png",  "Purple Socks");
    LoadTex(m_femaleFeet, fCloth + "Red Socks.png",     "Red Socks");
    LoadTex(m_femaleFeet, fCloth + "Skyblue Socks.png", "Skyblue Socks");
}

void CharacterCreatorScreen::Unload() {
    UnloadTexture(m_background);
    UnloadSet(m_maleSkin);  UnloadSet(m_maleHair);
    UnloadSet(m_maleTop);   UnloadSet(m_malePants); UnloadSet(m_maleFeet);

    UnloadSet(m_femaleSkin); UnloadSet(m_femaleHair);
    UnloadSet(m_femaleTop);  UnloadSet(m_femaleSkirt); UnloadSet(m_femaleFeet);
}

// ── Enter ─────────────────────────────────────────────────────────────────────
void CharacterCreatorScreen::Enter(int userId, CharacterService& charSvc) {
    m_userId = userId;
    m_saved  = false;

    CharacterData data;
    if (!charSvc.Load(userId, data)) return;

    // Restore saved choices, clamping to valid range
    m_gender = data.gender;
    auto clamp = [](int v, int sz) { return (sz > 0 && v < sz) ? v : 0; };

    if (m_gender == 0) {
        m_maleSkin.index  = clamp(data.skinIdx,  (int)m_maleSkin.textures.size());
        m_maleHair.index  = clamp(data.hairIdx,  (int)m_maleHair.textures.size());
        m_maleTop.index   = clamp(data.topIdx,   (int)m_maleTop.textures.size());
        m_malePants.index = clamp(data.pantsIdx, (int)m_malePants.textures.size());
        m_maleFeet.index  = clamp(data.feetIdx,  (int)m_maleFeet.textures.size());
    } else {
        m_femaleSkin.index = clamp(data.skinIdx, (int)m_femaleSkin.textures.size());
        m_femaleHair.index = clamp(data.hairIdx, (int)m_femaleHair.textures.size());
        m_femaleTop.index  = clamp(data.topIdx,  (int)m_femaleTop.textures.size());
        m_femaleSkirt.index = clamp(data.pantsIdx, (int)m_femaleSkirt.textures.size());
        m_femaleFeet.index = clamp(data.feetIdx, (int)m_femaleFeet.textures.size());
    }
}

// ── Selector widget ───────────────────────────────────────────────────────────
bool CharacterCreatorScreen::Selector(float cx, float y, float bSize,
                                      LayerSet& ls, const std::string& label) {
    if (ls.textures.empty()) return false;

    bool changed  = false;
    float nameW   = 200.f;
    float totalW  = bSize + nameW + bSize;
    float lx      = cx - totalW * .5f;

    Rectangle lBtn = { lx,                   y, bSize, bSize };
    Rectangle rBtn = { lx + bSize + nameW,   y, bSize, bSize };

    if (UI::Button(lBtn, "<", m_font, 20.f)) {
        ls.index = (ls.index - 1 + (int)ls.textures.size()) % (int)ls.textures.size();
        changed = true;
    }
    if (UI::Button(rBtn, ">", m_font, 20.f)) {
        ls.index = (ls.index + 1) % (int)ls.textures.size();
        changed = true;
    }

    Rectangle nameRect = { lx + bSize, y, nameW, bSize };
    DrawRectangleRec(nameRect, UI::C_PANEL_BG);
    DrawRectangleLinesEx(nameRect, 1, UI::C_BORDER_DARK);

    UI::LabelC(label,              cx, y - 16.f,                   13.f, UI::C_TEXT_DIM,   m_font);
    UI::LabelC(ls.names[ls.index], cx, y + (bSize - 20.f) * .5f,  18.f, UI::C_TEXT_LIGHT, m_font);

    return changed;
}

// ── DrawLayeredPreview ────────────────────────────────────────────────────────
void CharacterCreatorScreen::DrawLayeredPreview(float cx, float py, float maxH) const {
    // Collect active layers in draw order (back → front)
    std::vector<const Texture2D*> layers;
    int feetLayerIdx = -1;
    auto push = [&](const LayerSet& ls) {
        if (!ls.textures.empty()) layers.push_back(&ls.textures[ls.index]);
    };

    if (m_gender == 0) {
        push(m_maleSkin);
        push(m_malePants);
        push(m_maleTop);
        push(m_maleFeet);
        push(m_maleHair);
    } else {
        push(m_femaleSkin);
        push(m_femaleTop);
        feetLayerIdx = (int)layers.size(); push(m_femaleFeet);
        push(m_femaleSkirt);
        push(m_femaleHair);
    }

    if (layers.empty()) return;

    // Scale so that one frame fills maxH vertically
    float scale  = maxH / FRAME_H;
    float destW  = FRAME_W * scale;
    float destH  = FRAME_H * scale;
    Rectangle dest = { cx - destW * .5f, py, destW, destH };

    for (int i = 0; i < (int)layers.size(); ++i) {
        const Texture2D* t = layers[i];
        if (t->id == 0) continue;
        Rectangle d = dest;
        if (i == feetLayerIdx) d.y += 5.f;
        DrawTexturePro(*t, FRAME_SRC, d, { 0, 0 }, 0.f, WHITE);
    }
}

// ── Tick ──────────────────────────────────────────────────────────────────────
ScreenID CharacterCreatorScreen::Tick(float dt, CharacterService& charSvc) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    float cx = (float)sw * .5f;

    // Background image
    if (m_background.id != 0)
        DrawTexturePro(m_background,
            { 0, 0, (float)m_background.width, (float)m_background.height },
            { 0, 0, (float)sw, (float)sh },
            { 0, 0 }, 0.f, WHITE);
    else
        DrawRectangle(0, 0, sw, sh, { 10, 6, 2, 240 });

    // ── Title ─────────────────────────────────────────────────────────────────
    UI::LabelShadow("CHARACTER CREATOR", cx, (float)sh * .04f, 46.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(cx - 200.f, (float)sh * .04f + 54.f, 400.f);

    // ── Gender toggle ─────────────────────────────────────────────────────────
    float gY = (float)sh * .13f;
    float gW = 120.f, gH = 40.f, gGap = 10.f;

    Rectangle maleRect   = { cx - gW - gGap * .5f, gY, gW, gH };
    Rectangle femaleRect = { cx + gGap * .5f,       gY, gW, gH };

    DrawRectangleRec(m_gender == 0 ? maleRect : femaleRect, { 80, 55, 10, 120 });
    if (UI::Button(maleRect,   "MALE",   m_font, 18.f)) m_gender = 0;
    if (UI::Button(femaleRect, "FEMALE", m_font, 18.f)) m_gender = 1;

    // ── Character preview ─────────────────────────────────────────────────────
    float previewH  = (float)sh * .52f;
    float previewY  = (float)sh * .20f;
    float spriteW   = FRAME_W / FRAME_H * previewH;
    float panelW    = spriteW + 32.f;
    float panelH    = previewH + 20.f;

    Rectangle frameRect = { cx - panelW * .5f, previewY - 10.f, panelW, panelH };
    UI::DrawPanel(frameRect);
    DrawLayeredPreview(cx, previewY + (panelH - previewH) * .5f - 10.f, previewH);

    // ── Selectors (right column) ──────────────────────────────────────────────
    float selCX     = (float)sw * .80f;
    float selStartY = (float)sh * .22f;
    float selGapY   = (float)sh * .13f;
    float btnSz     = 34.f;

    LayerSet& skin  = (m_gender == 0) ? m_maleSkin  : m_femaleSkin;
    LayerSet& hair  = (m_gender == 0) ? m_maleHair  : m_femaleHair;
    LayerSet& top   = (m_gender == 0) ? m_maleTop   : m_femaleTop;
    LayerSet& feet  = (m_gender == 0) ? m_maleFeet  : m_femaleFeet;

    Selector(selCX, selStartY + selGapY * 0, btnSz, skin, "SKIN");
    Selector(selCX, selStartY + selGapY * 1, btnSz, hair, "HAIR");
    Selector(selCX, selStartY + selGapY * 2, btnSz, top,  (m_gender == 0) ? "SHIRT" : "TOP");
    Selector(selCX, selStartY + selGapY * 3, btnSz, feet, "FEET");

    if (m_gender == 0) {
        Selector(selCX, selStartY + selGapY * 4, btnSz, m_malePants, "PANTS");
    }

    // ── Back / Confirm ────────────────────────────────────────────────────────
    float btnW = 160.f, btnH = 48.f;
    float btnY = (float)sh - btnH - 20.f;

    if (UI::Button({ 30.f, btnY, btnW, btnH }, "BACK", m_font, 22.f))
        return ScreenID::MainMenu;

    if (UI::Button({ (float)sw - 200.f - 30.f, btnY, 200.f, btnH }, "CONFIRM", m_font, 22.f)) {
        CharacterData data;
        data.userId = m_userId;
        data.gender = m_gender;
        if (m_gender == 0) {
            data.skinIdx  = m_maleSkin.index;
            data.hairIdx  = m_maleHair.index;
            data.topIdx   = m_maleTop.index;
            data.pantsIdx = m_malePants.index;
            data.feetIdx  = m_maleFeet.index;
        } else {
            data.skinIdx  = m_femaleSkin.index;
            data.hairIdx  = m_femaleHair.index;
            data.topIdx   = m_femaleTop.index;
            data.pantsIdx = 1;
            data.feetIdx  = m_femaleFeet.index;
        }
        charSvc.Save(data);
        return ScreenID::MainMenu;
    }

    return ScreenID::CharacterCreate;
}
