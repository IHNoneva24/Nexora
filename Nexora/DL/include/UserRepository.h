#pragma once
#include <optional>
#include "Database.h"
#include "../../BLL/include/User.h"

// ─────────────────────────────────────────────
//  DATA LAYER – UserRepository
//  All SQL queries related to the users table.
//  Business layer calls this; never raw SQLite.
// ─────────────────────────────────────────────
class UserRepository {
public:
    explicit UserRepository(Database& db);

    // Creates the users table if it doesn't exist
    void InitTable();

    // Insert a new user (hashes password internally); returns false if username already exists
    bool CreateUser(const std::string& username, const std::string& password);

    // Returns true if username exists and password matches
    bool VerifyPassword(const std::string& username, const std::string& password);

    // Find by username; returns nullopt if not found
    std::optional<User> FindByUsername(const std::string& username);

    // Check whether a username is already taken
    bool UsernameExists(const std::string& username);

private:
    Database& m_db;

    static std::string HashPassword(const std::string& password);
};
