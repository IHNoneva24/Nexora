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
static constexpr uint16_t SESSION_PORT       = 7775;
static constexpr uint32_t SESSION_MAGIC      = 0x4E455853; // "NEXS"

struct SessionPacket {
    uint32_t magic;
    char     username[32];
};

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
    StopSessionBroadcast();
    if (m_checkSock != SOCK_NONE) {
        closesocket((SOCKET)m_checkSock);
        m_checkSock = SOCK_NONE;
    }
    WSACleanup();
}

void NetworkManager::Shutdown() {
    auto closeS = [](uintptr_t& h) {
        if (h != ~uintptr_t(0)) { closesocket((SOCKET)h); h = ~uintptr_t(0); }
    };
    closeS(m_listenSock);
    closeS(m_gameSock);
    closeS(m_udpSock);
    // Note: m_sessionSock is NOT closed here — it lives across game sessions

    m_role               = NetRole::None;
    m_connected          = false;
    m_gamePort           = 0;
    m_startGameReceived         = false;
    m_remoteCharReceived        = false;
    m_remoteUsernameReceived    = false;
    m_remoteQuestionsReceived   = false;
    m_remoteAnswerReceived      = false;
    m_remoteAnswer              = -1;
    m_remoteChar                = {};
    m_remoteUsername.clear();
    m_remoteQuestions.clear();
    m_announceTimer          = 0.f;
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
//                 0x04 = Username     (1 + 32 bytes = 33 bytes total)

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

void NetworkManager::SendUsername(const std::string& username) {
    if (!m_connected || m_gameSock == SOCK_NONE) return;
    uint8_t buf[33] = {};
    buf[0] = 0x04;
    strncpy_s((char*)buf + 1, 32, username.c_str(), 31);
    send((SOCKET)m_gameSock, (char*)buf, 33, 0);
}

bool NetworkManager::PollRemoteUsername(std::string& out) {
    if (!m_remoteUsernameReceived) return false;
    m_remoteUsernameReceived = false;
    out = m_remoteUsername;
    return true;
}

void NetworkManager::DisconnectClient() {
    HandleDisconnect();
}

void NetworkManager::SendStartGame() {
    if (!m_connected || m_gameSock == SOCK_NONE) return;
    uint8_t msg = 0x02;
    send((SOCKET)m_gameSock, (char*)&msg, 1, 0);
}

bool NetworkManager::PollStartGame() {
    if (!m_startGameReceived) return false;
    m_startGameReceived = false;
    return true;
}

// Message type 0x05 — Questions batch
// Per question (391 bytes): type(1) text(129) choice0(65) choice1(65) choice2(65) choice3(65) correctIdx(1)
static constexpr int QN_TEXT_SIZE   = 129;
static constexpr int QN_CHOICE_SIZE = 65;
static constexpr int QN_ENTRY_SIZE  = 1 + QN_TEXT_SIZE + 4 * QN_CHOICE_SIZE + 1; // 391

void NetworkManager::SendQuestions(const std::vector<QuestionData>& questions) {
    if (!m_connected || m_gameSock == SOCK_NONE) return;

    uint8_t count = (uint8_t)questions.size();
    int totalSize = 2 + count * QN_ENTRY_SIZE;
    std::vector<uint8_t> buf(totalSize, 0);
    buf[0] = 0x05;
    buf[1] = count;

    for (int i = 0; i < (int)count; ++i) {
        const QuestionData& q = questions[i];
        uint8_t* p = buf.data() + 2 + i * QN_ENTRY_SIZE;
        p[0] = (uint8_t)q.type;
        strncpy_s((char*)(p + 1), QN_TEXT_SIZE, q.text.c_str(), QN_TEXT_SIZE - 1);
        for (int c = 0; c < 4; ++c)
            strncpy_s((char*)(p + 1 + QN_TEXT_SIZE + c * QN_CHOICE_SIZE),
                      QN_CHOICE_SIZE, q.choices[c].c_str(), QN_CHOICE_SIZE - 1);
        p[QN_ENTRY_SIZE - 1] = (uint8_t)q.correctIdx;
    }

    int sent = 0;
    while (sent < totalSize) {
        int r = send((SOCKET)m_gameSock, (char*)buf.data() + sent, totalSize - sent, 0);
        if (r <= 0) break;
        sent += r;
    }
}

bool NetworkManager::PollRemoteQuestions(std::vector<QuestionData>& out) {
    if (!m_remoteQuestionsReceived) return false;
    m_remoteQuestionsReceived = false;
    out = std::move(m_remoteQuestions);
    m_remoteQuestions.clear();
    return true;
}

void NetworkManager::SendAnswer(int answerIdx) {
    if (!m_connected || m_gameSock == SOCK_NONE) return;
    uint8_t buf[2] = { 0x06, (uint8_t)answerIdx };
    send((SOCKET)m_gameSock, (char*)buf, 2, 0);
}

bool NetworkManager::PollRemoteAnswer(int& out) {
    if (!m_remoteAnswerReceived) return false;
    m_remoteAnswerReceived = false;
    out = m_remoteAnswer;
    return true;
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
    m_connected              = false;
    m_remoteDisconnected     = true;
    m_startGameReceived         = false;
    m_remoteCharReceived        = false;
    m_remoteUsernameReceived    = false;
    m_remoteQuestionsReceived   = false;
    m_remoteAnswerReceived      = false;
    m_remoteAnswer              = -1;
    m_remoteChar                = {};
    m_remoteUsername.clear();
    m_remoteQuestions.clear();
    m_announceTimer          = ANNOUNCE_INTERVAL; // host will broadcast again immediately
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
        } else if (type == 0x04) {
            if (avail < 33) break;
            uint8_t buf[33];
            recv(s, (char*)buf, 33, 0);
            buf[32] = '\0';
            m_remoteUsername = (const char*)(buf + 1);
            m_remoteUsernameReceived = true;
        } else if (type == 0x05) {
            if (avail < 2) break;
            uint8_t header[2];
            recv(s, (char*)header, 2, MSG_PEEK);
            uint8_t count = header[1];
            int expected = 2 + count * QN_ENTRY_SIZE;
            if ((int)avail < expected) break;
            std::vector<uint8_t> qbuf(expected);
            recv(s, (char*)qbuf.data(), expected, 0);
            m_remoteQuestions.clear();
            for (int i = 0; i < (int)count; ++i) {
                const uint8_t* p = qbuf.data() + 2 + i * QN_ENTRY_SIZE;
                QuestionData q;
                q.type       = (QuestionData::Type)p[0];
                q.text       = (char*)(p + 1);
                for (int c = 0; c < 4; ++c)
                    q.choices[c] = (char*)(p + 1 + QN_TEXT_SIZE + c * QN_CHOICE_SIZE);
                q.correctIdx = p[QN_ENTRY_SIZE - 1];
                m_remoteQuestions.push_back(q);
            }
            m_remoteQuestionsReceived = true;
        } else if (type == 0x06) {
            if (avail < 2) break;
            uint8_t buf[2];
            recv(s, (char*)buf, 2, 0);
            m_remoteAnswer         = buf[1];
            m_remoteAnswerReceived = true;
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

    // Session presence broadcast (runs independently of game networking)
    if (m_sessionSock != SOCK_NONE) {
        m_sessionTimer += dt;
        if (m_sessionTimer >= ANNOUNCE_INTERVAL) {
            BroadcastSession();
            m_sessionTimer = 0.f;
        }
    }
}

// ── Session presence ──────────────────────────────────────────────────────────

void NetworkManager::StartSessionBroadcast(const std::string& username) {
    StopSessionBroadcast();
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) return;
    int bcast = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&bcast, sizeof(bcast));
    m_sessionSock     = (uintptr_t)s;
    m_sessionUsername  = username;
    m_sessionTimer    = ANNOUNCE_INTERVAL; // broadcast immediately
}

void NetworkManager::StopSessionBroadcast() {
    if (m_sessionSock != SOCK_NONE) {
        closesocket((SOCKET)m_sessionSock);
        m_sessionSock = SOCK_NONE;
    }
    m_sessionUsername.clear();
    m_sessionTimer = 0.f;
}

void NetworkManager::BroadcastSession() {
    if (m_sessionSock == SOCK_NONE) return;
    SessionPacket pkt{};
    pkt.magic = SESSION_MAGIC;
    strncpy_s(pkt.username, m_sessionUsername.c_str(), 31);

    sockaddr_in dest{};
    dest.sin_family      = AF_INET;
    dest.sin_addr.s_addr = INADDR_BROADCAST;
    dest.sin_port        = htons(SESSION_PORT);
    sendto((SOCKET)m_sessionSock, (char*)&pkt, (int)sizeof(pkt), 0,
           (sockaddr*)&dest, (int)sizeof(dest));
}

static constexpr float CHECK_DURATION = 1.5f; // listen window for duplicate check

void NetworkManager::BeginUsernameCheck(const std::string& username) {
    // Close any previous check socket
    if (m_checkSock != SOCK_NONE) {
        closesocket((SOCKET)m_checkSock);
        m_checkSock = SOCK_NONE;
    }

    m_checkUsername = username;
    m_checkTimer   = 0.f;
    m_checkDone    = false;
    m_checkFound   = false;

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) { m_checkDone = true; return; }

    MakeNonBlocking(s);
    int reuse = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(SESSION_PORT);

    if (bind(s, (sockaddr*)&addr, sizeof(addr)) != 0) {
        closesocket(s);
        m_checkDone = true;
        return;
    }

    m_checkSock = (uintptr_t)s;
}

void NetworkManager::UpdateUsernameCheck(float dt) {
    if (m_checkDone || m_checkSock == SOCK_NONE) return;

    m_checkTimer += dt;

    // Drain all pending session packets (non-blocking)
    SessionPacket pkt;
    sockaddr_in from{};
    int fromLen;

    while (true) {
        fromLen = sizeof(from);
        int r = recvfrom((SOCKET)m_checkSock, (char*)&pkt, (int)sizeof(pkt), 0,
                         (sockaddr*)&from, &fromLen);
        if (r != sizeof(pkt)) break;
        if (pkt.magic != SESSION_MAGIC) continue;
        pkt.username[31] = '\0';
        if (m_checkUsername == pkt.username) {
            m_checkFound = true;
            break;
        }
    }

    // Finish early if found, or after the listen window expires
    if (m_checkFound || m_checkTimer >= CHECK_DURATION) {
        closesocket((SOCKET)m_checkSock);
        m_checkSock = SOCK_NONE;
        m_checkDone = true;
    }
}

bool NetworkManager::IsUsernameCheckDone() const { return m_checkDone; }
bool NetworkManager::WasUsernameFound()    const { return m_checkFound; }
