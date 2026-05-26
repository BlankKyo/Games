#include "ui/ScoreboardDialog.h"
#include "core/Database.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QDialogButtonBox>


static const char* TAG = "ScoreboardDialog";

ScoreboardDialog::ScoreboardDialog(const GameMetadata& meta, QWidget* parent)
    : QDialog(parent)
{
    try {
        LOG_INFO(TAG, "Opening scoreboard for game \"" + meta.title.toStdString() + "\".");

        setWindowTitle(QString("Scoreboard — %1").arg(meta.title));
        setMinimumSize(360, 400);
        setStyleSheet(R"(
            QDialog { background:#0f0f1a; color:#fff; }
            QLabel  { color:#aaa; }
            QTableWidget { background:#080810; color:#fff; gridline-color:#1e1e30;
                        border:none; }
            QHeaderView::section { background:#0d0d1a; color:#555; border:none;
                                padding:6px; }
            QTableWidget::item { padding:6px; }
        )");

        auto* root = new QVBoxLayout(this);

        auto* heading = new QLabel(QString("🏆  %1").arg(meta.title), this);
        heading->setStyleSheet("font-size:20px; font-weight:700; color:#ffd166; margin-bottom:8px;");
        root->addWidget(heading);

        auto* stats = new QLabel(
            QString("Times played: %1  ·  All-time best: %2").arg(meta.timesPlayed).arg(meta.highScore),
            this);
        stats->setStyleSheet("color:#555; font-family:monospace; font-size:12px; margin-bottom:12px;");
        root->addWidget(stats);

        const auto scores = Database::instance().topScores(meta.key, 10);
 
        if (scores.isEmpty()) {
            auto* empty = new QLabel("No scores yet — play a game first!", this);
            empty->setStyleSheet("color:#444; font-size:14px;");
            empty->setAlignment(Qt::AlignCenter);
            root->addWidget(empty, 1);
        } else {
            auto* table = new QTableWidget(0, 2, this);
            table->setHorizontalHeaderLabels({"Score", "Date"});
            table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            table->verticalHeader()->setVisible(false);
            table->setEditTriggers(QAbstractItemView::NoEditTriggers);
            table->setSelectionMode(QAbstractItemView::NoSelection);
            table->setRowCount(scores.size());
    
            for (int i = 0; i < scores.size(); ++i) {
                auto* scoreItem = new QTableWidgetItem(QString::number(scores[i].score));
                scoreItem->setForeground(i == 0 ? QColor("#ffd166") : QColor("#fff"));
                scoreItem->setTextAlignment(Qt::AlignCenter);
                table->setItem(i, 0, scoreItem);
    
                QString dateStr = scores[i].playedAt.isValid()
                    ? scores[i].playedAt.toString("yyyy-MM-dd hh:mm")
                    : QString("—");
                auto* dateItem = new QTableWidgetItem(dateStr);
                dateItem->setTextAlignment(Qt::AlignCenter);
                table->setItem(i, 1, dateItem);
            }
            root->addWidget(table, 1);
        }

        auto* btns = new QDialogButtonBox(QDialogButtonBox::Close, this);
        btns->setStyleSheet("QPushButton { background:#1e1e30; color:#aaa; border:none; padding:7px 20px; border-radius:7px; }");
        connect(btns, &QDialogButtonBox::rejected, this, &QDialog::accept);
        root->addWidget(btns);
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize ScoreboardDialog: " + std::string(e.what()));
    }   
}

ScoreboardDialog::~ScoreboardDialog() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}
