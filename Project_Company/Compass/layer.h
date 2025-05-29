#ifndef LAYER_H
#define LAYER_H

#include "main.h"

#define HandleSize 10
class OutputView;

class Layer : public QWidget {
    Q_OBJECT
public:
    explicit Layer(int idx, const QString &name, QWidget *parent = 0) : QWidget(parent), idx(idx), name(name) {}
    inline void updateGeo() {
        setGeometry(QRect(sPos*gScale+gOrigin, sSize*gScale));
    }
    void select();

    QPoint sPos;
    QSize sSize{1920, 1080};
    int idx = 1, rowIdx = 0;
    QString name;
    TreeWidgetItem *item = 0;
    OutputView *view = 0;

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
    void resizeEvent(QResizeEvent *event) override {
        hdlT  = QRectF(width()/2 - HandleSize/2, -HandleSize/2, HandleSize, HandleSize);
        hdlRT = QRectF(width() - HandleSize/2, - HandleSize/2, HandleSize, HandleSize);
        hdlL  = QRectF(-HandleSize/2, height()/2 - HandleSize/2, HandleSize, HandleSize);
        hdlR  = QRectF(width() - HandleSize/2, height()/2 - HandleSize/2, HandleSize, HandleSize);
        hdlLB = QRectF(-HandleSize/2, height() - HandleSize/2, HandleSize, HandleSize);
        hdlB  = QRectF(width()/2 - HandleSize/2, height() - HandleSize/2, HandleSize, HandleSize);
        hdlRB = QRectF(width() - HandleSize/2, height() - HandleSize/2, HandleSize, HandleSize);
    }

    void setFrmSec(const QPointF &);
    void setFrmSecIfNeed(Qt::WindowFrameSection frmSec, Qt::CursorShape cursor);
    void clearSnap();

    QRectF hdlLT = QRectF(-HandleSize/2, -HandleSize/2, HandleSize, HandleSize), hdlT, hdlRT, hdlL, hdlR, hdlLB, hdlB, hdlRB;
    QPointF mPressRel{FLT_MAX, FLT_MAX};
    Qt::WindowFrameSection mFrmSec = Qt::NoSection;
    char snapLR = 0, snapTB = 0;
};


class OutputView : public QGraphicsView {
    Q_OBJECT
public:
    explicit OutputView(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void leaveEvent(QEvent *) override;
    void clearSnap();

    QPointF mPressRel{FLT_MAX, FLT_MAX};
    char snapLR = 0, snapTB = 0;
};

#endif // LAYER_H
