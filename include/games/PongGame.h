#pragma once
#include "core/GameBase.h"
#include <QRectF>
#include <QPointF>
#include <QKeyEvent>

class PongGame : public GameBase {
    Q_OBJECT

public:
    explicit PongGame(QWidget* parent = nullptr);
    ~PongGame() override;

    void    startGame()              override;
    void    stopGame()               override;
    void    update(double dt)        override;
    void    paintEvent(QPaintEvent*) override;
    QString gameName() const         override { return "Pong"; }

protected:
    void keyPressEvent(QKeyEvent* e)   override;
    void keyReleaseEvent(QKeyEvent* e) override;

private:
    void reset();
    void checkPaddleCollision(QRectF& paddle);

    static constexpr double W      = 600;
    static constexpr double H      = 400;
    static constexpr double PWIDTH  = 12;
    static constexpr double PHEIGHT = 70;
    static constexpr double BALL_R  = 8;
    static constexpr double PSPEED  = 320;
    static constexpr double BALL_SPEED_INIT = 280;

    QRectF  m_leftPaddle;
    QRectF  m_rightPaddle;
    QPointF m_ball;
    QPointF m_ballVel;
    int     m_scoreLeft  = 0;
    int     m_scoreRight = 0;

    // Key states
    bool    m_wDown = false, m_sDown = false;
    bool    m_upDown = false, m_dnDown = false;

    bool    m_running = false;
};
