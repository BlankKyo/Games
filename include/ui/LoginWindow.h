#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;

// ─────────────────────────────────────────────────────────────────────────────
// LoginWindow
//   First screen shown at launch.
//   On successful login emits loginSucceeded() → main.cpp shows MainWindow.
// ─────────────────────────────────────────────────────────────────────────────
class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget* parent = nullptr);

signals:
    void loginSucceeded();

private slots:
    void attemptLogin();

private:
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLabel*    m_errorLabel;
};