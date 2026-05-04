#pragma once
#include "enum.h"
#include "MainMenuScreen.h"
#include "LoginScreen.h"
#include "RegisterScreen.h"
#include "HowToPlayScreen.h"
#include "../../BLL/include/AuthService.h"
#include "raylib.h"

inline ScreenID TickCurrentScreen(
    ScreenID current, float dt,
    MainMenuScreen& mainMenu,
    LoginScreen& login,
    RegisterScreen& reg,
    HowToPlayScreen& howToPlay,
    AuthService& auth,
    bool& running)
{
    ScreenID next = current;
    switch (current) {
        case ScreenID::MainMenu:  next = mainMenu.Tick(dt, auth);  break;
        case ScreenID::Login:     next = login.Tick(dt, auth);     break;
        case ScreenID::Register:  next = reg.Tick(dt, auth);       break;
        case ScreenID::HowToPlay: next = howToPlay.Tick(dt);       break;
        case ScreenID::Game:
            DrawText("GAME - Press ESC to return to menu", 40, 40, 30, WHITE);
            if (IsKeyPressed(KEY_ESCAPE)) next = ScreenID::MainMenu;
            break;
        case ScreenID::Exit:
            running = false;
            break;
    }
    return next;
}
