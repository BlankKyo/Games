// src/ui/MainWindow.cpp
#include "ui/MainWindow.h"
#include "ui/HubView.h"
#include "ui/CreateGameDialog.h"
#include "ui/ScoreboardDialog.h"
#include "ui/AdminPanel.h"
#include "core/GameRunner.h"
#include "core/GameRegistry.h"
#include "core/Database.h"
#include "core/UserSession.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"

#include <QStackedWidget>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QWidget>
#include <QTimer>

static const char* TAG = "MainWindow";

// ─────────────────────────────────────────────────────────────────────────────
//  Helper: build the slim user bar shown above the hub
// ─────────────────────────────────────────────────────────────────────────────
QWidget* MainWindow::buildUserBar() {
    const auto& session = UserSession::instance();

    auto* bar = new QWidget(this);
    bar->setFixedHeight(52);
    bar->setStyleSheet("background:#0d0d1a; border-bottom:1px solid #1e1e30;");

    auto* row = new QHBoxLayout(bar);
    row->setContentsMargins(16, 0, 16, 0);
    row->setSpacing(10);

    // ── Avatar circle ─────────────────────────────────────────────────────
    auto* avatar = new QLabel(bar);
    // Initials: first char of each word in displayName, up to 2
    QString initials;
    const QStringList parts = session.displayName().split(' ', Qt::SkipEmptyParts);
    for (const auto& p : parts) {
        if (!p.isEmpty()) initials += p[0].toUpper();
        if (initials.size() == 2) break;
    }
    if (initials.isEmpty()) initials = session.username().left(1).toUpper();
    avatar->setText(initials);
    avatar->setFixedSize(32, 32);
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setStyleSheet(QString(R"(
        background: %1;
        border-radius: 16px;
        color: white;
        font-size: 12px;
        font-weight: 700;
    )").arg(session.avatarColor()));
    row->addWidget(avatar);

    // ── Name + role ───────────────────────────────────────────────────────
    auto* nameLabel = new QLabel(session.displayName(), bar);
    nameLabel->setStyleSheet("font-size:13px; font-weight:600; color:#ddddee;");
    row->addWidget(nameLabel);

    if (session.isAdmin()) {
        auto* roleBadge = new QLabel("admin", bar);
        roleBadge->setStyleSheet(R"(
            font-size:10px; font-weight:700; color:#ff4d6d;
            background:#2a1020; border-radius:4px; padding:2px 6px;
        )");
        row->addWidget(roleBadge);
    }

    LOG_INFO(TAG, "Initializing MainWindow UI.", __FILE__, __LINE__);
    setWindowTitle("Game Hub");
    setMinimumSize(960, 640);
    resize(1100, 700);

    // Central widget wraps [user bar] + [stack]
    auto* central      = new QWidget(this);
    auto* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    centralLayout->addWidget(buildUserBar());

    m_stack  = new QStackedWidget(central);
    m_hub    = new HubView(central);
    m_runner = new GameRunner(central);

    m_stack->addWidget(m_hub);    // index 0
    m_stack->addWidget(m_runner); // index 1
    centralLayout->addWidget(m_stack);

    setCentralWidget(central);

    // ── Connections ───────────────────────────────────────────────────────
    connect(m_hub,    &HubView::playRequested,       this, &MainWindow::onPlayRequested);
    connect(m_hub,    &HubView::deleteRequested,     this, &MainWindow::onDeleteRequested);
    connect(m_hub,    &HubView::scoreboardRequested, this, &MainWindow::onScoreboardRequested);
    connect(m_hub,    &HubView::createRequested,     this, &MainWindow::onCreateRequested);
    connect(m_runner, &GameRunner::gameFinished,     this, &MainWindow::onGameFinished);
    connect(m_runner, &GameRunner::returnToHub,      this, &MainWindow::onReturnToHub);

    seedBuiltins();
    refreshHub();
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)), __FILE__, __LINE__);
}

MainWindow::~MainWindow() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)), __FILE__, __LINE__);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Logout / Admin
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onLogout() {
    auto res = QMessageBox::question(this, "Logout",
        "Are you sure you want to log out?",
        QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;

    UserSession::instance().logout();
    emit loggedOut();
    close();
}

void MainWindow::onAdminPanel() {
    AdminPanel panel(this);
    panel.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Existing logic (unchanged)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::seedBuiltins() {
    try {
        auto& db = Database::instance();

        struct Info { QString key, title, desc; };
        const QList<Info> builtins = {
            {"snake",     "Snake",       "Classic snake — eat apples, don't hit the walls."},
            {"pong",      "Pong",        "Two-player pong. W/S vs ↑/↓. First to 7 wins."},
            {"breakout",  "Breakout",    "Smash all the bricks. Multiple levels & speeds."},
            {"bullscows", "Bulls & Cows","Guess the 4-digit number. 🐂 = right place, 🐄 = wrong place."},
        };

        for (const auto& b : builtins) {
            if (db.gameByKey(b.key).id == -1) {
                GameMetadata m;
                m.key         = b.key;
                m.title       = b.title;
                m.description = b.desc;
                m.author      = "Built-in";
                m.isBuiltin   = true;
                db.upsertGame(m);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to register builtin games: " + std::string(e.what()), __FILE__, __LINE__);
        return;
     }
}

void MainWindow::refreshHub() {
    m_hub->setGames(Database::instance().allGames());
}

void MainWindow::onPlayRequested(const GameMetadata& meta) {
    try {
        m_gameOverActive = false;
        auto& reg = GameRegistry::instance();
        if (!reg.hasGame(meta.key)) {
            std::string keyStr = meta.key.toStdString();
            LOG_WARNING(TAG, "No game registered for key \"" + keyStr + "\".", __FILE__, __LINE__);
            return;
        }

        GameBase* game = reg.create(meta.key, nullptr);
        m_runner->runGame(game, meta);
        m_stack->setCurrentIndex(1);
        QTimer::singleShot(0, game, [game]() {
            game->setFocus();
        });
    }
    catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to start game: " + std::string(e.what()), __FILE__, __LINE__);
        return;
    }
}

void MainWindow::onGameFinished(int score, const GameMetadata& meta) {
    try {
        if (m_gameOverActive) return;
        m_gameOverActive = true;
        LOG_INFO(TAG, QString("Game finished: %1 (score: %2)").arg(meta.title).arg(score).toStdString(), __FILE__, __LINE__);
        Database::instance().recordScore(meta.key, score);

        QMessageBox msg(this);
        msg.setWindowTitle("Game Over");
        msg.setText(QString("<b style='color:#fff'>%1</b><br><br>"
                            "Final score: <span style='color:#00ff88; font-size:22px;'>%2</span>")
                        .arg(meta.title).arg(score));
        msg.setStyleSheet("QMessageBox { background:#0f0f1a; } QLabel { color:#aaa; }");

        QAbstractButton* playAgainBtn = msg.addButton("Play Again",   QMessageBox::AcceptRole);
        QAbstractButton* backBtn      = msg.addButton("Back to Hub",  QMessageBox::RejectRole);
        msg.exec();

        if (msg.clickedButton() == playAgainBtn)
            QTimer::singleShot(0, this, [this, meta]() { onPlayRequested(meta); });
        else if (msg.clickedButton() == backBtn)
            QTimer::singleShot(0, this, [this]() { onReturnToHub(); });

    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to record score: " + std::string(e.what()), __FILE__, __LINE__);
        onReturnToHub();
    }
}

void MainWindow::onReturnToHub() {
    try {
        m_gameOverActive = false; // Reset the game over flag
        LOG_INFO(TAG, "Returning to hub view.", __FILE__, __LINE__);
        m_runner->stopCurrent();
        m_stack->setCurrentIndex(0);
        refreshHub();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to return to hub: " + std::string(e.what()), __FILE__, __LINE__);
    }
}

void MainWindow::onCreateRequested() {
    try {
        CreateGameDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted) return;

        GameMetadata meta = dlg.result();
        if (meta.title.isEmpty()) {
            LOG_WARNING(TAG, "Game creation failed: Title cannot be empty.", __FILE__, __LINE__);
            return;
        }
        if (!GameRegistry::instance().hasGame(meta.key)) {
            LOG_WARNING(TAG, "Game creation failed: Unknown game type.", __FILE__, __LINE__);
            return;
        }

        Database::instance().upsertGame(meta);
        refreshHub();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to create game: " + std::string(e.what()), __FILE__, __LINE__);
    }
}

void MainWindow::onDeleteRequested(int id) {
    try {
        auto res = QMessageBox::question(this, "Delete Game",
            "Remove this game entry? (Scores will also be deleted.)",
            QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            Database::instance().deleteGame(id);
            refreshHub();
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to delete game: " + std::string(e.what()), __FILE__, __LINE__);
    }
}

void MainWindow::onScoreboardRequested(const GameMetadata& meta) {
    try {
        ScoreboardDialog dlg(meta, this);
        dlg.exec();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to open scoreboard: " + std::string(e.what()), __FILE__, __LINE__);
    }
}