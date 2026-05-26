#pragma once
#include <QFrame>
#include "core/GameMetadata.h"

class QLabel;
class QPushButton;

class GameCard : public QFrame {
    Q_OBJECT

public:
    explicit GameCard(const GameMetadata& meta, QWidget* parent = nullptr);
    ~GameCard() override;
    void refresh(const GameMetadata& meta);

signals:
    void playRequested(const GameMetadata& meta);
    void deleteRequested(int id);
    void scoreboardRequested(const GameMetadata& meta);

private:
    GameMetadata m_meta;
    QLabel*      m_titleLabel;
    QLabel*      m_descLabel;
    QLabel*      m_statsLabel;
    QPushButton* m_playBtn;
    QPushButton* m_scoreBtn;
    QPushButton* m_delBtn;
};
