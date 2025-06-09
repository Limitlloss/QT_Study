#ifndef PLANPANEL_H
#define PLANPANEL_H

#include "gutil/qgui.h"
#include <QGraphicsItem>
#include <QMediaPlayer>

#define CellWidth 90
#define CellHeight 72
#define TitleHeight 20

// PlanPanel 类继承自 QWidget，用于显示和管理计划面板
class PlanPanel : public QWidget {
    Q_OBJECT
public:
    explicit PlanPanel(QWidget* parent = 0);

    // 添加一行到面板
    void addRow();
    // 播放与按钮关联的媒体
    void play(QAbstractButton*);
};

// Cell 类用于表示面板中的单元格，包含图形项和媒体播放器
class Cell {
public:
    Cell(char type, QGraphicsItem* wgt, QMediaPlayer* player) : wgt(wgt), player(player), type(type) {}
    ~Cell() {
        if (player) player->deleteLater();
        if (wgt) delete wgt;
    }
    QGraphicsItem* wgt;
    QMediaPlayer* player;
    char type;
};

// ImgItem 类继承自 QGraphicsObject，用于显示图像项
class ImgItem : public QGraphicsObject {
public:
    using QGraphicsObject::QGraphicsObject;
    QRectF boundingRect() const override {
        return QRectF(0, 0, size.width(), size.height());
    }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) override;
    QPixmap img;
    QSize size;
};

// PlanTable 类继承自 TableWidget，重写了拖放事件以支持自定义的拖放行为
class PlanTable : public TableWidget {
    Q_OBJECT
public:
    using TableWidget::TableWidget;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};

// PlanTableH 类继承自 TableWidget，重写了 dropEvent 以支持水平拖放
class PlanTableH : public TableWidget {
    Q_OBJECT
public:
    using TableWidget::TableWidget;

protected:
    void dropEvent(QDropEvent* event) override;
};

// PlanTableV 类继承自 TableWidget，重写了 dropEvent 以支持垂直拖放
class PlanTableV : public TableWidget {
    Q_OBJECT
public:
    using TableWidget::TableWidget;

protected:
    void dropEvent(QDropEvent* event) override;
};
#endif // PLANPANEL_H
