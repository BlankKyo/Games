#include "games/BullsCowsGame.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QMetaObject>
#include <QFont>
#include <algorithm>


static const char* TAG = "BullsCowsGame";

BullsCowsGame::BullsCowsGame(QWidget* parent) : GameBase(parent) {
    try {
        // Logging:
        LOG_INFO(TAG, "Initializing BullsCowsGame.");
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));

        // Body:
        setFixedSize(W, H);
        setStyleSheet("background:#080810;");

        easyModeBtn = new QPushButton("Easy Mode", this);
        easyModeBtn->setStyleSheet(R"(
            QPushButton {
                background: #222;
                color: white;
                border-radius: 6px;
                padding: 6px 12px;
            }

            QPushButton:checked {
                background: #00aa66;
            }
        )");
        easyModeBtn->setFocusPolicy(Qt::NoFocus);
        easyModeBtn->setCheckable(true);
        easyModeBtn->setChecked(false);
        easyModeBtn->move(W - easyModeBtn->sizeHint().width() - 20, 20);

        connect(easyModeBtn, &QPushButton::toggled, this, &BullsCowsGame::setEasyModeEnabled);
        
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize BullsCowsGame: " + std::string(e.what()));
    }
}

BullsCowsGame::~BullsCowsGame() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────────────────────────────────────
void BullsCowsGame::startGame() {
    reset();
    m_running = true;
}

void BullsCowsGame::stopGame() {
    m_running = false;
}

void BullsCowsGame::reset() {
    generateTarget();
    m_rows.clear();
    for (int i = 0; i < MAX_TRIES; ++i)
        m_rows.append(Row{});
    m_tryIdx  = 0;
    m_inputIdx = 0;
    m_current  = {};
    std::fill(std::begin(m_used), std::end(m_used), false);
    m_state = State::Playing;
    setScore(0);
}

void BullsCowsGame::setEasyModeEnabled(bool enabled) {
    m_easyMode = enabled;
}

void BullsCowsGame::generateTarget() {
    QVector<int> pool = {0,1,2,3,4,5,6,7,8,9};
    // shuffle
    for (int i = 9; i > 0; --i) {
        int j = QRandomGenerator::global()->bounded(i + 1);
        std::swap(pool[i], pool[j]);
    }
    for (int i = 0; i < 4; ++i)
        m_target[i] = pool[i];
}

// ─────────────────────────────────────────────────────────────────────────────
//  Input
// ─────────────────────────────────────────────────────────────────────────────
void BullsCowsGame::keyPressEvent(QKeyEvent* e) {
    try {
        if (!m_running || m_state != State::Playing) return;

        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            if (m_inputIdx == 4) submitGuess();
            return;
        }

        if (e->key() == Qt::Key_Backspace) {
            if (m_inputIdx > 0) {
                --m_inputIdx;
                m_used[m_current[m_inputIdx]] = false;
                m_current[m_inputIdx] = 0;
                repaint();
            }
            return;
        }

        // Digit key
        if (e->text().size() == 1 && e->text()[0].isDigit()) {
            int digit = e->text()[0].digitValue();
            if (!m_used[digit] && m_inputIdx < 4) {
                m_current[m_inputIdx] = digit;
                m_used[digit] = true;
                ++m_inputIdx;
                repaint();
            }
        }
    } catch (const std::exception& ex) {
        LOG_ERROR(TAG, "Error in keyPressEvent: " + std::string(ex.what()));
    }
}

void BullsCowsGame::submitGuess() {
    Row& row    = m_rows[m_tryIdx];
    row.digits  = m_current;
    row.bulls   = countBulls(m_current);
    row.cows    = countCows(m_current);
    row.filled  = true;

    // Score: more bulls/cows earlier = more points
    int points = (row.bulls * 100 + row.cows * 25) * (MAX_TRIES - m_tryIdx);
    setScore(m_score + points);

    ++m_tryIdx;

    if (row.bulls == 4) {
        m_state = State::Won;
        m_running = false;
        QMetaObject::invokeMethod(this, [this]{
            emit gameOver(m_score);
        }, Qt::QueuedConnection);
    } else if (m_tryIdx >= MAX_TRIES) {
        m_state = State::Lost;
        m_running = false;
        QMetaObject::invokeMethod(this, [this]{
            emit gameOver(0);
        }, Qt::QueuedConnection);
    }

    // Reset input for next row
    m_inputIdx = 0;
    m_current  = {};
    std::fill(std::begin(m_used), std::end(m_used), false);
    repaint();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Logic
// ─────────────────────────────────────────────────────────────────────────────
int BullsCowsGame::countBulls(const std::array<int,4>& guess) const {
    int bulls = 0;
    for (int i = 0; i < 4; ++i)
        if (guess[i] == m_target[i]) ++bulls;
    return bulls;
}

int BullsCowsGame::countCows(const std::array<int,4>& guess) const {
    int cows = 0;
    for (int i = 0; i < 4; ++i)
        if (guess[i] != m_target[i])
            for (int j = 0; j < 4; ++j)
                if (i != j && guess[i] == m_target[j]) { ++cows; break; }
    return cows;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Paint
// ─────────────────────────────────────────────────────────────────────────────
void BullsCowsGame::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#080810"));

    // ── Constants ─────────────────────────────────────────────────────────
    const int CELL_W   = 52;
    const int CELL_H   = 42;
    const int CELL_GAP = 8;
    const int ROW_GAP  = 6;
    const int START_X  = 40;
    const int START_Y  = 50;
    const int BC_X     = START_X + 4*(CELL_W+CELL_GAP) + 20;

    QFont digitFont;
    digitFont.setPixelSize(22);
    digitFont.setBold(true);

    QFont labelFont;
    labelFont.setPixelSize(12);

    QFont bigFont;
    bigFont.setPixelSize(28);
    bigFont.setBold(true);

    // ── Column headers ────────────────────────────────────────────────────
    p.setFont(labelFont);
    p.setPen(QColor("#444"));
    p.drawText(QRect(BC_X, START_Y - 20, 40, 18), Qt::AlignCenter, "🐂");
    p.drawText(QRect(BC_X + 44, START_Y - 20, 40, 18), Qt::AlignCenter, "🐄");

    // ── Rows ──────────────────────────────────────────────────────────────
    for (int r = 0; r < MAX_TRIES; ++r) {
        const Row& row = m_rows[r];
        int y = START_Y + r * (CELL_H + ROW_GAP);
        bool isCurrent = (r == m_tryIdx && m_state == State::Playing);

        for (int c = 0; c < 4; ++c) {
            int x = START_X + c * (CELL_W + CELL_GAP);

            // Cell background
            QColor bg;
            if (row.filled) {
                // Color by result
                bool isBull = (row.digits[c] == m_target[c]);
                bg = isBull && m_easyMode ? QColor("#003d20") : QColor("#1a1a2e");
            } else if (isCurrent) {
                bg = QColor("#12122a");
            } else {
                bg = QColor("#0d0d1a");
            }

            QColor border = row.filled
                ? (row.digits[c] == m_target[c] && m_easyMode ? QColor("#00ff88") : QColor("#333"))
                : (isCurrent ? QColor("#7c3aed") : QColor("#1e1e30"));

            p.setBrush(bg);
            p.setPen(QPen(border, isCurrent ? 2 : 1));
            p.drawRoundedRect(x, y, CELL_W, CELL_H, 7, 7);

            // Digit text
            QString txt;
            if (row.filled) {
                txt = QString::number(row.digits[c]);
            } else if (isCurrent && c < m_inputIdx) {
                txt = QString::number(m_current[c]);
            }

            if (!txt.isEmpty()) {
                p.setPen(row.filled && row.digits[c] == m_target[c] && m_easyMode
                         ? QColor("#00ff88") : QColor("#fff"));
                p.setFont(digitFont);
                p.drawText(QRect(x, y, CELL_W, CELL_H), Qt::AlignCenter, txt);
            }
        }

        // Bulls & Cows count
        if (row.filled) {
            p.setFont(digitFont);
            // Bulls
            p.setPen(QColor("#00ff88"));
            p.drawText(QRect(BC_X, y, 40, CELL_H), Qt::AlignCenter,
                       QString::number(row.bulls));
            // Cows
            p.setPen(QColor("#ffd166"));
            p.drawText(QRect(BC_X + 44, y, 40, CELL_H), Qt::AlignCenter,
                       QString::number(row.cows));
        }

        // Row number
        p.setFont(labelFont);
        p.setPen(isCurrent ? QColor("#7c3aed") : QColor("#222"));
        p.drawText(QRect(START_X - 28, y, 20, CELL_H), Qt::AlignCenter,
                   QString::number(r + 1));
    }

    // ── Input hint ────────────────────────────────────────────────────────
    if (m_easyMode && m_state == State::Playing) {
        p.setFont(labelFont);
        p.setPen(QColor("#444"));
        QString hint = m_inputIdx < 4
            ? QString("Enter %1 more digit%2").arg(4 - m_inputIdx).arg(4 - m_inputIdx == 1 ? "" : "s")
            : "Press ENTER to confirm";
        p.drawText(QRect(0, H - 30, W, 20), Qt::AlignCenter, hint);
    }

    // ── Win / Lose overlay ────────────────────────────────────────────────
    if (m_state != State::Playing) {
        p.fillRect(rect(), QColor(0, 0, 0, 160));

        p.setFont(bigFont);
        if (m_state == State::Won) {
            p.setPen(QColor("#00ff88"));
            p.drawText(rect(), Qt::AlignCenter,
                       QString("🎉 You got it!\nIn %1 tries\nScore: %2")
                           .arg(m_tryIdx).arg(m_score));
        } else {
            // Show target on loss
            QString targetStr;
            for (int d : m_target) targetStr += QString::number(d);

            p.setPen(QColor("#ff4d6d"));
            QFont lostFont; lostFont.setPixelSize(22); lostFont.setBold(true);
            p.setFont(lostFont);
            p.drawText(rect(), Qt::AlignCenter,
                       QString("Out of tries!\nAnswer was: %1").arg(targetStr));
        }
    }
}