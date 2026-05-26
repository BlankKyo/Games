#include "core/GameRegistry.h"
#include "games/SnakeGame.h"
#include "games/PongGame.h"
#include "games/BreakoutGame.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"

static const char* TAG = "GameRegistry";

void GameRegistry::registerBuiltins() {
    try {
        LOG_INFO(TAG, "Registering builtin games.");
        registerGame("snake",   [](QWidget* p) { return new SnakeGame(p);   });
        registerGame("pong",    [](QWidget* p) { return new PongGame(p);    });
        registerGame("breakout",[](QWidget* p) { return new BreakoutGame(p);});
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to register builtin games: " + std::string(e.what()));
    }   
}

GameRegistry::~GameRegistry() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}
