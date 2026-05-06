#pragma once
#include "../../DL/include/CharacterData.h"
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
    void SendStartGame();       // host → client
    bool PollStartGame();       // client polls; returns true once
    bool PollDisconnected();    // both sides; returns true once when remote drops

    // ── Per-frame tick ────────────────────────────────────────────────────────
    void Update(float dt);

    void    Shutdown();
    NetRole GetRole()     const { return m_role; }
    bool    IsConnected() const { return m_connected; }

    const std::string& GetGameName() const { return m_gameName; }

private:
    void BroadcastAnnounce();
    void PollDiscoveryPackets(float dt);
    void TryAcceptClient();
    void PollMessages();      // drains TCP stream and dispatches by message type
    void HandleDisconnect();  // cleans up game socket; host resumes broadcasting

    NetRole   m_role      = NetRole::None;
    bool      m_connected = false;
    uint16_t  m_gamePort  = 0;
    bool      m_startGameReceived    = false;
    bool      m_remoteCharReceived   = false;
    bool      m_remoteDisconnected   = false;
    CharacterData m_remoteChar       = {};

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
};
