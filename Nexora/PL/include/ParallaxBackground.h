#pragma once
#include "raylib.h"
#include <array>
#include <string>

class ParallaxBackground {
public:
    static constexpr int LAYER_COUNT = 5;

    void Load(const std::string& bgFolder) {
        for (int i = 0; i < LAYER_COUNT; ++i) {
            std::string path = bgFolder +
                "/GandalfHardcore Background layers_layer " +
                std::to_string(i + 1) + ".png";
            m_textures[i] = LoadTexture(path.c_str());
            SetTextureFilter(m_textures[i], TEXTURE_FILTER_BILINEAR);
        }
    }

    void Unload() {
        for (auto& t : m_textures) UnloadTexture(t);
    }

    void Update(float) {}  // no-op: static background

    void Draw(int screenW, int screenH) const {
        Rectangle dest = { 0, 0, (float)screenW, (float)screenH };
        for (int i = 0; i < LAYER_COUNT; ++i) {
            if (m_textures[i].id == 0) continue;
            Rectangle src = { 0, 0,
                (float)m_textures[i].width,
                (float)m_textures[i].height };
            DrawTexturePro(m_textures[i], src, dest, { 0, 0 }, 0.f, WHITE);
        }
    }

private:
    std::array<Texture2D, LAYER_COUNT> m_textures = {};
};
