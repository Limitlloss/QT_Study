#include "AgingFeature.h"

AgingFeature::AgingFeature(DisplayWindow* display, QObject* parent)
    : QObject(parent), displayWindow(display) {
    buildDefaultSequence();
}

void AgingFeature::buildDefaultSequence() {
    // 纯色阶段
    QVector<QColor> colors = {
        Qt::red, Qt::green, Qt::blue,
        Qt::yellow, Qt::cyan, Qt::magenta,
        Qt::white, Qt::black
    };

    for (const QColor& c : colors) {
        AgingFrame f;
        f.type = COLOR;
        f.color = c;
        frames.push_back(f);
    }

    // 滚动线阶段（黑色背景 + 线条）
    QVector<QVector<bool>> patterns = {
        {true, false, false, false},   // 横线
        {false, true, false, false},   // 竖线
        {false, false, true, false},   // /
        {false, false, false, true},   // \
                {true, true, false, false},    // 十字
                {false, false, true, true},    // X
                {true, true, true, true}       // 所有
    };

    for (const auto& p : patterns) {
        AgingFrame f;
        f.type = LINE_SCROLL;
        f.color = Qt::black;
        f.linePattern = p;
        frames.push_back(f);
    }
}

void AgingFeature::setEnabled(bool en) {
    enabled = en;
    tickCounter = 0;
    frameIndex = 0;

    if (displayWindow) {
        displayWindow->setUseCurrentColor(false);
        displayWindow->setLineRendererEnabled(false);
    }
}

bool AgingFeature::isEnabled() const {
    return enabled;
}

void AgingFeature::tick() {
    if (!enabled) return;

    tickCounter++;
    if (tickCounter < tickInterval) return;
    tickCounter = 0;

    if (frames.isEmpty()) return;
    const AgingFrame& frame = frames[frameIndex % frames.size()];

    if (frame.type == COLOR) {
        displayWindow->setLineRendererEnabled(false);
        displayWindow->setUseCurrentColor(false);
        displayWindow->setBackgroundColor(frame.color);
    }
    else if (frame.type == LINE_SCROLL) {
        displayWindow->setUseCurrentColor(false);
        displayWindow->setBackgroundColor(frame.color);
        displayWindow->setLinePattern(
            frame.linePattern[0],
            frame.linePattern[1],
            frame.linePattern[2],
            frame.linePattern[3]
        );
        displayWindow->setLineRendererEnabled(true);
    }

    displayWindow->update();
    frameIndex++;
}

void AgingFeature::setInterval(int ms) {
    tickInterval = ms / baseTickMs;
    if (tickInterval < 1) tickInterval = 1;
}
