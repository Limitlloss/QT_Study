#ifndef LINEPATTERNRENDERER_H
#define LINEPATTERNRENDERER_H

#include <QPainter>
#include <QSize>

class LinePatternRenderer {
public:
    void setLineOptions(bool h, bool v, bool d1, bool d2);
    void renderLines(QPainter& painter, const QSize& size, int offset = 0);

    void setLineSpacing(int spacing);  // 设置线条间距

private:
    bool showHorizontal = false;
    bool showVertical = false;
    bool showDiagonal1 = false; // 左上 → 右下
    bool showDiagonal2 = false; // 右上 → 左下

    int lineSpacing = 30;
};

#endif // LINEPATTERNRENDERER_H
