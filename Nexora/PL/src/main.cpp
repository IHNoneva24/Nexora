#include "../include/ScreenLoop.h"
<<<<<<< HEAD
=======
#include "../include/NetworkManager.h"
#include "../include/GameContext.h"
>>>>>>> a71f47d (Add game)
#include "../../DL/include/Database.h"
#include "../../DL/include/UserRepository.h"
#include "../../DL/include/CharacterRepository.h"

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

    // Shared game context (populated by lobby/question screens before game starts)
    GameContext ctx;

    // Screens
    MainMenuScreen         mainMenu;
    LoginScreen            login;
    RegisterScreen         reg;
    HowToPlayScreen        howToPlay;
    CharacterCreatorScreen charCreator;
    HostLobbyScreen        hostLobby;
    JoinLobbyScreen        joinLobby;
    QuestionCreateScreen   questionCreate;
    GameScreen             gameScreen;
<<<<<<< HEAD
    SinglePlayerScreen     singlePlayer;
=======
>>>>>>> a71f47d (Add game)

    mainMenu.Load(assetRoot, font);
    login.Load(assetRoot, font);
    reg.Load(assetRoot, font);
    howToPlay.Load(assetRoot, font);
    charCreator.Load(assetRoot, font);
    hostLobby.Load(assetRoot, font);
    joinLobby.Load(assetRoot, font);
    questionCreate.Load(assetRoot, font);
    gameScreen.Load(assetRoot, font);
<<<<<<< HEAD
    singlePlayer.Load(assetRoot, font);
=======
>>>>>>> a71f47d (Add game)

    ScreenID current = ScreenID::MainMenu;
    ScreenID prev    = ScreenID::MainMenu;
    bool running = true;

    while (!WindowShouldClose() && running) {
        float dt = GetFrameTime();

        net.Update(dt);

        if (current != prev) {
            if (current == ScreenID::CharacterCreate)
                charCreator.Enter(auth.GetUserId(), charSvc);
            if (current == ScreenID::HostLobby)
                hostLobby.Enter(auth, charSvc);
            if (current == ScreenID::JoinLobby)
                joinLobby.Enter(auth, charSvc, mainMenu.GetJoinedGameName(), net);
            if (current == ScreenID::QuestionCreate) {
                // Save character data from whichever lobby we came from
                if (prev == ScreenID::HostLobby) hostLobby.FillGameContext(ctx);
                else                              joinLobby.FillGameContext(ctx);
                questionCreate.Enter(net);
            }
            if (current == ScreenID::Game) {
                // Save questions from the question creation phase
                questionCreate.FillGameContext(ctx);
<<<<<<< HEAD
                // Reload own character directly so it's always current
                charSvc.Load(auth.GetUserId(), ctx.myChar);
                gameScreen.Enter(ctx, net);
            }
            if (current == ScreenID::SinglePlayerGame) {
                CharacterData charData;
                charSvc.Load(auth.GetUserId(), charData);
                singlePlayer.Enter(charData);
            }
=======
                gameScreen.Enter(ctx, net);
            }
>>>>>>> a71f47d (Add game)
        }

        BeginDrawing();
        ClearBackground(BLACK);

        ScreenID next = TickCurrentScreen(
            current, dt,
            mainMenu, login, reg, howToPlay, charCreator,
<<<<<<< HEAD
            hostLobby, joinLobby, questionCreate, gameScreen, singlePlayer, net,
=======
            hostLobby, joinLobby, questionCreate, gameScreen, net,
>>>>>>> a71f47d (Add game)
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
    questionCreate.Unload();
    gameScreen.Unload();
<<<<<<< HEAD
    singlePlayer.Unload();
=======
>>>>>>> a71f47d (Add game)
    UnloadFont(font);
    CloseWindow();
    return 0;
}
