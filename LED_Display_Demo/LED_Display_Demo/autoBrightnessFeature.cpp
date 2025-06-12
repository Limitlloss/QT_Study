#include "AutoBrightnessFeature.h"

AutoBrightnessFeature::AutoBrightnessFeature(QSlider* slider, QSpinBox* spinbox, QObject* parent)
    : QObject(parent), slider(slider), spinbox(spinbox) {
}

void AutoBrightnessFeature::tick() {
    tickCounter++;
    if (tickCounter >= tickInterval) {
        tickCounter = 0;
        int value = slider->value();
        value = (value + 1) % 256;
        slider->setValue(value);         // 自动推动 slider
    }
}

bool AutoBrightnessFeature::isEnabled() const {
    return enabled;
}

void AutoBrightnessFeature::setEnabled(bool en) {
    enabled = en;
}

void AutoBrightnessFeature::setSpeedInterval(int ms) { 
    tickInterval = ms / baseTickMs;
    if (tickInterval < 1) tickInterval = 1;
}
