#include "ui/CreateGameDialog.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>

static const char* TAG = "CreateGameDialog";

CreateGameDialog::CreateGameDialog(QWidget* parent) : QDialog(parent) {
    try {
        LOG_INFO(TAG, "Initializing CreateGameDialog UI.");

        setWindowTitle("Add Game");
        setMinimumWidth(400);
        setStyleSheet(R"(
            QDialog    { background:#0f0f1a; color:#fff; }
            QLabel     { color:#aaa; }
            QLineEdit, QTextEdit, QComboBox {
                background:#080810; color:#fff; border:1px solid #1e1e30;
                border-radius:6px; padding:6px;
            }
            QLineEdit:focus, QTextEdit:focus, QComboBox:focus { border:1px solid #7c3aed; }
            QDialogButtonBox QPushButton {
                background:#7c3aed; color:#fff; border:none; padding:7px 20px;
                border-radius:7px; font-weight:700;
            }
            QDialogButtonBox QPushButton:hover { background:#6d28d9; }
            QDialogButtonBox QPushButton[text="Cancel"] {
                background:#1e1e30; color:#aaa;
            }
        )");

        auto* root = new QVBoxLayout(this);

        auto* heading = new QLabel("New Game Entry", this);
        heading->setStyleSheet("font-size:18px; font-weight:700; color:#fff; margin-bottom:8px;");
        root->addWidget(heading);

        auto* form = new QFormLayout;
        form->setSpacing(10);

        m_typeCombo = new QComboBox(this);
        m_typeCombo->addItems({"snake", "pong", "breakout"});

        m_titleEdit   = new QLineEdit(this);
        m_authorEdit  = new QLineEdit(this);
        m_versionEdit = new QLineEdit("1.0", this);
        m_descEdit    = new QTextEdit(this);
        m_descEdit->setFixedHeight(72);
        m_descEdit->setPlaceholderText("Short description…");

        form->addRow("Game Type",   m_typeCombo);
        form->addRow("Title",       m_titleEdit);
        form->addRow("Author",      m_authorEdit);
        form->addRow("Version",     m_versionEdit);
        form->addRow("Description", m_descEdit);

        root->addLayout(form);

        auto* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(btns, &QDialogButtonBox::rejected, this, &QDialog::reject);
        root->addWidget(btns);

        // Auto-fill title from game type
        connect(m_typeCombo, &QComboBox::currentTextChanged, this, [this](const QString& key){
            if (m_titleEdit->text().isEmpty()) {
                QString t = key;
                t[0] = t[0].toUpper();
                m_titleEdit->setText(t);
            }
        });
        m_typeCombo->currentTextChanged(m_typeCombo->currentText());

        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize CreateGameDialog: " + std::string(e.what()));
    }
}

CreateGameDialog::~CreateGameDialog() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}

GameMetadata CreateGameDialog::result() const {
    try {
        GameMetadata m;
        m.key         = m_typeCombo->currentText();
        m.title       = m_titleEdit->text().trimmed();
        m.author      = m_authorEdit->text().trimmed();
        m.version     = m_versionEdit->text().trimmed();
        m.description = m_descEdit->toPlainText().trimmed();
        m.isBuiltin   = false;
        return m;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to retrieve game metadata from dialog: " + std::string(e.what()));
        return {};  
    }
}
