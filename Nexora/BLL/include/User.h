#pragma once
#include <string>

// ─────────────────────────────────────────────
//  User  –  plain data object (DTO)
//  Shared between Data and Business layers.
//  The Presentation layer only sees username.
// ─────────────────────────────────────────────
struct User {
    int         id           = 0;
    std::string username;
    std::string passwordHash;   // djb2 hash stored as decimal string
};
