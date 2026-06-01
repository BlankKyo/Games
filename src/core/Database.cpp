#include "core/Database.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QVariant>
#include <QCryptographicHash>

static const char* TAG = "Database";

Database::Database() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
}

Database::~Database() {
    close();
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}
bool Database::open() {
    try {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        const QString path = dir + "/gamehub.db";
        LOG_DEBUG(TAG, "Opening database at: " + path.toStdString());

        m_db = QSqlDatabase::addDatabase("QSQLITE", "gamehub");
        m_db.setDatabaseName(path);

        if (!m_db.open()) {
            LOG_ERROR(TAG, "Failed to open DB: " + m_db.lastError().text().toStdString());
            return false;
        }
        return migrate();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during database open: " + std::string(e.what()));
        return false;
    }
}

void Database::close() {
    try {
        m_db.close();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during database close: " + std::string(e.what()));
    }
}

bool Database::isOpen() const {
    try {
        return m_db.isOpen();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception while checking database open status: " + std::string(e.what()));
        return false;
    }
}

bool Database::migrate() {
    try {
        QSqlQuery q(m_db);
        q.exec("PRAGMA journal_mode=WAL;");
        q.exec("PRAGMA foreign_keys=ON;");

        bool ok = true;
        ok &= q.exec(R"(
            CREATE TABLE IF NOT EXISTS users (
                id            INTEGER PRIMARY KEY AUTOINCREMENT,
                username      TEXT    NOT NULL UNIQUE,
                display_name  TEXT    NOT NULL,
                password_hash TEXT    NOT NULL,
                role          TEXT    NOT NULL DEFAULT 'user',
                avatar_color  TEXT    NOT NULL DEFAULT '#7c3aed',
                created_at    TEXT    DEFAULT (datetime('now'))
            );
        )");

        ok &= q.exec(R"(
            CREATE TABLE IF NOT EXISTS games (
                id           INTEGER PRIMARY KEY AUTOINCREMENT,
                key          TEXT    NOT NULL UNIQUE,
                title        TEXT    NOT NULL,
                description  TEXT,
                author       TEXT,
                version      TEXT    DEFAULT '1.0',
                is_builtin   INTEGER DEFAULT 0,
                times_played INTEGER DEFAULT 0,
                high_score   INTEGER DEFAULT 0,
                created_at   TEXT    DEFAULT (datetime('now')),
                last_played  TEXT
            );
        )");

        ok &= q.exec(R"(
            CREATE TABLE IF NOT EXISTS scores (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id    INTEGER NOT NULL,
                game_key   TEXT    NOT NULL,
                score      INTEGER NOT NULL,
                played_at  TEXT    DEFAULT (datetime('now')),
                FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE,
                FOREIGN KEY(game_key) REFERENCES games(key) ON DELETE CASCADE
            );
        )");

        if (!ok)
            LOG_WARNING(TAG, "Database::migrate error: " + q.lastError().text().toStdString());

        return ok;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during database migration: " + std::string(e.what()));
        return false;
    }
}

bool Database::upsertGame(GameMetadata& meta) {
    try {
        QSqlQuery q(m_db);
        q.prepare(R"(
            INSERT INTO games (key, title, description, author, version, is_builtin)
            VALUES (:key, :title, :desc, :author, :ver, :builtin)
            ON CONFLICT(key) DO UPDATE SET
                title       = excluded.title,
                description = excluded.description,
                author      = excluded.author,
                version     = excluded.version
        )");
        q.bindValue(":key",     meta.key);
        q.bindValue(":title",   meta.title);
        q.bindValue(":desc",    meta.description);
        q.bindValue(":author",  meta.author);
        q.bindValue(":ver",     meta.version);
        q.bindValue(":builtin", meta.isBuiltin ? 1 : 0);

        if (!q.exec()) {
            LOG_ERROR(TAG, "upsertGame failed: " + q.lastError().text().toStdString());
            return false;
        }

        // Retrieve the id
        QSqlQuery id_q(m_db);
        id_q.prepare("SELECT id FROM games WHERE key = :key");
        id_q.bindValue(":key", meta.key);
        if (id_q.exec() && id_q.next())
            meta.id = id_q.value(0).toInt();

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during upsertGame: " + std::string(e.what()));
        return false;
    }
}

bool Database::deleteGame(int id) {
    try {
        QSqlQuery q(m_db);
        q.prepare("DELETE FROM games WHERE id = :id AND is_builtin = 0");
        q.bindValue(":id", id);
        return q.exec();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during deleteGame: " + std::string(e.what()));
        return false;
    }
}

static GameMetadata rowToMeta(QSqlQuery& q) {
    GameMetadata m;
    m.id          = q.value("id").toInt();
    m.key         = q.value("key").toString();
    m.title       = q.value("title").toString();
    m.description = q.value("description").toString();
    m.author      = q.value("author").toString();
    m.version     = q.value("version").toString();
    m.isBuiltin   = q.value("is_builtin").toBool();
    m.timesPlayed = q.value("times_played").toInt();
    m.highScore   = q.value("high_score").toInt();
    m.createdAt   = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    m.lastPlayed  = QDateTime::fromString(q.value("last_played").toString(), Qt::ISODate);
    return m;
}

GameMetadata Database::gameByKey(const QString& key) const {
    try {
        QSqlQuery q(m_db);
        q.prepare("SELECT * FROM games WHERE key = :key");
        q.bindValue(":key", key);
        if (q.exec() && q.next()) return rowToMeta(q);
        return {};
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during gameByKey: " + std::string(e.what()));
        return {};
    }
}

QList<GameMetadata> Database::allGames() const {
    try {
        QSqlQuery q("SELECT * FROM games ORDER BY is_builtin DESC, title ASC", m_db);
        QList<GameMetadata> list;
        while (q.next()) list.append(rowToMeta(q));
        return list;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during allGames: " + std::string(e.what()));
        return {};
    }
}

bool Database::recordScore(const QString& gameKey, int score) {
    try {
        QSqlQuery q(m_db);
        q.prepare("INSERT INTO scores (game_key, score) VALUES (:key, :score)");
        q.bindValue(":key",   gameKey);
        q.bindValue(":score", score);
        if (!q.exec()) {
            LOG_ERROR(TAG, "recordScore failed: " + q.lastError().text().toStdString());
            return false;
        }

        // Update high score and play count
        q.prepare(R"(
            UPDATE games SET
                times_played = times_played + 1,
                last_played  = datetime('now'),
                high_score   = MAX(high_score, :score)
            WHERE key = :key
        )");
        q.bindValue(":score", score);
        q.bindValue(":key",   gameKey);
        return q.exec();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during recordScore: " + std::string(e.what()));
        return false;
    }
}

int Database::highScore(const QString& gameKey) const {
    try {
        QSqlQuery q(m_db);
        q.prepare("SELECT high_score FROM games WHERE key = :key");
        q.bindValue(":key", gameKey);
        if (q.exec() && q.next()) return q.value(0).toInt();
        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during highScore: " + std::string(e.what()));
        return 0;
    }
}

QList<Database::ScoreEntry> Database::topScores(const QString& gameKey, int limit) const {
    try {
        QSqlQuery q(m_db);
        q.prepare("SELECT score, played_at FROM scores WHERE game_key = :key ORDER BY score DESC LIMIT :lim");
        q.bindValue(":key", gameKey);
        q.bindValue(":lim", limit);
        QList<ScoreEntry> list;
        q.exec();
        while (q.next()) {
            list.append({ q.value(0).toInt(),
                        QDateTime::fromString(q.value(1).toString(), Qt::ISODate) });
        }
        return list;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during topScores: " + std::string(e.what()));
        return {};
    }
}

void Database::seedAdmin() {
    try {
        QSqlQuery q(m_db);
        q.exec("SELECT COUNT(*) FROM users WHERE role = 'admin'");
        if (q.next() && q.value(0).toInt() == 0) {
            createUser("admin", "Administrator", "admin123", "admin", "#ff4d6d");
            LOG_DEBUG(TAG, "Default admin created — username: admin / password: admin123");
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during seedAdmin: " + std::string(e.what()));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Password hashing
// ─────────────────────────────────────────────────────────────────────────────
QString Database::hashPassword(const QString& password) {
    return QString(QCryptographicHash::hash(
        password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

// ─────────────────────────────────────────────────────────────────────────────
//  User CRUD
// ─────────────────────────────────────────────────────────────────────────────
bool Database::authenticate(const QString& username,
                             const QString& password,
                             UserRecord& outUser) const {
    try {
        QSqlQuery q(m_db);
        q.prepare("SELECT * FROM users WHERE username = :u AND password_hash = :h");
        q.bindValue(":u", username.trimmed().toLower());
        q.bindValue(":h", hashPassword(password));
        if (!q.exec() || !q.next()) return false;

        outUser.id           = q.value("id").toInt();
        outUser.username     = q.value("username").toString();
        outUser.displayName  = q.value("display_name").toString();
        outUser.passwordHash = q.value("password_hash").toString();
        outUser.role         = q.value("role").toString();
        outUser.avatarColor  = q.value("avatar_color").toString();
        outUser.createdAt    = q.value("created_at").toString();
        LOG_DEBUG(TAG, "User authenticated: " + outUser.username.toStdString());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during authenticate: " + std::string(e.what()));
        return false;
    }
}

bool Database::createUser(const QString& username,
                           const QString& displayName,
                           const QString& password,
                           const QString& role,
                           const QString& avatarColor) {
        try {
            QSqlQuery q(m_db);
            q.prepare(R"(
                INSERT INTO users (username, display_name, password_hash, role, avatar_color)
                VALUES (:u, :dn, :h, :r, :ac)
            )");
            q.bindValue(":u",  username.trimmed().toLower());
            q.bindValue(":dn", displayName.trimmed());
            q.bindValue(":h",  hashPassword(password));
            q.bindValue(":r",  role);
            q.bindValue(":ac", avatarColor);
            if (!q.exec()) {
                LOG_ERROR(TAG, "Exception during createUser: " + std::string(q.lastError().text().toStdString()));
                return false;
            }
            LOG_DEBUG(TAG, "User created: " + username.toStdString());
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR(TAG, "Exception during createUser: " + std::string(e.what()));
            return false;
        }
}

bool Database::deleteUser(int id) {
    try {
        QSqlQuery q(m_db);
        q.prepare("DELETE FROM users WHERE id = :id AND role != 'admin'");
        q.bindValue(":id", id);
        return q.exec();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during deleteUser: " + std::string(e.what()));
        return false;
    }
}

bool Database::usernameExists(const QString& username) const {
    try {
        QSqlQuery q(m_db);
        q.prepare("SELECT COUNT(*) FROM users WHERE username = :u");
        q.bindValue(":u", username.trimmed().toLower());
        return q.exec() && q.next() && q.value(0).toInt() > 0;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during usernameExists: " + std::string(e.what()));
        return false;
    }
}

QList<UserRecord> Database::allUsers() const {
    try {
        QSqlQuery q("SELECT * FROM users ORDER BY role DESC, username ASC", m_db);
        QList<UserRecord> list;
        while (q.next()) {
            UserRecord u;
            u.id          = q.value("id").toInt();
            u.username    = q.value("username").toString();
            u.displayName = q.value("display_name").toString();
            u.role        = q.value("role").toString();
            u.avatarColor = q.value("avatar_color").toString();
            u.createdAt   = q.value("created_at").toString();
            list.append(u);
        }
        return list;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Exception during allUsers: " + std::string(e.what()));
        return QList<UserRecord>();
    }
}
