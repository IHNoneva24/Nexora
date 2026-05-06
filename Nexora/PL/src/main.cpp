#include "raylib.h"
#include "../include/enum.h"
#include "../include/ScreenLoop.h"
#include "../include/NetworkManager.h"
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

    // Networking
    NetworkManager net;

    // Screens
    MainMenuScreen         mainMenu;
    LoginScreen            login;
    RegisterScreen         reg;
    HowToPlayScreen        howToPlay;
    CharacterCreatorScreen charCreator;
    HostLobbyScreen        hostLobby;
    JoinLobbyScreen        joinLobby;

    mainMenu.Load(assetRoot, font);
    login.Load(assetRoot, font);
    reg.Load(assetRoot, font);
    howToPlay.Load(assetRoot, font);
    charCreator.Load(assetRoot, font);
    hostLobby.Load(assetRoot, font);
    joinLobby.Load(assetRoot, font);

    ScreenID current = ScreenID::MainMenu;
    ScreenID prev    = ScreenID::MainMenu;
    bool running = true;

    while (!WindowShouldClose() && running) {
        float dt = GetFrameTime();

        // Network tick (broadcasts, accepts, discovery) — always runs
        net.Update(dt);

        // Call Enter when transitioning into a screen that needs it
        if (current != prev) {
            if (current == ScreenID::CharacterCreate)
                charCreator.Enter(auth.GetUserId(), charSvc);
            if (current == ScreenID::HostLobby)
                hostLobby.Enter(auth, charSvc);
            if (current == ScreenID::JoinLobby)
                joinLobby.Enter(auth, charSvc, mainMenu.GetJoinedGameName(), net);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        ScreenID next = TickCurrentScreen(
            current, dt,
            mainMenu, login, reg, howToPlay, charCreator,
            hostLobby, joinLobby, net,
            auth, charSvc, running);

        EndDrawing();

        prev    = current;
        current = next;
    }

    net.Shutdown();
    mainMenu.Unload();
    login.Unload();
    reg.Unload();
    howToPlay.Unload();
    charCreator.Unload();
    hostLobby.Unload();
    joinLobby.Unload();
    UnloadFont(font);
    CloseWindow();
    return 0;
}
