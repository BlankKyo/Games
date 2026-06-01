// include/core/Database.h
#pragma once
#include <QSqlDatabase>
#include <QList>
#include <QString>
#include "Metadata.h"
// ─────────────────────────────────────────────────────────────────────────────
// Database — thin SQLite wrapper
// ─────────────────────────────────────────────────────────────────────────────
class Database {
public:
    static Database& instance() {
        static Database db;
        return db;
    }

    bool open();
    void close();
    bool isOpen() const;

    // ── Game metadata ─────────────────────────────────────────────────────
    bool                  upsertGame(GameMetadata& meta);
    bool                  deleteGame(int id);
    GameMetadata          gameByKey(const QString& key) const;
    QList<GameMetadata>   allGames() const;

    // ── Scores ────────────────────────────────────────────────────────────
    bool  recordScore(const QString& gameKey, int score);
    int   highScore(const QString& gameKey) const;

    struct ScoreEntry {
        int       score;
        QDateTime playedAt;
    };
    QList<ScoreEntry> topScores(const QString& gameKey, int limit = 10) const;

    // ── Users ─────────────────────────────────────────────────────────────
    bool                authenticate(const QString& username,
                                     const QString& password,
                                     UserRecord& outUser) const;

    bool                createUser(const QString& username,
                                   const QString& displayName,
                                   const QString& password,
                                   const QString& role,
                                   const QString& avatarColor);

    bool                deleteUser(int id);
    bool                usernameExists(const QString& username) const;
    QList<UserRecord>   allUsers() const;

private:
    Database();
    ~Database();
    bool migrate();
    void seedAdmin();
    static QString hashPassword(const QString& password);

    QSqlDatabase m_db;
};



