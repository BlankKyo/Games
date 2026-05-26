# 🎮 GameHub

A personal desktop game hub built with **C++20 and Qt6**.  
Create, manage, and play mini-games — with score tracking backed by a local SQLite database.

---

## Features

- **Hub UI** — scrollable card grid showing all games with play counts and high scores
- **Game Runner** — 60 fps fixed-timestep loop with pause/resume and a live score HUD
- **3 built-in games** — Snake, Pong, Breakout (more below)
- **Game registry** — add new games with a single `registerGame()` call
- **SQLite persistence** — scores, play counts, and high scores stored in `~/.local/share/GameHub/gamehub.db` (Linux) or the platform equivalent
- **Scoreboard dialog** — top-10 score history per game
- **Create Game dialog** — register a named entry for any game type in the registry
- **Dark theme** — Fusion palette, no external stylesheet file needed

---

## Prerequisites

| Tool | Minimum version |
|------|----------------|
| C++ compiler | C++20 (GCC 11, Clang 13, MSVC 2022) |
| CMake | 3.20 |
| Qt | 6.4 (Core, Gui, Widgets, Sql, Multimedia) |

### Install Qt6 on common platforms

**Ubuntu / Debian**
```bash
sudo apt install qt6-base-dev qt6-multimedia-dev libqt6sql6-sqlite
```

**Arch Linux**
```bash
sudo pacman -S qt6-base qt6-multimedia
```

**macOS (Homebrew)**
```bash
brew install qt
```

**Windows**  
Use the [Qt Online Installer](https://www.qt.io/download-qt-installer) and select Qt 6.x → Desktop.

---

## Build

```bash
git clone https://github.com/you/gamehub.git
cd gamehub

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run
./build/GameHub          # Linux / macOS
build\GameHub.exe        # Windows
```

### Debug build

```bash
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
./build-debug/GameHub
```

---

## Project Structure

```
gamehub/
├── CMakeLists.txt
├── README.md
|
├── include/
|   ├── core/
|   │   ├── GameBase.h          
|   │   ├── GameMetadata.h      
|   │   ├── GameRegistry.h 
|   │   ├── GameRunner.h   
|   │   └── Database.h     
|   ├── games/
|   │   ├── SnakeGame.h
|   │   ├── PongGame.h
|   |   ├── BullsCowsGame.h  
|   │   └── BreakoutGame.h
|   |   
|   └── ui/
|       ├── MainWindow.h
|       ├── HubView.h
|       ├── GameCard.h
|       ├── CreateGameDialog.h
|       └── ScoreboardDialog.h
└── src/
    ├── main.cpp                # Entry point, palette, DB init
    ├── core/
    │   ├── GameRegistry.cpp 
    │   ├── GameRunner.cpp   
    │   └── Database.cpp     
    ├── games/
    │   ├── SnakeGame.cpp
    │   ├── PongGame.cpp
    │   ├── BullsCowsGame.cpp
    │   └── BreakoutGame.cpp
    └── ui/
        ├── MainWindow.cpp       
        ├── HubView.cpp          
        ├── GameCard.cpp         
        ├── CreateGameDialog.cpp 
        └── ScoreboardDialog.cpp 
```

---

## Adding a New Game

1. **Create the class** in `src/games/`:

```cpp
// src/games/MyGame.h
#pragma once
#include "core/GameBase.h"

class MyGame : public GameBase {
    Q_OBJECT
public:
    explicit MyGame(QWidget* parent = nullptr);

    void    startGame()              override;
    void    stopGame()               override;
    void    update(double dt)        override;  // dt = seconds since last frame
    void    paintEvent(QPaintEvent*) override;
    QString gameName() const         override { return "My Game"; }
};
```

2. **Register it** in `GameRegistry.cpp`:

```cpp
#include "games/MyGame.h"

void GameRegistry::registerBuiltins() {
    // existing entries …
    registerGame("mygame", [](QWidget* p) { return new MyGame(p); });
}
```

3. **Seed it** in `MainWindow::seedBuiltins()` (optional — users can also add it via the UI):

```cpp
{"mygame", "My Game", "One-line description here."},
```

4. **Rebuild** — your game appears in the hub automatically.

### GameBase contract

| Method | Called by | Notes |
|--------|-----------|-------|
| `startGame()` | Runner, once | Initialize state, start sounds |
| `stopGame()` | Runner | Stop timers, free resources |
| `update(dt)` | Runner, 60×/s | Advance simulation; dt ≤ 0.05s |
| `paintEvent(e)` | Qt, after repaint() | Draw everything with QPainter |
| `gameOver(score)` | Your code | Emit when the game ends |

Call `setScore(int)` to update the HUD score label in real-time.  
Call `emit gameOver(m_score)` when the game ends — the runner handles the rest.

---

## Built-in Games

### 🐍 Snake
- Arrow keys or WASD
- Eats red apples, speeds up every 50 points
- Score: +10 per apple

### 🏓 Pong  
- Left paddle: W / S  
- Right paddle: ↑ / ↓  
- First to 7 points wins
- Ball spin varies with paddle hit position

### 🐂 Bulls & Cows
- Guess the hidden 4-digit number
- Each digit is unique
- 🐂 Bull = correct digit in the correct position
- 🐄 Cow = correct digit in the wrong position
- Use logic and deduction to discover the secret number
- Easy Mode can highlight correctly guessed digits
- Fewer attempts lead to a higher score

### 🧱 Breakout
- Left / Right (or A / D) to move paddle
- Space to launch the ball
- Multi-level: bricks get harder and ball gets faster
- Score: +10 × level per brick

---

## Database Schema

The SQLite database lives at the platform `AppData` path reported by `QStandardPaths::AppDataLocation`.

```sql
CREATE TABLE games (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    key          TEXT    NOT NULL UNIQUE,
    title        TEXT    NOT NULL,
    description  TEXT,
    author       TEXT,
    version      TEXT    DEFAULT '1.0',
    is_builtin   INTEGER DEFAULT 0,
    times_played INTEGER DEFAULT 0,
    high_score   INTEGER DEFAULT 0,
    created_at   TEXT    DEFAULT (datetime('now')),
    last_played  TEXT
);

CREATE TABLE scores (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    game_key   TEXT    NOT NULL,
    score      INTEGER NOT NULL,
    played_at  TEXT    DEFAULT (datetime('now')),
    FOREIGN KEY(game_key) REFERENCES games(key) ON DELETE CASCADE
);
```

---

## Roadmap / Future Ideas

- [ ] Plugin system — load games from `.so` / `.dll` at runtime
- [ ] Game creator wizard — configure simple games (quiz, reaction timer) without writing C++
- [ ] Cloud sync — upload scores to a backend REST API
- [ ] Achievements system
- [ ] Controller / gamepad support via `QGamepad`
- [ ] Per-game settings dialog (difficulty, keybindings)
- [ ] Animated hub background / particle system

---

## License

MIT — do whatever you want with it.
