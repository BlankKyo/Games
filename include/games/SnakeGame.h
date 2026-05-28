#pragma once
#include "core/GameBase.h"
#include <deque>
#include <QPoint>
#include <QKeyEvent>
#include <QPainter>

class SnakeGame : public GameBase {
    Q_OBJECT

public:
    explicit SnakeGame(QWidget* parent = nullptr);
    ~SnakeGame() override;

    void    startGame()                   override;
    void    stopGame()                    override;
    void    update(double dt)             override;
    void    paintEvent(QPaintEvent*)      override;
    QString gameName() const              override { return "Snake"; }

protected:
    void keyPressEvent(QKeyEvent* e)      override;

private:
    void reset();
    void spawnFood();
    bool checkCollision() const;

    static constexpr int CELL  = 20;
    static constexpr int COLS  = 28;
    static constexpr int ROWS  = 22;

    struct Dir { int dx, dy; };

    std::deque<QPoint> m_snake;
    QPoint         m_food;
    Dir            m_dir     = {1, 0};
    Dir            m_nextDir = {1, 0};
    double         m_accum   = 0.0;
    double         m_speed   = 0.24; // seconds per step
    bool           m_alive   = false;
};
