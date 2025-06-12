#pragma once
#ifndef AGINGFEATURE_H
#define AGINGFEATURE_H

#include <QObject>
#include <QColor>
#include <QVector>
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
    void setColorSequence(const QVector<QColor>& sequence);
    void setLinePatterns(const QVector<QVector<bool>>& linePatternSequence);  // 每一帧的线条配置

private:
    DisplayWindow* displayWindow;
    bool enabled = false;
    int tickCounter = 0;
    int tickInterval = 20;  // tick周期：20 * 10ms = 200ms

    int frameIndex = 0;
    QVector<QColor> colorFrames;
    QVector<QVector<bool>> lineFrames;  // 每帧线条 [horizontal, vertical, d1, d2]

    const int baseTickMs = 10;
};

#endif // AGINGFEATURE_H
