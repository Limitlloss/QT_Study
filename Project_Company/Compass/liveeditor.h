#ifndef LIVEEDITOR_H
#define LIVEEDITOR_H

#include <QGraphicsView>
#include <QLabel>

class Layer;
class LiveEditor : public QWidget {
    Q_OBJECT
public:
    explicit LiveEditor(QWidget* parent = 0);
    // 缩放改变时的处理函数
    void scaleChanged();
    // 图块满载时的处理函数
    void onTileFull();

    QLabel* edScale;
};

class OriginWgt : public QWidget {
public:
    using QWidget::QWidget;
    // 重写绘图事件，绘制十字线
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        painter.drawLine(0, 10, 20, 10);
        painter.drawLine(10, 0, 10, 20);
    }
};

class EditView : public QGraphicsView {
    Q_OBJECT
public:
    using QGraphicsView::QGraphicsView;
    // 选择图层
    void select(Layer* layer);

    Layer* selected = 0;
    std::vector<Layer*> layers;
    OriginWgt* originWgt;
protected:
    // 鼠标按下事件处理
    void mousePressEvent(QMouseEvent*) override;
    // 鼠标释放事件处理
    void mouseReleaseEvent(QMouseEvent*) override;
    // 鼠标移动事件处理
    void mouseMoveEvent(QMouseEvent*) override;

    QPoint pressRel{ INT_MAX, INT_MAX };
};

#endif // LIVEEDITOR_H
