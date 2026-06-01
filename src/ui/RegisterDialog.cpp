// src/ui/RegisterDialog.cpp
#include "ui/RegisterDialog.h"
#include "core/Database.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QDialogButtonBox>



const QStringList RegisterDialog::s_palette = {
    "#7c3aed", "#2563eb", "#059669", "#d97706",
    "#dc2626", "#db2777", "#0891b2", "#65a30d"
};

RegisterDialog::RegisterDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Create Account");
    setModal(true);
    setFixedWidth(360);
    setStyleSheet("QDialog { background:#13131f; } QLabel { color:#ccccdd; border:none; background:transparent; }");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(28, 28, 28, 28);
    layout->setSpacing(0);

    auto* heading = new QLabel("Create Account", this);
    heading->setStyleSheet("font-size:20px; font-weight:700; color:#ffffff;");
    layout->addWidget(heading);
    layout->addSpacing(4);

    auto* sub = new QLabel("All fields required.", this);
    sub->setStyleSheet("font-size:12px; color:#555570;");
    layout->addWidget(sub);
    layout->addSpacing(24);

    const QString fieldStyle = R"(
        QLineEdit {
            background:#1e1e2e; border:1px solid #2a2a40;
            border-radius:7px; color:#e0e0f0;
            font-size:13px; padding:9px 12px;
        }
        QLineEdit:focus { border-color:#7c3aed; }
    )";

    auto addField = [&](const QString& lbl, QLineEdit*& edit, bool pw = false) {
        auto* l = new QLabel(lbl, this);
        l->setStyleSheet("font-size:11px; font-weight:600; color:#888899;");
        layout->addWidget(l);
        layout->addSpacing(5);
        edit = new QLineEdit(this);
        edit->setFixedHeight(38);
        edit->setStyleSheet(fieldStyle);
        if (pw) edit->setEchoMode(QLineEdit::Password);
        layout->addWidget(edit);
        layout->addSpacing(14);
    };

    addField("Username",     m_usernameEdit);
    addField("Display Name", m_displayNameEdit);
    addField("Password",     m_passwordEdit,  true);
    addField("Confirm",      m_confirmEdit,   true);

    // ── Avatar color ──────────────────────────────────────────────────────
    auto* colorLabel = new QLabel("Avatar Color", this);
    colorLabel->setStyleSheet("font-size:11px; font-weight:600; color:#888899;");
    layout->addWidget(colorLabel);
    layout->addSpacing(8);

    auto* swatchRow = new QHBoxLayout();
    swatchRow->setSpacing(8);
    for (const QString& hex : s_palette) {
        auto* btn = new QPushButton(this);
        btn->setFixedSize(28, 28);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background:%1; border-radius:14px; border:2px solid transparent; }"
            "QPushButton:hover { border-color:#ffffff; }"
        ).arg(hex));
        connect(btn, &QPushButton::clicked, this, [this, hex]() { pickColor(hex); });
        swatchRow->addWidget(btn);
    }
    swatchRow->addStretch();
    layout->addLayout(swatchRow);
    layout->addSpacing(6);

    // Preview swatch
    m_colorSwatch = new QWidget(this);
    m_colorSwatch->setFixedSize(40, 40);
    m_colorSwatch->setStyleSheet(QString(
        "background:%1; border-radius:20px; border:2px solid #2a2a40;"
    ).arg(m_avatarColor));
    layout->addWidget(m_colorSwatch, 0, Qt::AlignLeft);
    layout->addSpacing(20);

    // ── Error ─────────────────────────────────────────────────────────────
    m_errorLabel = new QLabel("", this);
    m_errorLabel->setStyleSheet("font-size:12px; color:#ff4d6d;");
    m_errorLabel->setVisible(false);
    layout->addWidget(m_errorLabel);
    layout->addSpacing(4);

    // ── Buttons ───────────────────────────────────────────────────────────
    auto* createBtn = new QPushButton("Create Account", this);
    createBtn->setFixedHeight(40);
    createBtn->setCursor(Qt::PointingHandCursor);
    createBtn->setStyleSheet(R"(
        QPushButton { background:#7c3aed; color:white; border:none; border-radius:7px; font-size:13px; font-weight:700; }
        QPushButton:hover  { background:#6d28d9; }
        QPushButton:pressed{ background:#5b21b6; }
    )");
    layout->addWidget(createBtn);
    layout->addSpacing(10);

    auto* cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setFixedHeight(36);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background:transparent; color:#666680; border:1px solid #2a2a40; border-radius:7px; font-size:13px; }
        QPushButton:hover { color:#aaaacc; border-color:#44445a; }
    )");
    layout->addWidget(cancelBtn);

    connect(createBtn, &QPushButton::clicked, this, &RegisterDialog::attemptRegister);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void RegisterDialog::pickColor(const QString& hex) {
    m_avatarColor = hex;
    m_colorSwatch->setStyleSheet(QString(
        "background:%1; border-radius:20px; border:2px solid #2a2a40;"
    ).arg(hex));
}

void RegisterDialog::attemptRegister() {
    const QString username    = m_usernameEdit->text().trimmed();
    const QString displayName = m_displayNameEdit->text().trimmed();
    const QString password    = m_passwordEdit->text();
    const QString confirm     = m_confirmEdit->text();

    if (username.isEmpty() || displayName.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText("All fields are required.");
        m_errorLabel->setVisible(true);
        return;
    }
    if (password != confirm) {
        m_errorLabel->setText("Passwords do not match.");
        m_errorLabel->setVisible(true);
        return;
    }
    if (password.length() < 4) {
        m_errorLabel->setText("Password must be at least 4 characters.");
        m_errorLabel->setVisible(true);
        return;
    }

    auto& db = Database::instance();
    if (db.usernameExists(username)) {
        m_errorLabel->setText("Username already taken.");
        m_errorLabel->setVisible(true);
        return;
    }

    if (!db.createUser(username, displayName, password, "user", m_avatarColor)) {
        m_errorLabel->setText("Failed to create account.");
        m_errorLabel->setVisible(true);
        return;
    }

    accept();
}