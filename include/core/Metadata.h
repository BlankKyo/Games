#pragma once
#include <QString>
#include <QDateTime>

// ─────────────────────────────────────────────────────────────────────────────
// GameMetadata
//   Plain data object that describes a game entry.
//   Stored in the DB and passed around the UI.
// ─────────────────────────────────────────────────────────────────────────────
struct GameMetadata {
    int       id          = -1;
    QString   key;           // unique string key used by GameRegistry, e.g. "snake"
    QString   title;
    QString   description;
    QString   author;
    QString   version      = "1.0";
    QDateTime createdAt;
    QDateTime lastPlayed;
    int       timesPlayed  = 0;
    int       highScore    = 0;
    bool      isBuiltin    = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UserRecord — plain data struct for a user account
// ─────────────────────────────────────────────────────────────────────────────
struct UserRecord {
    int     id           = -1;
    QString username;
    QString displayName;
    QString passwordHash;
    QString role;          // "admin" | "user"
    QString avatarColor;
    QString createdAt;
};
