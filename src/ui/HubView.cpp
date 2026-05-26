#include "ui/HubView.h"
#include "ui/GameCard.h"
#include "utils/Logger.h"
#include "utils/MemoryUtils.h"
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>


static const char* TAG = "HubView";

HubView::HubView(QWidget* parent) : QWidget(parent) {
    try {
        LOG_INFO(TAG, "Initializing HubView UI.");

        setStyleSheet("background:#080810;");

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        // ── Header bar ────────────────────────────────────────────────────────
        auto* header = new QWidget(this);
        header->setFixedHeight(70);
        header->setStyleSheet("background:#0a0a14; border-bottom:1px solid #1e1e30;");
        auto* hl = new QHBoxLayout(header);
        hl->setContentsMargins(28, 0, 28, 0);

        auto* title = new QLabel("GAME HUB", this);
        title->setStyleSheet(
            "font-size:26px; font-weight:900; letter-spacing:3px;"
            "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #00ff88,stop:1 #7c3aed);"
            "-webkit-background-clip:text; color:transparent;");
        // Qt doesn't support gradient text via stylesheet; use plain color instead
        title->setStyleSheet("font-size:26px; font-weight:900; color:#7c3aed; letter-spacing:3px;");

        auto* sub = new QLabel("Your personal arcade", this);
        sub->setStyleSheet("color:#333; font-size:13px; margin-left:12px;");

        auto* createBtn = new QPushButton("＋  Create Game", this);
        createBtn->setFixedHeight(38);
        createBtn->setStyleSheet(
            "QPushButton { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #7c3aed,stop:1 #4f46e5);"
            " color:#fff; border:none; padding:0 22px; border-radius:10px; font-weight:700; font-size:14px; }"
            "QPushButton:hover { background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #6d28d9,stop:1 #4338ca); }");

        hl->addWidget(title);
        hl->addWidget(sub, 0, Qt::AlignVCenter);
        hl->addStretch();
        hl->addWidget(createBtn);

        connect(createBtn, &QPushButton::clicked, this, &HubView::createRequested);

        // ── Scrollable card area ───────────────────────────────────────────────
        auto* scroll = new QScrollArea(this);
        scroll->setWidgetResizable(true);
        scroll->setStyleSheet("QScrollArea { border:none; background:#080810; }"
                            "QScrollBar:vertical { width:8px; background:#0f0f1a; }"
                            "QScrollBar::handle:vertical { background:#1e1e30; border-radius:4px; }");

        m_inner = new QWidget;
        m_inner->setStyleSheet("background:#080810;");
        m_grid = new QGridLayout(m_inner);
        m_grid->setContentsMargins(28, 28, 28, 28);
        m_grid->setSpacing(16);
        m_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        scroll->setWidget(m_inner);

        root->addWidget(header);
        root->addWidget(scroll, 1);
        LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Constructor", this, sizeof(*this)));
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to initialize HubView: " + std::string(e.what()));
    }
    
}

HubView::~HubView() {
    LOG_DEBUG(TAG, MemoryUtils::formatLifecycleLog("Destructor", this, sizeof(*this)));
}
void HubView::clearCards() {
    try {
        for (auto* c : m_cards) c->deleteLater();
        m_cards.clear();
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to clear game cards: " + std::string(e.what()));
    }   
}

void HubView::setGames(const QList<GameMetadata>& games) {
    try {
        clearCards();
        const int COLS = 4;
        int i = 0;
        for (const auto& meta : games) {
            auto* card = new GameCard(meta, m_inner);
            connect(card, &GameCard::playRequested,       this, &HubView::playRequested);
            connect(card, &GameCard::deleteRequested,     this, &HubView::deleteRequested);
            connect(card, &GameCard::scoreboardRequested, this, &HubView::scoreboardRequested);
            m_grid->addWidget(card, i / COLS, i % COLS);
            m_cards.append(card);
            ++i;
        }
    } catch (const std::exception& e) {
        LOG_ERROR(TAG, "Failed to set games in HubView: " + std::string(e.what()));
    }   
}

void HubView::refresh(const QList<GameMetadata>& games) {
    setGames(games);
}
