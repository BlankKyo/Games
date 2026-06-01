// include/ui/AdminPanel.h
#pragma once
#include <QDialog>

class QTableWidget;
class QLabel;

// ─────────────────────────────────────────────────────────────────────────────
// AdminPanel
//   Modal dialog accessible only to admin users.
//   Lists all accounts; allows creating or deleting non-admin users.
// ─────────────────────────────────────────────────────────────────────────────
class AdminPanel : public QDialog {
    Q_OBJECT

public:
    explicit AdminPanel(QWidget* parent = nullptr);

private slots:
    void refreshTable();
    void onCreateUser();
    void onDeleteSelected();

private:
    QTableWidget* m_table = nullptr;
    QLabel*       m_statusLabel = nullptr;
};