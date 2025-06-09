#ifndef LIVEEDITOR_H
#define LIVEEDITOR_H

#include <QGraphicsView>
#include <QLabel>

class Layer;
class LiveEditor : public QWidget {
    Q_OBJECT
public:
    explicit LiveEditor(QWidget *parent = 0);
    void scaleChanged();
    void onTileFull();

    QLabel *edScale;
};

class OriginWgt : public QWidget {
public:
    using QWidget::QWidget;
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.drawLine(0, 10, 20, 10);
        painter.drawLine(10, 0, 10, 20);
    }
};

class EditView : public QGraphicsView {
    Q_OBJECT
public:
    using QGraphicsView::QGraphicsView;
    void select(Layer *layer);

    Layer* selected = 0;
    std::vector<Layer*> layers;
    OriginWgt *originWgt;
protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

    QPoint pressRel{INT_MAX, INT_MAX};
};

#endif // LIVEEDITOR_H
