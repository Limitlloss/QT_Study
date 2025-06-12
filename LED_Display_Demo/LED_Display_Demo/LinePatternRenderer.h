#ifndef LINEPATTERNRENDERER_H
#define LINEPATTERNRENDERER_H

#include <QPainter>
#include <QSize>

class LinePatternRenderer {
public:
    void setLineOptions(bool h, bool v, bool d1, bool d2);
    void renderLines(QPainter& painter, const QSize& size);

private:
    bool showHorizontal = false;
    bool showVertical = false;
    bool showDiagonal1 = false; // 左上 -> 右下
    bool showDiagonal2 = false; // 右上 -> 左下
};

#endif // LINEPATTERNRENDERER_H
