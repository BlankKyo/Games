// include/games/BullsCowsGame.h
#pragma once
#include "core/GameBase.h"
#include <QVector>
#include <QString>
#include <QKeyEvent>
#include <QPushButton>
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
// BullsCowsGame
//   4-digit Bulls & Cows.
//   • Digits must all be unique (0–9)
//   • Bull  = right digit, right position
//   • Cow   = right digit, wrong position
//   • 9 attempts max
// ─────────────────────────────────────────────────────────────────────────────
class BullsCowsGame : public GameBase {
    Q_OBJECT

public:
    explicit BullsCowsGame(QWidget* parent = nullptr);
    ~BullsCowsGame() override;

    void    startGame()              override;
    void    stopGame()               override;
    void    update(double dt)        override {}   // input-driven, no tick needed
    void    paintEvent(QPaintEvent*) override;
    void    setEasyModeEnabled(bool enabled);
    QString gameName() const         override { return "Bulls & Cows"; }

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    void generateTarget();
    void submitGuess();
    void reset();

    QPushButton* easyModeBtn = nullptr;

    int  countBulls(const std::array<int,4>& guess) const;
    int  countCows (const std::array<int,4>& guess) const;
    bool m_easyMode = false;  
    
    static constexpr int MAX_TRIES = 9;
    static constexpr int W = 560;
    static constexpr int H = 520;

    std::array<int,4>  m_target;

    // current input
    std::array<int,4>  m_current;
    bool               m_used[10] = {};   // which digits are typed this row
    int                m_inputIdx = 0;    // 0–4

    struct Row {
        std::array<int,4> digits;
        int bulls = 0;
        int cows  = 0;
        bool filled = false;
    };

    QVector<Row>  m_rows;
    int           m_tryIdx  = 0;    // current row (0–8)
    bool          m_running = false;

    enum class State { Playing, Won, Lost };
    State m_state = State::Playing;
};