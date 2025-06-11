#include "LedEffectManager.h"

LedEffectManager::LedEffectManager(QObject* parent) : QObject(parent) {
    connect(&timer, &QTimer::timeout, this, &LedEffectManager::updateColor);
    timer.setInterval(100);
}

void LedEffectManager::start() {
    timer.start();
}

void LedEffectManager::stop() {
    timer.stop();
}

void LedEffectManager::updateColor() {
    QColor color = QColor::fromHsv(hue, 255, 255);
    emit colorChanged(color);
    hue = (hue + 5) % 360;
}
