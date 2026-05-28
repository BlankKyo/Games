#include "ui/MainWindow.h"
#include "ui/HubView.h"
#include "ui/CreateGameDialog.h"
#include "ui/ScoreboardDialog.h"
#include "core/GameRunner.h"
#include "core/GameRegistry.h"
#include "core/Database.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QStackedWidget>
#include <QMessageBox>


static const char* TAG = "MainWindow";

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {

    LOG_INFO(TAG, "Initializing MainWindow UI.");
    setWindowTitle("Game Hub");
    setMinimumSize(960, 640);
    resize(1100, 700);

    m_stack  = new QStackedWidget(this);
    m_hub    = new HubView(this);
    m_runner = new GameRunner(this);

    m_stack->addWidget(m_hub);    // index 0
    m_stack->addWidget(m_runner); // index 1
    setCentralWidget(m_stack);

    // ── Connections ───────────────────────────────────────────────────────
    connect(m_hub,    &HubView::playRequested,       this, &MainWindow::onPlayRequested);
    connect(m_hub,    &HubView::deleteRequested,     this, &MainWindow::onDeleteRequested);
    connect(m_hub,    &HubView::scoreboardRequested, this, &MainWindow::onScoreboardRequested);
    connect(m_hub,    &HubView::createRequested,     this, &MainWindow::onCreateRequested);
    connect(m_runner, &GameRunner::gameFinished,     this, &MainWindow::onGameFinished);
    connect(m_runner, &GameRunner::returnToHub,      this, &MainWindow::onReturnToHub);

    seedBuiltins();
    refreshHub();
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
}

MainWindow::~MainWindow() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}

void MainWindow::seedBuiltins() {
    try {
        auto& db = Database::instance();

        struct Info { QString key, title, desc; };
        const QList<Info> builtins = {
            {"snake",    "Snake",    "Classic snake — eat apples, don't hit the walls."},
            {"pong",     "Pong",     "Two-player pong. W/S vs ↑/↓. First to 7 wins."},
            {"breakout", "Breakout", "Smash all the bricks. Multiple levels & speeds."},
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
        LOG_ERROR(TAG, "Failed to register builtin games: " + std::string(e.what()));
        return;
     }
}

void MainWindow::refreshHub() {
    m_hub->setGames(Database::instance().allGames());
}

void MainWindow::onPlayRequested(const GameMetadata& meta) {
    try{
        m_gameOverActive = false; // Reset game over flag in case it was left active
        auto& reg = GameRegistry::instance();
        if (!reg.hasGame(meta.key)) {
            std::string keyStr = meta.key.toStdString();
            LOG_WARNING(TAG, "No game registered for key \"" + keyStr + "\".");
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
        LOG_ERROR(TAG, "Failed to start game: " + std::string(e.what()));
        return;
    }
}

void MainWindow::onGameFinished(int score, const GameMetadata& meta) {
    try {
        if (m_gameOverActive)
            return; // ❗ ignore duplicates completely

        m_gameOverActive = true;
        LOG_INFO(TAG, QString("Game finished: %1 (score: %2)").arg(meta.title).arg(score).toStdString());
        Database::instance().recordScore(meta.key, score);

        QMessageBox msg(this);

        msg.setWindowTitle("Game Over");

        msg.setText(QString("<b style='color:#fff'>%1</b><br><br>"
                            "Final score: <span style='color:#00ff88; font-size:22px;'>%2</span>")
                        .arg(meta.title)
                        .arg(score));

        msg.setStyleSheet(
            "QMessageBox { background:#0f0f1a; } "
            "QLabel { color:#aaa; }"
        );

        QAbstractButton* playAgainBtn =
            msg.addButton("Play Again", QMessageBox::AcceptRole);

        QAbstractButton* backBtn =
            msg.addButton("Back to Hub", QMessageBox::RejectRole);

        msg.exec();
        QAbstractButton* clicked = msg.clickedButton();

        if (clicked == playAgainBtn) {
            QTimer::singleShot(0, this, [this, meta]() {
                onPlayRequested(meta);
            });
        } else if (clicked == backBtn) {
            QTimer::singleShot(0, this, [this]() {
                onReturnToHub();
            });
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to record score: " + std::string(e.what()));
        onReturnToHub();
    }
}

void MainWindow::onReturnToHub() {
    try {
        m_gameOverActive = false; // Reset the game over flag
        LOG_INFO(TAG, "Returning to hub view.");
        m_runner->stopCurrent();
        m_stack->setCurrentIndex(0);
        refreshHub();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to return to hub: " + std::string(e.what()));
    }
}

void MainWindow::onCreateRequested() {
    try{
        CreateGameDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted) return;

        GameMetadata meta = dlg.result();
        if (meta.title.isEmpty()) {
            LOG_WARNING(TAG, "Game creation failed: Title cannot be empty.");
            return;
        }
        if (!GameRegistry::instance().hasGame(meta.key)) {
            LOG_WARNING(TAG, "Game creation failed: Unknown game type.");
            return;
        }

        Database::instance().upsertGame(meta);
        refreshHub();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to create game: " + std::string(e.what()));
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
        LOG_ERROR(TAG, "Failed to delete game: " + std::string(e.what()));
    }
}

void MainWindow::onScoreboardRequested(const GameMetadata& meta) {
    try {
        ScoreboardDialog dlg(meta, this);
        dlg.exec();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to open scoreboard: " + std::string(e.what()));
    }
}
