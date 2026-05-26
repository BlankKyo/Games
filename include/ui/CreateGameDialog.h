#pragma once
#include <QDialog>
#include "core/GameMetadata.h"

class QLineEdit;
class QTextEdit;
class QComboBox;

// ─────────────────────────────────────────────────────────────────────────────
// CreateGameDialog
//   Modal dialog for registering a new custom game.
//   Currently supports built-in game types; extend to load plugins later.
// ─────────────────────────────────────────────────────────────────────────────
class CreateGameDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateGameDialog(QWidget* parent = nullptr);
    ~CreateGameDialog() override;

    GameMetadata result() const;

private:
    QLineEdit* m_titleEdit;
    QLineEdit* m_authorEdit;
    QLineEdit* m_versionEdit;
    QTextEdit* m_descEdit;
    QComboBox* m_typeCombo;
};
