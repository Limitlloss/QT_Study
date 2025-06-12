#include "AgingFeature.h"

AgingFeature::AgingFeature(DisplayWindow* display, QObject* parent)
    : QObject(parent), displayWindow(display) {
    // 默认的颜色轮换
    colorFrames = {
        Qt::red, Qt::green, Qt::blue,
        Qt::yellow, Qt::cyan, Qt::magenta,
        Qt::white, Qt::black
    };

    // 默认线条样式
    lineFrames = {
        {true, false, false, false},   // Horizontal
        {false, true, false, false},   // Vertical
        {false, false, true, false},   // Diagonal /
        {false, false, false, true},   // Diagonal \
                {true, true, false, false},    // Cross
                {false, false, true, true},    // X
                {true, true, true, true},      // All
    };
}

void AgingFeature::setEnabled(bool en) {
    enabled = en;
    if (en) {
        displayWindow->setUseCurrentColor(true);
        displayWindow->setLineRendererEnabled(true);
    }
}

bool AgingFeature::isEnabled() const {
    return enabled;
}

void AgingFeature::tick() {
    if (!enabled) return;

    tickCounter++;
    if (tickCounter < tickInterval) return;
    QColor color = colorFrames[frameIndex % colorFrames.size()];
    displayWindow->setCurrentColor(color);  //  添加

    tickCounter = 0;



    // 线条模式
    QVector<bool> pattern = lineFrames[frameIndex % lineFrames.size()];
    displayWindow->setLinePattern(pattern[0], pattern[1], pattern[2], pattern[3]);

    displayWindow->update();

    frameIndex++;
}

void AgingFeature::setInterval(int ms) {
    tickInterval = ms / baseTickMs;
    if (tickInterval < 1) tickInterval = 1;
}

void AgingFeature::setColorSequence(const QVector<QColor>& sequence) {
    colorFrames = sequence;
}

void AgingFeature::setLinePatterns(const QVector<QVector<bool>>& sequence) {
    lineFrames = sequence;
}
