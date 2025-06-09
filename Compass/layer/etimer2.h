#ifndef ETIMER2_H
#define ETIMER2_H

#include "ebase.h"
#include <QDateTime>
#include <QTextDocument>

class ETimer2 : public EBase {
    Q_OBJECT
public:
    explicit ETimer2(EBase *multiWin = 0);
    explicit ETimer2(const JObj &json, EBase *multiWin = 0);
    void init();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return EBase::Timer2; }
    QWidget* attrWgt() override;
    JObj attrJson() const override;

    QDateTime targetTime;
    QString html;
    QColor backColor = Qt::transparent;
    bool isUp;

    QImage img;

    int secs = 0;
    QTextDocument doc;

public slots:
    void updImg();
};

#endif // ETIMER2_H
