#include "core/GameRunner.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QStyle>

static const char* TAG = "GameRunner";

GameRunner::GameRunner(QWidget* parent)
    : QWidget(parent)
{
    try {
        LOG_INFO(TAG, "Initializing GameRunner UI.");
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        // ── HUD bar ───────────────────────────────────────────────────────────
        m_hudBar = new QWidget(this);
        m_hudBar->setFixedHeight(44);
        m_hudBar->setStyleSheet("background:#0d0d1a; border-bottom:1px solid #1e1e30;");

        auto* hudLayout = new QHBoxLayout(m_hudBar);
        hudLayout->setContentsMargins(12, 4, 12, 4);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setStyleSheet("color:#7c3aed; font-weight:700; font-size:15px;");

        m_scoreLabel = new QLabel("Score: 0", this);
        m_scoreLabel->setStyleSheet("color:#00ff88; font-family:monospace; font-size:15px;");

        m_pauseBtn = new QPushButton("⏸ Pause", this);
        m_pauseBtn->setStyleSheet(
            "QPushButton { background:#1e1e30; color:#fff; border:1px solid #333;"
            " padding:4px 14px; border-radius:6px; }"
            "QPushButton:hover { background:#2a2a44; }");
        m_pauseBtn->setFocusPolicy(Qt::NoFocus);

        m_quitBtn = new QPushButton("✕ Hub", this);
        m_quitBtn->setStyleSheet(
            "QPushButton { background:#1e1e30; color:#ff4d6d; border:1px solid #ff4d6d;"
            " padding:4px 14px; border-radius:6px; }"
            "QPushButton:hover { background:#2a0010; }");
        m_quitBtn->setFocusPolicy(Qt::NoFocus);

        hudLayout->addWidget(m_titleLabel);
        hudLayout->addStretch();
        hudLayout->addWidget(m_scoreLabel);
        hudLayout->addSpacing(12);
        hudLayout->addWidget(m_pauseBtn);
        hudLayout->addWidget(m_quitBtn);

        root->addWidget(m_hudBar);

        // placeholder — game widget inserted here at runGame()
        root->addStretch(1);

        connect(m_pauseBtn, &QPushButton::clicked, this, &GameRunner::togglePause);
        connect(m_quitBtn,  &QPushButton::clicked, this, [this]{ stopCurrent(); LOG_DEBUG(TAG, "Quit button clicked."); emit returnToHub(); });
        connect(&m_timer,   &QTimer::timeout,      this, &GameRunner::tick);
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize GameRunner UI: " + std::string(e.what()));
    }
}

GameRunner::~GameRunner() {
    stopCurrent();
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}

void GameRunner::runGame(GameBase* game, const GameMetadata& meta) {
    try {
        stopCurrent();
        m_game = game;
        m_meta = meta;

        m_titleLabel->setText(meta.title);
        m_scoreLabel->setText("Score: 0");

        // Insert game widget into layout (below HUD)
        auto* root = qobject_cast<QVBoxLayout*>(layout());
        // Remove stretch placeholder
        if (root->count() > 1)
            root->takeAt(1);
        root->addWidget(m_game, 1);

        connect(m_game, &GameBase::gameOver,      this, &GameRunner::onGameOver);
        connect(m_game, &GameBase::scoreChanged,  this, [this](int s){
            m_scoreLabel->setText(QString("Score: %1").arg(s));
        });

        m_game->startGame();
        m_elapsed.start();
        m_timer.start(TICK_MS);
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to run game: " + std::string(e.what()));
        stopCurrent();
    }
}

void GameRunner::stopCurrent() {
    m_timer.stop();
    if (m_game) {
        m_game->stopGame();
        m_game->deleteLater();
        m_game = nullptr;
    }
}

void GameRunner::tick() {
    if (!m_game || m_game->isPaused()) return;
    double dt = m_elapsed.restart() / 1000.0;
    // Clamp dt to avoid spiral-of-death on lag spikes
    dt = std::min(dt, 0.05);
    m_game->update(dt);
    m_game->update(dt);   // trigger repaint via update() inside game
    m_game->repaint();
}

void GameRunner::onGameOver(int score) {
    LOG_DEBUG(TAG, "Game over! Final score: " + std::to_string(score));
    m_timer.stop();
    emit gameFinished(score, m_meta);
}

void GameRunner::togglePause() {
    if (!m_game) return;
    if (m_game->isPaused()) {
        m_game->resumeGame();
        m_pauseBtn->setText("⏸ Pause");
        m_elapsed.restart();
    } else {
        m_game->pauseGame();
        m_pauseBtn->setText("▶ Resume");
    }
}
