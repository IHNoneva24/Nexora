#pragma once
#include <string>
#include <optional>
#include "../../DL/include/UserRepository.h"

// ─────────────────────────────────────────────
//  BUSINESS LAYER – AuthService
//  Handles all authentication logic:
//    • Password hashing (djb2 – demo purposes;
//      use bcrypt / Argon2 in production)
//    • Input validation
//    • Session state (who is logged in)
// ─────────────────────────────────────────────
class AuthService {
public:
    explicit AuthService(UserRepository& repo);

    // Register a new user.
    // Returns "" on success, or an error message.
    std::string Register(const std::string& username,
                         const std::string& password,
                         const std::string& confirmPassword);

    // Login an existing user.
    // Returns "" on success, or an error message.
    std::string Login(const std::string& username,
                      const std::string& password);

    void Logout();

    bool        IsLoggedIn()       const { return m_loggedIn; }
    std::string GetUsername()      const { return m_currentUser; }
    int         GetUserId()        const { return m_currentUserId; }

private:
    UserRepository& m_repo;
    bool            m_loggedIn      = false;
    std::string     m_currentUser;
    int             m_currentUserId = 0;

    // djb2 hash → stored as decimal string
    // NOTE: Not cryptographically secure – use bcrypt/Argon2 in production!
    static bool IsValidUsername(const std::string& username);
    static bool IsValidPassword(const std::string& password);
};
