cmake_minimum_required(VERSION 3.20)
project(GameHub VERSION 1.0.0 LANGUAGES CXX)

# =========================
# C++ CONFIG
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# QT CONFIG
# =========================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Find Qt6 ──────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Sql
    Multimedia
)

# ── Include paths ─────────────────────────────────────────
include_directories(include)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp

    # Core
    include/core/GameBase.h
    include/core/GameRunner.h
    src/core/GameRunner.cpp
    include/core/GameRegistry.h
    src/core/GameRegistry.cpp
    include/core/GameMetadata.h
    include/core/Database.h
    src/core/Database.cpp

    # UI
    include/ui/MainWindow.h
    src/ui/MainWindow.cpp
    include/ui/HubView.h
    src/ui/HubView.cpp
    include/ui/GameCard.h
    src/ui/GameCard.cpp
    include/ui/CreateGameDialog.h
    src/ui/CreateGameDialog.cpp
    include/ui/ScoreboardDialog.h
    src/ui/ScoreboardDialog.cpp

    # Games
    include/games/SnakeGame.h
    src/games/SnakeGame.cpp
    include/games/PongGame.h
    src/games/PongGame.cpp
    include/games/BreakoutGame.h
    src/games/BreakoutGame.cpp
    include/games/BullsCowsGame.h
    src/games/BullsCowsGame.cpp

    # Utils
    include/utils/MemoryUtils.h
    src/utils/MemoryUtils.cpp
    include/utils/Logger.h
    src/utils/Logger.cpp
)

# ── Executable ────────────────────────────────────────────────────────────────
qt_add_executable(GameHub ${SOURCES} ${RESOURCES})

target_include_directories(GameHub PRIVATE include)

target_link_libraries(GameHub PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Multimedia
)

# ── Platform-specific ─────────────────────────────────────────────────────────
if(WIN32)
    add_custom_command(TARGET GameHub POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
                --debug
                "$<TARGET_FILE:GameHub>"
        COMMENT "Deploying Qt DLLs..."
    )
    set_target_properties(GameHub PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(APPLE)
    set_target_properties(GameHub PROPERTIES MACOSX_BUNDLE TRUE)
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS GameHub
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
