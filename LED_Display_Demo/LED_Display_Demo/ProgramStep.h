#pragma once
#include <QString>
#include <QColor>
#include <QVector>

struct ProgramStep {
    enum Type {
        COLOR,
        LINE_PATTERN
    } type;

    QColor color;
    QVector<bool> linePattern;

    ProgramStep(QColor c) : type(COLOR), color(c) {}
    ProgramStep(bool h, bool v, bool d1, bool d2)
        : type(LINE_PATTERN), linePattern{ h, v, d1, d2 } {
    }
};
