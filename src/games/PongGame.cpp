#include "games/PongGame.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QPainter>
#include <QRandomGenerator>
#include <cmath>

static const char* TAG = "PongGame";

PongGame::PongGame(QWidget* parent) : GameBase(parent) {
    try {
        LOG_INFO(TAG, "Initializing PongGame.");
        setFixedSize((int)W, (int)H);
        setStyleSheet("background:#080810;");
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize PongGame: " + std::string(e.what()));
    }  
}

PongGame::~PongGame() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}

void PongGame::startGame() {
    m_scoreLeft = m_scoreRight = 0;
    reset();
    m_running = true;
}

void PongGame::stopGame() { m_running = false; }

void PongGame::reset() {
    try {
        m_leftPaddle  = {8,       H/2 - PHEIGHT/2, PWIDTH, PHEIGHT};
        m_rightPaddle = {W-8-PWIDTH, H/2 - PHEIGHT/2, PWIDTH, PHEIGHT};
        m_ball        = {W/2, H/2};

        // Random launch angle ±30°
        double angle = (QRandomGenerator::global()->bounded(60) - 30) * M_PI / 180.0;
        double dir   = QRandomGenerator::global()->bounded(2) ? 1.0 : -1.0;
        m_ballVel    = {dir * BALL_SPEED_INIT * std::cos(angle),
                        BALL_SPEED_INIT * std::sin(angle)};
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to reset game state: " + std::string(e.what()));
    }
}

void PongGame::checkPaddleCollision(QRectF& paddle) {
    try {
        QRectF ballRect(m_ball.x() - BALL_R, m_ball.y() - BALL_R, BALL_R*2, BALL_R*2);
        if (ballRect.intersects(paddle)) {
            m_ballVel.setX(-m_ballVel.x() * 1.05); // slight speed-up
            // Add spin based on hit position
            double rel = (m_ball.y() - paddle.center().y()) / (PHEIGHT / 2.0);
            m_ballVel.setY(rel * std::abs(m_ballVel.x()) * 0.75);
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during paddle collision check: " + std::string(e.what()));
    }
}

void PongGame::update(double dt) {
    try {
        if (!m_running) return;

        // Move paddles
        if (m_wDown && m_leftPaddle.top()  > 0)  m_leftPaddle.moveTop(m_leftPaddle.top()  - PSPEED*dt);
        if (m_sDown && m_leftPaddle.bottom() < H) m_leftPaddle.moveTop(m_leftPaddle.top()  + PSPEED*dt);
        if (m_upDown && m_rightPaddle.top() > 0)  m_rightPaddle.moveTop(m_rightPaddle.top()- PSPEED*dt);
        if (m_dnDown && m_rightPaddle.bottom() < H)m_rightPaddle.moveTop(m_rightPaddle.top()+ PSPEED*dt);

        // Move ball
        m_ball += m_ballVel * dt;

        // Top/bottom bounce
        if (m_ball.y() - BALL_R <= 0)  { m_ball.setY(BALL_R);   m_ballVel.setY(std::abs(m_ballVel.y())); }
        if (m_ball.y() + BALL_R >= H)  { m_ball.setY(H-BALL_R); m_ballVel.setY(-std::abs(m_ballVel.y()));}

        // Paddle collision
        checkPaddleCollision(m_leftPaddle);
        checkPaddleCollision(m_rightPaddle);

        // Score
        if (m_ball.x() < 0) {
            ++m_scoreRight;
            setScore(m_scoreLeft * 10 + m_scoreRight);
            reset();
        }
        if (m_ball.x() > W) {
            ++m_scoreLeft;
            setScore(m_scoreLeft * 10 + m_scoreRight);
            reset();
        }

        if (m_scoreLeft >= 7 || m_scoreRight >= 7)
            emit gameOver(m_score);
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during game update: " + std::string(e.what()));
    }
}

void PongGame::keyPressEvent(QKeyEvent* e) {
    try {
        switch (e->key()) {
        case Qt::Key_W:     m_wDown  = true; break;
        case Qt::Key_S:     m_sDown  = true; break;
        case Qt::Key_Up:    m_upDown = true; break;
        case Qt::Key_Down:  m_dnDown = true; break;
        default: GameBase::keyPressEvent(e);
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during key press event: " + std::string(e.what()));
    }
}

void PongGame::keyReleaseEvent(QKeyEvent* e) {
    try {
        switch (e->key()) {
        case Qt::Key_W:     m_wDown  = false; break;
        case Qt::Key_S:     m_sDown  = false; break;
        case Qt::Key_Up:    m_upDown = false; break;
        case Qt::Key_Down:  m_dnDown = false; break;
        default: GameBase::keyReleaseEvent(e);
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during key release event: " + std::string(e.what()));
    }
}

void PongGame::paintEvent(QPaintEvent*) {
    try {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#080810"));

        // Center dashed line
        p.setPen(QPen(QColor(255,255,255,30), 2, Qt::DashLine));
        p.drawLine((int)W/2, 0, (int)W/2, (int)H);

        // Paddles
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#7c3aed"));
        p.drawRoundedRect(m_leftPaddle, 4, 4);
        p.setBrush(QColor("#00ff88"));
        p.drawRoundedRect(m_rightPaddle, 4, 4);

        // Ball
        p.setBrush(QColor("#ffffff"));
        p.drawEllipse(m_ball, BALL_R, BALL_R);

        // Score
        p.setPen(QColor(255,255,255,60));
        QFont f; f.setPixelSize(42); f.setBold(true);
        p.setFont(f);
        p.drawText(QRectF(W/2-120, 20, 100, 60), Qt::AlignRight|Qt::AlignVCenter,
                QString::number(m_scoreLeft));
        p.drawText(QRectF(W/2+20, 20, 100, 60), Qt::AlignLeft|Qt::AlignVCenter,
                QString::number(m_scoreRight));

        // Labels
        QFont small; small.setPixelSize(11);
        p.setFont(small);
        p.setPen(QColor(255,255,255,30));
        p.drawText(20, (int)H - 10, "W/S");
        p.drawText((int)W - 40, (int)H - 10, "↑/↓");
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during paint event: " + std::string(e.what()));
    }
}
