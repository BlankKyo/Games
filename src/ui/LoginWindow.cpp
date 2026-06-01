#include "ui/LoginWindow.h"
#include "ui/RegisterDialog.h"
#include "core/Database.h"
#include "core/UserSession.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QFrame>

LoginWindow::LoginWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("GameHub — Login");
    setMinimumSize(420, 560);
    resize(420, 560);
    setStyleSheet("background:#0a0a12;");

    // ── Root layout ───────────────────────────────────────────────────────
    auto* root = new QVBoxLayout(this);
    root->setAlignment(Qt::AlignCenter);
    root->setContentsMargins(0, 0, 0, 0);

    // ── Card ──────────────────────────────────────────────────────────────
    auto* card = new QFrame(this);
    card->setFixedWidth(340);
    card->setStyleSheet(R"(
        QFrame {
            background: #13131f;
            border: 1px solid #2a2a40;
            border-radius: 16px;
        }
    )");

    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 160));
    card->setGraphicsEffect(shadow);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 40, 36, 36);
    cardLayout->setSpacing(0);

    // ── Logo / title ──────────────────────────────────────────────────────
    auto* logo = new QLabel("🎮", card);
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet("font-size:40px; background:transparent; border:none;");
    cardLayout->addWidget(logo);
    cardLayout->addSpacing(12);

    auto* title = new QLabel("GameHub", card);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(R"(
        font-size: 26px;
        font-weight: 700;
        color: #ffffff;
        letter-spacing: 1px;
        background: transparent;
        border: none;
    )");
    cardLayout->addWidget(title);
    cardLayout->addSpacing(4);

    auto* subtitle = new QLabel("Sign in to continue", card);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size:13px; color:#555570; background:transparent; border:none;");
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(32);

    // ── Field helper ──────────────────────────────────────────────────────
    const QString fieldStyle = R"(
        QLineEdit {
            background: #1e1e2e;
            border: 1px solid #2a2a40;
            border-radius: 8px;
            color: #e0e0f0;
            font-size: 14px;
            padding: 10px 14px;
        }
        QLineEdit:focus {
            border-color: #7c3aed;
        }
        QLineEdit::placeholder {
            color: #44445a;
        }
    )";

    auto addField = [&](const QString& labelText, QLineEdit*& edit, bool password = false) {
        auto* lbl = new QLabel(labelText, card);
        lbl->setStyleSheet("font-size:12px; font-weight:600; color:#888899; background:transparent; border:none;");
        cardLayout->addWidget(lbl);
        cardLayout->addSpacing(6);

        edit = new QLineEdit(card);
        edit->setFixedHeight(42);
        edit->setStyleSheet(fieldStyle);
        if (password) {
            edit->setEchoMode(QLineEdit::Password);
            edit->setPlaceholderText("••••••••");
        }
        cardLayout->addWidget(edit);
        cardLayout->addSpacing(16);
    };

    addField("Username", m_usernameEdit);
    m_usernameEdit->setPlaceholderText("Enter username");

    addField("Password", m_passwordEdit, true);

    // ── Error label ───────────────────────────────────────────────────────
    m_errorLabel = new QLabel("", card);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setStyleSheet("font-size:12px; color:#ff4d6d; background:transparent; border:none;");
    m_errorLabel->setVisible(false);
    cardLayout->addWidget(m_errorLabel);
    cardLayout->addSpacing(4);

    // ── Login button ──────────────────────────────────────────────────────
    auto* loginBtn = new QPushButton("Sign In", card);
    loginBtn->setFixedHeight(44);
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(R"(
        QPushButton {
            background: #7c3aed;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 700;
        }
        QPushButton:hover  { background: #6d28d9; }
        QPushButton:pressed{ background: #5b21b6; }
    )");
    cardLayout->addWidget(loginBtn);
    cardLayout->addSpacing(16);

    // ── Register link ─────────────────────────────────────────────────────
    auto* regRow = new QHBoxLayout();
    regRow->setAlignment(Qt::AlignCenter);
    auto* regHint = new QLabel("No account?", card);
    regHint->setStyleSheet("font-size:12px; color:#555570; background:transparent; border:none;");
    auto* regBtn = new QPushButton("Create one", card);
    regBtn->setCursor(Qt::PointingHandCursor);
    regBtn->setStyleSheet(R"(
        QPushButton {
            font-size:12px; color:#7c3aed;
            background:transparent; border:none; padding:0;
        }
        QPushButton:hover { color:#a78bfa; }
    )");
    regRow->addWidget(regHint);
    regRow->addSpacing(4);
    regRow->addWidget(regBtn);
    cardLayout->addLayout(regRow);

    root->addWidget(card, 0, Qt::AlignCenter);

    // ── Connections ───────────────────────────────────────────────────────
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::attemptLogin);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginWindow::attemptLogin);
    connect(m_usernameEdit, &QLineEdit::returnPressed, this, &LoginWindow::attemptLogin);
    connect(regBtn, &QPushButton::clicked, this, [this]() {
        RegisterDialog dlg(this);
        dlg.exec();
    });
}

void LoginWindow::attemptLogin() {
    const QString user = m_usernameEdit->text().trimmed();
    const QString pass = m_passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        m_errorLabel->setText("Please fill in all fields.");
        m_errorLabel->setVisible(true);
        return;
    }

    UserRecord rec;
    if (!Database::instance().authenticate(user, pass, rec)) {
        m_errorLabel->setText("Invalid username or password.");
        m_errorLabel->setVisible(true);
        m_passwordEdit->clear();
        return;
    }

    UserSession::instance().login(rec);
    emit loginSucceeded();
}