#include "LinePatternRenderer.h"

void LinePatternRenderer::setLineOptions(bool h, bool v, bool d1, bool d2) {
    showHorizontal = h;
    showVertical = v;
    showDiagonal1 = d1;
    showDiagonal2 = d2;
}

void LinePatternRenderer::setLineSpacing(int spacing) {
    if (spacing < 5) spacing = 5;  // 防止太小
    lineSpacing = spacing;
}

void LinePatternRenderer::renderLines(QPainter& painter, const QSize& size, int offset) {
    painter.setPen(QPen(Qt::white, 1));

    if (showHorizontal) {
        for (int y = offset % lineSpacing; y < size.height(); y += lineSpacing) {
            painter.drawLine(0, y, size.width(), y);
        }
    }

    if (showVertical) {
        for (int x = offset % lineSpacing; x < size.width(); x += lineSpacing) {
            painter.drawLine(x, 0, x, size.height());
        }
    }

    if (showDiagonal1) {
        // 正斜线 ↘
        for (int i = -size.height(); i < size.width(); i += lineSpacing) {
            int x1 = i + (offset % lineSpacing);
            painter.drawLine(x1, 0, x1 + size.height(), size.height());
        }
    }

    if (showDiagonal2) {
        // 反斜线 ↗
        for (int i = 0; i < size.width() + size.height(); i += lineSpacing) {
            int x1 = i - (offset % lineSpacing);
            painter.drawLine(x1, 0, x1 - size.height(), size.height());
        }
    }
}
