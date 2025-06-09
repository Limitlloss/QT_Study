#ifndef ETIMER_H
#define ETIMER_H

#include <QTimer>
#include <QColor>
#include <QFont>
#include <QDateTime>
#include "ebase.h"

class ETimer : public EBase {
    Q_OBJECT
public:
    explicit ETimer(EBase *multiWin = nullptr);
    explicit ETimer(const JObj &json, EBase *multiWin = nullptr);

    void init();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return EBase::Timer; }
    QWidget* attrWgt() override;
    JObj attrJson() const override;

    int secs = 0;

    QDateTime targetTime;
    QFont font;
    QString text;
    QColor textColor;
    QColor backColor;
    bool isDown;
    bool isMultiline;
    bool hasDay;
    bool hasHour;
    bool hasMin;
    bool hasSec;
};

#endif // ETIMER_H
