#pragma once
#include <QWidget>
#include <QList>
#include "core/GameMetadata.h"

class QScrollArea;
class QGridLayout;
class GameCard;

class HubView : public QWidget {
    Q_OBJECT

public:
    explicit HubView(QWidget* parent = nullptr);
    ~HubView() override;

    void setGames(const QList<GameMetadata>& games);
    void refresh(const QList<GameMetadata>& games);

signals:
    void playRequested(const GameMetadata& meta);
    void deleteRequested(int id);
    void scoreboardRequested(const GameMetadata& meta);
    void createRequested();

private:
    void clearCards();
    

    QGridLayout* m_grid   = nullptr;
    QWidget*     m_inner  = nullptr;
    QList<GameCard*> m_cards;
};
