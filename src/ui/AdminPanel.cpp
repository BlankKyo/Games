// src/ui/AdminPanel.cpp
#include "ui/AdminPanel.h"
#include "core/Database.h"
#include "core/UserSession.h"
#include "core/Metadata.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QFrame>

AdminPanel::AdminPanel(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Admin Panel — User Management");
    setModal(true);
    setMinimumSize(640, 460);
    setStyleSheet(R"(
        QDialog          { background: #0f0f1a; }
        QLabel           { color: #ccccdd; background: transparent; border: none; }
        QTableWidget     { background: #13131f; border: 1px solid #2a2a40; border-radius: 8px;
                           color: #ccccdd; gridline-color: #1e1e30; font-size: 13px; }
        QHeaderView::section { background: #1a1a2e; color: #888899; border: none;
                               padding: 8px; font-size: 12px; font-weight: 600; }
        QTableWidget::item          { padding: 6px 10px; }
        QTableWidget::item:selected { background: #2a1f4a; color: #ffffff; }
    )");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 20);
    layout->setSpacing(16);

    // ── Header ────────────────────────────────────────────────────────────
    auto* headerRow = new QHBoxLayout();
    auto* heading = new QLabel("User Management", this);
    heading->setStyleSheet("font-size:20px; font-weight:700; color:#ffffff;");
    headerRow->addWidget(heading);
    headerRow->addStretch();

    auto* createBtn = new QPushButton("＋ New User", this);
    createBtn->setFixedHeight(36);
    createBtn->setCursor(Qt::PointingHandCursor);
    createBtn->setStyleSheet(R"(
        QPushButton { background:#7c3aed; color:white; border:none; border-radius:7px;
                      font-size:13px; font-weight:600; padding:0 16px; }
        QPushButton:hover { background:#6d28d9; }
    )");
    headerRow->addWidget(createBtn);
    layout->addLayout(headerRow);

    // ── Table ─────────────────────────────────────────────────────────────
    m_table = new QTableWidget(0, 5, this);
    m_table->setHorizontalHeaderLabels({"ID", "Username", "Display Name", "Role", "Created"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->setColumnWidth(0, 48);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    layout->addWidget(m_table);

    // ── Footer row ────────────────────────────────────────────────────────
    auto* footerRow = new QHBoxLayout();

    m_statusLabel = new QLabel("", this);
    m_statusLabel->setStyleSheet("font-size:12px; color:#ff4d6d;");
    footerRow->addWidget(m_statusLabel);
    footerRow->addStretch();

    auto* deleteBtn = new QPushButton("🗑 Delete Selected", this);
    deleteBtn->setFixedHeight(34);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet(R"(
        QPushButton { background:transparent; color:#ff4d6d; border:1px solid #ff4d6d;
                      border-radius:7px; font-size:12px; padding:0 14px; }
        QPushButton:hover { background:#ff4d6d; color:white; }
    )");
    footerRow->addWidget(deleteBtn);

    auto* closeBtn = new QPushButton("Close", this);
    closeBtn->setFixedHeight(34);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background:transparent; color:#666680; border:1px solid #2a2a40;
                      border-radius:7px; font-size:12px; padding:0 14px; }
        QPushButton:hover { color:#aaaacc; border-color:#44445a; }
    )");
    footerRow->addWidget(closeBtn);
    layout->addLayout(footerRow);

    // ── Connections ───────────────────────────────────────────────────────
    connect(createBtn, &QPushButton::clicked, this, &AdminPanel::onCreateUser);
    connect(deleteBtn, &QPushButton::clicked, this, &AdminPanel::onDeleteSelected);
    connect(closeBtn,  &QPushButton::clicked, this, &QDialog::accept);

    refreshTable();
}

void AdminPanel::refreshTable() {
    m_table->setRowCount(0);
    const auto users = Database::instance().allUsers();
    for (const auto& u : users) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::number(u.id)));
        m_table->setItem(row, 1, new QTableWidgetItem(u.username));
        m_table->setItem(row, 2, new QTableWidgetItem(u.displayName));

        auto* roleItem = new QTableWidgetItem(u.role);
        roleItem->setForeground(u.role == "admin" ? QColor("#ff4d6d") : QColor("#00cc88"));
        m_table->setItem(row, 3, roleItem);
        m_table->setItem(row, 4, new QTableWidgetItem(u.createdAt));

        // Store user id in first column's UserRole data for later retrieval
        m_table->item(row, 0)->setData(Qt::UserRole, u.id);
    }
    m_statusLabel->clear();
}

void AdminPanel::onCreateUser() {
    // Inline mini-dialog for quick user creation
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("New User");
    dlg->setModal(true);
    dlg->setFixedWidth(320);
    dlg->setStyleSheet("QDialog { background:#13131f; } QLabel { color:#aaaacc; background:transparent; border:none; }");

    auto* l = new QVBoxLayout(dlg);
    l->setContentsMargins(24, 24, 24, 24);
    l->setSpacing(10);

    auto* heading = new QLabel("Create New User", dlg);
    heading->setStyleSheet("font-size:16px; font-weight:700; color:#ffffff;");
    l->addWidget(heading);
    l->addSpacing(8);

    const QString fs = R"(
        QLineEdit { background:#1e1e2e; border:1px solid #2a2a40; border-radius:6px;
                    color:#e0e0f0; font-size:13px; padding:8px 10px; }
        QLineEdit:focus { border-color:#7c3aed; }
    )";

    auto addF = [&](const QString& lbl, QLineEdit*& edit, bool pw = false) {
        auto* lb = new QLabel(lbl, dlg);
        lb->setStyleSheet("font-size:11px; font-weight:600; color:#666680;");
        l->addWidget(lb);
        edit = new QLineEdit(dlg);
        edit->setFixedHeight(36);
        edit->setStyleSheet(fs);
        if (pw) edit->setEchoMode(QLineEdit::Password);
        l->addWidget(edit);
    };

    QLineEdit *uEdit, *dnEdit, *pwEdit;
    addF("Username",     uEdit);
    addF("Display Name", dnEdit);
    addF("Password",     pwEdit, true);

    l->addSpacing(8);
    auto* err = new QLabel("", dlg);
    err->setStyleSheet("font-size:11px; color:#ff4d6d;");
    err->setVisible(false);
    l->addWidget(err);

    auto* row = new QHBoxLayout();
    auto* ok = new QPushButton("Create", dlg);
    ok->setFixedHeight(36);
    ok->setStyleSheet("QPushButton { background:#7c3aed; color:white; border:none; border-radius:6px; font-size:13px; font-weight:600; } QPushButton:hover { background:#6d28d9; }");
    auto* cancel = new QPushButton("Cancel", dlg);
    cancel->setFixedHeight(36);
    cancel->setStyleSheet("QPushButton { background:transparent; color:#555570; border:1px solid #2a2a40; border-radius:6px; font-size:13px; } QPushButton:hover { color:#aaaacc; }");
    row->addWidget(ok); row->addWidget(cancel);
    l->addLayout(row);

    connect(cancel, &QPushButton::clicked, dlg, &QDialog::reject);
    connect(ok, &QPushButton::clicked, dlg, [&, dlg, uEdit, dnEdit, pwEdit, err]() {
        const QString u  = uEdit->text().trimmed();
        const QString dn = dnEdit->text().trimmed();
        const QString pw = pwEdit->text();
        if (u.isEmpty() || dn.isEmpty() || pw.isEmpty()) {
            err->setText("All fields required.");
            err->setVisible(true);
            return;
        }
        if (Database::instance().usernameExists(u)) {
            err->setText("Username already exists.");
            err->setVisible(true);
            return;
        }
        Database::instance().createUser(u, dn, pw, "user", "#7c3aed");
        dlg->accept();
    });

    if (dlg->exec() == QDialog::Accepted)
        refreshTable();
}

void AdminPanel::onDeleteSelected() {
    const auto selected = m_table->selectedItems();
    if (selected.isEmpty()) {
        m_statusLabel->setText("Select a row first.");
        return;
    }

    int row    = m_table->currentRow();
    int userId = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    QString role = m_table->item(row, 3)->text();

    if (role == "admin") {
        m_statusLabel->setText("Admin accounts cannot be deleted.");
        return;
    }

    if (userId == UserSession::instance().user().id) {
        m_statusLabel->setText("You cannot delete your own account.");
        return;
    }

    auto res = QMessageBox::question(this, "Delete User",
        QString("Delete user \"%1\"?").arg(m_table->item(row, 1)->text()),
        QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        Database::instance().deleteUser(userId);
        refreshTable();
    }
}