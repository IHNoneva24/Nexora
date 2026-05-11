#pragma once
#include "enum.h"
#include "MainMenuScreen.h"
#include "LoginScreen.h"
#include "RegisterScreen.h"
#include "HowToPlayScreen.h"
#include "CharacterCreatorScreen.h"
#include "HostLobbyScreen.h"
#include "JoinLobbyScreen.h"
#include "QuestionCreateScreen.h"
#include "GameScreen.h"
#include "NetworkManager.h"
#include "../../BLL/include/AuthService.h"
#include "../../BLL/include/CharacterService.h"
#include "raylib.h"

inline ScreenID TickCurrentScreen(
    ScreenID current, float dt,
    MainMenuScreen& mainMenu,
    LoginScreen& login,
    RegisterScreen& reg,
    HowToPlayScreen& howToPlay,
    CharacterCreatorScreen& charCreator,
    HostLobbyScreen& hostLobby,
    JoinLobbyScreen& joinLobby,
    QuestionCreateScreen& questionCreate,
    GameScreen& gameScreen,
    NetworkManager& net,
    AuthService& auth,
    CharacterService& charSvc,
    bool& running)
{
    ScreenID next = current;
    switch (current) {
        case ScreenID::MainMenu:        next = mainMenu.Tick(dt, auth, charSvc, net); break;
        case ScreenID::Login:           next = login.Tick(dt, auth, net);             break;
        case ScreenID::Register:        next = reg.Tick(dt, auth);                    break;
        case ScreenID::HowToPlay:       next = howToPlay.Tick(dt);                    break;
        case ScreenID::CharacterCreate: next = charCreator.Tick(dt, charSvc);         break;
        case ScreenID::HostLobby:       next = hostLobby.Tick(dt, net);               break;
        case ScreenID::JoinLobby:       next = joinLobby.Tick(dt, net);               break;
        case ScreenID::QuestionCreate:  next = questionCreate.Tick(dt, net);          break;
        case ScreenID::Game:            next = gameScreen.Tick(dt, net);              break;
        case ScreenID::Exit:
            running = false;
            break;
    }
    return next;
}
