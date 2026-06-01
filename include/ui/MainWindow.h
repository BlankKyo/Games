// include/ui/MainWindow.h
#pragma once
#include <QMainWindow>
#include "core/Metadata.h"

class QStackedWidget;
class HubView;
class GameRunner;

// ─────────────────────────────────────────────────────────────────────────────
// MainWindow
//   Top-level window.  Owns a QStackedWidget with two pages:
//     0 → HubView    (game library)
//     1 → GameRunner (active game)
//
//   A slim user bar at the top shows the avatar + display name, an optional
//   Admin button (admins only), and a Logout button.
// ─────────────────────────────────────────────────────────────────────────────
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void loggedOut();   ///< Emitted when the user clicks "Logout"
    
private slots:
    void onPlayRequested(const GameMetadata& meta);
    void onGameFinished(int score, const GameMetadata& meta);
    void onReturnToHub();
    void onCreateRequested();
    void onDeleteRequested(int id);
    void onScoreboardRequested(const GameMetadata& meta);
    void onLogout();
    void onAdminPanel();

private:
    void refreshHub();
    void seedBuiltins();
    QWidget* buildUserBar();   ///< Creates the top user-bar widget

    QStackedWidget* m_stack   = nullptr;
    HubView*        m_hub     = nullptr;
    GameRunner*     m_runner  = nullptr;
    bool m_gameOverActive = false; // flag to prevent duplicate game over handling
};
