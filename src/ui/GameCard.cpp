#include "ui/GameCard.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

static const char* TAG = "GameCard";
GameCard::GameCard(const GameMetadata& meta, QWidget* parent)
    : QFrame(parent), m_meta(meta)
{
    try {
        LOG_INFO(TAG, "Initializing game card for \"" + meta.title.toStdString() + "\".");
        setFixedSize(220, 190);
        
        setStyleSheet(R"(
            GameCard {
                background: #0f0f1a;
                border: 1px solid #1e1e30;
                border-radius: 12px;
            }
            GameCard:hover {
                border: 1px solid #7c3aed;
            }
        )");

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(6);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setStyleSheet("color:#fff; font-size:16px; font-weight:700;");
        m_titleLabel->setWordWrap(true);

        m_descLabel = new QLabel(this);
        m_descLabel->setStyleSheet("color:#555; font-size:12px;");
        m_descLabel->setWordWrap(true);

        m_statsLabel = new QLabel(this);
        m_statsLabel->setStyleSheet("color:#333; font-size:11px; font-family:monospace;");

        // Button row
        auto* btnRow = new QHBoxLayout;
        m_playBtn = new QPushButton("▶ Play", this);
        m_playBtn->setStyleSheet(
            "QPushButton { background:#7c3aed; color:#fff; border:none; padding:6px 0;"
            " border-radius:7px; font-weight:700; }"
            "QPushButton:hover { background:#6d28d9; }");

        m_scoreBtn = new QPushButton("🏆", this);
        m_scoreBtn->setFixedWidth(32);
        m_scoreBtn->setStyleSheet(
            "QPushButton { background:#1e1e30; color:#ffd166; border:none; padding:6px 0;"
            " border-radius:7px; }"
            "QPushButton:hover { background:#2a2a44; }");

        m_delBtn = new QPushButton("✕", this);
        m_delBtn->setFixedWidth(32);
        m_delBtn->setVisible(!meta.isBuiltin);
        m_delBtn->setStyleSheet(
            "QPushButton { background:#1e1e30; color:#ff4d6d; border:1px solid #ff4d6d;"
            " padding:6px 0; border-radius:7px; }"
            "QPushButton:hover { background:#2a0010; }");

        btnRow->addWidget(m_playBtn, 1);
        btnRow->addWidget(m_scoreBtn);
        if (!meta.isBuiltin) btnRow->addWidget(m_delBtn);

        layout->addWidget(m_titleLabel);
        layout->addWidget(m_descLabel);
        layout->addStretch();
        layout->addWidget(m_statsLabel);
        layout->addLayout(btnRow);

        connect(m_playBtn,  &QPushButton::clicked, this, [this]{ emit playRequested(m_meta); });
        connect(m_scoreBtn, &QPushButton::clicked, this, [this]{ emit scoreboardRequested(m_meta); });
        connect(m_delBtn,   &QPushButton::clicked, this, [this]{ emit deleteRequested(m_meta.id); });

        refresh(meta);
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor of " + meta.title.toStdString(), this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize game card: " + std::string(e.what()));
    }
}

GameCard::~GameCard() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor of " + m_meta.title.toStdString(), this, sizeof(*this)));
}

void GameCard::refresh(const GameMetadata& meta) {
    try {
        m_meta = meta;
        m_titleLabel->setText(meta.title);
        m_descLabel->setText(meta.description.isEmpty() ? meta.key : meta.description);
        m_statsLabel->setText(
            QString("Played: %1  ·  Best: %2").arg(meta.timesPlayed).arg(meta.highScore));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to refresh game card: " + std::string(e.what()));
    }
}
