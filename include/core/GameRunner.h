#pragma once
#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include "GameBase.h"
#include "GameMetadata.h"

// ─────────────────────────────────────────────────────────────────────────────
// GameRunner
//   Widget that wraps a GameBase and provides:
//     • A fixed-timestep game loop via QTimer (target 60 fps)
//     • HUD overlay: score label, pause button, quit button
//     • Forwards gameOver signal upward
// ─────────────────────────────────────────────────────────────────────────────
class GameRunner : public QWidget {
    Q_OBJECT

public:
    explicit GameRunner(QWidget* parent = nullptr);
    ~GameRunner() override;

    // Launch a game.  Takes ownership of the GameBase*.
    void runGame(GameBase* game, const GameMetadata& meta);

    // Stop and destroy the running game (also called on gameOver).
    void stopCurrent();

signals:
    void gameFinished(int score, const GameMetadata& meta);
    void returnToHub();

private slots:
    void tick();
    void onGameOver(int score);
    void togglePause();

private:
    void buildHud();

    static constexpr int TARGET_FPS     = 60;
    static constexpr int TICK_MS        = 1000 / TARGET_FPS;

    GameBase*      m_game    = nullptr;
    GameMetadata   m_meta;

    QTimer         m_timer;
    QElapsedTimer  m_elapsed;

    // HUD widgets
    QLabel*        m_titleLabel  = nullptr;
    QLabel*        m_scoreLabel  = nullptr;
    QPushButton*   m_pauseBtn    = nullptr;
    QPushButton*   m_quitBtn     = nullptr;
    QWidget*       m_hudBar      = nullptr;
};
