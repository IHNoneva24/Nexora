#pragma once
#include "raylib.h"
#include <string>
#include <algorithm>

// ─────────────────────────────────────────────
//  PRESENTATION LAYER – UIHelpers
//  Fantasy / medieval style.
//  All functions are safe to call inside a
//  BeginDrawing() / EndDrawing() block.
// ─────────────────────────────────────────────
namespace UI {

constexpr Color C_PANEL_BG    = { 18,  12,   6, 215 };
constexpr Color C_BORDER      = {200, 160,  50, 255 };
constexpr Color C_BORDER_DARK = {120,  90,  20, 255 };
constexpr Color C_BTN_NORMAL  = { 40,  25,  10, 230 };
constexpr Color C_BTN_HOVER   = { 70,  45,  15, 245 };
constexpr Color C_BTN_PRESS   = { 15,  10,   4, 255 };
constexpr Color C_FIELD_BG    = { 12,   8,   3, 210 };
constexpr Color C_TEXT_GOLD   = {255, 215,  80, 255 };
constexpr Color C_TEXT_LIGHT  = {230, 210, 170, 255 };
constexpr Color C_TEXT_DIM    = {160, 140, 100, 255 };
constexpr Color C_TEXT_ERR    = {255,  80,  60, 255 };
constexpr Color C_TEXT_OK     = { 90, 210,  90, 255 };

inline void DrawCornerGems(Rectangle r) {
    const float s = 5.f, p = 4.f;
    DrawRectangle((int)(r.x + p),                (int)(r.y + p),                (int)s, (int)s, C_BORDER);
    DrawRectangle((int)(r.x + r.width - p - s),  (int)(r.y + p),                (int)s, (int)s, C_BORDER);
    DrawRectangle((int)(r.x + p),                (int)(r.y + r.height - p - s), (int)s, (int)s, C_BORDER);
    DrawRectangle((int)(r.x + r.width - p - s),  (int)(r.y + r.height - p - s),(int)s, (int)s, C_BORDER);
}

inline void DrawPanel(Rectangle r, int bw = 2) {
    DrawRectangleRec(r, C_PANEL_BG);
    DrawRectangleLinesEx(r, (float)bw, C_BORDER);
    DrawRectangleLinesEx({ r.x+bw+1, r.y+bw+1, r.width-(bw+1)*2.f, r.height-(bw+1)*2.f },
                         1, C_BORDER_DARK);
}

// Returns true when clicked (button released while hovered)
inline bool Button(Rectangle r, const std::string& label, Font font, float fs = 22.f) {
    try {
        Vector2 m  = GetMousePosition();
        bool hov   = CheckCollisionPointRec(m, r);
        bool down  = hov && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        bool click = hov && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

        DrawRectangleRec(r, down ? C_BTN_PRESS : (hov ? C_BTN_HOVER : C_BTN_NORMAL));
        DrawRectangleLinesEx(r, 2, hov ? C_TEXT_GOLD : C_BORDER);
        DrawCornerGems(r);

        Color tc = hov ? C_TEXT_GOLD : C_TEXT_LIGHT;
        Vector2 sz = MeasureTextEx(font, label.c_str(), fs, 1);
        Vector2 tp = { r.x + (r.width - sz.x) * .5f, r.y + (r.height - sz.y) * .5f };
        DrawTextEx(font, label.c_str(), { tp.x+1, tp.y+1 }, fs, 1, {0,0,0,150});
        DrawTextEx(font, label.c_str(), tp, fs, 1, tc);
        return click;
    } catch (...) {
        return false;
    }
}

// Draws an input field and handles keyboard input when active==true.
// Returns true when Enter is pressed while active.
inline bool InputField(Rectangle r, std::string& text, bool active,
                       bool password, Font font, float fs = 20.f,
                       size_t maxLen = 32) {
    try {
        DrawRectangleRec(r, C_FIELD_BG);
        DrawRectangleLinesEx(r, 2.f, active ? C_TEXT_GOLD : C_BORDER_DARK);

        std::string disp = password ? std::string(text.size(), '*') : text;
        if (active && (int)(GetTime() * 2) % 2 == 0) disp += '|';

        const float pad = 10.f;
        while (!disp.empty() &&
               MeasureTextEx(font, disp.c_str(), fs, 1).x > r.width - pad * 2.f)
            disp.erase(disp.begin());

        DrawTextEx(font, disp.c_str(),
                   { r.x + pad, r.y + (r.height - fs) * .5f }, fs, 1, C_TEXT_LIGHT);

        bool enter = false;
        if (active) {
            int ch = GetCharPressed();
            while (ch > 0) {
                if (ch >= 32 && ch <= 125 && text.size() < maxLen) text += (char)ch;
                ch = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !text.empty()) text.pop_back();
            if (IsKeyPressed(KEY_ENTER)) enter = true;
        }
        return enter;
    } catch (...) {
        return false;
    }
}

// Grayed-out non-clickable button
inline void ButtonDisabled(Rectangle r, const std::string& label, Font font, float fs = 22.f) {
    DrawRectangleRec(r, { 22, 16, 8, 160 });
    DrawRectangleLinesEx(r, 2, { 80, 65, 30, 160 });
    Color tc = { 100, 88, 55, 160 };
    Vector2 sz = MeasureTextEx(font, label.c_str(), fs, 1);
    Vector2 tp = { r.x + (r.width - sz.x) * .5f, r.y + (r.height - sz.y) * .5f };
    DrawTextEx(font, label.c_str(), tp, fs, 1, tc);
}

// Tooltip shown when mouse hovers over rect
inline void Tooltip(Rectangle r, const std::string& msg, Font f) {
    if (!CheckCollisionPointRec(GetMousePosition(), r)) return;
    Vector2 sz = MeasureTextEx(f, msg.c_str(), 14.f, 1);
    float tx = r.x + (r.width - sz.x) * .5f;
    float ty = r.y - sz.y - 12.f;
    DrawRectangle((int)tx - 8, (int)ty - 4, (int)sz.x + 16, (int)sz.y + 8, { 10,6,2,220 });
    DrawRectangleLinesEx({ tx-8, ty-4, sz.x+16, sz.y+8 }, 1, C_BORDER_DARK);
    DrawTextEx(f, msg.c_str(), { tx, ty }, 14.f, 1, C_TEXT_DIM);
}

inline void Label(const std::string& t, float x, float y, float fs, Color c, Font f) {
    DrawTextEx(f, t.c_str(), {x, y}, fs, 1, c);
}
inline void LabelC(const std::string& t, float cx, float y, float fs, Color c, Font f) {
    Vector2 sz = MeasureTextEx(f, t.c_str(), fs, 1);
    DrawTextEx(f, t.c_str(), {cx - sz.x*.5f, y}, fs, 1, c);
}
inline void LabelShadow(const std::string& t, float cx, float y, float fs, Color c, Font f) {
    Vector2 sz = MeasureTextEx(f, t.c_str(), fs, 1);
    float x = cx - sz.x * .5f;
    for (int d = 4; d >= 1; --d)
        DrawTextEx(f, t.c_str(), {x+(float)d, y+(float)d}, fs, 1, {0,0,0,(unsigned char)(50/d)});
    DrawTextEx(f, t.c_str(), {x, y}, fs, 1, c);
}
inline void Divider(float x, float y, float w) {
    DrawLineEx({x, y+5}, {x+w, y+5}, 1, C_BORDER_DARK);
    float mx = x + w * .5f;
    DrawRectangle((int)(mx-4), (int)(y+1), 8, 8, C_BORDER);
    DrawRectangleLinesEx({mx-6, y-1, 12, 12}, 1, C_BORDER_DARK);
}
inline void Toast(const std::string& msg, float cx, float y, float fs,
                  Color c, Font f, float timer, float maxTime = 3.f) {
    if (timer <= 0.f) return;
    c.a = (unsigned char)(std::min(1.f, timer) * 255.f);
    LabelC(msg, cx, y, fs, c, f);
}

}
