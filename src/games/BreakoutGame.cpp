#include "games/BreakoutGame.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QPainter>
#include <QKeyEvent>
#include <cmath>

static const char* TAG = "BreakoutGame";

static const QColor BRICK_COLORS[] = {
    "#ff4d6d", "#ff8c42", "#ffd166", "#06d6a0", "#118ab2", "#7c3aed"
};

BreakoutGame::BreakoutGame(QWidget* parent) : GameBase(parent) {
    try {
        LOG_INFO(TAG, "Initializing BreakoutGame.");
        setFixedSize((int)W, (int)H);
        setStyleSheet("background:#080810;");
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize BreakoutGame: " + std::string(e.what()));
    }   
}

BreakoutGame::~BreakoutGame() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}

void BreakoutGame::startGame() {
    m_lives = 3;
    m_level = 1;
    buildLevel(1);
    resetBall();
    m_running = true;
}

void BreakoutGame::stopGame() { m_running = false; }

void BreakoutGame::buildLevel(int level) {
    try {
        m_bricks.clear();
        const int COLS  = 10;
        const int ROWS  = 5 + level;
        const double bw = (W - 20) / COLS;
        const double bh = 22;

        for (int r = 0; r < ROWS; ++r) {
            int hits = (r < 2 && level >= 3) ? 2 : 1;
            for (int c = 0; c < COLS; ++c) {
                Brick b;
                b.rect  = {10 + c*bw + 2, 40 + r*(bh+4) + 2, bw - 4, bh};
                b.hits  = hits;
                b.alive = true;
                b.color = BRICK_COLORS[r % 6];
                m_bricks.append(b);
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to build level " + std::to_string(level) + ": " + std::string(e.what()));
    }   
}

void BreakoutGame::resetBall() {
    m_paddle   = {W/2 - PWIDTH/2, H - 30, PWIDTH, PHEIGHT};
    m_ball     = {W/2, H - 50};
    m_ballVel  = {0, 0};
    m_launched = false;
}

void BreakoutGame::update(double dt) {
    try {
        if (!m_running) return;

        // Move paddle
        if (m_leftDown  && m_paddle.left()  > 0)  m_paddle.moveLeft(m_paddle.left()  - PSPEED*dt);
        if (m_rightDown && m_paddle.right() < W)   m_paddle.moveLeft(m_paddle.left()  + PSPEED*dt);

        if (!m_launched) {
            m_ball = {m_paddle.center().x(), m_paddle.top() - BALL_R - 1};
            return;
        }

        // Move ball
        double speed = std::sqrt(m_ballVel.x()*m_ballVel.x() + m_ballVel.y()*m_ballVel.y());
        double targetSpeed = 260 + m_level * 15.0;
        // Gradually correct speed
        if (speed > 0) m_ballVel *= targetSpeed / speed;

        m_ball += m_ballVel * dt;

        // Walls
        if (m_ball.x() - BALL_R < 0)  { m_ball.setX(BALL_R);   m_ballVel.setX(std::abs(m_ballVel.x())); }
        if (m_ball.x() + BALL_R > W)  { m_ball.setX(W-BALL_R); m_ballVel.setX(-std::abs(m_ballVel.x()));}
        if (m_ball.y() - BALL_R < 0)  { m_ball.setY(BALL_R);   m_ballVel.setY(std::abs(m_ballVel.y())); }

        // Paddle
        QRectF ballRect(m_ball.x()-BALL_R, m_ball.y()-BALL_R, BALL_R*2, BALL_R*2);
        if (ballRect.intersects(m_paddle) && m_ballVel.y() > 0) {
            double rel = (m_ball.x() - m_paddle.center().x()) / (PWIDTH / 2.0);
            m_ballVel.setX(rel * 280);
            m_ballVel.setY(-std::abs(m_ballVel.y()));
        }

        // Lost ball
        if (m_ball.y() > H + 20) {
            --m_lives;
            if (m_lives <= 0) { emit gameOver(m_score); return; }
            resetBall();
            return;
        }

        checkBrickCollision();

        // Level complete
        bool anyAlive = false;
        for (auto& b : m_bricks) if (b.alive) { anyAlive = true; break; }
        if (!anyAlive) {
            ++m_level;
            buildLevel(m_level);
            resetBall();
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during game update: " + std::string(e.what())); 
    }
}

void BreakoutGame::checkBrickCollision() {
    try {
        QRectF ballRect(m_ball.x()-BALL_R, m_ball.y()-BALL_R, BALL_R*2, BALL_R*2);
        for (auto& b : m_bricks) {
            if (!b.alive || !ballRect.intersects(b.rect)) continue;

            // Determine collision axis
            double overlapL = ballRect.right()  - b.rect.left();
            double overlapR = b.rect.right()    - ballRect.left();
            double overlapT = ballRect.bottom() - b.rect.top();
            double overlapB = b.rect.bottom()   - ballRect.top();

            double minH = std::min(overlapL, overlapR);
            double minV = std::min(overlapT, overlapB);

            if (minH < minV) m_ballVel.setX(-m_ballVel.x());
            else             m_ballVel.setY(-m_ballVel.y());

            --b.hits;
            if (b.hits <= 0) {
                b.alive = false;
                setScore(m_score + 10 * m_level);
            }
            break; // one brick per frame
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during brick collision check: " + std::string(e.what())); 
    }   
}

void BreakoutGame::keyPressEvent(QKeyEvent* e) {
    try {
        switch (e->key()) {
        case Qt::Key_Left:  case Qt::Key_A: m_leftDown  = true; break;
        case Qt::Key_Right: case Qt::Key_D: m_rightDown = true; break;
        case Qt::Key_Space:
            if (!m_launched) {
                m_ballVel  = {80.0, -260.0};
                m_launched = true;
            }
            break;
        default: GameBase::keyPressEvent(e);
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during key press event: " + std::string(e.what())); 
    }   
}

void BreakoutGame::keyReleaseEvent(QKeyEvent* e) {
    try {
        switch (e->key()) {
        case Qt::Key_Left:  case Qt::Key_A: m_leftDown  = false; break;
        case Qt::Key_Right: case Qt::Key_D: m_rightDown = false; break;
        default: GameBase::keyReleaseEvent(e);
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during key release event: " + std::string(e.what())); 
    }   
}

void BreakoutGame::paintEvent(QPaintEvent*) {
    try {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor("#080810"));

        // Bricks
        for (const auto& b : m_bricks) {
            if (!b.alive) continue;
            QColor c = b.color;
            if (b.hits == 2) c = c.lighter(130);
            p.setBrush(c);
            p.setPen(QPen(c.darker(160), 1));
            p.drawRoundedRect(b.rect, 3, 3);
        }

        // Paddle
        p.setBrush(QColor("#7c3aed"));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(m_paddle, 5, 5);

        // Ball
        p.setBrush(QColor("#ffffff"));
        p.drawEllipse(m_ball, BALL_R, BALL_R);

        // HUD
        QFont f; f.setPixelSize(13);
        p.setFont(f);
        p.setPen(QColor("#aaa"));
        p.drawText(10, 20, QString("Lives: %1  Level: %2").arg(m_lives).arg(m_level));

        if (!m_launched) {
            p.setPen(QColor(255,255,255,80));
            QFont big; big.setPixelSize(18); big.setBold(true);
            p.setFont(big);
            p.drawText(rect(), Qt::AlignCenter, "Press SPACE to launch");
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during paint event: " + std::string(e.what()));
    }
}
