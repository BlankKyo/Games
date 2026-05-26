#include "games/SnakeGame.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFont>
#include <algorithm>


static const char* TAG = "SnakeGame";

SnakeGame::SnakeGame(QWidget* parent) : GameBase(parent) {
    try {
        LOG_INFO(TAG, "Initializing SnakeGame.");
        setFixedSize(COLS * CELL, ROWS * CELL);
        setStyleSheet("background:#080810;");
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize SnakeGame: " + std::string(e.what()));
    }
}

SnakeGame::~SnakeGame() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}

void SnakeGame::startGame() {
    try {
        reset();
        m_alive = true;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to start game: " + std::string(e.what()));
    }
}

void SnakeGame::stopGame() {
    m_alive = false;
}

void SnakeGame::reset() {
    try {
        m_snake.clear();
        m_snake.push_back({COLS / 2, ROWS / 2});
        m_snake.push_back({COLS / 2 - 1, ROWS / 2});
        m_snake.push_back({COLS / 2 - 2, ROWS / 2});
        m_dir = m_nextDir = {1, 0};
        m_accum = 0.0;
        m_speed = 0.12;
        m_dead  = false;
        m_deadTimer = 0.0;
        setScore(0);
        spawnFood();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to reset game state: " + std::string(e.what()));
    }   
}
 
void SnakeGame::spawnFood() {
    try {
        QPoint p;
        do {
            p = { (int)QRandomGenerator::global()->bounded(COLS),
                (int)QRandomGenerator::global()->bounded(ROWS) };
        } while (std::find(m_snake.begin(), m_snake.end(), p) != m_snake.end());
        m_food = p;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to spawn food: " + std::string(e.what()));
    }
}

bool SnakeGame::checkCollision() const {
    try {
        const QPoint& head = m_snake.front();
        if (head.x() < 0 || head.x() >= COLS || head.y() < 0 || head.y() >= ROWS)
            return true;
        for (int i = 1; i < (int)m_snake.size(); ++i)
            if (m_snake[i] == head) return true;
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during collision check: " + std::string(e.what()));
        return false;
    }
}

void SnakeGame::update(double dt) {
    try {
        if (!m_alive) return;

        if (m_dead) {
            m_deadTimer += dt;
            if (m_deadTimer > 1.8) emit gameOver(m_score);
            return;
        }

        m_accum += dt;
        if (m_accum < m_speed) return;
        m_accum -= m_speed;

        m_dir = m_nextDir;
        QPoint newHead = m_snake.front() + QPoint(m_dir.dx, m_dir.dy);
        m_snake.push_front(newHead);

        if (checkCollision()) {
            m_dead = true;
            m_alive = false;
            return;
        }

        if (newHead == m_food) {
            setScore(m_score + 10);
            // Speed up slightly every 5 apples
            if (m_score % 50 == 0) m_speed = std::max(0.06, m_speed - 0.01);
            spawnFood();
        } else {
            m_snake.pop_back();
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during game update: " + std::string(e.what()));
    }
}

void SnakeGame::keyPressEvent(QKeyEvent* e) {
    try {
        switch (e->key()) {
            case Qt::Key_Up:    case Qt::Key_W: if (m_dir.dy ==  0) m_nextDir = {0,-1}; break;
            case Qt::Key_Down:  case Qt::Key_S: if (m_dir.dy ==  0) m_nextDir = {0, 1}; break;
            case Qt::Key_Left:  case Qt::Key_A: if (m_dir.dx ==  0) m_nextDir = {-1,0}; break;
            case Qt::Key_Right: case Qt::Key_D: if (m_dir.dx ==  0) m_nextDir = { 1,0}; break;
        default: GameBase::keyPressEvent(e);
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to handle key press event: " + std::string(e.what()));
    }
}
void SnakeGame::paintEvent(QPaintEvent*) {
    try {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // Background
        p.fillRect(rect(), QColor("#080810"));

        // Grid lines
        p.setPen(QColor(255, 255, 255, 8));
        for (int x = 0; x <= COLS; ++x) p.drawLine(x*CELL, 0, x*CELL, ROWS*CELL);
        for (int y = 0; y <= ROWS; ++y) p.drawLine(0, y*CELL, COLS*CELL, y*CELL);

        // Food
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#ff4d6d"));
        p.drawRoundedRect(m_food.x()*CELL+3, m_food.y()*CELL+3, CELL-6, CELL-6, 4, 4);

        // Snake
        const int n = m_snake.size();
        for (int i = 0; i < n; ++i) {
            const QPoint& seg = m_snake[i];
            int alpha = 120 + 135 * i / std::max(1, n-1);
            QColor col = (i == 0) ? QColor("#00ff88") : QColor(0, 180, 80, alpha);
            if (m_dead) col.setAlpha(80);
            p.setBrush(col);
            p.drawRoundedRect(seg.x()*CELL+1, seg.y()*CELL+1, CELL-2, CELL-2, 4, 4);
        }

        // Death overlay
        if (m_dead) {
            p.fillRect(rect(), QColor(0, 0, 0, 120));
            p.setPen(QColor("#ff4d6d"));
            QFont f; f.setPixelSize(36); f.setBold(true);
            p.setFont(f);
            p.drawText(rect(), Qt::AlignCenter, "GAME OVER");
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Error during paint event: " + std::string(e.what()));
    }
}
