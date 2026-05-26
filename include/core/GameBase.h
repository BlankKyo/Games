#pragma once
#include <QWidget>
#include <QString>

// ─────────────────────────────────────────────────────────────────────────────
// GameBase
//   Abstract base class for every mini-game.
//
//   Lifecycle:
//     1. GameRunner creates the game via GameRegistry::create()
//     2. Runner calls startGame() once
//     3. Runner drives the loop: update(dt) → triggers repaint() → paintEvent()
//     4. Game emits gameOver(score) when finished
//     5. Runner calls stopGame(), then destroys the widget
//
//   To add a new game:
//     • Subclass GameBase
//     • Override all pure-virtual methods
//     • Register it in GameRegistry::registerBuiltins()
// ─────────────────────────────────────────────────────────────────────────────
class GameBase : public QWidget {
    Q_OBJECT

public:
    explicit GameBase(QWidget* parent = nullptr) : QWidget(parent) {
        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_OpaquePaintEvent);
    }

    virtual ~GameBase() = default;

    // ── Must override ─────────────────────────────────────────────────────
    virtual void   startGame()                         = 0;
    virtual void   stopGame()                          = 0;
    virtual void   update(double deltaSeconds)         = 0;
    virtual void   paintEvent(QPaintEvent* e) override = 0;
    virtual QString gameName() const                   = 0;

    // ── Optional overrides ────────────────────────────────────────────────
    virtual void   pauseGame()  { m_paused = true; emit pauseToggled(true); }
    virtual void   resumeGame() { m_paused = false; emit pauseToggled(false); }
    virtual bool   isPaused()   const { return m_paused; }
    virtual int    score()      const { return m_score; }

signals:
    void gameOver(int finalScore);
    void scoreChanged(int newScore);
    void pauseToggled(bool paused);

protected:
    void setScore(int s) { m_score = s; emit scoreChanged(s); }
    void setScore(int s, bool /*unused*/) { setScore(s); }
    void setPaused(bool p) { m_paused = p; emit pauseToggled(p); }

    int  m_score  = 0;
    bool m_paused = false;
};
