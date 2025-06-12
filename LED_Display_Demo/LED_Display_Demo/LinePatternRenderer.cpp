#include "LinePatternRenderer.h"

void LinePatternRenderer::setLineOptions(bool h, bool v, bool d1, bool d2) {
    showHorizontal = h;
    showVertical = v;
    showDiagonal1 = d1;
    showDiagonal2 = d2;
}

void LinePatternRenderer::renderLines(QPainter& painter, const QSize& size) {
    painter.setPen(QPen(Qt::black, 2));

    if (showHorizontal) {
        painter.drawLine(0, size.height() / 2, size.width(), size.height() / 2);
    }

    if (showVertical) {
        painter.drawLine(size.width() / 2, 0, size.width() / 2, size.height());
    }

    if (showDiagonal1) {
        painter.drawLine(0, 0, size.width(), size.height());
    }

    if (showDiagonal2) {
        painter.drawLine(size.width(), 0, 0, size.height());
    }
}
