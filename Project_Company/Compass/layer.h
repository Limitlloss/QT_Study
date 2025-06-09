#ifndef LAYER_H
#define LAYER_H

#include "main.h"

#define HandleSize 10
class OutputView;

// Layer类继承自QWidget，用于表示一个图层
class Layer : public QWidget {
    Q_OBJECT
public:
    // 构造函数，初始化图层的索引和名称
    explicit Layer(int idx, const QString& name, QWidget* parent = 0) : QWidget(parent), idx(idx), name(name) {}
    // 更新图层的几何位置和大小
    inline void updateGeo() {
        setGeometry(QRect(sPos * gScale + gOrigin, sSize * gScale));
    }
    // 选择图层的方法
    void select();

    // 图层的位置
    QPoint sPos;
    // 图层的大小，默认为1920x1080
    QSize sSize{ 1920, 1080 };
    // 图层的索引
    int idx = 1, rowIdx = 0;
    // 图层的名称
    QString name;
    // 图层对应的树形控件项
    TreeWidgetItem* item = 0;
    // 图层对应的输出视图
    OutputView* view = 0;

protected:
    // 重写paintEvent方法，用于绘制图层
    void paintEvent(QPaintEvent*) override;
    // 重写mousePressEvent方法，处理鼠标按下事件
    void mousePressEvent(QMouseEvent*) override;
    // 重写mouseReleaseEvent方法，处理鼠标释放事件
    void mouseReleaseEvent(QMouseEvent*) override;
    // 重写mouseMoveEvent方法，处理鼠标移动事件
    void mouseMoveEvent(QMouseEvent*) override;
    // 重写leaveEvent方法，处理鼠标离开事件
    void leaveEvent(QEvent*) override;
    // 重写resizeEvent方法，更新图层调整大小时的句柄位置
    void resizeEvent(QResizeEvent* event) override {
        hdlT = QRectF(width() / 2 - HandleSize / 2, -HandleSize / 2, HandleSize, HandleSize);
        hdlRT = QRectF(width() - HandleSize / 2, -HandleSize / 2, HandleSize, HandleSize);
        hdlL = QRectF(-HandleSize / 2, height() / 2 - HandleSize / 2, HandleSize, HandleSize);
        hdlR = QRectF(width() - HandleSize / 2, height() / 2 - HandleSize / 2, HandleSize, HandleSize);
        hdlLB = QRectF(-HandleSize / 2, height() - HandleSize / 2, HandleSize, HandleSize);
        hdlB = QRectF(width() / 2 - HandleSize / 2, height() - HandleSize / 2, HandleSize, HandleSize);
        hdlRB = QRectF(width() - HandleSize / 2, height() - HandleSize / 2, HandleSize, HandleSize);
    }

    // 设置窗口框架部分的方法
    void setFrmSec(const QPointF&);
    // 根据需要设置窗口框架部分和光标形状的方法
    void setFrmSecIfNeed(Qt::WindowFrameSection frmSec, Qt::CursorShape cursor);
    // 清除快照的方法
    void clearSnap();

    // 各个方向的句柄矩形区域
    QRectF hdlLT = QRectF(-HandleSize / 2, -HandleSize / 2, HandleSize, HandleSize), hdlT, hdlRT, hdlL, hdlR, hdlLB, hdlB, hdlRB;
    // 鼠标按下时相对于图层的位置
    QPointF mPressRel{ FLT_MAX, FLT_MAX };
    // 当前鼠标按下的窗口框架部分
    Qt::WindowFrameSection mFrmSec = Qt::NoSection;
    // 快照标志，水平和垂直
    char snapLR = 0, snapTB = 0;
};


// OutputView类继承自QGraphicsView，用于显示输出内容
class OutputView : public QGraphicsView {
    Q_OBJECT
public:
    // 构造函数
    explicit OutputView(QWidget* parent = 0);

protected:
    // 处理鼠标按下事件
    void mousePressEvent(QMouseEvent*) override;
    // 处理鼠标释放事件
    void mouseReleaseEvent(QMouseEvent*) override;
    // 处理鼠标移动事件
    void mouseMoveEvent(QMouseEvent*) override;
    // 处理鼠标离开事件
    void leaveEvent(QEvent*) override;
    // 清除快照的方法
    void clearSnap();

    // 鼠标按下时相对于视图的位置
    QPointF mPressRel{ FLT_MAX, FLT_MAX };
    // 快照标志，水平和垂直
    char snapLR = 0, snapTB = 0;
};

#endif // LAYER_H
