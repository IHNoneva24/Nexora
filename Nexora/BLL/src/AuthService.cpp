#include "AuthService.h"

AuthService::AuthService(UserRepository& repo) : m_repo(repo) {}

// ── Validation ────────────────────────────────────────────────────────────────
bool AuthService::IsValidUsername(const std::string& u) {
    if (u.size() < 3 || u.size() > 20) return false;
    for (char c : u)
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') return false;
    return true;
}

bool AuthService::IsValidPassword(const std::string& p) {
    return p.size() >= 6;
}

// ── Register ──────────────────────────────────────────────────────────────────
std::string AuthService::Register(const std::string& username,
                                  const std::string& password,
                                  const std::string& confirmPassword) {
    if (username.empty() || password.empty())
        return "Fields cannot be empty.";

    if (!IsValidUsername(username))
        return "Username: 3-20 chars, letters/digits/underscore only.";

    if (!IsValidPassword(password))
        return "Password must be at least 6 characters.";

    if (password != confirmPassword)
        return "Passwords do not match.";

    if (m_repo.UsernameExists(username))
        return "Username already taken.";

    if (!m_repo.CreateUser(username, password))
        return "Registration failed (database error).";

    return "";  // success
}

// ── Login ─────────────────────────────────────────────────────────────────────
std::string AuthService::Login(const std::string& username,
                               const std::string& password) {
    if (username.empty() || password.empty())
        return "Fields cannot be empty.";

    if (!m_repo.VerifyPassword(username, password))
        return "Invalid username or password.";

    auto user = m_repo.FindByUsername(username);
    m_loggedIn      = true;
    m_currentUser   = username;
    m_currentUserId = user.has_value() ? user->id : 0;
    return "";  // success
}

// ── Logout ────────────────────────────────────────────────────────────────────
void AuthService::Logout() {
    m_loggedIn      = false;
    m_currentUser.clear();
    m_currentUserId = 0;
}
