#ifndef EDCLOCK_H
#define EDCLOCK_H

#include "ebase.h"
#include "gutil/qgui.h"
#include <QPainter>
#include <QTextDocument>
#include <QTimeZone>
#include <QDate>
#include <QTime>
#include <QTimer>

class EDClock : public EBase {
    Q_OBJECT
public:
    explicit EDClock(EBase *multiWin = nullptr);
    explicit EDClock(const JObj &json, EBase *multiWin = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    int type() const override { return EBase::DClock; }
    QWidget* attrWgt() override;
    JObj attrJson() const override;

    QTimeZone timeZone;
    QFont font = qfont("Arial", 12);
    QColor color{Qt::red};
    int dateStyle = 0;
    int timeStyle = 0;
    int lineHeight = 0;
    bool hasYear = true;
    bool hasMonth = true;
    bool hasDay = true;
    bool hasHour = true;
    bool hasMin = true;
    bool hasSec = true;
    bool hasWeek = true;
    bool hasAmPm = true;
    bool isFullYear = true;
    bool is12Hour = true;
    bool isMultiline = true;
private:
    void init();

    QString img_dir;
    QDateTime datetime;
    bool isSingleMD = false;
};

#endif // EDCLOCK_H
