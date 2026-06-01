// include/core/UserSession.h
#pragma once
#include "Database.h"

// ─────────────────────────────────────────────────────────────────────────────
// UserSession
//   Singleton that holds whoever is currently logged in.
//   Access it anywhere with UserSession::instance().
// ─────────────────────────────────────────────────────────────────────────────
class UserSession {
public:
    static UserSession& instance() {
        static UserSession s;
        return s;
    }

    void login(const UserRecord& user) {
        m_user      = user;
        m_loggedIn  = true;
    }

    void logout() {
        m_user     = {};
        m_loggedIn = false;
    }

    bool            isLoggedIn()  const { return m_loggedIn; }
    bool            isAdmin()     const { return m_user.role == "admin"; }
    const UserRecord& user()      const { return m_user; }
    QString         displayName() const { return m_user.displayName; }
    QString         username()    const { return m_user.username; }
    QString         avatarColor() const { return m_user.avatarColor; }

private:
    UserSession() = default;
    UserRecord m_user;
    bool       m_loggedIn = false;
};