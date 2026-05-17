#pragma once
#include "raylib.h"
#include "../../DL/include/CharacterData.h"
#include <vector>
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  CharacterRenderer  —  loads the right sprite layers for a CharacterData
//  and draws the standing frame, shared between lobby screens.
// ─────────────────────────────────────────────────────────────────────────────
namespace CharacterRenderer {

constexpr float     FRAME_W   = 28.f;
constexpr float     FRAME_H   = 49.f;
constexpr Rectangle FRAME_SRC = { 265.f, 14.f, FRAME_W, FRAME_H };

inline std::vector<Texture2D> LoadLayers(const CharacterData& data,
                                          const std::string& assetRoot) {
    std::vector<Texture2D> layers;
    const std::string base = assetRoot + "/GandalfHardcore Character Asset Pack/";

    auto load = [&](const std::string& path) {
        Texture2D t = LoadTexture(path.c_str());
        if (t.id != 0) {
            SetTextureFilter(t, TEXTURE_FILTER_POINT);
            layers.push_back(t);
        }
    };
    auto n = [](int i) { return std::to_string(i + 1); };

    if (data.gender == 0) {
        const std::string skin  = base + "Character skin colors/";
        const std::string hair  = base + "Male Hair/";
        const std::string cloth = base + "Male Clothing/";

        const char* tops[]  = { "Shirt.png","Shirt v2.png","Blue Shirt v2.png",
                                 "Green Shirt v2.png","orange Shirt v2.png","Purple Shirt v2.png" };
        const char* pants[] = { "Pants.png","Blue Pants.png","Green Pants.png",
                                 "Orange Pants.png","Purple Pants.png" };
        const char* feet[]  = { "Boots.png","Shoes.png" };

        load(skin + "Male Skin" + n(data.skinIdx) + ".png");
        if (data.pantsIdx < 5) load(cloth + pants[data.pantsIdx]);
        if (data.topIdx   < 6) load(cloth + tops[data.topIdx]);
        if (data.feetIdx  < 2) load(cloth + feet[data.feetIdx]);
        load(hair + "Male Hair" + n(data.hairIdx) + ".png");
    } else {
        const std::string skin  = base + "Character skin colors/";
        const std::string hair  = base + "Female Hair/";
        const std::string cloth = base + "Female Clothing/";

        const char* tops[] = {
            "Corset.png","Corset v2.png","Blue Corset.png","Blue Corset v2.png",
            "Green Corset.png","Green Corset v2.png","Orange Corset.png","Orange Corset v2.png",
            "Purple Corset.png","Purple Corset v2.png"
        };
        const char* feet[] = {
            "Boots.png","Socks.png","Green Socks.png","Orange Socks.png",
            "Purple Socks.png","Red Socks.png","Skyblue Socks.png"
        };

        load(skin + "Female Skin" + n(data.skinIdx) + ".png");
        if (data.topIdx  < 10) load(cloth + tops[data.topIdx]);
        if (data.pantsIdx > 0) load(cloth + "Skirt.png");
        if (data.feetIdx < 7)  load(cloth + feet[data.feetIdx]);
        load(hair + "Female Hair" + n(data.hairIdx) + ".png");
    }
    return layers;
}

inline void UnloadLayers(std::vector<Texture2D>& layers) {
    for (auto& t : layers) UnloadTexture(t);
    layers.clear();
}

// Draw the standing frame centered at cx, sitting on top of platformY.
// Pass flipped=true to mirror the sprite horizontally (no GPU readback needed).
inline void Draw(const std::vector<Texture2D>& layers,
                 float cx, float platformY, float charH, bool flipped = false) {
    if (layers.empty()) return;
    float scale = charH / FRAME_H;
    float destW = FRAME_W * scale;
    float destH = FRAME_H * scale;
    Rectangle dest = { cx - destW * .5f, platformY - destH, destW, destH };
    Rectangle src  = FRAME_SRC;
    if (flipped) src.width = -src.width;
    for (const auto& t : layers)
        if (t.id != 0)
            DrawTexturePro(t, src, dest, { 0, 0 }, 0.f, WHITE);
}

} // namespace CharacterRenderer
