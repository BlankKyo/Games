#include <QApplication>
#include <QFontDatabase>
#include <QStyleFactory>
#include "ui/MainWindow.h"
#include "core/Database.h"
#include "core/GameRegistry.h"
#include "utils/Logger.h"


// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
LogLevel parseLogLevel(const std::string& s) {
    if (s == "DEBUG")   return LogLevel::DEBUG;
    if (s == "WARNING") return LogLevel::WARNING;
    if (s == "ERROR")   return LogLevel::ERROR;
    return LogLevel::INFO;
}

std::string extractQtObjectName(const std::string& msg){
    // Find last quote
    size_t firstQuote = msg.find_last_of('"');
    if (firstQuote == std::string::npos)
        return "Qt";

    // find previous quote
    size_t secondQuote = msg.find_last_of('"', firstQuote - 1);
    if (secondQuote == std::string::npos)
        return "Qt";

    return msg.substr(secondQuote + 1, firstQuote - secondQuote - 1);
}
static void qtHandler(QtMsgType type, const QMessageLogContext&, const QString& msg) {
    std::string m = msg.toStdString();

    switch (type) {
        case QtDebugMsg:    Logger::instance().debug  (extractQtObjectName(m), "[Qt] " + m); break;
        case QtWarningMsg:  Logger::instance().warning(extractQtObjectName(m), "[Qt] " + m); break;
        case QtCriticalMsg: Logger::instance().error  (extractQtObjectName(m), "[Qt] " + m); break;
        case QtFatalMsg:    Logger::instance().error  (extractQtObjectName(m), "[Qt][FATAL] " + m); break;
        default: break;
    }
}


static const char* TAG = "Main";

int main(int argc, char* argv[]) {

    // ── Logger ────────────────────────────────────────────
    std::string logFile  = "logs/algo.log";
    std::string logLevel = "DEBUG";
    try {
        Logger::instance().init(logFile, parseLogLevel(logLevel));
        qInstallMessageHandler(qtHandler);
    } catch (const std::exception& e) {
        LOG_WARNING(TAG, "Failed to initialize logger: " + std::string(e.what()));
    }
    LOG_INFO(TAG, "Logger initialized.");

    QApplication app(argc, argv);

    // ── App identity (used by QStandardPaths for DB location) ─────────────
    QApplication::setOrganizationName("GameHub");
    QApplication::setApplicationName("GameHub");
    QApplication::setApplicationVersion("1.0.0");

    // ── Dark fusion palette ────────────────────────────────────────────────
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette dark;
    dark.setColor(QPalette::Window,          QColor("#080810"));
    dark.setColor(QPalette::WindowText,      QColor("#ffffff"));
    dark.setColor(QPalette::Base,            QColor("#0f0f1a"));
    dark.setColor(QPalette::AlternateBase,   QColor("#0d0d16"));
    dark.setColor(QPalette::Text,            QColor("#ffffff"));
    dark.setColor(QPalette::Button,          QColor("#1e1e30"));
    dark.setColor(QPalette::ButtonText,      QColor("#ffffff"));
    dark.setColor(QPalette::Highlight,       QColor("#7c3aed"));
    dark.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    app.setPalette(dark);

    // ── Register built-in games ────────────────────────────────────────────
    GameRegistry::instance().registerBuiltins();

    // ── Open database ──────────────────────────────────────────────────────
    if (!Database::instance().open()) {
        LOG_ERROR(TAG, "Could not open database – scores will not persist.");
    }

    // ── Launch ────────────────────────────────────────────────────────────
    MainWindow window;
    window.show();

    const int ret = app.exec();

    Database::instance().close();
    return ret;
}
