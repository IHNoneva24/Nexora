#include "../include/HowToPlayScreen.h"

void HowToPlayScreen::Load(const std::string& assetRoot, Font font) {
    m_font = font;
    m_bg.Load(assetRoot +
        "/GandalfHardcore Background layers/Normal BG");
}
void HowToPlayScreen::Unload() { m_bg.Unload(); }

ScreenID HowToPlayScreen::Tick(float dt) {
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    float cx = (float)sw * .5f;

    m_bg.Update(dt);
    m_bg.Draw(sw, sh);

    // Panel
    const float pW = 620.f, pH = 460.f;
    const float pX = cx - pW*.5f, pY = (float)sh*.5f - pH*.5f;
    UI::DrawPanel({ pX, pY, pW, pH }, 3);

    UI::LabelC("HOW TO PLAY", cx, pY + 18.f, 30.f, UI::C_TEXT_GOLD, m_font);
    UI::Divider(pX + 40.f, pY + 58.f, pW - 80.f);

    // Control entries
    float rowH = 46.f;
    float startY = pY + 80.f;
    float colKey = pX + 40.f;
    float colDesc = cx + 20.f;

    UI::Label("Control", colKey,  startY, 15.f, UI::C_BORDER, m_font);
    UI::Label("Action",  colDesc, startY, 15.f, UI::C_BORDER, m_font);
    UI::Divider(pX + 40.f, startY + 20.f, pW - 80.f);

    for (int i = 0; i < (int)m_controls.size(); ++i) {
        float y = startY + 32.f + (float)i * rowH;
        if (i % 2 == 0) {
            DrawRectangle((int)pX + 6, (int)y - 4,
                          (int)pW - 12, (int)rowH - 4,
                          { 255, 255, 255, 8 });
        }
        UI::Label("[" + m_controls[i].key + "]", colKey,  y + 6.f, 18.f, UI::C_TEXT_GOLD, m_font);
        UI::Label(m_controls[i].desc,             colDesc, y + 6.f, 18.f, UI::C_TEXT_LIGHT, m_font);
    }

    // Story blurb
    float blurbY = startY + 32.f + (float)m_controls.size() * rowH + 10.f;
    UI::Divider(pX + 40.f, blurbY, pW - 80.f);
    UI::LabelC("Explore dungeons, defeat enemies, and claim the ancient relics.",
               cx, blurbY + 18.f, 16.f, UI::C_TEXT_DIM, m_font);

    // Back button
    if (UI::Button({ cx - 90.f, pY + pH + 24.f, 180.f, 42.f }, "< BACK", m_font, 22.f))
        return ScreenID::MainMenu;

    return ScreenID::HowToPlay;
}
