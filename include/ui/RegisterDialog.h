// include/ui/RegisterDialog.h
#pragma once
#include <QDialog>

class QLineEdit;
class QLabel;

// ─────────────────────────────────────────────────────────────────────────────
// RegisterDialog
//   Opened from LoginWindow's "Create one" link.
//   Creates a new regular (non-admin) account.
// ─────────────────────────────────────────────────────────────────────────────
class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget* parent = nullptr);

private slots:
    void attemptRegister();

private:
    QLineEdit* m_usernameEdit;
    QLineEdit* m_displayNameEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmEdit;
    QLabel*    m_errorLabel;

    // Avatar color picker
    QString    m_avatarColor = "#7c3aed";
    QWidget*   m_colorSwatch = nullptr;

    static const QStringList s_palette;
    void pickColor(const QString& hex);
};