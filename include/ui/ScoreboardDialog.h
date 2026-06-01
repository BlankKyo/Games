#pragma once
#include <QDialog>
#include "core/Metadata.h"

class ScoreboardDialog : public QDialog {
    Q_OBJECT
public:
    explicit ScoreboardDialog(const GameMetadata& meta, QWidget* parent = nullptr);
    ~ScoreboardDialog() override;
};
