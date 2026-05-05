#include "raylib.h"
#include "../include/enum.h"
#include "../include/ScreenLoop.h"
#include "../../DL/include/Database.h"
#include "../../DL/include/UserRepository.h"
#include "../../DL/include/CharacterRepository.h"
#include "../../BLL/include/AuthService.h"
#include "../../BLL/include/CharacterService.h"
#include <string>

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Nexora");
    MaximizeWindow();
    SetTargetFPS(60);

    const std::string assetRoot = "assets";

    Font font = GetFontDefault();

    // Database / service stack
    Database             db("../DL/data/nexora.db");
    UserRepository       userRepo(db);
    CharacterRepository  charRepo(db);
    userRepo.InitTable();
    AuthService          auth(userRepo);
    CharacterService     charSvc(charRepo);
    charSvc.Init();

    // Screens
    MainMenuScreen         mainMenu;
    LoginScreen            login;
    RegisterScreen         reg;
    HowToPlayScreen        howToPlay;
    CharacterCreatorScreen charCreator;

    mainMenu.Load(assetRoot, font);
    login.Load(assetRoot, font);
    reg.Load(assetRoot, font);
    howToPlay.Load(assetRoot, font);
    charCreator.Load(assetRoot, font);

    ScreenID current = ScreenID::MainMenu;
    ScreenID prev    = ScreenID::MainMenu;
    bool running = true;

    while (!WindowShouldClose() && running) {
        float dt = GetFrameTime();

        // Call Enter when transitioning into CharacterCreate
        if (current == ScreenID::CharacterCreate && prev != ScreenID::CharacterCreate)
            charCreator.Enter(auth.GetUserId(), charSvc);

        BeginDrawing();
        ClearBackground(BLACK);

        ScreenID next = TickCurrentScreen(
            current, dt,
            mainMenu, login, reg, howToPlay, charCreator,
            auth, charSvc, running);

        EndDrawing();

        prev    = current;
        current = next;
    }

    mainMenu.Unload();
    login.Unload();
    reg.Unload();
    howToPlay.Unload();
    charCreator.Unload();
    UnloadFont(font);
    CloseWindow();
    return 0;
}