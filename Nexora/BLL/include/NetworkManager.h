#pragma once
#include "../../DL/include/CharacterData.h"
<<<<<<< HEAD:Nexora/BLL/include/NetworkManager.h
#include "QuestionData.h"
=======
#include "../../BLL/include/QuestionData.h"
>>>>>>> a71f47d (Add game):Nexora/PL/include/NetworkManager.h
#include <string>
#include <vector>
#include <cstdint>

struct DiscoveredGame {
    std::string gameName;
    std::string hostName;
    std::string hostIP;
    uint16_t    port = 7777;
};

enum class NetRole { None, Host, Client };

// ─────────────────────────────────────────────────────────────────────────────
//  NetworkManager  —  LAN multiplayer via Winsock (no external libraries).
//
//  Discovery : host broadcasts UDP on port 7776 every second.
//              client listens on UDP 7776 and collects announcements.
//  Connection: TCP on port 7777.
//  Messaging : single-byte type prefix  (0x02 = StartGame).
//
//  Call Update(dt) once per frame from main.cpp.
// ─────────────────────────────────────────────────────────────────────────────
class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // ── Host side ─────────────────────────────────────────────────────────────
    bool StartHost(const std::string& gameName, const std::string& hostName);
    void StopHost();
    bool IsClientConnected() const { return m_connected; }

    // ── Client discovery ──────────────────────────────────────────────────────
    void StartDiscovery();
    void StopDiscovery();
    const std::vector<DiscoveredGame>& GetDiscoveredGames() const { return m_publicList; }

    // ── Client connect ────────────────────────────────────────────────────────
    // Blocking with ~2 s LAN timeout.  Fills errOut on failure.
    bool Connect(const std::string& hostIP, uint16_t port, std::string& errOut);

    // ── Shared messaging (once connected) ─────────────────────────────────────
    void SendCharacterData(const CharacterData& data);  // call right after connect
    bool PollRemoteCharacterData(CharacterData& out);   // returns true once per receive
    void SendUsername(const std::string& username);      // send our username to remote
    bool PollRemoteUsername(std::string& out);           // returns true once per receive
    const std::string& GetRemoteUsername() const { return m_remoteUsername; }
    void SendStartGame();       // host → client
    bool PollStartGame();       // client polls; returns true once
    bool PollDisconnected();    // both sides; returns true once when remote drops

    void SendQuestions(const std::vector<QuestionData>& questions);
    bool PollRemoteQuestions(std::vector<QuestionData>& out);

    void SendAnswer(int answerIdx);           // 0-3
    bool PollRemoteAnswer(int& out);          // returns true once per receive
    void DisconnectClient();    // host kicks the connected client

    // ── Per-frame tick ────────────────────────────────────────────────────────
    void Update(float dt);

    void    Shutdown();
    NetRole GetRole()     const { return m_role; }
    bool    IsConnected() const { return m_connected; }

    const std::string& GetGameName() const { return m_gameName; }

    // ── Session presence (duplicate-login prevention) ─────────────────────────
    void StartSessionBroadcast(const std::string& username);
    void StopSessionBroadcast();

    // Non-blocking duplicate-login check (call over multiple frames):
    void BeginUsernameCheck(const std::string& username); // open listener
    bool IsUsernameCheckDone() const;   // true once listen window elapsed
    bool WasUsernameFound() const;      // result (only valid after done)
    void UpdateUsernameCheck(float dt); // call each frame while checking

private:
    void BroadcastAnnounce();
    void PollDiscoveryPackets(float dt);
    void TryAcceptClient();
    void PollMessages();      // drains TCP stream and dispatches by message type
    void HandleDisconnect();  // cleans up game socket; host resumes broadcasting
    void BroadcastSession();  // sends session presence packet

    NetRole   m_role      = NetRole::None;
    bool      m_connected = false;
    uint16_t  m_gamePort  = 0;
    bool      m_startGameReceived    = false;
    bool      m_remoteCharReceived   = false;
    bool      m_remoteDisconnected   = false;
    bool      m_remoteUsernameReceived  = false;
    bool      m_remoteQuestionsReceived = false;
    bool      m_remoteAnswerReceived    = false;
    int       m_remoteAnswer            = -1;
    CharacterData m_remoteChar       = {};
    std::string   m_remoteUsername;
    std::vector<QuestionData> m_remoteQuestions;

    // Winsock SOCKET stored as uintptr_t to keep winsock out of the header.
    static constexpr uintptr_t SOCK_NONE = ~uintptr_t(0);
    uintptr_t m_listenSock = SOCK_NONE;
    uintptr_t m_gameSock   = SOCK_NONE;
    uintptr_t m_udpSock    = SOCK_NONE;

    std::string m_gameName;
    std::string m_hostName;
    float       m_announceTimer = 0.f;

    struct DiscoveredEntry {
        DiscoveredGame game;
        float          staleness = 0.f; // seconds since last seen; pruned at 6 s
    };
    std::vector<DiscoveredEntry> m_entries;
    std::vector<DiscoveredGame>  m_publicList;

    // Session presence
    uintptr_t   m_sessionSock      = SOCK_NONE;
    std::string m_sessionUsername;
    float       m_sessionTimer     = 0.f;

    // Non-blocking duplicate-login check state
    uintptr_t   m_checkSock        = SOCK_NONE;
    std::string m_checkUsername;
    float       m_checkTimer       = 0.f;
    bool        m_checkDone        = false;
    bool        m_checkFound       = false;
};
