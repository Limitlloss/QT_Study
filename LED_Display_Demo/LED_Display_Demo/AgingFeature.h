#pragma once
#ifndef AGINGFEATURE_H
#define AGINGFEATURE_H

#include <QObject>
#include <QVector>
#include <QColor>
#include "ILedFeature.h"
#include "DisplayWindow.h"

class AgingFeature : public QObject, public ILedFeature {
    Q_OBJECT

public:
    explicit AgingFeature(DisplayWindow* display, QObject* parent = nullptr);

    void tick() override;
    void setEnabled(bool enabled) override;
    bool isEnabled() const override;

    void setInterval(int ms);

private:
    enum FrameType {
        COLOR,
        LINE_SCROLL
    };

    struct AgingFrame {
        FrameType type;
        QColor color;
        QVector<bool> linePattern; // {H, V, D1, D2}
    };

    DisplayWindow* displayWindow;
    bool enabled = false;
    int tickCounter = 0;
    int tickInterval = 20;
    const int baseTickMs = 10;

    int frameIndex = 0;
    QVector<AgingFrame> frames;

    void buildDefaultSequence();
};

#endif // AGINGFEATURE_H
