#pragma once
#include <QMainWindow>
#include "core/GameMetadata.h"

class QStackedWidget;
class HubView;
class GameRunner;

// ─────────────────────────────────────────────────────────────────────────────
// MainWindow
//   Top-level window.  Owns a QStackedWidget with two pages:
//     0 → HubView    (game library)
//     1 → GameRunner (active game)
// ─────────────────────────────────────────────────────────────────────────────
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onPlayRequested(const GameMetadata& meta);
    void onGameFinished(int score, const GameMetadata& meta);
    void onReturnToHub();
    void onCreateRequested();
    void onDeleteRequested(int id);
    void onScoreboardRequested(const GameMetadata& meta);

private:
    void refreshHub();
    void seedBuiltins();

    QStackedWidget* m_stack   = nullptr;
    HubView*        m_hub     = nullptr;
    GameRunner*     m_runner  = nullptr;
};
