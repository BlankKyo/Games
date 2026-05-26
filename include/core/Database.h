#pragma once
#include <QSqlDatabase>
#include <QList>
#include "GameMetadata.h"

// ─────────────────────────────────────────────────────────────────────────────
// Database
//   Thin wrapper around a local SQLite database (~/.gamehub/gamehub.db).
//   Handles schema migration, game metadata CRUD, and score recording.
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
    bool                  upsertGame(GameMetadata& meta);   // inserts or updates; sets meta.id
    bool                  deleteGame(int id);
    GameMetadata          gameByKey(const QString& key) const;
    QList<GameMetadata>   allGames() const;

    // ── Scores ────────────────────────────────────────────────────────────
    bool  recordScore(const QString& gameKey, int score);
    int   highScore(const QString& gameKey) const;

    struct ScoreEntry {
        int      score;
        QDateTime playedAt;
    };
    QList<ScoreEntry> topScores(const QString& gameKey, int limit = 10) const;

private:
    Database();
    ~Database();
    bool migrate();

    QSqlDatabase m_db;
};
