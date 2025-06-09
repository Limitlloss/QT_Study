#ifndef PLANPANEL_H
#define PLANPANEL_H

#include "gutil/qgui.h"
#include <QGraphicsItem>
#include <QMediaPlayer>

#define CellWidth 90
#define CellHeight 72
#define TitleHeight 20

class PlanPanel : public QWidget {
    Q_OBJECT
public:
    explicit PlanPanel(QWidget *parent = 0);

    void addRow();
    void play(QAbstractButton *);
};
class Cell {
public:
    Cell(char type, QGraphicsItem *wgt, QMediaPlayer *player) : wgt(wgt), player(player), type(type) {}
    ~Cell() {
        if(player) player->deleteLater();
        if(wgt) delete wgt;
    }
    QGraphicsItem *wgt;
    QMediaPlayer *player;
    char type;
};

class ImgItem : public QGraphicsObject {
public:
    using QGraphicsObject::QGraphicsObject;
    QRectF boundingRect() const override {
        return QRectF(0, 0, size.width(), size.height());
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    QPixmap img;
    QSize size;
};


class PlanTable : public TableWidget {
    Q_OBJECT
public:
    using TableWidget::TableWidget;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

class PlanTableH : public TableWidget {
    Q_OBJECT
public:
    using TableWidget::TableWidget;

protected:
    void dropEvent(QDropEvent *event) override;
};

class PlanTableV : public TableWidget {
    Q_OBJECT
public:
    using TableWidget::TableWidget;

protected:
    void dropEvent(QDropEvent *event) override;
};
#endif // PLANPANEL_H
