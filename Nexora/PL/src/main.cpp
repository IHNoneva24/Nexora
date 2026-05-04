#include "raylib.h"
#include "../include/enum.h"
#include "../include/ScreenLoop.h"
#include "../../DL/include/Database.h"
#include "../../DL/include/UserRepository.h"
#include "../../BLL/include/AuthService.h"
#include <string>

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Nexora");
    MaximizeWindow();
    SetTargetFPS(60);

    // Paths relative to PL/ (the working directory set in PL.vcxproj.user)
    const std::string assetRoot = "assets";

    Font font = GetFontDefault();

    // Database / auth stack
    Database       db("../DL/data/nexora.db");
    UserRepository repo(db);
    repo.InitTable();
    AuthService    auth(repo);

    // Screens
    MainMenuScreen  mainMenu;
    LoginScreen     login;
    RegisterScreen  reg;
    HowToPlayScreen howToPlay;

    mainMenu.Load(assetRoot, font);
    login.Load(assetRoot, font);
    reg.Load(assetRoot, font);
    howToPlay.Load(assetRoot, font);

    ScreenID current = ScreenID::MainMenu;
    bool running = true;

    while (!WindowShouldClose() && running) {
        float dt = GetFrameTime();
        ScreenID next = current;

        BeginDrawing();
        ClearBackground(BLACK);

        next = TickCurrentScreen(current, dt, mainMenu, login, reg, howToPlay, auth, running);

        EndDrawing();
        current = next;
    }

    mainMenu.Unload();
    login.Unload();
    reg.Unload();
    howToPlay.Unload();
    UnloadFont(font);
    CloseWindow();
    return 0;
}