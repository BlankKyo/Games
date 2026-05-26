#pragma once
#include <QMap>
#include <QString>
#include <functional>
#include "GameBase.h"

// ─────────────────────────────────────────────────────────────────────────────
// GameRegistry
//   Singleton factory.  Maps a string key → a factory lambda.
//
//   Usage:
//     GameRegistry::instance().registerGame("snake", [](QWidget* p){
//         return new SnakeGame(p);
//     });
//     GameBase* g = GameRegistry::instance().create("snake", parent);
// ─────────────────────────────────────────────────────────────────────────────
class GameRegistry {
public:
    using Factory = std::function<GameBase*(QWidget*)>;

    static GameRegistry& instance() {
        static GameRegistry reg;
        return reg;
    }

    void registerGame(const QString& key, Factory factory) {
        m_factories[key] = std::move(factory);
    }

    GameBase* create(const QString& key, QWidget* parent = nullptr) const {
        if (auto it = m_factories.find(key); it != m_factories.end())
            return it.value()(parent);
        return nullptr;
    }

    bool hasGame(const QString& key) const {
        return m_factories.contains(key);
    }

    QList<QString> registeredKeys() const {
        return m_factories.keys();
    }

    // Called once at startup to register all built-in games.
    void registerBuiltins();

private:
    GameRegistry() = default;
    ~GameRegistry();
    QMap<QString, Factory> m_factories;
};
