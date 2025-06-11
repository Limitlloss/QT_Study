#ifndef AUTOBRIGHTNESSFEATURE_H
#define AUTOBRIGHTNESSFEATURE_H

#include <QObject>
#include <QSlider>
#include <QSpinBox>
#include "ILedFeature.h"

class AutoBrightnessFeature : public QObject, public ILedFeature {
    Q_OBJECT

public:
    AutoBrightnessFeature(QSlider* slider, QSpinBox* spinbox, QObject* parent = nullptr);

    void tick() override;
    bool isEnabled() const override;
    void setEnabled(bool en) override;

    void setSpeedInterval(int ms);  // 设置速度，单位：毫秒

private:
    bool enabled = false;
    int tickCounter = 0;
    int tickInterval = 1;           // 多少个tick触发一次逻辑
    const int baseTickMs = 10;      // 主timer的间隔（与 mainTimer->setInterval 相同）

    QSlider* slider;
    QSpinBox* spinbox;
};

#endif // AUTOBRIGHTNESSFEATURE_H
