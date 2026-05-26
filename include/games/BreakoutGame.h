#pragma once
#include "core/GameBase.h"
#include <QVector>
#include <QRectF>
#include <QPointF>
#include <QKeyEvent>

struct Brick {
    QRectF rect;
    int    hits;       // hits remaining
    bool   alive;
    QColor color;
};

class BreakoutGame : public GameBase {
    Q_OBJECT

public:
    explicit BreakoutGame(QWidget* parent = nullptr);
    ~BreakoutGame() override;

    void    startGame()              override;
    void    stopGame()               override;
    void    update(double dt)        override;
    void    paintEvent(QPaintEvent*) override;
    QString gameName() const         override { return "Breakout"; }

protected:
    void keyPressEvent(QKeyEvent* e)   override;
    void keyReleaseEvent(QKeyEvent* e) override;

private:
    void buildLevel(int level);
    void resetBall();
    void checkBrickCollision();

    static constexpr double W       = 560;
    static constexpr double H       = 480;
    static constexpr double PWIDTH  = 80;
    static constexpr double PHEIGHT = 12;
    static constexpr double BALL_R  = 7;
    static constexpr double PSPEED  = 350;

    QRectF         m_paddle;
    QPointF        m_ball;
    QPointF        m_ballVel;
    QVector<Brick> m_bricks;
    int            m_lives = 3;
    int            m_level = 1;
    bool           m_launched = false;

    bool           m_leftDown  = false;
    bool           m_rightDown = false;
    bool           m_running   = false;
};
