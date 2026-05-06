#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "../include/NetworkManager.h"
#include <cstring>
#include <algorithm>

static constexpr uint16_t DISC_PORT          = 7776;
static constexpr uint16_t GAME_PORT_BASE     = 7777;
static constexpr uint16_t GAME_PORT_MAX      = 7787; // try up to 10 ports
static constexpr float    ANNOUNCE_INTERVAL  = 1.0f;
static constexpr float    ENTRY_TIMEOUT      = 6.0f;
static constexpr uint32_t MAGIC              = 0x4E455852; // "NEXR"

struct DiscoveryPacket {
    uint32_t magic;
    char     gameName[32];
    char     hostName[32];
    uint16_t gamePort; // actual TCP port this host is listening on
};

static void MakeNonBlocking(SOCKET s) {
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

NetworkManager::NetworkManager() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

NetworkManager::~NetworkManager() {
    Shutdown();
    WSACleanup();
}

void NetworkManager::Shutdown() {
    auto closeS = [](uintptr_t& h) {
        if (h != ~uintptr_t(0)) { closesocket((SOCKET)h); h = ~uintptr_t(0); }
    };
    closeS(m_listenSock);
    closeS(m_gameSock);
    closeS(m_udpSock);

    m_role               = NetRole::None;
    m_connected          = false;
    m_gamePort           = 0;
    m_startGameReceived  = false;
    m_remoteCharReceived = false;
    m_remoteChar         = {};
    m_announceTimer      = 0.f;
    m_entries.clear();
    m_publicList.clear();
}

// ── Host ──────────────────────────────────────────────────────────────────────

bool NetworkManager::StartHost(const std::string& gameName, const std::string& hostName) {
    Shutdown();
    m_role     = NetRole::Host;
    m_gameName = gameName;
    m_hostName = hostName;

    // TCP listen socket — try ports starting at GAME_PORT_BASE until one is free
    SOCKET ls = INVALID_SOCKET;
    m_gamePort = 0;
    for (uint16_t port = GAME_PORT_BASE; port < GAME_PORT_MAX; ++port) {
        ls = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ls == INVALID_SOCKET) return false;

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(port);
        if (bind(ls, (sockaddr*)&addr, sizeof(addr)) == 0) {
            m_gamePort = port;
            break;
        }
        closesocket(ls);
        ls = INVALID_SOCKET;
    }
    if (ls == INVALID_SOCKET || m_gamePort == 0) return false;

    listen(ls, 1);
    MakeNonBlocking(ls);
    m_listenSock = (uintptr_t)ls;

    // UDP broadcast socket
    SOCKET us = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (us == INVALID_SOCKET) return false;
    int bcast = 1;
    setsockopt(us, SOL_SOCKET, SO_BROADCAST, (char*)&bcast, sizeof(bcast));
    m_udpSock = (uintptr_t)us;

    m_announceTimer = ANNOUNCE_INTERVAL; // broadcast immediately on first Update
    return true;
}

void NetworkManager::StopHost() { Shutdown(); }

void NetworkManager::BroadcastAnnounce() {
    if (m_udpSock == SOCK_NONE) return;

    DiscoveryPacket pkt{};
    pkt.magic    = MAGIC;
    pkt.gamePort = m_gamePort;
    strncpy_s(pkt.gameName, m_gameName.c_str(), 31);
    strncpy_s(pkt.hostName, m_hostName.c_str(), 31);

    sockaddr_in dest{};
    dest.sin_family      = AF_INET;
    dest.sin_addr.s_addr = INADDR_BROADCAST;
    dest.sin_port        = htons(DISC_PORT);

    sendto((SOCKET)m_udpSock, (char*)&pkt, (int)sizeof(pkt), 0,
           (sockaddr*)&dest, (int)sizeof(dest));
}

void NetworkManager::TryAcceptClient() {
    if (m_listenSock == SOCK_NONE || m_connected) return;

    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);
    SOCKET cs = accept((SOCKET)m_listenSock, (sockaddr*)&clientAddr, &addrLen);
    if (cs == INVALID_SOCKET) return;

    MakeNonBlocking(cs);
    m_gameSock  = (uintptr_t)cs;
    m_connected = true;
}

// ── Client discovery ──────────────────────────────────────────────────────────

void NetworkManager::StartDiscovery() {
    if (m_udpSock != SOCK_NONE) return;

    SOCKET us = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (us == INVALID_SOCKET) return;

    MakeNonBlocking(us);
    int reuse = 1;
    setsockopt(us, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(DISC_PORT);
    bind(us, (sockaddr*)&addr, sizeof(addr));

    m_udpSock = (uintptr_t)us;
    m_role    = NetRole::Client;
    m_entries.clear();
    m_publicList.clear();
}

void NetworkManager::StopDiscovery() {
    if (m_udpSock != SOCK_NONE) {
        closesocket((SOCKET)m_udpSock);
        m_udpSock = SOCK_NONE;
    }
    m_entries.clear();
    m_publicList.clear();
    if (!m_connected) m_role = NetRole::None;
}

void NetworkManager::PollDiscoveryPackets(float dt) {
    if (m_udpSock == SOCK_NONE) return;

    // Age all entries; remove stale ones
    for (auto& e : m_entries) e.staleness += dt;
    m_entries.erase(
        std::remove_if(m_entries.begin(), m_entries.end(),
            [](const DiscoveredEntry& e) { return e.staleness > ENTRY_TIMEOUT; }),
        m_entries.end());

    // Drain incoming datagrams
    DiscoveryPacket pkt;
    sockaddr_in     from{};
    int             fromLen = sizeof(from);

    while (true) {
        int r = recvfrom((SOCKET)m_udpSock, (char*)&pkt, (int)sizeof(pkt), 0,
                         (sockaddr*)&from, &fromLen);
        if (r != sizeof(pkt)) break;
        if (pkt.magic != MAGIC) continue;

        pkt.gameName[31] = '\0';
        pkt.hostName[31] = '\0';

        char ipBuf[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, &from.sin_addr, ipBuf, sizeof(ipBuf));

        bool found = false;
        for (auto& e : m_entries) {
            if (e.game.hostIP == ipBuf && e.game.gameName == pkt.gameName) {
                e.staleness = 0.f;
                found = true;
                break;
            }
        }
        if (!found) {
            DiscoveredEntry e;
            e.game.gameName = pkt.gameName;
            e.game.hostName = pkt.hostName;
            e.game.hostIP   = ipBuf;
            e.game.port     = pkt.gamePort ? pkt.gamePort : GAME_PORT_BASE;
            m_entries.push_back(e);
        }
    }

    // Rebuild public list
    m_publicList.clear();
    for (auto& e : m_entries)
        m_publicList.push_back(e.game);
}

// ── Client connect ────────────────────────────────────────────────────────────

bool NetworkManager::Connect(const std::string& hostIP, uint16_t port, std::string& errOut) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) { errOut = "Failed to create socket."; return false; }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, hostIP.c_str(), &addr.sin_addr);

    // Non-blocking connect + select for 2-second timeout
    MakeNonBlocking(s);
    connect(s, (sockaddr*)&addr, sizeof(addr));

    fd_set wset, eset;
    FD_ZERO(&wset); FD_SET(s, &wset);
    FD_ZERO(&eset); FD_SET(s, &eset);
    timeval tv{ 2, 0 };

    int r = select(0, nullptr, &wset, &eset, &tv);
    if (r <= 0 || FD_ISSET(s, &eset)) {
        closesocket(s);
        errOut = "Could not reach host. Make sure they are hosting.";
        return false;
    }

    int sockErr = 0, len = sizeof(sockErr);
    getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&sockErr, &len);
    if (sockErr != 0) {
        closesocket(s);
        errOut = "Connection refused by host.";
        return false;
    }

    MakeNonBlocking(s);
    m_gameSock  = (uintptr_t)s;
    m_connected = true;
    m_role      = NetRole::Client;
    return true;
}

// ── Messaging ─────────────────────────────────────────────────────────────────

// Message types:  0x02 = StartGame (1 byte total)
//                 0x03 = CharacterData (1 + 24 bytes = 25 bytes total)

void NetworkManager::SendCharacterData(const CharacterData& data) {
    if (!m_connected || m_gameSock == SOCK_NONE) return;
    uint8_t buf[25];
    buf[0] = 0x03;
    memcpy(buf +  1, &data.gender,   4);
    memcpy(buf +  5, &data.skinIdx,  4);
    memcpy(buf +  9, &data.hairIdx,  4);
    memcpy(buf + 13, &data.topIdx,   4);
    memcpy(buf + 17, &data.pantsIdx, 4);
    memcpy(buf + 21, &data.feetIdx,  4);
    send((SOCKET)m_gameSock, (char*)buf, 25, 0);
}

bool NetworkManager::PollRemoteCharacterData(CharacterData& out) {
    if (!m_remoteCharReceived) return false;
    m_remoteCharReceived = false;
    out = m_remoteChar;
    return true;
}

void NetworkManager::SendStartGame() {
    if (!m_connected || m_gameSock == SOCK_NONE) return;
    uint8_t msg = 0x02;
    send((SOCKET)m_gameSock, (char*)&msg, 1, 0);
}

bool NetworkManager::PollStartGame() {
    return m_startGameReceived;
}

bool NetworkManager::PollDisconnected() {
    if (!m_remoteDisconnected) return false;
    m_remoteDisconnected = false;
    return true;
}

void NetworkManager::HandleDisconnect() {
    if (m_gameSock != SOCK_NONE) {
        closesocket((SOCKET)m_gameSock);
        m_gameSock = SOCK_NONE;
    }
    m_connected          = false;
    m_remoteDisconnected = true;
    m_startGameReceived  = false;
    m_remoteCharReceived = false;
    m_remoteChar         = {};
    m_announceTimer      = ANNOUNCE_INTERVAL; // host will broadcast again immediately
}

// Drains the TCP receive buffer and dispatches messages by type.
// Detects disconnection via recv returning 0 (graceful) or an error.
void NetworkManager::PollMessages() {
    if (!m_connected || m_gameSock == SOCK_NONE) return;
    SOCKET s = (SOCKET)m_gameSock;

    while (true) {
        u_long avail = 0;
        ioctlsocket(s, FIONREAD, &avail);

        if (avail == 0) {
            // No data — peek to check if the connection is still alive
            uint8_t tmp;
            int r = recv(s, (char*)&tmp, 1, MSG_PEEK);
            if (r == 0 || (r == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK))
                HandleDisconnect();
            break;
        }

        // Peek the type byte without consuming it
        uint8_t type = 0;
        int peekResult = recv(s, (char*)&type, 1, MSG_PEEK);
        if (peekResult != 1) { HandleDisconnect(); break; }

        if (type == 0x02) {
            recv(s, (char*)&type, 1, 0);
            m_startGameReceived = true;
        } else if (type == 0x03) {
            if (avail < 25) break;
            uint8_t buf[25];
            recv(s, (char*)buf, 25, 0);
            memcpy(&m_remoteChar.gender,   buf +  1, 4);
            memcpy(&m_remoteChar.skinIdx,  buf +  5, 4);
            memcpy(&m_remoteChar.hairIdx,  buf +  9, 4);
            memcpy(&m_remoteChar.topIdx,   buf + 13, 4);
            memcpy(&m_remoteChar.pantsIdx, buf + 17, 4);
            memcpy(&m_remoteChar.feetIdx,  buf + 21, 4);
            m_remoteCharReceived = true;
        } else {
            recv(s, (char*)&type, 1, 0);
        }
    }
}

// ── Update ────────────────────────────────────────────────────────────────────

void NetworkManager::Update(float dt) {
    if (m_role == NetRole::Host) {
        if (!m_connected) {
            m_announceTimer += dt;
            if (m_announceTimer >= ANNOUNCE_INTERVAL) {
                BroadcastAnnounce();
                m_announceTimer = 0.f;
            }
            TryAcceptClient();
        } else {
            PollMessages();
        }
    } else if (m_role == NetRole::Client) {
        if (!m_connected)
            PollDiscoveryPackets(dt);
        else
            PollMessages();
    }
}
